#pragma once
#include "Algorithm.h"
#include "Allocator.h"
#include "Assertion.h"
#include "Iterator.h"
#include "Types.h"
#include "cstring_compat.h"

namespace ARLib {
    class StringView;
    class Ordering;
    template <typename T>
    class Vector;

    class String {
        static constexpr size_t SMALL_STRING_CAP = 15;
        union {
            char m_local_buf[SMALL_STRING_CAP + 1] = {0};
            size_t m_allocated_capacity;
        };
        char* m_data_buf = PointerTraits<char*>::pointer_to(*m_local_buf);
        size_t m_size = 0;

        constexpr char* data_internal() { return m_data_buf; }

        constexpr const char* data_internal() const { return m_data_buf; }

        constexpr char* local_data_internal() { return PointerTraits<char*>::pointer_to(*m_local_buf); }

        constexpr const char* local_data_internal() const {
            return PointerTraits<const char*>::pointer_to(*m_local_buf);
        }

        constexpr bool is_local() const { return data_internal() == local_data_internal(); }

        constexpr const char* get_buf_internal() const { return data_internal(); }

        constexpr char* get_buf_internal() { return data_internal(); }

        void grow_internal(size_t requested_capacity) {
            if (is_local()) {
                // grow outside of locality, copy buffer and change active member of union
                requested_capacity = basic_growth(requested_capacity);
                m_data_buf = allocate<char>(requested_capacity);
                memmove(m_data_buf, m_local_buf, SMALL_STRING_CAP + 1);
                m_allocated_capacity = requested_capacity;
            } else {
                m_allocated_capacity = basic_growth(requested_capacity);
                HARD_ASSERT(m_allocated_capacity >= requested_capacity && m_allocated_capacity > m_size,
                            "Allocated capacity failure")
                char* new_buf = allocate<char>(m_allocated_capacity);
                new_buf[m_size] = '\0';
                if (m_size != 0) memmove(new_buf, m_data_buf, m_size + 1);
                deallocate<char, DeallocType::Multiple>(m_data_buf);
                m_data_buf = new_buf;
            }
        }

        constexpr void grow_if_needed(size_t newsize) {
            if (is_local()) {
                if (newsize > SMALL_STRING_CAP) grow_internal(newsize + 1);
            } else {
                if (newsize > m_allocated_capacity - 1 || m_allocated_capacity == 0) { grow_internal(newsize + 1); }
            }
        }

        Vector<size_t> all_indexes_internal(StringView any, size_t start_index = 0ull) const;
        Vector<size_t> all_last_indexes_internal(StringView any, size_t end_index = npos) const;
        Vector<size_t> all_not_indexes_internal(StringView any, size_t start_index = 0ull) const;
        Vector<size_t> all_last_not_indexes_internal(StringView any, size_t end_index = npos) const;

        public:
        static constexpr auto npos = static_cast<size_t>(-1);

        // constructors, destructor equality operators
        constexpr String() = default;

        template <size_t N>
        explicit constexpr String(const char (&src)[N]) : m_size(N - 1) {
            grow_if_needed(N);
            strncpy(m_data_buf, src, N);
        }

        explicit String(size_t size, char c) {
            if (size > SMALL_STRING_CAP) grow_if_needed(size);
            memset(get_buf_internal(), static_cast<uint8_t>(c), size);
            m_size = size;
            get_buf_internal()[m_size] = '\0';
        }
        explicit constexpr String(const char* begin, const char* end) {
            HARD_ASSERT_FMT((end >= begin), "End pointer (%p) must not be before begin pointer (%p)", end, begin)
            m_size = static_cast<size_t>(end - begin);
            grow_if_needed(m_size);
            strncpy(m_data_buf, begin, m_size);
            m_data_buf[m_size] = '\0';
        }
        constexpr String(const char* other, size_t size) : m_size(size) {
            grow_if_needed(size);
            strncpy(m_data_buf, other, size);
            m_data_buf[m_size] = '\0';
        }
        template <typename T, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
        explicit constexpr String(T other) : m_size(strlen(other)) {
            grow_if_needed(m_size);
            strncpy(m_data_buf, other, m_size);
            m_data_buf[m_size] = '\0';
        }
        String(const String& other) noexcept : m_size(other.m_size) {
            grow_if_needed(m_size);
            strncpy(m_data_buf, other.m_data_buf, m_size + 1);
            m_data_buf[m_size] = '\0';
        }
        String(String&& other) noexcept : m_size(other.m_size) {
            if (other.is_local()) {
                memcpy(m_local_buf, other.m_local_buf, SMALL_STRING_CAP + 1);
            } else {
                m_allocated_capacity = other.m_allocated_capacity;
                m_size = other.m_size;
                m_data_buf = other.m_data_buf;
                other.m_allocated_capacity = SMALL_STRING_CAP;
            }
            other.m_data_buf = PointerTraits<char*>::pointer_to(*other.m_local_buf);
            other.m_size = 0;
        }

        explicit String(StringView other);

        String& operator=(const String& other) {
            if (this != &other) {
                if (!is_local()) deallocate<char, DeallocType::Multiple>(m_data_buf);
                m_size = other.m_size;
                grow_if_needed(m_size);
                memcpy(m_data_buf, other.m_data_buf, m_size + 1);
            }
            return *this;
        }
        String& operator=(String&& other) noexcept {
            if (this != &other) {
                if (!is_local()) deallocate<char, DeallocType::Multiple>(m_data_buf);
                m_size = other.m_size;
                if (other.is_local()) {
                    memcpy(m_local_buf, other.m_local_buf, SMALL_STRING_CAP + 1);
                    m_data_buf = PointerTraits<char*>::pointer_to(*m_local_buf);
                } else {
                    m_data_buf = other.m_data_buf;
                    m_allocated_capacity = other.m_allocated_capacity;
                    other.m_data_buf = PointerTraits<char*>::pointer_to(*other.m_local_buf);
                    other.m_size = 0;
                    other.m_allocated_capacity = SMALL_STRING_CAP;
                }
            }
            return *this;
        }

        template <typename... Args>
        static String formatted(const char* format, Args... args) {
            String str{};
            auto len = static_cast<size_t>(scprintf(format, args...));
            str.reserve(len);
            str.set_size(static_cast<size_t>(sprintf(str.rawptr(), format, args...)));
            return str;
        }

        ~String() {
            m_size = 0;
            if (!is_local()) {
                deallocate<char, DeallocType::Multiple>(m_data_buf);
                m_data_buf = nullptr;
            }
        }

        // releases the inner char* buffer. May allocate if buffer is in-situ. May return nullptr if the string is
        // empty.
        char* release() {
            if (m_size == 0) return nullptr;
            if (is_local()) {
                char* buffer = allocate<char>(SMALL_STRING_CAP + 1);
                strcpy(buffer, local_data_internal());
                return buffer;
            } else {
                char* buffer = m_data_buf;
                m_data_buf = local_data_internal();
                m_size = 0;
                return buffer;
            }
        }

        [[nodiscard]] String substring(size_t first = 0, size_t last = npos) const {
            if (last == npos) last = length();
            return String{get_buf_internal() + first, get_buf_internal() + last};
        }
        [[nodiscard]] StringView view();
        [[nodiscard]] StringView view() const;
        [[nodiscard]] StringView substringview(size_t first = 0, size_t last = npos) const;

        // comparison operators
        [[nodiscard]] bool operator==(const String& other) const {
            if (other.m_size == m_size) { return strncmp(m_data_buf, other.m_data_buf, m_size) == 0; }
            return false;
        }
        template <typename T, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
        [[nodiscard]] bool operator==(T other) const {
            return strcmp(get_buf_internal(), other) == 0;
        }
        template <typename T, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
        [[nodiscard]] bool operator!=(T other) const {
            return strcmp(get_buf_internal(), other) != 0;
        }

        template <size_t N>
        [[nodiscard]] bool operator==(const char (&other)[N]) const {
            if (N - 1 != m_size) return false;
            return strncmp(get_buf_internal(), other, N - 1) == 0;
        }
        template <size_t N>
        [[nodiscard]] bool operator!=(const char (&other)[N]) const {
            if (N - 1 != m_size) return true;
            return strncmp(get_buf_internal(), other, N - 1) != 0;
        }
        [[nodiscard]] bool operator==(const StringView& other) const;
        [[nodiscard]] bool operator!=(const String& other) const { return !(*this == other); }
        [[nodiscard]] bool operator!=(const StringView& other) const;
        [[nodiscard]] bool operator<(const String& other) const {
            return strncmp(get_buf_internal(), other.get_buf_internal(), other.m_size) < 0;
        }
        [[nodiscard]] bool operator<(const StringView& other) const;
        [[nodiscard]] bool operator>(const String& other) const { return !(*this < other) && !(*this == other); }
        [[nodiscard]] bool operator<=(const String& other) const { return (*this < other || *this == other); }
        [[nodiscard]] bool operator>=(const String& other) const { return (*this > other || *this == other); }
        [[nodiscard]] Ordering operator<=>(const String& other) const;
        [[nodiscard]] Ordering operator<=>(const StringView& other) const;

        void set_size(size_t size) {
            m_size = size;
            get_buf_internal()[m_size] = '\0';
        }

        void resize(size_t size) {
            reserve(size);
            set_size(size);
        }

        [[nodiscard]] size_t size() const { return m_size; }
        [[nodiscard]] size_t length() const { return m_size; }
        [[nodiscard]] size_t capacity() const { return is_local() ? SMALL_STRING_CAP : m_allocated_capacity; }
        [[nodiscard]] const char* data() const { return get_buf_internal(); }
        [[nodiscard]] char* rawptr() { return get_buf_internal(); }
        [[nodiscard]] bool is_empty() const { return m_size == 0; }

        void clear() { set_size(0); }

        // starts/ends with
        [[nodiscard]] bool starts_with(const String& other) const {
            if (other.m_size > m_size) return false;
            if (other.m_size == m_size) return other == *this;
            auto res = strncmp(other.get_buf_internal(), get_buf_internal(), other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool starts_with(StringView) const;
        [[nodiscard]] bool ends_with(const String& other) const {
            if (other.m_size > m_size) return false;
            if (other.m_size == m_size) return other == *this;
            auto ptrdiff = m_size - other.m_size;
            const char* buf = other.get_buf_internal();
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, buf, other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(StringView other) const;

        // concatenation
        void append(char c) {
            auto new_size = m_size + 1;
            grow_if_needed(new_size);
            get_buf_internal()[m_size] = c;
            set_size(m_size + 1);
        }
        void append(const String& other) {
            auto new_size = m_size + other.m_size;
            grow_if_needed(new_size);
            strcat_eff(get_buf_internal() + m_size, other.get_buf_internal());
            set_size(m_size + other.m_size);
        }
        void append(StringView other);
        void append(const char* other);

        [[nodiscard]] String concat(char c) const {
            String copy{*this};
            auto new_size = m_size + 1;
            copy.grow_if_needed(new_size);
            copy.get_buf_internal()[m_size] = c;
            copy.set_size(m_size + 1);
            return copy;
        }

        [[nodiscard]] String concat(const String& other) const {
            String copy{*this};
            auto new_size = m_size + other.m_size;
            copy.grow_if_needed(new_size);
            strcat_eff(copy.get_buf_internal() + m_size, other.get_buf_internal());
            copy.set_size(new_size);
            return copy;
        }
        [[nodiscard]] String concat(StringView other) const;
        [[nodiscard]] String concat(const char* other) const;

        String operator+(const String& other) const { return concat(other); }
        String operator+(const char* other) const { return concat(other); }
        String operator+(char c) const { return concat(c); }

        String& operator+=(const String& other) {
            append(other);
            return *this;
        }
        String& operator+=(const char* other) {
            append(other);
            return *this;
        }
        String& operator+=(char c) {
            append(c);
            return *this;
        }

        // reverse
        String reversed() {
            String reversed{};
            reversed.reserve(size());
            for (auto rc = this->rbegin(); rc != this->rend(); rc--)
                reversed.append(*rc);
            return reversed;
        }

        // iterator support
        [[nodiscard]] Iterator<char> begin() { return Iterator<char>{get_buf_internal()}; }
        [[nodiscard]] ConstIterator<char> begin() const { return ConstIterator<char>{get_buf_internal()}; }
        [[nodiscard]] Iterator<char> rbegin() { return end() - 1; }
        [[nodiscard]] Iterator<char> end() { return Iterator<char>{get_buf_internal() + m_size}; }
        [[nodiscard]] ConstIterator<char> end() const { return ConstIterator<char>{get_buf_internal() + m_size}; }
        [[nodiscard]] Iterator<char> rend() { return begin() - 1; }
        [[nodiscard]] char front() const { return get_buf_internal()[0]; }
        [[nodiscard]] char back() const { return get_buf_internal()[m_size - 1]; }

        // indexing access
        [[nodiscard]] char at(size_t index) const {
            SOFT_ASSERT_FMT((index >= m_size), "Index of %llu was out of bounds of String with size %llu", index,
                            m_size)
            return get_buf_internal()[index];
        }
        [[nodiscard]] char& operator[](size_t index) { return get_buf_internal()[index]; }
        [[nodiscard]] char operator[](size_t index) const { return get_buf_internal()[index]; }

        // various char checks

        // single char [last_]index[_not]_of functions
        [[nodiscard]] size_t index_of(char c, size_t start_index = 0) const {
            if (m_size == 0) return npos;
            if (start_index >= m_size) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = start_index; i < m_size; i++) {
                if (buf[i] == c) return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(char c, size_t end_index = npos) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = (end_index > m_size - 1) ? m_size - 1 : end_index;; i--) {
                if (buf[i] == c) return i;
                if (i == 0) break;
            }
            if (buf[0] == c) return 0ull;
            return npos;
        }
        [[nodiscard]] size_t index_not_of(char c, size_t start_index = 0) const {
            if (m_size == 0) return npos;
            if (start_index >= m_size) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = start_index; i < m_size; i++) {
                if (buf[i] != c) return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(char c, size_t end_index = npos) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = (end_index > m_size - 1) ? m_size - 1 : end_index;; i--) {
                if (buf[i] != c) return i;
                if (i == 0) break;
            }
            return npos;
        }

        // span [last_]index[_not]_of functions
        [[nodiscard]] size_t index_of(StringView c, size_t start = 0) const;
        [[nodiscard]] size_t last_index_of(StringView c, size_t end = npos) const;
        [[nodiscard]] size_t index_not_of(StringView c, size_t start = 0) const;
        [[nodiscard]] size_t last_index_not_of(StringView c, size_t end = npos) const;

        // any char in span
        [[nodiscard]] size_t index_of_any(StringView any, size_t start_index = 0ull) const;
        [[nodiscard]] size_t last_index_of_any(StringView any, size_t end_index = npos) const;
        [[nodiscard]] size_t index_not_of_any(StringView any, size_t start_index = 0ull) const;
        [[nodiscard]] size_t last_index_not_of_any(StringView any, size_t end_index = npos) const;

        [[nodiscard]] bool contains(StringView other) const;
        [[nodiscard]] bool contains(char c) const { return index_of(c) != npos; }

        // trim
        void irtrim() {
            if (m_size == 0) return;
            if (!isspace(m_data_buf[0])) return;
            if (is_local()) {
                size_t count = 0;
                while (isspace(m_local_buf[count++]))
                    ;
                count--;
                if (count != 0) {
                    m_size -= count;
                    memmove(m_local_buf, m_local_buf + count, m_size);
                }
            } else {
                size_t count = 0;
                while (isspace(m_data_buf[count++]))
                    ;
                count--;
                if (count != 0) {
                    m_size -= count;
                    // if the m_size is now small enough, let's swap to small String
                    if (m_size <= SMALL_STRING_CAP) {
                        memmove(m_local_buf, m_data_buf + count, m_size);
                        deallocate<char, DeallocType::Multiple>(m_data_buf);
                        m_data_buf = local_data_internal();
                    } else {
                        memmove(m_data_buf, m_data_buf + count, m_size);
                    }
                }
            }
        }
        void iltrim() {
            if (m_size == 0) return;
            if (!isspace(m_data_buf[m_size - 1])) return;
            if (is_local()) {
                while (isspace(m_local_buf[m_size - 1]) && m_size > 0)
                    m_size--;
                m_local_buf[m_size] = '\0';
            } else {
                while (isspace(m_data_buf[m_size - 1]) && m_size > 0)
                    m_size--;
                m_data_buf[m_size] = '\0';
                // if the m_size is now small enough, let's swap to small String
                if (m_size <= SMALL_STRING_CAP) {
                    memcpy(m_local_buf, m_data_buf, SMALL_STRING_CAP + 1);
                    deallocate<char, DeallocType::Multiple>(m_data_buf);
                    m_data_buf = local_data_internal();
                }
            }
        }
        void itrim() {
            irtrim();
            iltrim();
        }
        [[nodiscard]] String ltrim() const {
            String ret(*this);
            ret.iltrim();
            return ret;
        }
        [[nodiscard]] String rtrim() const {
            String ret(*this);
            ret.irtrim();
            return ret;
        }
        [[nodiscard]] String trim() const {
            String ret(*this);
            ret.itrim();
            return ret;
        }

        Vector<String> split_at_any(const char* sep = " \n\t\v") const;
        Vector<StringView> split_view_at_any(const char* sep = " \n\t\v") const;

        Vector<String> split(const char* sep = " ") const;
        Vector<StringView> split_view(const char* sep = " ") const;

        // upper/lower
        void iupper() {
            char* buf = get_buf_internal();
            for (size_t i = 0; i < m_size; i++) {
                buf[i] = toupper(buf[i]);
            }
        }
        void ilower() {
            char* buf = get_buf_internal();
            for (size_t i = 0; i < m_size; i++) {
                buf[i] = tolower(buf[i]);
            }
        }
        [[nodiscard]] String upper() const& {
            String str(*this);
            str.iupper();
            return str;
        }
        [[nodiscard]] String lower() const& {
            String str(*this);
            str.ilower();
            return str;
        }
        [[nodiscard]] String upper() && {
            String str{move(*this)};
            str.iupper();
            return str;
        }
        [[nodiscard]] String lower() && {
            String str{move(*this)};
            str.ilower();
            return str;
        }

        // replace
        void ireplace(char n, char s, size_t times = String::npos) {
            char* buf = get_buf_internal();
            for (size_t i = 0, j = 0; i < m_size && j < times; i++) {
                if (buf[i] == n) {
                    buf[i] = s;
                    j++;
                }
            }
        }

        String replace(char n, char s, size_t times = String::npos) {
            String cp{*this};
            cp.ireplace(n, s, times);
            return cp;
        }

        void ireplace(StringView n, StringView s, size_t times = String::npos);
        String replace(StringView n, StringView s, size_t times = String::npos) const;

        void reserve(size_t new_capacity) { grow_if_needed(new_capacity); }
    };

    inline String operator""_s(const char* source, size_t len) {
        return String{source, len};
    }

    template <>
    struct Hash<String> {
        [[nodiscard]] size_t operator()(const String& key) const noexcept {
            return hash_array_representation(key.data(), key.size());
        }
    };
} // namespace ARLib
