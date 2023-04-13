#include "Matrix.hpp"
#include "CharConv.hpp"
#include "Memory.hpp"
namespace ARLib {
void Matrix2D::print_debug_matrix(double** const og_matrix, size_t N, size_t M) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { printf("%lf ", og_matrix[i][j]); }
        puts("");
    }
}
void Matrix2D::gauss_reduce(double** matrix, size_t N, size_t M) {
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
                double coeff = -(matrix[i][current_col] / matrix[current_row][current_col]);
                for (size_t j = current_col; j < M; j++) {
                    double sum   = matrix[current_row][j] * coeff + matrix[i][j];
                    matrix[i][j] = sum;
                }
            }
        }
        if (matrix[current_row][current_col] == 0.0) return;
        current_col++;
        current_row++;
    }
}
void Matrix2D::row_echelon_transform(double** matrix, size_t N, size_t M) {
    size_t lead     = 0;
    size_t rowCount = N;
    size_t colCount = M;

    for (size_t r = 0; r < rowCount; r++) {
        if (colCount <= lead) return;
        size_t i = r;
        while (matrix[i][lead] == 0.0) {
            i++;
            if (rowCount == i) {
                i = r;
                lead++;
                if (colCount == lead) return;
            }
        }
        if (i != r) { swap_row(matrix, i, r); }
        //  Divide row r by M[r, lead]
        if (double divider = matrix[r][lead];
            divider != 0.0) {    // this check is here because otherwise I'd divide by zero
            for (size_t j = 0; j < colCount; j++) matrix[r][j] /= divider;
        }
        for (size_t j = 0; j < rowCount; j++) {
            if (j != r) {
                double multiplier = matrix[j][lead];
                // Subtract M[j, lead] multiplied by row r from row j
                for (size_t k = 0; k < colCount; k++) matrix[j][k] -= (multiplier * matrix[r][k]);
            }
        }
        lead++;
    }
}
int Matrix2D::rank_internal(double** const og_matrix, size_t N, size_t M) {
    double** matrix{};
    matrix = allocate<double*>(N);
    for (size_t i = 0; i < N; i++) {
        matrix[i] = allocate<double>(N);
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
    for (size_t i = 0; i < N; i++) { deallocate<double, DeallocType::Multiple>(matrix[i]); }
    deallocate<double*, DeallocType::Multiple>(matrix);
    return rank;
}
double Matrix2D::det_internal(double** const og_matrix, size_t N, size_t M) {
    HARD_ASSERT(N == M, "Matrix has to be square to calculate determinant");
    double** matrix{};
    matrix = allocate<double*>(N);
    for (size_t i = 0; i < N; i++) {
        matrix[i] = allocate<double>(N);
        ConditionalBitCopy(matrix[i], og_matrix[i], N);
    }
    gauss_reduce(matrix, N, M);
    double det = 1.0;
    for (size_t i = 0; i < N; i++) {
        if (matrix[i][i] == 0.0) {
            det = 0.0;
            break;
        }
        det *= matrix[i][i];
    }
    for (size_t i = 0; i < N; i++) { deallocate<double, DeallocType::Multiple>(matrix[i]); }
    deallocate<double*, DeallocType::Multiple>(matrix);
    return det;
}
double Matrix2D::det() const {
    HARD_ASSERT(m_rows == m_columns, "Matrix must be square to calculate the determinant");

    // i'll have hardcoded math for 2x2 and 3x3
    if (m_rows == 2) {
        return m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0];
    } else if (m_rows == 3) {
        // ignore first row first column
        double first_partial = m_matrix[1][1] * m_matrix[2][2] - m_matrix[1][2] * m_matrix[2][1];
        // ignore first row second column
        double second_partial = m_matrix[1][0] * m_matrix[2][2] - m_matrix[1][2] * m_matrix[2][0];
        // ignore first row third column
        double third_partial = m_matrix[1][0] * m_matrix[2][1] - m_matrix[1][1] * m_matrix[2][0];

        return (first_partial * m_matrix[0][0]) - (second_partial * m_matrix[0][1]) + (third_partial * m_matrix[0][2]);
    } else {
        // generic math for 4x4 and up
        return det_internal(m_matrix, m_rows, m_columns);
    }
}
String PrintInfo<Matrix2D>::repr() const {
    String ret{};
    for (size_t i = 0; i < m_matrix.row_number(); i++) {
        ret += "[ "_s;
        for (size_t j = 0; j < m_matrix.column_number(); j++) {
            ret += PrintInfo<double>{ m_matrix[{ i, j }] }.repr();
            if (j != m_matrix.column_number() - 1) ret += ", "_s;
        }
        ret += " ]"_s;
        if (i != m_matrix.row_number() - 1) ret += '\n';
    }
    return ret;
}
}    // namespace ARLib
