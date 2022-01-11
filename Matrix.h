#pragma once
#include "CharConv.h"
#include "Concepts.h"
#include "Iterator.h"
#include "Memory.h"
#include "Optional.h"
#include "Pair.h"
#include "PrintInfo.h"
#include "Printer.h"

namespace ARLib {
    // clang-format off
    template <Numeric T, size_t N, size_t M>
    class Matrix2D;
    template <Numeric T, size_t N, size_t M, bool IsConst, bool ByRow>
    class MatrixIterator;

    template <Numeric T, size_t N, size_t M>
    struct MatrixTraits {
        static constexpr bool Square = N == M;
        static constexpr size_t MAX_INLINE_SIZE = 20;
        static constexpr bool ValidToInline = N <= MAX_INLINE_SIZE && M <= MAX_INLINE_SIZE;

        using MatType = ConditionalT<ValidToInline, T[N][M], T**>;
        using MatRefType = ConditionalT<ValidToInline, T (&)[N][M], T**>;
        using ConstMatRefType = AddConstT<MatRefType>;

        using SubOpRetType = ConditionalT<ValidToInline, T (&)[M], T*>;
        using ConstSubOpRetType = AddConstT<SubOpRetType>;
    };

    template <Numeric T, size_t N, size_t M, bool IsConst>
    class ColumnIterator {
        using traits = MatrixTraits<T, N, M>;
        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;

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

    template <Numeric T, size_t N, size_t M = N, bool IsConst = false>
    class ColumnArray {
        friend Matrix2D<T, N, M>;
        friend MatrixIterator<T, N, M, IsConst, false>;

        using traits = MatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;
        using InputType = typename traits::MatRefType;
        using ConstInputType = typename traits::ConstMatRefType;

        Tp* m_column = nullptr;

        ColumnArray() = default;

        ColumnArray(InputType matrix, size_t idx) requires(!IsConst) {
            m_column = allocate<Tp>(N);
            for (size_t i = 0; i < N; i++) {
                m_column[i] = &matrix[i][idx];
            };
        }
        ColumnArray(ConstInputType matrix, size_t idx) requires IsConst {
            m_column = allocate<Tp>(N);
            for (size_t i = 0; i < N; i++) {
                m_column[i] = &matrix[i][idx];
            };
        }
        public: 

        ColumnArray& operator=(ColumnArray&& other) {
            deallocate<Tp, DeallocType::Multiple>(m_column);
            m_column = other.m_column;
            other.m_column = nullptr;
            return *this;
        }
        T& operator[](size_t index) requires(!IsConst) { return *m_column[index]; }
        const T& operator[](size_t index) const { return *m_column[index]; }

        auto begin() const { return ColumnIterator<T, N, M, true>{m_column, 0}; }
        auto end() const { return ColumnIterator<T, N, M, true>{m_column, N}; }

        auto begin() { return ColumnIterator<T, N, M, false>{m_column, 0}; }
        auto end() { return ColumnIterator<T, N, M, false>{m_column, N}; }

        ~ColumnArray() {
            deallocate<Tp, DeallocType::Multiple>(m_column);
        }
    };

    template <Numeric T, size_t N, size_t M, bool IsConst>
    class RowIterator {
        using traits = MatrixTraits<T, N, M>;
        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;

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

    template <Numeric T, size_t N, size_t M = N, bool IsConst = false>
    class RowArray {
        friend Matrix2D<T, N, M>;
        friend MatrixIterator<T, N, M, IsConst, true>;

        using traits = MatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;
        using InputType = typename traits::MatRefType;
        using ConstInputType = typename traits::ConstMatRefType;

        Tp m_row;

        RowArray(InputType matrix, size_t idx) requires(!IsConst) { m_row = matrix[idx]; }
        RowArray(ConstInputType matrix, size_t idx) requires IsConst { m_row = matrix[idx]; }

        public: 
        T& operator[](size_t index) requires(!IsConst) { return m_row[index]; }
        const T& operator[](size_t index) const { return m_row[index]; }
        
        auto begin() const { return RowIterator<T, N, M, true>{m_row, 0}; }
        auto end() const { return RowIterator<T, N, M, true>{m_row, M}; }

        auto begin() { return RowIterator<T, N, M, false>{m_row, 0}; }
        auto end() { return RowIterator<T, N, M, false>{m_row, M}; }
    };

    // clang-format on
    template <Numeric T, size_t N, size_t M, bool IsConst, bool ByRow = true>
    class MatrixIterator {
        using traits = MatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, typename traits::ConstMatRefType, typename traits::MatRefType>;
        using Ret = ConditionalT<ByRow, RowArray<T, N, M, IsConst>, ColumnArray<T, N, M, IsConst>>;

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
            m_current = Ret{m_matrix, m_current_row};
            return *this;
        }
        MatrixIterator operator++(int) {
            auto copy = *this;
            m_current_row++;
            m_current = Ret{m_matrix, m_current_row};
            return copy;
        }

        MatrixIterator& operator--() {
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row};
            return *this;
        }
        MatrixIterator operator--(int) {
            auto copy = *this;
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row};
            return copy;
        }

        bool operator==(const MatrixIterator& other) const {
            if constexpr (traits::ValidToInline) {
                return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row;
            } else {
                return m_matrix == other.m_matrix && m_current_row == other.m_current_row;
            }
        }
        bool operator!=(const MatrixIterator& other) const {
            if constexpr (traits::ValidToInline) {
                return addressof(m_matrix) != addressof(other.m_matrix) || m_current_row != other.m_current_row;
            } else {
                return m_matrix != other.m_matrix || m_current_row != other.m_current_row;
            }
        }
    };

    template <Numeric T, size_t N, size_t M = N>
    class Matrix2D {
        using traits = MatrixTraits<T, N, M>;

        static constexpr bool ValidToInline = traits::ValidToInline;
        static constexpr bool Square = traits::Square;

        using MatType = typename traits::MatType;
        using SubOpRetType = typename traits::SubOpRetType;
        using ConstSubOpRetType = typename traits::ConstSubOpRetType;

        Optional<double> m_cached_det{};
        MatType m_matrix{};

        static void print_debug_matrix(T** og_matrix) {
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < N; j++) {
                    printf("%lf ", og_matrix[i][j]);
                }
                puts("");
            }
        }

        static void swap_row(typename traits::MatRefType matrix, size_t row1, size_t row2) {
            if constexpr (!ValidToInline) {
                // this relies on the fact that rows are allocated each on their own
                // this means that to swap 2 rows we can just swap the pointers
                T* temp = matrix[row1];
                matrix[row1] = matrix[row2];
                matrix[row2] = temp;
            } else {
                T temp[M]{};
                ConditionalBitCopy(temp, matrix[row1], M);
                ConditionalBitCopy(matrix[row1], matrix[row2], M);
                ConditionalBitCopy(matrix[row2], temp, M);
            }
        }

        static void gauss_reduce(typename traits::MatRefType matrix) {
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

        static bool check_row_all_zeros(typename traits::ConstSubOpRetType row) {
            constexpr T zero_val = T{0};
            for (size_t i = 0; i < M; i++) {
                if (row[i] != zero_val) return false;
            }
            return true;
        }

        static void row_echelon_transform(typename traits::MatRefType matrix) {
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
                if (matrix[r][lead] != T{0}) { // this check is here because otherwise I'd divide by zero
                    for (size_t j = 0; j < colCount; j++)
                        matrix[r][j] /= matrix[r][lead];
                }
                for (size_t j = 0; j < rowCount; j++) {
                    if (j != r) { 
                        // Subtract M[j, lead] multiplied by row r from row j
                        for (size_t k = 0; k < colCount; k++)
                            matrix[j][k] -= (matrix[j][lead] * matrix[r][k]);
                    }
                }
                lead++;
            }
        }

        static void gauss_jordan_reduce(typename traits::MatRefType matrix) {

            using RowT = typename traits::SubOpRetType;

            // 1. Swap the rows so that all rows with all zero entries are on the bottom
            size_t row_to_move_to = N - 1;
            for (size_t i = 0; i < N; i++) {
                if (check_row_all_zeros(matrix[i])) {
                    swap_row(matrix, i, row_to_move_to);
                    row_to_move_to--;
                }
            }

            size_t top_row_idx = 0;
            size_t top_col_idx = 0;
            do {
                // 2. Swap the rows so that the row with the largest, leftmost nonzero entry is on top
                for (size_t j = 0; j < M; j++) {
                    size_t largest_row_idx = npos_;
                    T max = matrix[top_row_idx][j];
                    for (size_t i = top_row_idx; i < N; i++) {
                        if (matrix[i][j] > max) {
                            max = matrix[i][j];
                            largest_row_idx = i;
                        }
                    }
                    if (largest_row_idx != npos_) {
                        swap_row(matrix, top_row_idx, largest_row_idx);
                        break;
                    }
                }

                // 3. Multiply the top row by a scalar so that top row's leading entry becomes 1.
                double val = 1.0 / static_cast<double>(matrix[top_row_idx][top_col_idx]);
                for (size_t i = top_col_idx; i < M; i++)
                    matrix[top_row_idx][i] *= val;

                // 4. Add/subtract multiples of the top row to the other rows so that all other entries in the column
                // containing the top row's leading entry are all zero.
                for (size_t i = top_row_idx + 1; i < N; i++) {
                    if (matrix[i][top_col_idx] != T{0}) {
                        for (size_t j = top_col_idx; j < M; j++) {
                            if (matrix[top_row_idx][j] != T{0}) {
                                T to_sub = matrix[i][j] / matrix[top_row_idx][j];
                                matrix[i][j] -= to_sub;
                            }
                        }
                    }
                }

                // 5. Repeat steps 2-4 for the next leftmost nonzero entry until all the leading entries are 1.
                top_row_idx++;
                top_col_idx++;
            } while (top_row_idx < N && top_col_idx < M);
        }

        static double rank_internal(typename traits::ConstMatRefType og_matrix) {
            typename traits::MatType matrix{};
            if constexpr (!ValidToInline) { matrix = allocate<T*>(N); }
            for (size_t i = 0; i < N; i++) {
                if constexpr (!ValidToInline) { matrix[i] = allocate<T>(N); }
                ConditionalBitCopy(matrix[i], og_matrix[i], N);
            }
            gauss_reduce(matrix);
            int rank = 0;
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    if (matrix[i][j] != 0) {
                        rank++;
                        break;
                    }
                }
            }
            if constexpr (!ValidToInline) {
                for (size_t i = 0; i < N; i++) {
                    deallocate<T, DeallocType::Multiple>(matrix[i]);
                }
                deallocate<T*, DeallocType::Multiple>(matrix);
            }
            return rank;
        }

        static double det_internal(typename traits::ConstMatRefType og_matrix) requires Square {
            typename traits::MatType matrix{};
            if constexpr (!ValidToInline) { matrix = allocate<T*>(N); }
            for (size_t i = 0; i < N; i++) {
                if constexpr (!ValidToInline) { matrix[i] = allocate<T>(N); }
                ConditionalBitCopy(matrix[i], og_matrix[i], N);
            }
            gauss_reduce(matrix);
            double det = 1.0;
            for (size_t i = 0; i < N; i++) {
                if (matrix[i][i] == T{0}) {
                    det = 0.0;
                    break;
                }
                det *= matrix[i][i];
            }
            if constexpr (!ValidToInline) {
                for (size_t i = 0; i < N; i++) {
                    deallocate<T, DeallocType::Multiple>(matrix[i]);
                }
                deallocate<T*, DeallocType::Multiple>(matrix);
            }
            return det;
        }

        void allocate_memory(bool zeroinit = false) requires(!ValidToInline) {
            m_matrix = allocate<T*>(N);
            for (size_t i = 0; i < N; i++) {
                m_matrix[i] = allocate<T>(M);
                if (zeroinit) memset(m_matrix[i], 0, M * sizeof(T));
            }
        }
        void deallocate_memory() requires(!ValidToInline) {
            for (size_t i = 0; i < N; i++) {
                deallocate<T, DeallocType::Multiple>(m_matrix[i]);
            }
            deallocate<T*, DeallocType::Multiple>(m_matrix);
        }

        // clang-format off
        public: 
        /* CONSTRUCTORS */
        static Matrix2D eye() requires Square {
            Matrix2D mat{};
            for (size_t i = 0; i < N; i++) {
                mat[{i, i}] = T{1};
            }
            return mat;
        }
        Matrix2D() requires ValidToInline = default;
        Matrix2D() requires(!ValidToInline) {
            allocate_memory(true);
        }
        // clang-format on

        template <typename... Args>
        Matrix2D(Args... args) requires(AllOfV<T, Args...> && sizeof...(Args) == (N * M)) {
            T mm[N * M]{args...};
            if constexpr (!ValidToInline) m_matrix = allocate<T*>(N);
            for (size_t i = 0; i < N; i++) {
                if constexpr (!ValidToInline) m_matrix[i] = allocate<T>(M);
                for (size_t j = 0; j < M; j++) {
                    m_matrix[i][j] = mm[i * M + j];
                }
            }
        }

        Matrix2D(const T (&matrix)[N][M]) requires ValidToInline {
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], matrix[i], M);
            }
        }

        /* COPY AND MOVE CTORS AND ASSIGNMENT OPS*/

        Matrix2D(const Matrix2D& other) {
            if constexpr (!ValidToInline) { allocate_memory(); }
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
            }
        }
        Matrix2D(Matrix2D&& other) noexcept {
            if constexpr (ValidToInline) {
                for (size_t i = 0; i < N; i++) {
                    ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
                }
            } else {
                m_matrix = other.m_matrix;
                other.m_matrix = nullptr;
            }
        }

        Matrix2D& operator=(const Matrix2D& other) {
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
            }
        }

        Matrix2D& operator=(Matrix2D&& other) noexcept {
            if constexpr (ValidToInline) {
                for (size_t i = 0; i < N; i++) {
                    ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
                }
            } else {
                deallocate_memory();
                m_matrix = other.m_matrix;
                other.m_matrix = nullptr;
            }
        }

        /* ITERATORS */
        template <bool ByRow = true>
        auto begin() const {
            return MatrixIterator<T, N, M, true, ByRow>{m_matrix, 0};
        }

        template <bool ByRow = true>
        auto end() const {
            return MatrixIterator<T, N, M, true, ByRow>{m_matrix, N};
        }

        template <bool ByRow = true>
        auto begin() {
            return MatrixIterator<T, N, M, false, ByRow>{m_matrix, 0};
        }

        template <bool ByRow = true>
        auto end() {
            return MatrixIterator<T, N, M, false, ByRow>{m_matrix, N};
        }

        auto columns_begin(size_t begin_idx = 0) const {
            return MatrixIterator<T, N, M, true, false>{m_matrix, begin_idx};
        }
        auto columns_end() const { return MatrixIterator<T, N, M, true, false>{m_matrix, N}; }

        auto columns_begin(size_t begin_idx = 0) { return MatrixIterator<T, N, M, false, false>{m_matrix, begin_idx}; }
        auto columns_end() { return MatrixIterator<T, N, M, false, false>{m_matrix, N}; }

        auto rows_begin(size_t begin_idx = 0) const { return MatrixIterator<T, N, M, true, true>{m_matrix, begin_idx}; }
        auto rows_end() const { return MatrixIterator<T, N, M, true, true>{m_matrix, N}; }

        auto rows_begin(size_t begin_idx = 0) { return MatrixIterator<T, N, M, false, true>{m_matrix, begin_idx}; }
        auto rows_end() { return MatrixIterator<T, N, M, false, true>{m_matrix, N}; }

        /* INDEX OPERATORS */
        T& operator[](Pair<size_t, size_t> idx) { return m_matrix[idx.first()][idx.second()]; }
        const T& operator[](Pair<size_t, size_t> idx) const { return m_matrix[idx.first()][idx.second()]; }
        auto columns(size_t index) const { return ColumnArray<T, N, M, true>{m_matrix, index}; }
        auto columns(size_t index) { return ColumnArray<T, N, M, false>{m_matrix, index}; }
        auto rows(size_t index) const { return RowArray<T, N, M, true>{m_matrix, index}; }
        auto rows(size_t index) { return RowArray<T, N, M, false>{m_matrix, index}; }
        auto operator[](size_t index) const { return RowArray<T, N, M, true>{m_matrix, index}; }
        auto operator[](size_t index) { return RowArray<T, N, M, false>{m_matrix, index}; }

#define MAT_LOOP(op)                                                                                                   \
    for (size_t i = 0; i < N; i++) {                                                                                   \
        for (size_t j = 0; j < M; j++) {                                                                               \
            op;                                                                                                        \
        }                                                                                                              \
    }

#define DEFINE_INPLACE_OP(op)                                                                                          \
    Matrix2D& operator##op(const Matrix2D& other) {                                                                    \
        MAT_LOOP(m_matrix[i][j] op other.m_matrix[i][j]);                                                              \
        return *this;                                                                                                  \
    }

/* clang-format off */
#define DEFINE_OP(op)                                                                                                  \
    Matrix2D operator##op(const Matrix2D& other) const {                                                               \
        auto copy = *this;                                                                                             \
        MAT_LOOP(copy.m_matrix[i][j] ##op= other.m_matrix[i][j]);                                                      \
        return copy;                                                                                                   \
    }
        /* clang-format on */

        /* MATHEMATICAL OPERATORS */
        DEFINE_INPLACE_OP(+=);
        DEFINE_INPLACE_OP(-=);
        DEFINE_INPLACE_OP(*=);
        DEFINE_INPLACE_OP(/=);

        DEFINE_OP(+);
        DEFINE_OP(-);
        DEFINE_OP(*);
        DEFINE_OP(/);

        Matrix2D& operator+=(T val) {
            MAT_LOOP(m_matrix[i][j] += val);
            return *this;
        }
        Matrix2D& operator-=(T val) {
            MAT_LOOP(m_matrix[i][j] -= val);
            return *this;
        }
        Matrix2D& operator*=(T val) {
            MAT_LOOP(m_matrix[i][j] *= val);
            return *this;
        }
        Matrix2D& operator/=(T val) {
            MAT_LOOP(m_matrix[i][j] /= val);
            return *this;
        }

        Matrix2D operator+(T val) const {
            auto copy = *this;
            MAT_LOOP(copy.m_matrix[i][j] += val);
            return copy;
        }
        Matrix2D operator-(T val) const {
            auto copy = *this;
            MAT_LOOP(copy.m_matrix[i][j] -= val);
            return copy;
        }
        Matrix2D operator*(T val) const {
            auto copy = *this;
            MAT_LOOP(copy.m_matrix[i][j] *= val);
            return copy;
        }
        Matrix2D operator/(T val) const {
            auto copy = *this;
            MAT_LOOP(copy.m_matrix[i][j] /= val);
            return copy;
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

        /* MATHEMATICAL OPERATIONS */
        constexpr Pair<size_t, size_t> size() const { return {N, M}; }
        constexpr size_t num_rows() const { return N; }
        constexpr size_t num_columns() const { return M; }
        void reduce() {
            row_echelon_transform(m_matrix);
            // constexpr T zero_val{0};
            // constexpr auto min_size = N < M ? N : M;
            // // matrix is reduced but not triangular yet
            // for (size_t i = 0; i < N; i++) {
            //     for (size_t j = 0; j < min_size; j++) {
            //         if (m_matrix[i][j] != zero_val && j != i) {
            //             swap_row(m_matrix, i, j);
            //             break;
            //         }
            //     }
            // }
        }
        int rank() const { return rank_internal(m_matrix); }
        double det() const requires Square {
            if (m_cached_det) return m_cached_det.value();

            // i'll have hardcoded math for 2x2 and 3x3
            if constexpr (N == 2) {
                m_cached_det = m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0];
            } else if constexpr (N == 3) {
                // ignore first row first column
                double first_partial = m_matrix[1][1] * m_matrix[2][2] - m_matrix[1][2] * m_matrix[2][1];
                // ignore first row second column
                double second_partial = m_matrix[1][0] * m_matrix[2][2] - m_matrix[1][2] * m_matrix[2][0];
                // ignore first row third column
                double third_partial = m_matrix[1][0] * m_matrix[2][1] - m_matrix[1][1] * m_matrix[2][0];

                m_cached_det =
                (first_partial * m_matrix[0][0]) - (second_partial * m_matrix[0][1]) + (third_partial * m_matrix[0][2]);
            } else {
                // generic math for 4x4 and up
                m_cached_det = det_internal(m_matrix);
            }
            return m_cached_det.value();
        }

        T sum() const {
            T res = T{0};
            MAT_LOOP(res += m_matrix[i][j]);
            return res;
        }

        T avg() const { return sum() / T{N * M}; }

        template <size_t NewSizeRows, size_t NewSizeCols = NewSizeRows>
        auto sub(size_t start_row = 0, size_t start_col = 0) const {
            HARD_ASSERT(start_row + NewSizeRows <= N, "Submatrix is impossible to construct");
            HARD_ASSERT(start_col + NewSizeCols <= M, "Submatrix is impossible to construct");
            Matrix2D<T, NewSizeRows, NewSizeCols> submat{};
            for (size_t i = start_row; i < start_row + NewSizeRows; i++) {
                for (size_t j = start_col; j < start_col + NewSizeCols; j++) {
                    submat[{i - start_row, j - start_col}] = m_matrix[i][j];
                }
            }
            return submat;
        }

        Matrix2D inv() requires Square {
            Matrix2D<T, N, M * 2> mat_glued{};
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    mat_glued[{i, j}] = m_matrix[i][j];
                }
            }
            for (size_t j = M; j < M * 2; j++) {
                mat_glued[{j - M, j}] = T{1};
            }
            Printer::print("{}", mat_glued);
            mat_glued.reduce();
            Printer::print("{}", mat_glued);
            return *this;
        }

        ~Matrix2D() {
            if constexpr (!ValidToInline) { deallocate_memory(); }
        }

#undef MAT_LOOP
#undef DEFINE_INPLACE_OP
#undef DEFINE_OP
    };

    template <Numeric T, size_t N, size_t M>
    struct PrintInfo<Matrix2D<T, N, M>> {
        const Matrix2D<T, N, M>& m_matrix;
        PrintInfo(const Matrix2D<T, N, M>& matrix) : m_matrix(matrix) {}
        String repr() const {
            String ret{};
            for (size_t i = 0; i < N; i++) {
                ret += "[ "_s;
                for (size_t j = 0; j < M; j++) {
                    ret += PrintInfo<T>{m_matrix[{i, j}]}.repr();
                    if (j != M - 1) ret += ", "_s;
                }
                ret += " ]"_s;
                if (i != N - 1) ret += '\n';
            }
            return ret;
        }
    };

} // namespace ARLib