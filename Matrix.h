#pragma once
#include "Concepts.h"
#include "Optional.h"
#include "Pair.h"
#include "PrintInfo.h"

namespace ARLib {
    template <Numeric T>
    class Matrix2D {
        mutable Optional<double> m_cached_det{};
        mutable Optional<int> m_cached_rank{};
        T** m_matrix{};
        const size_t m_rows;
        const size_t m_columns;

        static void print_debug_matrix(const T** og_matrix, size_t N, size_t M) {
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    printf("%lf ", og_matrix[i][j]);
                }
                puts("");
            }
        }

        static void swap_row(T** matrix, size_t row1, size_t row2) {
            T* temp = matrix[row1];
            matrix[row1] = matrix[row2];
            matrix[row2] = temp;
        }

        static void gauss_reduce(T** matrix, size_t N, size_t M) {
            // This for some reason makes it return a negative determinant for some inputs
            // e.g. determinant should be 66.xxx but it returns -66.xxx
            size_t current_row = 0;
            size_t current_col = 0;
            while (current_row < N - 1 && current_col < M - 1) {
                if (matrix[current_row][current_col] == 0) {
                    // search row with non-zero first element
                    for (size_t i = 0; i < N; i++) {
                        if (matrix[i][current_col] != 0) {
                            // swap
                            swap_row(matrix, current_row, i);
                            break;
                        }
                    }
                }
                for (size_t i = current_row + 1; i < N; i++) {
                    if (matrix[i][current_col] != 0) {
                        T coeff = -(matrix[i][current_col] / matrix[current_row][current_col]);
                        for (size_t j = current_col; j < M; j++) {
                            T sum = matrix[current_row][j] * coeff + matrix[i][j];
                            matrix[i][j] = sum;
                        }
                    }
                }
                if (matrix[current_row][current_col] == T{0}) return;
                current_col++;
                current_row++;
            }
        }

        static bool check_row_all_zeros(T* row, size_t M) {
            constexpr T zero_val = T{0};
            for (size_t i = 0; i < M; i++) {
                if (row[i] != zero_val) return false;
            }
            return true;
        }

        static void row_echelon_transform(T** matrix, size_t N, size_t M) {
            size_t lead = 0;
            constexpr size_t rowCount = N;
            constexpr size_t colCount = M;

            for (size_t r = 0; r < rowCount; r++) {
                if (colCount <= lead) return;
                size_t i = r;
                while (matrix[i][lead] == T{0}) {
                    i++;
                    if (rowCount == i) {
                        i = r;
                        lead++;
                        if (colCount == lead) return;
                    }
                }
                if (i != r) { swap_row(matrix, i, r); }
                //  Divide row r by M[r, lead]
                if (T divider = matrix[r][lead];
                    divider != T{0}) { // this check is here because otherwise I'd divide by zero
                    for (size_t j = 0; j < colCount; j++)
                        matrix[r][j] /= divider;
                }
                for (size_t j = 0; j < rowCount; j++) {
                    if (j != r) {
                        T multiplier = matrix[j][lead];
                        // Subtract M[j, lead] multiplied by row r from row j
                        for (size_t k = 0; k < colCount; k++)
                            matrix[j][k] -= (multiplier * matrix[r][k]);
                    }
                }
                lead++;
            }
        }

        static int rank_internal(const T** og_matrix, size_t N, size_t M) {
            T** matrix{};
            matrix = allocate<T*>(N);
            for (size_t i = 0; i < N; i++) {
                matrix[i] = allocate<T>(N);
                ConditionalBitCopy(matrix[i], og_matrix[i], N);
            }
            gauss_reduce(matrix, N, M);
            int rank = 0;
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    if (matrix[i][j] != 0) {
                        rank++;
                        break;
                    }
                }
            }
            for (size_t i = 0; i < N; i++) {
                deallocate<T, DeallocType::Multiple>(matrix[i]);
            }
            deallocate<T*, DeallocType::Multiple>(matrix);
            return rank;
        }

        static double det_internal(const T** og_matrix, size_t N, size_t M) {
            HARD_ASSERT(N == M, "Matrix has to be square to calculate determinant");
            T** matrix{};
            matrix = allocate<T*>(N);
            for (size_t i = 0; i < N; i++) {
                matrix[i] = allocate<T>(N);
                ConditionalBitCopy(matrix[i], og_matrix[i], N);
            }
            gauss_reduce(matrix, N, M);
            double det = 1.0;
            for (size_t i = 0; i < N; i++) {
                if (matrix[i][i] == T{0}) {
                    det = 0.0;
                    break;
                }
                det *= matrix[i][i];
            }
            for (size_t i = 0; i < N; i++) {
                deallocate<T, DeallocType::Multiple>(matrix[i]);
            }
            deallocate<T*, DeallocType::Multiple>(matrix);
            return det;
        }

        void allocate_memory(bool zeroinit = false) {
            m_matrix = allocate<T*>(m_rows);
            for (size_t i = 0; i < m_rows; i++) {
                m_matrix[i] = allocate<T>(m_columns);
                if (zeroinit) memset(m_matrix[i], 0, m_columns * sizeof(T));
            }
        }
        void deallocate_memory() {
            for (size_t i = 0; i < m_rows; i++) {
                deallocate<T, DeallocType::Multiple>(m_matrix[i]);
            }
            deallocate<T*, DeallocType::Multiple>(m_matrix);
        }

        public:
        static Matrix2D eye(size_t size) {
            Matrix2D mat{size, size};
            for (size_t i = 0; i < size; i++) {
                mat.m_matrix[i][i] = T{1};
            }
            return mat;
        }

        Matrix2D(size_t rows, size_t columns) : m_rows(rows), m_columns(columns) { allocate_memory(true); }

        template <size_t N, size_t M>
        Matrix2D(const T (&mat)[N][M]) : m_rows(N), m_columns(M) {
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
            HARD_ASSERT(m_rows == other.m_rows && m_columns == other.m_columns,
                        "Can't copy matrices of different dimentions");
            for (size_t i = 0; i < m_rows; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], m_columns);
            }
        }

        Matrix2D& operator=(Matrix2D&& other) noexcept {
            HARD_ASSERT(m_rows == other.m_rows && m_columns == other.m_columns,
                        "Can't move matrices of different dimentions");
            deallocate_memory();
            m_matrix = other.m_matrix;
            other.m_matrix = nullptr;
        }

        size_t row_number() const { return m_rows; }
        size_t column_number() const { return m_columns; }

        T& operator[](Pair<size_t, size_t> idx) { return m_matrix[idx.first()][idx.second()]; }
        const T& operator[](Pair<size_t, size_t> idx) const { return m_matrix[idx.first()][idx.second()]; }

        ~Matrix2D() { deallocate_memory(); }
    };

    template <Numeric T>
    struct PrintInfo<Matrix2D<T>> {
        const Matrix2D<T>& m_matrix;
        PrintInfo(const Matrix2D<T>& matrix) : m_matrix(matrix) {}
        String repr() const {
            String ret{};
            for (size_t i = 0; i < m_matrix.row_number(); i++) {
                ret += "[ "_s;
                for (size_t j = 0; j < m_matrix.column_number(); j++) {
                    ret += PrintInfo<T>{m_matrix[{i, j}]}.repr();
                    if (j != m_matrix.column_number() - 1) ret += ", "_s;
                }
                ret += " ]"_s;
                if (i != m_matrix.row_number() - 1) ret += '\n';
            }
            return ret;
        }
    };
} // namespace ARLib