#pragma once
#include "Concepts.h"
#include "Optional.h"
#include "Pair.h"
#include "Memory.h"

namespace ARLib {
    template <typename T>
    struct PrintInfo;

    class Matrix2D {
        mutable Optional<double> m_cached_det{};
        mutable Optional<int> m_cached_rank{};
        double** m_matrix{};
        size_t m_rows;
        size_t m_columns;

        static void print_debug_matrix(double** const og_matrix, size_t N, size_t M);

        static void swap_row(double** matrix, size_t row1, size_t row2) {
            double* temp = matrix[row1];
            matrix[row1] = matrix[row2];
            matrix[row2] = temp;
        }

        static void gauss_reduce(double** matrix, size_t N, size_t M);
        static bool check_row_all_zeros(double* row, size_t M) {
            constexpr double zero_val = 0.0;
            for (size_t i = 0; i < M; i++) {
                if (row[i] != zero_val) return false;
            }
            return true;
        }

        static void row_echelon_transform(double** matrix, size_t N, size_t M);
        static int rank_internal(double** const og_matrix, size_t N, size_t M);
        static double det_internal(double** const og_matrix, size_t N, size_t M);

        void allocate_memory(bool zeroinit = false) {
            m_matrix = allocate<double*>(m_rows);
            for (size_t i = 0; i < m_rows; i++) {
                m_matrix[i] = allocate<double>(m_columns);
                if (zeroinit) memset(m_matrix[i], 0, m_columns * sizeof(double));
            }
        }
        void deallocate_memory() {
            for (size_t i = 0; i < m_rows; i++) {
                deallocate<double, DeallocType::Multiple>(m_matrix[i]);
            }
            deallocate<double*, DeallocType::Multiple>(m_matrix);
        }

        public:
        static Matrix2D eye(size_t size) {
            Matrix2D mat{size, size};
            for (size_t i = 0; i < size; i++) {
                mat.m_matrix[i][i] = 1.0;
            }
            return mat;
        }

        Matrix2D(size_t rows, size_t columns) : m_rows(rows), m_columns(columns) { allocate_memory(true); }

        template <size_t N, size_t M>
        Matrix2D(const double (&mat)[N][M]) : m_rows(N), m_columns(M) {
            allocate_memory();
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], mat[i], M);
            }
        }

        Matrix2D(const Matrix2D& other) : m_rows(other.m_rows), m_columns(other.m_columns) {
            allocate_memory();
            for (size_t i = 0; i < m_rows; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], m_columns);
            }
        }
        Matrix2D(Matrix2D&& other) noexcept : m_rows(other.m_rows), m_columns(other.m_columns) {
            m_matrix = other.m_matrix;
            other.m_matrix = nullptr;
        }

        Matrix2D& operator=(const Matrix2D& other) {
            deallocate_memory();
            m_rows = other.m_rows;
            m_columns = other.m_columns;
            allocate_memory();
            for (size_t i = 0; i < m_rows; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], m_columns);
            }
            return *this;
        }

        Matrix2D& operator=(Matrix2D&& other) noexcept {
            deallocate_memory();
            m_rows = other.m_rows;
            m_columns = other.m_columns;
            m_matrix = other.m_matrix;
            other.m_matrix = nullptr;
            return *this;
        }

        size_t row_number() const { return m_rows; }
        size_t column_number() const { return m_columns; }

        double& operator[](Pair<size_t, size_t> idx) { return m_matrix[idx.first()][idx.second()]; }
        const double& operator[](Pair<size_t, size_t> idx) const { return m_matrix[idx.first()][idx.second()]; }

#define MAT_LOOP(op)                                                                                                   \
    for (size_t i = 0; i < m_rows; i++) {                                                                              \
        for (size_t j = 0; j < m_columns; j++) {                                                                       \
            op;                                                                                                        \
        }                                                                                                              \
    }

#define DEFINE_INPLACE_OP(op)                                                                                          \
    Matrix2D& operator op(double val) {                                                                                \
        MAT_LOOP(m_matrix[i][j] op val);                                                                               \
        return *this;                                                                                                  \
    }

#define DEFINE_OP(op, in_op)                                                                                           \
    Matrix2D operator op(double val) const {                                                                           \
        auto copy = *this;                                                                                             \
        MAT_LOOP(copy.m_matrix[i][j] in_op val);                                                                       \
        return copy;                                                                                                   \
    }

        /* MATHEMATICAL OPERATORS */
        DEFINE_INPLACE_OP(+=);
        DEFINE_INPLACE_OP(-=);
        DEFINE_INPLACE_OP(*=);
        DEFINE_INPLACE_OP(/=);

        DEFINE_OP(+, +=);
        DEFINE_OP(-, -=);
        DEFINE_OP(*, *=);
        DEFINE_OP(/, /=);

        Matrix2D& operator++() {
            MAT_LOOP(m_matrix[i][j] += 1);
            return *this;
        }
        Matrix2D operator++(int) {
            auto copy = *this;
            MAT_LOOP(m_matrix[i][j] += 1);
            return copy;
        }
        Matrix2D& operator--() {
            MAT_LOOP(m_matrix[i][j] -= 1);
            return *this;
        }
        Matrix2D operator--(int) {
            auto copy = *this;
            MAT_LOOP(m_matrix[i][j] -= 1);
            return copy;
        }

        Pair<size_t, size_t> size() const { return {m_rows, m_columns}; }
        size_t num_rows() const { return m_rows; }
        size_t num_columns() const { return m_columns; }
        void reduce() { row_echelon_transform(m_matrix, m_rows, m_columns); }
        int rank() const {
            if (m_cached_rank) return m_cached_rank.value();
            m_cached_rank = rank_internal(m_matrix, m_rows, m_columns);
            return m_cached_rank.value();
        }

        double det() const;

        double sum() const {
            double res = 0.0;
            MAT_LOOP(res += m_matrix[i][j]);
            return res;
        }

        double avg() const { return sum() / static_cast<double>(m_rows * m_columns); }

        auto sub(size_t n_rows, size_t n_columns, size_t start_row = 0, size_t start_col = 0) const {
            HARD_ASSERT(start_row + n_rows <= m_rows, "Submatrix is impossible to construct");
            HARD_ASSERT(start_col + n_columns <= m_columns, "Submatrix is impossible to construct");
            Matrix2D submat{n_rows, n_columns};
            for (size_t i = start_row; i < start_row + n_rows; i++) {
                for (size_t j = start_col; j < start_col + n_columns; j++) {
                    submat[{i - start_row, j - start_col}] = m_matrix[i][j];
                }
            }
            return submat;
        }

        Matrix2D inv() const {
            HARD_ASSERT(m_rows == m_columns, "Matrix must be square to calculate the inverse");
            Matrix2D mat_glued{m_rows, m_columns * 2};
            for (size_t i = 0; i < m_rows; i++) {
                for (size_t j = 0; j < m_columns; j++) {
                    mat_glued[{i, j}] = m_matrix[i][j];
                }
            }
            for (size_t j = m_columns; j < m_columns * 2; j++) {
                mat_glued[{j - m_columns, j}] = 1.0;
            }
            mat_glued.reduce();
            return mat_glued.sub(m_rows, m_columns, 0, m_columns);
        }

        Matrix2D transpose() const {
            Matrix2D mat{m_rows, m_columns};
            for (size_t i = 0; i < m_rows; i++) {
                for (size_t j = 0; j < m_columns; j++) {
                    mat[{j, i}] = m_matrix[i][j];
                }
            }
            return mat;
        }

        ~Matrix2D() {
            if (m_matrix) deallocate_memory();
        }
    };

    template <>
    struct PrintInfo<Matrix2D> {
        const Matrix2D& m_matrix;
        PrintInfo(const Matrix2D& matrix) : m_matrix(matrix) {}
        String repr() const;
    };
} // namespace ARLib