#pragma once
#include "Concepts.hpp"
#include "FixedMatrix.hpp"
#include "Memory.hpp"
#include "Pair.hpp"
namespace ARLib {
template <typename T>
struct PrintInfo;

enum class MatIterType { ByRow, ByCol };

template <bool, MatIterType>
class MatrixIterator;

template <bool>
class CartesianIterator;

template <bool>
class RowArray;
class Matrix2D {
    double* m_matrix{};
    size_t m_rows;
    size_t m_columns;
    struct IndexCalculator {
        size_t N;
        size_t M;
        constexpr size_t index(size_t r, size_t c) const { return r * M + c; };
    };
    static void print_debug_matrix(double* const og_matrix, size_t N, size_t M);
    static void swap_row(double* matrix, size_t row1, size_t row2, size_t N, size_t M) {
        IndexCalculator calc{ N, M };
        for (size_t i = 0; i < M; ++i) {
            double temp                 = matrix[calc.index(row1, i)];
            matrix[calc.index(row1, i)] = matrix[calc.index(row2, i)];
            matrix[calc.index(row2, i)] = temp;
        }
    }
    static void gauss_reduce(double* matrix, size_t N, size_t M);
    static void row_echelon_transform(double* matrix, size_t N, size_t M);
    static int rank_internal(double* const og_matrix, size_t N, size_t M);
    static double det_internal(double* const og_matrix, size_t N, size_t M);
    void allocate_memory(bool zeroinit = false) {
        m_matrix = allocate_uninitialized<double>(m_rows * m_columns);
        if (zeroinit) memset(m_matrix, 0, m_columns * m_rows * sizeof(double));
    }
    void deallocate_memory() { deallocate<double, DeallocType::Multiple>(m_matrix); }
    constexpr arlib_forceinline size_t index(size_t row, size_t col) const { return row * m_columns + col; };
    constexpr arlib_forceinline size_t index(Pair<size_t, size_t> size) const {
        return size.first() * m_columns + size.second();
    };

    public:
    static Matrix2D eye(size_t size) {
        Matrix2D mat{ size, size };
        for (size_t i = 0; i < size; i++) { mat.m_matrix[mat.index(i, i)] = 1.0; }
        return mat;
    }
    template <size_t N, size_t M>
    explicit Matrix2D(const FixedMatrix2D<N, M>& matrix) : m_rows(N), m_columns(M) {
        allocate_memory();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) { m_matrix[index(i, j)] = matrix[{ i, j }]; }
        }
    }
    Matrix2D(size_t rows, size_t columns) : m_rows(rows), m_columns(columns) { allocate_memory(true); }
    template <size_t N, size_t M>
    Matrix2D(const double (&mat)[N][M]) : m_rows(N), m_columns(M) {
        allocate_memory();
        for (size_t i = 0; i < N; i++) { ConditionalBitCopy(m_matrix + (i * m_columns), mat[i], M); }
    }
    Matrix2D(const Matrix2D& other) : m_rows(other.m_rows), m_columns(other.m_columns) {
        allocate_memory();
        ConditionalBitCopy(m_matrix, other.m_matrix, m_columns * m_rows);
    }
    Matrix2D(Matrix2D&& other) noexcept : m_rows(other.m_rows), m_columns(other.m_columns) {
        m_matrix       = other.m_matrix;
        other.m_matrix = nullptr;
    }
    Matrix2D& operator=(const Matrix2D& other) {
        deallocate_memory();
        m_rows    = other.m_rows;
        m_columns = other.m_columns;
        allocate_memory();
        ConditionalBitCopy(m_matrix, other.m_matrix, m_columns * m_rows);
        return *this;
    }
    Matrix2D& operator=(Matrix2D&& other) noexcept {
        deallocate_memory();
        m_rows         = other.m_rows;
        m_columns      = other.m_columns;
        m_matrix       = other.m_matrix;
        other.m_matrix = nullptr;
        return *this;
    }
    /* ITERATORS */
    CartesianIterator<true> cartesian() const;
    CartesianIterator<false> cartesian();
    template <MatIterType IterType = MatIterType::ByRow>
    MatrixIterator<true, IterType> begin() const;
    template <MatIterType IterType = MatIterType::ByRow>
    MatrixIterator<true, IterType> end() const;
    template <MatIterType IterType = MatIterType::ByRow>
    MatrixIterator<false, IterType> begin();
    template <MatIterType IterType = MatIterType::ByRow>
    MatrixIterator<false, IterType> end();
    MatrixIterator<true, MatIterType::ByCol> columns_begin(size_t begin_idx = 0) const;
    MatrixIterator<true, MatIterType::ByCol> columns_end() const;
    MatrixIterator<false, MatIterType::ByCol> columns_begin(size_t begin_idx = 0);
    MatrixIterator<false, MatIterType::ByCol> columns_end();
    MatrixIterator<true, MatIterType::ByRow> rows_begin(size_t begin_idx = 0) const;
    MatrixIterator<true, MatIterType::ByRow> rows_end() const;
    MatrixIterator<false, MatIterType::ByRow> rows_begin(size_t begin_idx = 0);
    MatrixIterator<false, MatIterType::ByRow> rows_end();
    double& operator[](Pair<size_t, size_t> idx) { return m_matrix[index(idx)]; }
    const double& operator[](Pair<size_t, size_t> idx) const { return m_matrix[index(idx)]; }
    RowArray<false> operator[](size_t idx);
    RowArray<true> operator[](size_t idx) const;
#define MAT_LOOP(op)                                                                                                   \
    for (size_t i = 0; i < m_rows; i++) {                                                                              \
        for (size_t j = 0; j < m_columns; j++) { op; }                                                                 \
    }

#define DEFINE_INPLACE_OP(op)                                                                                          \
    Matrix2D& operator op(double val) {                                                                                \
        MAT_LOOP(m_matrix[index(i, j)] op val);                                                                        \
        return *this;                                                                                                  \
    }

#define DEFINE_OP(op, in_op)                                                                                           \
    Matrix2D operator op(double val) const {                                                                           \
        auto copy = *this;                                                                                             \
        MAT_LOOP(copy.m_matrix[index(i, j)] in_op val);                                                                \
        return copy;                                                                                                   \
    }

    /* MATHEMATICAL OPERATORS */
    DEFINE_INPLACE_OP(+=);
    DEFINE_INPLACE_OP(-=);

    DEFINE_OP(+, +=);
    DEFINE_OP(-, -=);
    Matrix2D operator*(const Matrix2D& other) const {
        HARD_ASSERT(m_rows == other.num_rows(), "Impossible to do the multiplication, sizes do not match");
        Matrix2D mat{ m_rows, other.num_columns() };
        for (size_t col = 0; col < other.num_columns(); col++) {
            for (size_t row = 0; row < m_rows; row++) {
                double intermediate{ 0 };
                for (size_t i = 0; i < m_columns; i++) { intermediate += m_matrix[index(row, i)] * other[{ i, col }]; }
                mat[{ row, col }] = intermediate;
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
        MAT_LOOP(m_matrix[index(i, j)] += 1);
        return *this;
    }
    Matrix2D operator++(int) {
        auto copy = *this;
        MAT_LOOP(m_matrix[index(i, j)] += 1);
        return copy;
    }
    Matrix2D& operator--() {
        MAT_LOOP(m_matrix[index(i, j)] -= 1);
        return *this;
    }
    Matrix2D operator--(int) {
        auto copy = *this;
        MAT_LOOP(m_matrix[index(i, j)] -= 1);
        return copy;
    }
    Pair<size_t, size_t> shape() const { return { m_rows, m_columns }; }
    size_t size() const { return m_rows; }
    size_t num_rows() const { return m_rows; }
    size_t num_columns() const { return m_columns; }
    void reduce() { row_echelon_transform(m_matrix, m_rows, m_columns); }
    int rank() const { return rank_internal(m_matrix, m_rows, m_columns); }
    double det() const;
    double sum() const {
        double res = 0.0;
        MAT_LOOP(res += m_matrix[index(i, j)]);
        return res;
    }
    double avg() const { return sum() / static_cast<double>(m_rows * m_columns); }
    auto sub(size_t n_rows, size_t n_columns, size_t start_row = 0, size_t start_col = 0) const {
        HARD_ASSERT(start_row + n_rows <= m_rows, "Submatrix is impossible to construct");
        HARD_ASSERT(start_col + n_columns <= m_columns, "Submatrix is impossible to construct");
        Matrix2D submat{ n_rows, n_columns };
        for (size_t i = start_row; i < start_row + n_rows; i++) {
            for (size_t j = start_col; j < start_col + n_columns; j++) {
                submat[{ i - start_row, j - start_col }] = m_matrix[index(i, j)];
            }
        }
        return submat;
    }
    Matrix2D inv() const {
        HARD_ASSERT(m_rows == m_columns, "Matrix must be square to calculate the inverse");
        Matrix2D mat_glued{ m_rows, m_columns * 2 };
        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_columns; j++) { mat_glued[{ i, j }] = m_matrix[index(i, j)]; }
        }
        for (size_t j = m_columns; j < m_columns * 2; j++) { mat_glued[{ j - m_columns, j }] = 1.0; }
        mat_glued.reduce();
        return mat_glued.sub(m_rows, m_columns, 0, m_columns);
    }
    Matrix2D transpose() const {
        Matrix2D mat{ m_rows, m_columns };
        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_columns; j++) { mat[{ j, i }] = m_matrix[index(i, j)]; }
        }
        return mat;
    }
    bool operator==(const Matrix2D& other) const {
        if (shape() != other.shape()) return false;
        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_columns; j++) {
                if (m_matrix[index(i, j)] != other.m_matrix[index(i, j)]) return false;
            }
        }
        return true;
    }
    bool operator!=(const Matrix2D& other) const {
        if (shape() != other.shape()) return true;
        for (size_t i = 0; i < m_rows; i++) {
            for (size_t j = 0; j < m_columns; j++) {
                if (m_matrix[index(i, j)] != other.m_matrix[index(i, j)]) return true;
            }
        }
        return false;
    }
    ~Matrix2D() {
        if (m_matrix) deallocate_memory();
    }
};
using Mat2DRef = AddLvalueReferenceT<Matrix2D>;
template <bool IsConst>
class ColumnIterator {
    using Tp = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;
    Tp m_matrix;
    size_t m_current_col;
    size_t m_current_index;

    public:
    ColumnIterator(Tp matrix, size_t current_col, size_t current_index) :
        m_matrix(matrix), m_current_col(current_col), m_current_index(current_index) {}
    auto& operator*() { return m_matrix[{ m_current_index, m_current_col }]; }
    const auto& operator*() const { return m_matrix[{ m_current_index, m_current_col }]; }
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
        return addressof(m_matrix) == addressof(other.m_matrix) && m_current_col == other.m_current_col &&
               m_current_index == other.m_current_index;
    }
    bool operator!=(const ColumnIterator& other) const {
        return addressof(m_matrix) != addressof(other.m_matrix) || m_current_col != other.m_current_col ||
               m_current_index != other.m_current_index;
    }
};
template <bool IsConst = false>
class ColumnArray {
    friend Matrix2D;
    friend MatrixIterator<IsConst, MatIterType::ByCol>;
    using Tp = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;

    Tp m_matrix;
    size_t m_current_col{ 0 };
    ColumnArray(Tp matrix, size_t idx) : m_matrix(matrix), m_current_col(idx) {}
    void reset_to(size_t new_col) { m_current_col = new_col; }

    public:
    double& operator[](size_t index)
    requires(!IsConst)
    {
        return m_matrix[{ index, m_current_col }];
    }
    const double& operator[](size_t index) const { return m_matrix[{ index, m_current_col }]; }
    size_t size() const { return m_matrix.num_rows(); }
    auto begin() const { return ColumnIterator<true>{ m_matrix, m_current_col, 0 }; }
    auto end() const { return ColumnIterator<true>{ m_matrix, m_current_col, size() }; }
    auto begin() { return ColumnIterator<false>{ m_matrix, m_current_col, 0 }; }
    auto end() { return ColumnIterator<false>{ m_matrix, m_current_col, size() }; }
};
template <bool IsConst>
class RowIterator {
    using Tp = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;

    Tp m_matrix;
    size_t m_current_row;
    size_t m_current_index;

    public:
    RowIterator(Tp matrix, size_t current_row, size_t current_index) :
        m_matrix(matrix), m_current_row(current_row), m_current_index(current_index) {}
    auto& operator*() { return m_matrix[{ m_current_row, m_current_index }]; }
    const auto& operator*() const { return m_matrix[{ m_current_row, m_current_index }]; }
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
        return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row &&
               m_current_index == other.m_current_index;
    }
    bool operator!=(const RowIterator& other) const {
        return addressof(m_matrix) != addressof(other.m_matrix) || m_current_row != other.m_current_row ||
               m_current_index != other.m_current_index;
    }
};
template <bool IsConst = false>
class RowArray {
    friend Matrix2D;
    friend MatrixIterator<IsConst, MatIterType::ByRow>;

    using Tp = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;

    Tp m_matrix;
    size_t m_row_index;
    RowArray(Tp matrix, size_t idx) : m_matrix(matrix), m_row_index(idx) {}
    void reset_to(size_t new_row) { m_row_index = new_row; }

    public:
    double& operator[](size_t index)
    requires(!IsConst)
    {
        return m_matrix[{ m_row_index, index }];
    }
    const double& operator[](size_t index) const { return m_matrix[{ m_row_index, index }]; }
    size_t size() const { return m_matrix.num_columns(); }
    auto begin() const { return RowIterator<true>{ m_matrix, m_row_index, 0 }; }
    auto end() const { return RowIterator<true>{ m_matrix, m_row_index, size() }; }
    auto begin() { return RowIterator<false>{ m_matrix, m_row_index, 0 }; }
    auto end() { return RowIterator<false>{ m_matrix, m_row_index, size() }; }
};
template <bool IsConst, MatIterType IterType = MatIterType::ByRow>
class MatrixIterator {
    using Tp  = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;
    using Ret = ConditionalT<IterType == MatIterType::ByRow, RowArray<IsConst>, ColumnArray<IsConst>>;

    Tp m_matrix;
    size_t m_current_row;
    Ret m_current;

    public:
    MatrixIterator(Tp matrix, size_t current_row) :
        m_matrix(matrix), m_current_row(current_row), m_current(matrix, current_row) {}
    Ret& operator*() { return m_current; }
    const Ret& operator*() const { return m_current; }
    MatrixIterator& operator++() {
        m_current_row++;
        m_current.reset_to(m_current_row);
        return *this;
    }
    MatrixIterator operator++(int) {
        auto copy = *this;
        m_current_row++;
        m_current.reset_to(m_current_row);
        return copy;
    }
    MatrixIterator& operator--() {
        m_current_row--;
        m_current.reset_to(m_current_row);
        return *this;
    }
    MatrixIterator operator--(int) {
        auto copy = *this;
        m_current_row--;
        m_current.reset_to(m_current_row);
        return copy;
    }
    bool operator==(const MatrixIterator& other) const {
        return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row;
    }
    bool operator!=(const MatrixIterator& other) const {
        return m_matrix != other.m_matrix || m_current_row != other.m_current_row;
    }
};
template <bool IsConst>
class CartesianIterator {
    using Mat = ConditionalT<IsConst, AddConstT<Mat2DRef>, Mat2DRef>;
    Mat m_matrix;
    size_t m_current_row;
    size_t m_current_col;

    public:
    CartesianIterator(Mat matrix) : m_matrix(matrix), m_current_row(0), m_current_col(0) {}
    CartesianIterator(Mat matrix, size_t current_row, size_t current_col) :
        m_matrix(matrix), m_current_row(current_row), m_current_col(current_col) {}
    CartesianIterator begin() const { return *this; }
    CartesianIterator end() const { return { m_matrix, m_matrix.num_rows(), m_matrix.num_columns() }; }
    CartesianIterator begin() { return *this; }
    CartesianIterator end() { return { m_matrix, m_matrix.num_rows(), m_matrix.num_columns() }; }
    double& operator*() { return m_matrix[{ m_current_row, m_current_col }]; }
    const double& operator*() const { return m_matrix[{ m_current_row, m_current_col }]; }
    CartesianIterator& operator++() {
        if (++m_current_col == m_matrix.num_columns()) {
            m_current_col = 0;
            ++m_current_row;
        }
        return *this;
    }
    CartesianIterator operator++(int) {
        auto copy = *this;
        if (++m_current_col == m_matrix.num_columns()) {
            m_current_col = 0;
            ++m_current_row;
        }
        return copy;
    }
    CartesianIterator& operator--() {
        if (m_current_col == 0) {
            m_current_col = m_matrix.num_columns() - 1;
            --m_current_row;
        } else {
            --m_current_col;
        }
        return *this;
    }
    CartesianIterator operator--(int) {
        auto copy = *this;
        if (m_current_col == 0) {
            m_current_col = m_matrix.num_columns() - 1;
            --m_current_row;
        } else {
            --m_current_col;
        }
        return copy;
    }
    bool operator==(const CartesianIterator& other) const {
        return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row &&
               m_current_col == other.m_current_col;
    }
    bool operator!=(const CartesianIterator& other) const {
        return addressof(m_matrix) != addressof(other.m_matrix) || m_current_row != other.m_current_row ||
               m_current_col == other.m_current_col;
    }
};
template <MatIterType IterType>
MatrixIterator<true, IterType> Matrix2D::begin() const {
    return { *this, 0 };
}
template <MatIterType IterType>
MatrixIterator<true, IterType> Matrix2D::end() const {
    return { *this, (IterType == MatIterType::ByRow) ? m_rows : m_columns };
}
template <MatIterType IterType>
MatrixIterator<false, IterType> Matrix2D::begin() {
    return { *this, 0 };
}
template <MatIterType IterType>
MatrixIterator<false, IterType> Matrix2D::end() {
    return { *this, (IterType == MatIterType::ByRow) ? m_rows : m_columns };
}
template <size_t N, size_t M>
Matrix2D operator*(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
    HARD_ASSERT(M == second.num_rows(), "Impossible to do the multiplication, sizes do not match");
    Matrix2D mat{ N, second.num_columns() };
    for (size_t col = 0; col < second.num_columns(); col++) {
        for (size_t row = 0; row < N; row++) {
            double intermediate{ 0 };
            for (size_t i = 0; i < M; i++) { intermediate += first[{ row, i }] * second[{ i, col }]; }
            mat[{ row, col }] = intermediate;
        }
    }
    return mat;
}
template <size_t N, size_t M>
Matrix2D operator*(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    HARD_ASSERT(N == first.num_columns(), "Impossible to do the multiplication, sizes do not match");
    Matrix2D mat{ first.num_rows(), M };
    for (size_t col = 0; col < M; col++) {
        for (size_t row = 0; row < first.num_rows(); row++) {
            double intermediate{ 0 };
            for (size_t i = 0; i < N; i++) { intermediate += first[{ row, i }] * second[{ i, col }]; }
            mat[{ row, col }] = intermediate;
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
    Matrix2D mat{ first };
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { mat[{ i, j }] += second[{ i, j }]; }
    }
    return mat;
}
template <size_t N, size_t M>
Matrix2D operator+(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
    Matrix2D mat{ first };
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { mat[{ i, j }] += second[{ i, j }]; }
    }
    return mat;
}
template <size_t N, size_t M>
FixedMatrix2D<N, M>& operator+=(FixedMatrix2D<N, M>& first, const Matrix2D& second) {
    HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { first[{ i, j }] += second[{ i, j }]; }
    }
    return first;
}
template <size_t N, size_t M>
Matrix2D& operator+=(Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { first[{ i, j }] += second[{ i, j }]; }
    }
    return first;
}
template <size_t N, size_t M>
Matrix2D operator-(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
    HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
    Matrix2D mat{ first };
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { mat[{ i, j }] -= second[{ i, j }]; }
    }
    return mat;
}
template <size_t N, size_t M>
Matrix2D operator-(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    HARD_ASSERT(first.shape() == second.shape(), "Matrices must have the same shape");
    Matrix2D mat{ first };
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) { mat[{ i, j }] -= second[{ i, j }]; }
    }
    return mat;
}
template <size_t N, size_t M>
bool operator==(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    if (first.shape() != second.shape()) return false;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            if (first[{ i, j }] != second[{ i, j }]) return false;
        }
    }
    return true;
}
template <size_t N, size_t M>
bool operator==(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
    if (first.shape() != second.shape()) return false;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            if (first[{ i, j }] != second[{ i, j }]) return false;
        }
    }
    return true;
}
template <size_t N, size_t M>
bool operator!=(const Matrix2D& first, const FixedMatrix2D<N, M>& second) {
    if (first.shape() != second.shape()) return true;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            if (first[{ i, j }] != second[{ i, j }]) return true;
        }
    }
    return false;
}
template <size_t N, size_t M>
bool operator!=(const FixedMatrix2D<N, M>& first, const Matrix2D& second) {
    if (first.shape() != second.shape()) return true;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            if (first[{ i, j }] != second[{ i, j }]) return true;
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
}    // namespace ARLib
