#include "Matrix.hpp"
#include "CharConv.hpp"
#include "Memory.hpp"
namespace ARLib {
RowArray<false> Matrix2D::operator[](size_t idx) {
    return { *this, idx };
}
RowArray<true> Matrix2D::operator[](size_t idx) const {
    return { *this, idx };
}
CartesianIterator<true> Matrix2D::cartesian() const {
    return { *this, 0, 0 };
}
CartesianIterator<false> Matrix2D::cartesian() {
    return { *this, 0, 0 };
}
MatrixIterator<true, MatIterType::ByCol> Matrix2D::columns_begin(size_t begin_idx) const {
    return { *this, begin_idx };
}
MatrixIterator<true, MatIterType::ByCol> Matrix2D::columns_end() const {
    return { *this, m_columns };
}
MatrixIterator<false, MatIterType::ByCol> Matrix2D::columns_begin(size_t begin_idx) {
    return { *this, begin_idx };
}
MatrixIterator<false, MatIterType::ByCol> Matrix2D::columns_end() {
    return { *this, m_columns };
}
MatrixIterator<true, MatIterType::ByRow> Matrix2D::rows_begin(size_t begin_idx) const {
    return { *this, begin_idx };
}
MatrixIterator<true, MatIterType::ByRow> Matrix2D::rows_end() const {
    return { *this, m_rows };
}
MatrixIterator<false, MatIterType::ByRow> Matrix2D::rows_begin(size_t begin_idx) {
    return { *this, begin_idx };
}
MatrixIterator<false, MatIterType::ByRow> Matrix2D::rows_end() {
    return { *this, m_rows };
}
void Matrix2D::print_debug_matrix(double* const og_matrix, size_t N, size_t M) {
    IndexCalculator calc{ N, M };
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { printf("%lf ", og_matrix[calc.index(i, j)]); }
        puts("");
    }
}
void Matrix2D::gauss_reduce(double* matrix, size_t N, size_t M) {
    // This for some reason makes it return a negative determinant for some inputs
    // e.g. determinant should be 66.xxx but it returns -66.xxx
    size_t current_row = 0;
    size_t current_col = 0;
    IndexCalculator calc{ N, M };
    while (current_row < N - 1 && current_col < M - 1) {
        if (matrix[calc.index(current_row, current_col)] == 0) {
            // search row with non-zero first element
            for (size_t i = 0; i < N; i++) {
                if (matrix[calc.index(i, current_col)] != 0) {
                    // swap
                    swap_row(matrix, current_row, i, N, M);
                    break;
                }
            }
        }
        for (size_t i = current_row + 1; i < N; i++) {
            if (matrix[calc.index(i, current_col)] != 0) {
                double coeff = -(matrix[calc.index(i, current_col)] / matrix[calc.index(current_row, current_col)]);
                for (size_t j = current_col; j < M; j++) {
                    double sum          = matrix[calc.index(current_row, j)] * coeff + matrix[calc.index(i, j)];
                    matrix[calc.index(i, j)] = sum;
                }
            }
        }
        if (matrix[calc.index(current_row, current_col)] == 0.0) return;
        current_col++;
        current_row++;
    }
}
void Matrix2D::row_echelon_transform(double* matrix, size_t N, size_t M) {
    size_t lead     = 0;
    size_t rowCount = N;
    size_t colCount = M;
    IndexCalculator calc{ N, M };
    for (size_t r = 0; r < rowCount; r++) {
        if (colCount <= lead) return;
        size_t i = r;
        while (matrix[calc.index(i, lead)] == 0.0) {
            i++;
            if (rowCount == i) {
                i = r;
                lead++;
                if (colCount == lead) return;
            }
        }
        if (i != r) { swap_row(matrix, i, r, N, M); }
        //  Divide row r by M[r, lead]
        if (double divider = matrix[calc.index(r, lead)];
            divider != 0.0) {    // this check is here because otherwise I'd divide by zero
            for (size_t j = 0; j < colCount; j++) matrix[calc.index(r, j)] /= divider;
        }
        for (size_t j = 0; j < rowCount; j++) {
            if (j != r) {
                double multiplier = matrix[calc.index(j, lead)];
                // Subtract M[j, lead] multiplied by row r from row j
                for (size_t k = 0; k < colCount; k++) matrix[calc.index(j, k)] -= (multiplier * matrix[calc.index(r, k)]);
            }
        }
        lead++;
    }
}
int Matrix2D::rank_internal(double* const og_matrix, size_t N, size_t M) {
    IndexCalculator calc{ N, M };
    double* matrix{};
    matrix = allocate_uninitialized<double>(N * M);
    ConditionalBitCopy(matrix, og_matrix, N * M);
    gauss_reduce(matrix, N, M);
    int rank = 0;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            if (matrix[calc.index(i, j)] != 0) {
                rank++;
                break;
            }
        }
    }
    deallocate<double, DeallocType::Multiple>(matrix);
    return rank;
}
double Matrix2D::det_internal(double* const og_matrix, size_t N, size_t M) {
    HARD_ASSERT(N == M, "Matrix has to be square to calculate determinant");
    IndexCalculator calc{ N, M };
    double* matrix{};
    matrix = allocate_uninitialized<double>(N * M);
    ConditionalBitCopy(matrix, og_matrix, N * M);
    gauss_reduce(matrix, N, M);
    double det = 1.0;
    for (size_t i = 0; i < N; i++) {
        if (matrix[calc.index(i, i)] == 0.0) {
            det = 0.0;
            break;
        }
        det *= matrix[calc.index(i, i)];
    }
    deallocate<double, DeallocType::Multiple>(matrix);
    return det;
}
double Matrix2D::det() const {
    HARD_ASSERT(m_rows == m_columns, "Matrix must be square to calculate the determinant");

    // i'll have hardcoded math for 2x2 and 3x3
    if (m_rows == 2) {
        return m_matrix[index(0, 0)] * m_matrix[index(1, 1)] - m_matrix[index(0, 1)] * m_matrix[index(1, 0)];
    } else if (m_rows == 3) {
        // ignore first row first column
        double first_partial = m_matrix[index(1, 1)] * m_matrix[index(2, 2)] - m_matrix[index(1, 2)] * m_matrix[index(2, 1)];
        // ignore first row second column
        double second_partial = m_matrix[index(1, 0)] * m_matrix[index(2, 2)] - m_matrix[index(1, 2)] * m_matrix[index(2, 0)];
        // ignore first row third column
        double third_partial = m_matrix[index(1, 0)] * m_matrix[index(2, 1)] - m_matrix[index(1, 1)] * m_matrix[index(2, 0)];

        return (first_partial * m_matrix[index(0, 0)]) - (second_partial * m_matrix[index(0, 1)]) + (third_partial * m_matrix[index(0, 2)]);
    } else {
        // generic math for 4x4 and up
        return det_internal(m_matrix, m_rows, m_columns);
    }
}
String PrintInfo<Matrix2D>::repr() const {
    String ret{};
    for (size_t i = 0; i < m_matrix.num_rows(); i++) {
        ret += "[ "_s;
        for (size_t j = 0; j < m_matrix.num_columns(); j++) {
            ret += PrintInfo<double>{ m_matrix[{ i, j }] }.repr();
            if (j != m_matrix.num_columns() - 1) ret += ", "_s;
        }
        ret += " ]"_s;
        if (i != m_matrix.num_rows() - 1) ret += '\n';
    }
    return ret;
}
}    // namespace ARLib
