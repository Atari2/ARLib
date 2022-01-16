#pragma once
#include "Algorithm.h"
#include "Concepts.h"
#include "Iterator.h"
#include "Memory.h"
#include "Optional.h"
#include "Pair.h"
#include "PrintInfo.h"
#include "Random.h"

namespace ARLib {
    // clang-format off
    template <Numeric T, size_t N, size_t M>
    class FixedMatrix2D;
    template <Numeric T, size_t N, size_t M, bool IsConst, bool ByRow>
    class FixedMatrixIterator;

    template <Numeric T, size_t N, size_t M>
    struct FixedMatrixTraits {
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
    class FixedColumnIterator {
        using traits = FixedMatrixTraits<T, N, M>;
        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;

        Tp* m_col;
        size_t m_current_index;

        public:
        FixedColumnIterator(Tp* col, size_t current_index) : m_col(col), m_current_index(current_index) {}
        auto& operator*() { return *m_col[m_current_index]; }
        const auto& operator*() const { return *m_col[m_current_index]; }

        FixedColumnIterator& operator++() {
            m_current_index++;
            return *this;
        }
        FixedColumnIterator operator++(int) {
            auto copy = *this;
            m_current_index++;
            return copy;
        }

        FixedColumnIterator& operator--() {
            m_current_index--;
            return *this;
        }
        FixedColumnIterator operator--(int) {
            auto copy = *this;
            m_current_index--;
            return copy;
        }

        bool operator==(const FixedColumnIterator& other) const {
            return m_col == other.m_col && m_current_index == other.m_current_index;
        }
        bool operator!=(const FixedColumnIterator& other) const {
            return m_col != other.m_col || m_current_index != other.m_current_index;
        }
    };

    template <Numeric T, size_t N, size_t M = N, bool IsConst = false>
    class FixedColumnArray {
        friend FixedMatrix2D<T, N, M>;
        friend FixedMatrixIterator<T, N, M, IsConst, false>;

        using traits = FixedMatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;
        using InputType = typename traits::MatRefType;
        using ConstInputType = typename traits::ConstMatRefType;

        Tp* m_column = nullptr;

        FixedColumnArray() = default;

        FixedColumnArray(InputType matrix, size_t idx) requires(!IsConst) {
            m_column = allocate<Tp>(N);
            for (size_t i = 0; i < N; i++) {
                m_column[i] = &matrix[i][idx];
            };
        }
        FixedColumnArray(ConstInputType matrix, size_t idx) requires IsConst {
            m_column = allocate<Tp>(N);
            for (size_t i = 0; i < N; i++) {
                m_column[i] = &matrix[i][idx];
            };
        }
        public: 

        FixedColumnArray& operator=(FixedColumnArray&& other) {
            deallocate<Tp, DeallocType::Multiple>(m_column);
            m_column = other.m_column;
            other.m_column = nullptr;
            return *this;
        }
        T& operator[](size_t index) requires(!IsConst) { return *m_column[index]; }
        const T& operator[](size_t index) const { return *m_column[index]; }

        auto begin() const { return FixedColumnIterator<T, N, M, true>{m_column, 0}; }
        auto end() const { return FixedColumnIterator<T, N, M, true>{m_column, N}; }

        auto begin() { return FixedColumnIterator<T, N, M, false>{m_column, 0}; }
        auto end() { return FixedColumnIterator<T, N, M, false>{m_column, N}; }

        ~FixedColumnArray() {
            deallocate<Tp, DeallocType::Multiple>(m_column);
        }
    };

    template <Numeric T, size_t N, size_t M, bool IsConst>
    class FixedRowIterator {
        using traits = FixedMatrixTraits<T, N, M>;
        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;

        Tp m_row;
        size_t m_current_index;

        public:
        FixedRowIterator(Tp row, size_t current_index) : m_row(row), m_current_index(current_index) {}
        auto& operator*() { return m_row[m_current_index]; }
        const auto& operator*() const { return m_row[m_current_index]; }

        FixedRowIterator& operator++() {
            m_current_index++;
            return *this;
        }
        FixedRowIterator operator++(int) {
            auto copy = *this;
            m_current_index++;
            return copy;
        }

        FixedRowIterator& operator--() {
            m_current_index--;
            return *this;
        }
        FixedRowIterator operator--(int) {
            auto copy = *this;
            m_current_index--;
            return copy;
        }

        bool operator==(const FixedRowIterator& other) const {
            return m_row == other.m_row && m_current_index == other.m_current_index;
        }
        bool operator!=(const FixedRowIterator& other) const {
            return m_row != other.m_row || m_current_index != other.m_current_index;
        }
    };

    template <Numeric T, size_t N, size_t M = N, bool IsConst = false>
    class FixedRowArray {
        friend FixedMatrix2D<T, N, M>;
        friend FixedMatrixIterator<T, N, M, IsConst, true>;

        using traits = FixedMatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, AddConstT<T>*, T*>;
        using InputType = typename traits::MatRefType;
        using ConstInputType = typename traits::ConstMatRefType;

        Tp m_row;

        FixedRowArray(InputType matrix, size_t idx) requires(!IsConst) { m_row = matrix[idx]; }
        FixedRowArray(ConstInputType matrix, size_t idx) requires IsConst { m_row = matrix[idx]; }

        public: 
        T& operator[](size_t index) requires(!IsConst) { return m_row[index]; }
        const T& operator[](size_t index) const { return m_row[index]; }
        
        auto begin() const { return FixedRowIterator<T, N, M, true>{m_row, 0}; }
        auto end() const { return FixedRowIterator<T, N, M, true>{m_row, M}; }

        auto begin() { return FixedRowIterator<T, N, M, false>{m_row, 0}; }
        auto end() { return FixedRowIterator<T, N, M, false>{m_row, M}; }
    };

    // clang-format on
    template <Numeric T, size_t N, size_t M, bool IsConst, bool ByRow = true>
    class FixedMatrixIterator {
        using traits = FixedMatrixTraits<T, N, M>;

        using Tp = ConditionalT<IsConst, typename traits::ConstMatRefType, typename traits::MatRefType>;
        using Ret = ConditionalT<ByRow, FixedRowArray<T, N, M, IsConst>, FixedColumnArray<T, N, M, IsConst>>;

        Tp m_matrix;
        size_t m_current_row;
        Ret m_current;

        public:
        FixedMatrixIterator(Tp matrix, size_t current_row) :
            m_matrix(matrix), m_current_row(current_row), m_current(matrix, current_row) {}

        Ret& operator*() { return m_current; }
        const Ret& operator*() const { return m_current; }

        FixedMatrixIterator& operator++() {
            m_current_row++;
            m_current = Ret{m_matrix, m_current_row};
            return *this;
        }
        FixedMatrixIterator operator++(int) {
            auto copy = *this;
            m_current_row++;
            m_current = Ret{m_matrix, m_current_row};
            return copy;
        }

        FixedMatrixIterator& operator--() {
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row};
            return *this;
        }
        FixedMatrixIterator operator--(int) {
            auto copy = *this;
            m_current_row--;
            m_current = Ret{m_matrix, m_current_row};
            return copy;
        }

        bool operator==(const FixedMatrixIterator& other) const {
            if constexpr (traits::ValidToInline) {
                return addressof(m_matrix) == addressof(other.m_matrix) && m_current_row == other.m_current_row;
            } else {
                return m_matrix == other.m_matrix && m_current_row == other.m_current_row;
            }
        }
        bool operator!=(const FixedMatrixIterator& other) const {
            if constexpr (traits::ValidToInline) {
                return addressof(m_matrix) != addressof(other.m_matrix) || m_current_row != other.m_current_row;
            } else {
                return m_matrix != other.m_matrix || m_current_row != other.m_current_row;
            }
        }
    };

    template <Numeric T, size_t N, size_t M = N>
    class FixedMatrix2D {
        using traits = FixedMatrixTraits<T, N, M>;

        static constexpr bool ValidToInline = traits::ValidToInline;
        static constexpr bool Square = traits::Square;

        using MatType = typename traits::MatType;
        using SubOpRetType = typename traits::SubOpRetType;
        using ConstSubOpRetType = typename traits::ConstSubOpRetType;

        mutable Optional<double> m_cached_det{};
        mutable Optional<int> m_cached_rank{};
        MatType m_matrix{};

        static void print_debug_matrix(typename traits::ConstMatRefType og_matrix) {
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
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

        static int rank_internal(typename traits::ConstMatRefType og_matrix) {
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
        static FixedMatrix2D random(T top = NumberTraits<T>::max) {
            FixedMatrix2D mat{};
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    mat[{i, j}] = Random::PCG::bounded_random_s(top);
                }
            }
            return mat;
        }
        static FixedMatrix2D eye() requires Square {
            FixedMatrix2D mat{};
            for (size_t i = 0; i < N; i++) {
                mat[{i, i}] = T{1};
            }
            return mat;
        }
        FixedMatrix2D() requires ValidToInline = default;
        FixedMatrix2D() requires(!ValidToInline) {
            allocate_memory(true);
        }
        // clang-format on

        template <typename... Args>
        FixedMatrix2D(Args... args) requires(AllOfV<T, Args...> && sizeof...(Args) == (N * M)) {
            T mm[N * M]{args...};
            if constexpr (!ValidToInline) m_matrix = allocate<T*>(N);
            for (size_t i = 0; i < N; i++) {
                if constexpr (!ValidToInline) m_matrix[i] = allocate<T>(M);
                for (size_t j = 0; j < M; j++) {
                    m_matrix[i][j] = mm[i * M + j];
                }
            }
        }

        FixedMatrix2D(const T (&matrix)[N][M]) requires ValidToInline {
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], matrix[i], M);
            }
        }

        /* COPY AND MOVE CTORS AND ASSIGNMENT OPS*/

        FixedMatrix2D(const FixedMatrix2D& other) {
            if constexpr (!ValidToInline) { allocate_memory(); }
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
            }
        }
        FixedMatrix2D(FixedMatrix2D&& other) noexcept {
            if constexpr (ValidToInline) {
                for (size_t i = 0; i < N; i++) {
                    ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
                }
            } else {
                m_matrix = other.m_matrix;
                other.m_matrix = nullptr;
            }
        }

        FixedMatrix2D& operator=(const FixedMatrix2D& other) {
            for (size_t i = 0; i < N; i++) {
                ConditionalBitCopy(m_matrix[i], other.m_matrix[i], M);
            }
        }

        FixedMatrix2D& operator=(FixedMatrix2D&& other) noexcept {
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
            return FixedMatrixIterator<T, N, M, true, ByRow>{m_matrix, 0};
        }

        template <bool ByRow = true>
        auto end() const {
            return FixedMatrixIterator<T, N, M, true, ByRow>{m_matrix, N};
        }

        template <bool ByRow = true>
        auto begin() {
            return FixedMatrixIterator<T, N, M, false, ByRow>{m_matrix, 0};
        }

        template <bool ByRow = true>
        auto end() {
            return FixedMatrixIterator<T, N, M, false, ByRow>{m_matrix, N};
        }

        auto columns_begin(size_t begin_idx = 0) const {
            return FixedMatrixIterator<T, N, M, true, false>{m_matrix, begin_idx};
        }
        auto columns_end() const { return FixedMatrixIterator<T, N, M, true, false>{m_matrix, N}; }

        auto columns_begin(size_t begin_idx = 0) {
            return FixedMatrixIterator<T, N, M, false, false>{m_matrix, begin_idx};
        }
        auto columns_end() { return FixedMatrixIterator<T, N, M, false, false>{m_matrix, N}; }

        auto rows_begin(size_t begin_idx = 0) const {
            return FixedMatrixIterator<T, N, M, true, true>{m_matrix, begin_idx};
        }
        auto rows_end() const { return FixedMatrixIterator<T, N, M, true, true>{m_matrix, N}; }

        auto rows_begin(size_t begin_idx = 0) { return FixedMatrixIterator<T, N, M, false, true>{m_matrix, begin_idx}; }
        auto rows_end() { return FixedMatrixIterator<T, N, M, false, true>{m_matrix, N}; }

        /* INDEX OPERATORS */
        T& operator[](Pair<size_t, size_t> idx) { return m_matrix[idx.first()][idx.second()]; }
        const T& operator[](Pair<size_t, size_t> idx) const { return m_matrix[idx.first()][idx.second()]; }
        auto columns(size_t index) const { return FixedColumnArray<T, N, M, true>{m_matrix, index}; }
        auto columns(size_t index) { return FixedColumnArray<T, N, M, false>{m_matrix, index}; }
        auto rows(size_t index) const { return FixedRowArray<T, N, M, true>{m_matrix, index}; }
        auto rows(size_t index) { return FixedRowArray<T, N, M, false>{m_matrix, index}; }
        auto operator[](size_t index) const { return FixedRowArray<T, N, M, true>{m_matrix, index}; }
        auto operator[](size_t index) { return FixedRowArray<T, N, M, false>{m_matrix, index}; }

#define MAT_LOOP(op)                                                                                                   \
    for (size_t i = 0; i < N; i++) {                                                                                   \
        for (size_t j = 0; j < M; j++) {                                                                               \
            op;                                                                                                        \
        }                                                                                                              \
    }

#define DEFINE_INPLACE_OP(op)                                                                                          \
    FixedMatrix2D& operator op(T val) {                                                                                \
        MAT_LOOP(m_matrix[i][j] op val);                                                                               \
        return *this;                                                                                                  \
    }

#define DEFINE_OP(op, in_op)                                                                                           \
    FixedMatrix2D operator op(T val) const {                                                                           \
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

        FixedMatrix2D& operator++() {
            MAT_LOOP(m_matrix[i][j] += 1);
            return *this;
        }
        FixedMatrix2D operator++(int) {
            auto copy = *this;
            MAT_LOOP(m_matrix[i][j] += 1);
            return copy;
        }
        FixedMatrix2D& operator--() {
            MAT_LOOP(m_matrix[i][j] -= 1);
            return *this;
        }
        FixedMatrix2D operator--(int) {
            auto copy = *this;
            MAT_LOOP(m_matrix[i][j] -= 1);
            return copy;
        }

        /* MATHEMATICAL OPERATIONS */
        constexpr Pair<size_t, size_t> size() const { return {N, M}; }
        constexpr size_t num_rows() const { return N; }
        constexpr size_t num_columns() const { return M; }
        void reduce() { row_echelon_transform(m_matrix); }
        int rank() const {
            if (m_cached_rank) return m_cached_rank.value();
            m_cached_rank = rank_internal(m_matrix);
            return m_cached_rank.value();
        }
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
            FixedMatrix2D<T, NewSizeRows, NewSizeCols> submat{};
            for (size_t i = start_row; i < start_row + NewSizeRows; i++) {
                for (size_t j = start_col; j < start_col + NewSizeCols; j++) {
                    submat[{i - start_row, j - start_col}] = m_matrix[i][j];
                }
            }
            return submat;
        }

        FixedMatrix2D inv() const requires Square {
            FixedMatrix2D<T, N, M * 2> mat_glued{};
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    mat_glued[{i, j}] = m_matrix[i][j];
                }
            }
            for (size_t j = M; j < M * 2; j++) {
                mat_glued[{j - M, j}] = T{1};
            }
            mat_glued.reduce();
            return mat_glued.template sub<N, N>(0, N);
        }

        FixedMatrix2D<T, M, N> transpose() const {
            FixedMatrix2D<T, M, N> mat{};
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < M; j++) {
                    mat[{j, i}] = m_matrix[i][j];
                }
            }
            return mat;
        }

        ~FixedMatrix2D() {
            if constexpr (!ValidToInline) { deallocate_memory(); }
        }

#undef MAT_LOOP
#undef DEFINE_INPLACE_OP
#undef DEFINE_OP
    };

    // OPERATORS
    template <Numeric T, size_t N1, size_t N2, size_t M>
    FixedMatrix2D<T, N1, N2> operator*(const FixedMatrix2D<T, N1, M>& first, const FixedMatrix2D<T, M, N2>& second) {
        FixedMatrix2D<T, N1, N2> mat{};
        for (size_t col = 0; col < N2; col++) {
            for (size_t row = 0; row < N1; row++) {
                T intermediate{0};
                for (size_t i = 0; i < M; i++) {
                    intermediate += first[{row, i}] * second[{i, col}];
                }
                mat[{row, col}] = intermediate;
            }
        }
        return mat;
    }

    template <Numeric T, size_t N1, size_t N2, size_t M, bool IsConst>
    FixedMatrix2D<T, N1, 1> operator*(const FixedMatrix2D<T, N1, M>& first,
                                      const FixedColumnArray<T, M, N2, IsConst>& column) {
        FixedMatrix2D<T, N1, 1> mat{};
        for (size_t row = 0; row < N1; row++) {
            T intermediate{0};
            for (size_t i = 0; i < M; i++) {
                intermediate += first[{row, i}] * column[i];
            }
            mat[{row, 0}] = intermediate;
        }
        return mat;
    }

    template <Numeric T, size_t N1, size_t N2, size_t M, bool IsConst>
    FixedMatrix2D<T, 1, N2> operator*(const FixedRowArray<T, N1, M, IsConst>& row,
                                      const FixedMatrix2D<T, M, N2>& second) {
        FixedMatrix2D<T, 1, N2> mat{};
        for (size_t col = 0; col < N2; col++) {
            T intermediate{0};
            for (size_t i = 0; i < M; i++) {
                intermediate += row[i] * second[{i, col}];
            }
            mat[{0, col}] = intermediate;
        }
        return mat;
    }

    template <Numeric T, size_t N1, size_t N2>
    auto operator/(const FixedMatrix2D<T, N1, N2>& first, const FixedMatrix2D<T, N2, N2>& second) {
        HARD_ASSERT(second.det() != 0, "Determinant of second matrix must not be 0 to be able to divide");
        return first * second.inv();
    }

    template <Numeric T, size_t N, size_t M>
    FixedMatrix2D<T, N, M> operator+(const FixedMatrix2D<T, N, M>& first, const FixedMatrix2D<T, N, M>& second) {
        FixedMatrix2D<T, N, M> mat{};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] = first[{i, j}] + second[{i, j}];
            }
        }
        return mat;
    }

    template <Numeric T, size_t N, size_t M>
    FixedMatrix2D<T, N, M> operator-(const FixedMatrix2D<T, N, M>& first, const FixedMatrix2D<T, N, M>& second) {
        FixedMatrix2D<T, N, M> mat{};
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                mat[{i, j}] = first[{i, j}] - second[{i, j}];
            }
        }
        return mat;
    }

    template <Numeric T, size_t N, size_t M>
    struct PrintInfo<FixedMatrix2D<T, N, M>> {
        const FixedMatrix2D<T, N, M>& m_matrix;
        PrintInfo(const FixedMatrix2D<T, N, M>& matrix) : m_matrix(matrix) {}
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