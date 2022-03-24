#pragma once
#include "Concepts.h"
#include "FixedMatrix.h"
#include "Memory.h"
#include "Pair.h"

namespace ARLib {
    template <typename T>
    struct PrintInfo;

    class Matrix2D;

    template <bool IsConst, bool ByRow>
    class MatrixIterator;

    // clang-format off
    template <bool IsConst>
    class ColumnIterator {
        using Tp = ConditionalT<IsConst, AddConstT<double>*, double*>;
        Tp* m_col;
        size_t m_current_index;

        public:
        ColumnIterator(Tp* col, size_t current_index) : m_col(col), m_current_index(current_index) {}
        auto& operator*() { return *m_col[m_current_index]; }
        const auto& operator*() const { return *m_col[m_current_index]; }

        ColumnIterator& operator++() {
            m_current_index++;
            return *this;
        }
        ColumnIterator operator++(int) {
            auto copy = *this;
            m_current_index++;
            return copy;
        }

        ColumnIterator& operator--() {
            m_current_index--;
            return *this;
        }
        ColumnIterator operator--(int) {
            auto copy = *this;
            m_current_index--;
            return copy;
        }

        bool operator==(const ColumnIterator& other) const {
            return m_col == other.m_col && m_current_index == other.m_current_index;
        }
        bool operator!=(const ColumnIterator& other) const {
            return m_col != other.m_col || m_current_index != other.m_current_index;
        }
    };

    template <bool IsConst = false>
    class ColumnArray {
        friend Matrix2D;
        friend MatrixIterator<IsConst, false>;
        using Tp = ConditionalT<IsConst, AddConstT<double>*, double*>;

        Tp* m_column = nullptr;
        size_t m_size{0}; 

        ColumnArray() = default;

        ColumnArray(double** matrix, size_t idx, size_t size) requires(!IsConst) : m_size(size) {
            if (idx >= size)
                m_column = nullptr;
            else {
                m_column = allocate<Tp>(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    m_column[i] = &matrix[i][idx];
                };
            }
        }
        ColumnArray(double** const matrix, size_t idx, size_t size) requires IsConst : m_size(size) {
            if (idx >= size)
                m_column = nullptr;
            else {
                m_column = allocate<Tp>(m_size);
                for (size_t i = 0; i < m_size; i++) {
                    m_column[i] = &matrix[i][idx];
                };
            }
        }

        public :

        ColumnArray& operator=(ColumnArray&& other) {
            deallocate<Tp, DeallocType::Multiple>(m_column);
            m_column = other.m_column;
            m_size = other.m_size;
            other.m_column = nullptr;
            return *this;
        }

        double& operator[](size_t index) requires(!IsConst) { return *m_column[index]; }
        const double& operator[](size_t index) const { return *m_column[index]; }

        size_t size() const { return m_size; }

        auto begin() const { return ColumnIterator<true>{m_column, 0}; }
        auto end() const { return ColumnIterator<true>{m_column, m_size}; }

        auto begin() { return ColumnIterator<false>{m_column, 0}; }
        auto end() { return ColumnIterator<false>{m_column, m_size}; }

        ~ColumnArray() { deallocate<Tp, DeallocType::Multiple>(m_column); }
    };

    template <bool IsConst>
    class RowIterator {
        using Tp = ConditionalT<IsConst, AddConstT<double>*, double*>;

        Tp m_row;
        size_t m_current_index;

        public:
        RowIterator(Tp row, size_t current_index) : m_row(row), m_current_index(current_index) {}
        auto& operator*() { return m_row[m_current_index]; }
        const auto& operator*() const { return m_row[m_current_index]; }

        RowIterator& operator++() {
            m_current_index++;
            return *this;
        }
        RowIterator operator++(int) {
            auto copy = *this;
            m_current_index++;
            return copy;
        }

        RowIterator& operator--() {
            m_current_index--;
            return *this;
        }
        RowIterator operator--(int) {
            auto copy = *this;
            m_current_index--;
            return copy;
        }

        bool operator==(const RowIterator& other) const {
            return m_row == other.m_row && m_current_index == other.m_current_index;
        }
        bool operator!=(const RowIterator& other) const {
            return m_row != other.m_row || m_current_index != other.m_current_index;
        }
    };

    template <bool IsConst = false>
    class RowArray {
        friend Matrix2D;
        friend MatrixIterator<IsConst, true>;

        using Tp = ConditionalT<IsConst, AddConstT<double>*, double*>;

        Tp m_row;
        size_t m_size;

        RowArray(double** matrix, size_t idx, size_t size) requires(!IsConst) : m_size(size) {
            if (idx >= size)
               m_row = nullptr;
            else
               m_row = matrix[idx]; 
        }
        RowArray(double** const matrix, size_t idx, size_t size) requires IsConst : m_size(size) { 
            if (idx >= size)
               m_row = nullptr;
            else
               m_row = matrix[idx];  
        }

        public: 
        double& operator[](size_t index) requires(!IsConst) { return m_row[index]; }
        const double& operator[](size_t index) const { return m_row[index]; }

        size_t size() const { return m_size; }

        auto begin() const { return RowIterator<true>{m_row, 0}; }
        auto end() const { return RowIterator<true>{m_row, m_size}; }

        auto begin() { return RowIterator<false>{m_row, 0}; }
        auto end() { return RowIterator<false>{m_row, m_size}; }
    };

    template <bool IsConst, bool ByRow = true>
    class MatrixIterator {
        using Tp = ConditionalT<IsConst, AddConstT<double**>, double**>;
        using Ret = ConditionalT<ByRow, RowArray<IsConst>, ColumnArray<IsConst>>;

        Tp m_matrix;
        size_t m_current_row;
        Ret m_current;
        size_t m_size;

        public:
        MatrixIterator(Tp matrix, size_t current_row, size_t size) :
            m_matrix(matrix), m_current_row(current_row), m_current(matrix, current_row, size), m_size(size) {}

        Ret& operator*() { return m_current; }
        const Ret& operator*() const { return m_current; }

        MatrixIterator& operator++() {
            m_current_row++;
            m_current = Ret{m_matrix, m_current_row, m_size};
            return *this;
        }
        MatrixIterator operator++(int) {
            auto copy = *this;
            m_current_row++;
            m_current = Ret{m_matrix, m_current_row, m_size};
            return copy;
        }

        MatrixIterator& operator--() {
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row, m_size};
            return *this;
        }
        MatrixIterator operator--(int) {
            auto copy = *this;
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row, m_size};
            return copy;
        }

        bool operator==(const MatrixIterator& other) const {
            return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row && m_size == other.m_size;
        }
        bool operator!=(const MatrixIterator& other) const {
            return m_matrix != other.m_matrix || m_current_row != other.m_current_row || m_size != other.m_size;
        }
    };

    // clang-format on

    class Matrix2D {
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

        template <size_t N, size_t M>
        explicit Matrix2D(const FixedMatrix2D<N, M>& matrix) : m_rows(N), m_columns(M) {
            allocate_memory();
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    m_matrix[i][j] = matrix[{i, j}];
                }
            }
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

        /* ITERATORS */
        template <bool ByRow = true>
        auto begin() const {
            return MatrixIterator<true, ByRow>{m_matrix, 0, ByRow ? m_rows : m_columns};
        }

        template <bool ByRow = true>
        auto end() const {
            return MatrixIterator<true, ByRow>{m_matrix, ByRow ? m_rows : m_columns, ByRow ? m_rows : m_columns};
        }

        template <bool ByRow = true>
        auto begin() {
            return MatrixIterator<false, ByRow>{m_matrix, 0, ByRow ? m_rows : m_columns};
        }

        template <bool ByRow = true>
        auto end() {
            return MatrixIterator<false, ByRow>{m_matrix, ByRow ? m_rows : m_columns, ByRow ? m_rows : m_columns};
        }

        auto columns_begin(size_t begin_idx = 0) const {
            return MatrixIterator<true, false>{m_matrix, begin_idx, m_columns};
        }
        auto columns_end() const { return MatrixIterator<true, false>{m_matrix, m_columns, m_columns}; }

        auto columns_begin(size_t begin_idx = 0) {
            return MatrixIterator<false, false>{m_matrix, begin_idx, m_columns};
        }
        auto columns_end() { return MatrixIterator<false, false>{m_matrix, m_columns, m_columns}; }

        auto rows_begin(size_t begin_idx = 0) const { return MatrixIterator<true, true>{m_matrix, begin_idx, m_rows}; }
        auto rows_end() const { return MatrixIterator<true, true>{m_matrix, m_rows, m_rows}; }

        auto rows_begin(size_t begin_idx = 0) { return MatrixIterator<false, true>{m_matrix, begin_idx, m_rows}; }
        auto rows_end() { return MatrixIterator<false, true>{m_matrix, m_rows, m_rows}; }

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

        DEFINE_OP(+, +=);
        DEFINE_OP(-, -=);

        Matrix2D operator*(const Matrix2D& other) const {
            HARD_ASSERT(m_rows == other.row_number(), "Impossible to do the multiplication, sizes do not match");
            Matrix2D mat{m_rows, other.column_number()};
            for (size_t col = 0; col < other.column_number(); col++) {
                for (size_t row = 0; row < m_rows; row++) {
                    double intermediate{0};
                    for (size_t i = 0; i < m_columns; i++) {
                        intermediate += m_matrix[row][i] * other[{i, col}];
                    }
                    mat[{row, col}] = intermediate;
                }
            }
            return mat;
        }

        Matrix2D& operator*=(const Matrix2D& other) {
            *this = *this * other;
            return *this;
        }

        Matrix2D operator/(const Matrix2D& other) const {
            HARD_ASSERT(shape() == other.shape(), "Matrices must have the same shape");
            HARD_ASSERT(other.det() != 0, "Determinant of second matrix must be non-zero");
            return *this * other.inv();
        }

        Matrix2D& operator/=(const Matrix2D& other) {
            *this = *this / other;
            return *this;
        }

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

        Pair<size_t, size_t> shape() const { return {m_rows, m_columns}; }
        size_t size() const { return m_rows; }
        size_t num_rows() const { return m_rows; }
        size_t num_columns() const { return m_columns; }
        void reduce() { row_echelon_transform(m_matrix, m_rows, m_columns); }
        int rank() const {
            return rank_internal(m_matrix, m_rows, m_columns);
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

        bool operator==(const Matrix2D& other) const {
            if (shape() != other.shape()) return false;
            for (size_t i = 0; i < m_rows; i++) {
                for (size_t j = 0; j < m_columns; j++) {
                    if (m_matrix[i][j] != other.m_matrix[i][j]) return false;
                }
            }
            return true;
        }

        bool operator!=(const Matrix2D& other) const {
            if (shape() != other.shape()) return true;
            for (size_t i = 0; i < m_rows; i++) {
                for (size_t j = 0; j < m_columns; j++) {
                    if (m_matrix[i][j] != other.m_matrix[i][j]) return true;
                }
            }
            return false;
        }

        ~Matrix2D() {
            if (m_matrix) deallocate_memory();
        }
    };

    template <size_t N, size_t M>
    Matrix2D operator*(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        HARD_ASSERT(M == second.row_number(), "Impossible to do the multiplication, sizes do not match");
        Matrix2D mat{N, second.column_number()};
        for (size_t col = 0; col < second.column_number(); col++) {
            for (size_t row = 0; row < N; row++) {
                double intermediate{0};
                for (size_t i = 0; i < M; i++) {
                    intermediate += first[{row, i}] * second[{i, col}];
                }
                mat[{row, col}] = intermediate;
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    Matrix2D operator*(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        HARD_ASSERT(N == first.column_number(), "Impossible to do the multiplication, sizes do not match");
        Matrix2D mat{first.row_number(), M};
        for (size_t col = 0; col < M; col++) {
            for (size_t row = 0; row < first.row_number(); row++) {
                double intermediate{0};
                for (size_t i = 0; i < N; i++) {
                    intermediate += first[{row, i}] * second[{i, col}];
                }
                mat[{row, col}] = intermediate;
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    Matrix2D operator/(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        HARD_ASSERT(second.det() != 0, "Determinant of second matrix must be non-zero");
        return first * second.inv();
    }

    template <size_t N, size_t M>
    Matrix2D operator/(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        HARD_ASSERT(second.det() != 0, "Determinant of second matrix must be non-zero");
        return first * second.inv();
    }

    template <size_t N, size_t M>
    Matrix2D operator+(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        Matrix2D mat{first};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] += second[{i, j}];
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    Matrix2D operator+(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        Matrix2D mat{first};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] += second[{i, j}];
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    FixedMatrix2D<N, M>& operator+=(FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                first[{i, j}] += second[{i, j}];
            }
        }
        return first;
    }

    template <size_t N, size_t M>
    Matrix2D& operator+=(Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                first[{i, j}] += second[{i, j}];
            }
        }
        return first;
    }

    template <size_t N, size_t M>
    Matrix2D operator-(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        Matrix2D mat{first};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] -= second[{i, j}];
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    Matrix2D operator-(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
        Matrix2D mat{first};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] -= second[{i, j}];
            }
        }
        return mat;
    }

    template <size_t N, size_t M>
    bool operator==(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        if (first.shape() != second.shape()) return false;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                if (first[{i, j}] != second[{i, j}]) return false;
            }
        }
        return true;
    }

    template <size_t N, size_t M>
    bool operator==(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        if (first.shape() != second.shape()) return false;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                if (first[{i, j}] != second[{i, j}]) return false;
            }
        }
        return true;
    }

    template <size_t N, size_t M>
    bool operator!=(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
        if (first.shape() != second.shape()) return true;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                if (first[{i, j}] != second[{i, j}]) return true;
            }
        }
        return false;
    }

    template <size_t N, size_t M>
    bool operator!=(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
        if (first.shape() != second.shape()) return true;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                if (first[{i, j}] != second[{i, j}]) return true;
            }
        }
        return false;
    }

    template <>
    struct PrintInfo<Matrix2D> {
        const Matrix2D& m_matrix;
        PrintInfo(const Matrix2D& matrix) : m_matrix(matrix) {}
        String repr() const;
    };
} // namespace ARLib