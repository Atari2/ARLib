#pragma once
#include "Algorithm.h"
#include "Assertion.h"
#include "Enumerate.h"
#include "Iterator.h"
#include "Ordering.h"
#include "Types.h"
#include "Vector.h"
#include "cstring_compat.h"

namespace ARLib {
    class StringView;

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
                m_data_buf = new char[requested_capacity];
                memmove(m_data_buf, m_local_buf, SMALL_STRING_CAP + 1);
                m_allocated_capacity = requested_capacity;
            } else {
                m_allocated_capacity = basic_growth(requested_capacity);
                HARD_ASSERT(m_allocated_capacity >= requested_capacity && m_allocated_capacity > m_size,
                            "Allocated capacity failure")
                char* new_buf = new char[m_allocated_capacity];
                new_buf[m_size] = '\0';
                if (m_size != 0) memmove(new_buf, m_data_buf, m_size + 1);
                delete[] m_data_buf;
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

        Vector<size_t> all_indexes_internal(const char* any, size_t start_index = 0ull) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            for (size_t i = 0; i < size; i++) {
                auto index = index_of(any[i], start_index);
                while (index != npos) {
                    indexes.push_back(index);
                    index = index_of(any[i], index + 1);
                }
            }
            return indexes;
        }
        Vector<size_t> all_last_indexes_internal(const char* any, size_t end_index = npos) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            for (size_t i = 0; i < size; i++) {
                auto index = last_index_of(any[i], end_index);
                while (index != npos && index != 0) {
                    indexes.push_back(index);
                    index = last_index_of(any[i], index - 1);
                }
            }
            return indexes;
        }
        Vector<size_t> all_not_indexes_internal(const char* any, size_t start_index = 0ull) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            for (size_t i = 0; i < size; i++) {
                auto index = index_not_of(any[i], start_index);
                while (index != npos) {
                    indexes.push_back(index);
                    index = index_not_of(any[i], index + 1);
                }
            }
            return indexes;
        }
        Vector<size_t> all_last_not_indexes_internal(const char* any, size_t end_index = npos) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            for (size_t i = 0; i < size; i++) {
                auto index = last_index_not_of(any[i], end_index);
                while (index != npos && index != 0) {
                    indexes.push_back(index);
                    index = last_index_not_of(any[i], index - 1);
                }
            }
            return indexes;
        }

        public:
        static constexpr auto npos = static_cast<size_t>(-1);

        // constructors, destructor equality operators
        constexpr String() = default;

        template <size_t N>
        constexpr String(const char (&src)[N]) : m_size(N - 1) {
            grow_if_needed(N);
            strncpy(m_data_buf, src, N);
        }

        explicit String(size_t size) { grow_if_needed(size); }
        explicit String(size_t size, char c) {
            grow_if_needed(size);
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
        constexpr String(T other) : m_size(strlen(other)) {
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

        String(StringView other);

        String& operator=(const String& other) {
            if (this != &other) {
                if (!is_local()) delete[] m_data_buf;
                m_size = other.m_size;
                grow_if_needed(m_size);
                memcpy(m_data_buf, other.m_data_buf, m_size + 1);
            }
            return *this;
        }
        String& operator=(String&& other) noexcept {
            if (this != &other) {
                if (!is_local()) delete[] m_data_buf;
                m_size = other.m_size;
                if (other.is_local()) {
                    memcpy(m_local_buf, other.m_local_buf, SMALL_STRING_CAP + 1);
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
            size_t len = static_cast<size_t>(scprintf(format, args...));
            str.reserve(len);
            str.set_size(static_cast<size_t>(sprintf(str.rawptr(), format, args...)));
            return str;
        }

        ~String() {
            m_size = 0;
            if (!is_local()) {
                delete[] m_data_buf;
                m_data_buf = nullptr;
            }
        }

        [[nodiscard]] String substring(size_t first = 0, size_t last = npos) const {
            if (last == npos) last = length();
            return String{get_buf_internal() + first, get_buf_internal() + last};
        }
        [[nodiscard]] StringView view();
        [[nodiscard]] StringView substringview(size_t first = 0, size_t last = npos) const;

        // comparison operators
        [[nodiscard]] bool operator==(const String& other) const {
            if (other.m_size == m_size) { return strncmp(m_data_buf, other.m_data_buf, m_size) == 0; }
            return false;
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
        [[nodiscard]] Ordering operator<=>(const String& other) const {
            auto val = strncmp(get_buf_internal(), other.get_buf_internal(), other.m_size);
            if (val == 0)
                return equal;
            else if (val < 0)
                return less;
            else
                return greater;
        }
        [[nodiscard]] Ordering operator<=>(const StringView& other) const;

        void set_size(size_t size) {
            m_size = size;
            get_buf_internal()[m_size] = '\0';
        }
        [[nodiscard]] size_t size() const { return m_size; }
        [[nodiscard]] size_t length() const { return m_size; }
        [[nodiscard]] size_t capacity() const { return is_local() ? SMALL_STRING_CAP : m_allocated_capacity; }
        [[nodiscard]] const char* data() const { return get_buf_internal(); }
        [[nodiscard]] char* rawptr() { return get_buf_internal(); }
        [[nodiscard]] bool is_empty() const { return m_size == 0; }

        // starts/ends with
        [[nodiscard]] bool starts_with(const String& other) const {
            if (other.m_size > m_size) return false;
            if (other.m_size == m_size) return other == *this;
            auto res = strncmp(other.get_buf_internal(), get_buf_internal(), other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool starts_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > m_size) return false;
            if (m_size == o_len) return strcmp(other, get_buf_internal()) == 0;
            auto res = strncmp(other, get_buf_internal(), o_len);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const String& other) const {
            if (other.m_size > m_size) return false;
            if (other.m_size == m_size) return other == *this;
            auto ptrdiff = m_size - other.m_size;
            const char* buf = other.get_buf_internal();
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, buf, other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > m_size) return false;
            if (m_size == o_len) return strcmp(other, get_buf_internal()) == 0;
            auto ptrdiff = m_size - o_len;
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, other, o_len);
            return res == 0;
        }

        // concatenation
        void append(char c) {
            auto new_size = m_size + 1;
            grow_if_needed(new_size);
            get_buf_internal()[m_size] = c;
            set_size(m_size + 1);
        }
        void concat(const String& other) {
            auto new_size = m_size + other.m_size;
            grow_if_needed(new_size);
            strcat_eff(get_buf_internal() + m_size, other.get_buf_internal());
            set_size(m_size + other.m_size);
        }
        void concat(StringView other);
        void concat(const char* other) {
            String sother(other);
            concat(sother);
        }

        String operator+(const String& other) const {
            String new_str(*this);
            new_str.concat(other);
            return new_str;
        }
        String& operator+=(const String& other) {
            concat(other);
            return *this;
        }

        // reverse
        String reversed() {
            String reversed{};
            for (auto rc = this->rbegin(); rc != this->rend(); rc--)
                reversed.append(*rc);
            return reversed;
        }

        // iterator support
        [[nodiscard]] Iterator<char> begin() { return {get_buf_internal()}; }
        [[nodiscard]] ConstIterator<char> begin() const { return {get_buf_internal()}; }
        [[nodiscard]] Iterator<char> rbegin() { return end() - 1; }
        [[nodiscard]] Iterator<char> end() { return {get_buf_internal() + m_size}; }
        [[nodiscard]] ConstIterator<char> end() const { return {get_buf_internal() + m_size}; }
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
        [[nodiscard]] size_t index_of(const char* c, size_t start = 0) const {
            if (m_size == 0 || start >= m_size) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > m_size) return npos;
            if (start + o_len > m_size) return npos;
            if (o_len == m_size && start == 0 && strcmp(buf, c) == 0) return 0;
            for (size_t i = start; i < m_size; i++) {
                if (strncmp(buf + i, c, o_len) == 0) return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(const char* c, size_t end = npos) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (end < o_len || o_len > m_size) return npos;
            if (end > m_size) end = m_size;
            if (o_len == end && strncmp(buf, c, end) == 0) return 0;
            for (size_t i = end - o_len;; i--) {
                if (strncmp(buf + i, c, o_len) == 0) return i;
                if (i == 0) break;
            }
            return npos;
        }
        [[nodiscard]] size_t index_not_of(const char* c, size_t start = 0) const {
            if (m_size == 0 || start >= m_size) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (start + o_len > m_size) return npos;
            if (o_len > m_size) return npos;
            if (o_len == m_size && start == 0 && strcmp(buf, c) != 0) return 0;
            for (size_t i = start; i < m_size; i++) {
                if (strncmp(buf + i, c, o_len) != 0) return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(const char* c, size_t end = npos) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (end < o_len || o_len > m_size) return npos;
            if (end > m_size) end = m_size;
            if (o_len == end && strncmp(buf, c, end) != 0) return 0;
            for (size_t i = end - o_len;; i--) {
                if (strncmp(buf + i, c, o_len) != 0) return i;
                if (i == 0) break;
            }
            return npos;
        }

        // any char in span
        [[nodiscard]] size_t index_of_any(const char* any, size_t start_index = 0ull) const {
            auto indexes = all_indexes_internal(any, start_index);
            return *min(indexes);
        }
        [[nodiscard]] size_t last_index_of_any(const char* any, size_t end_index = npos) const {
            auto indexes = all_last_indexes_internal(any, end_index);
            return *max(indexes);
        }
        [[nodiscard]] size_t index_not_of_any(const char* any, size_t start_index = 0ull) const {
            auto indexes = all_not_indexes_internal(any, start_index);
            return *min(indexes);
        }
        [[nodiscard]] size_t last_index_not_of_any(const char* any, size_t end_index = npos) const {
            auto indexes = all_last_not_indexes_internal(any, end_index);
            return *max(indexes);
        }

        [[nodiscard]] bool contains(const char* other) const { return index_of(other) != npos; }
        [[nodiscard]] bool contains(char c) const { return index_of(c) != npos; }

        // trim
        void irtrim() {
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
                        delete[] m_data_buf;
                        m_data_buf = local_data_internal();
                    } else {
                        memmove(m_data_buf, m_data_buf + count, m_size);
                    }
                }
            }
        }
        void iltrim() {
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
                    delete[] m_data_buf;
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

        Vector<String> split_at_any(const char* sep = " \n\t\v") const {
            auto indexes = all_indexes_internal(sep);
            Vector<String> vec{};
            vec.reserve(indexes.size() + 1);
            size_t prev_index = 0;
            for (auto index : indexes) {
                if (prev_index > index) prev_index = 0;
                vec.append(substring(prev_index, index));
                prev_index = index + 1;
            }
            vec.append(substring(prev_index));
            return vec;
        }
        Vector<StringView> split_view_at_any(const char* sep = " \n\t\v") const;

        Vector<String> split(const char* sep = " ") const { 
            Vector<String> vec{};
            size_t sep_len = strlen(sep);
            size_t prev_index = 0ull;
            size_t index = index_of(sep);
            while (index != npos) {
                vec.append(substring(prev_index, index));
                prev_index = index + sep_len;
                index = index_of(sep, prev_index);
            }
            vec.append(substring(prev_index));
            return vec;
        }
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
        [[nodiscard]] String upper() const {
            String str(*this);
            str.iupper();
            return str;
        }
        [[nodiscard]] String lower() const {
            String str(*this);
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

        void ireplace(const char* n, const char* s, size_t times = String::npos) {
            size_t orig_len = strlen(n);
            if (orig_len > m_size) return;
            Vector<size_t> indexes{};
            size_t cur_pos = 0;
            char* buf = get_buf_internal();
            while (cur_pos < m_size && indexes.size() <= times) {
                if (strncmp(buf + cur_pos, n, orig_len) == 0) {
                    indexes.push_back(cur_pos);
                    cur_pos += orig_len;
                } else {
                    cur_pos++;
                }
            }
            auto n_occurr = indexes.size();
            if (n_occurr == 0ull) return;
            size_t repl_len = strlen(s);
            bool repl_is_bigger = repl_len > orig_len;
            size_t diff_len = repl_is_bigger ? repl_len - orig_len : orig_len - repl_len;
            if (diff_len > 0) {
                reserve(m_size + n_occurr * diff_len);
                buf = get_buf_internal();
            }
            for (auto [count, index] : Enumerate{indexes}) {
                auto new_index = repl_is_bigger ? index + (count * diff_len) : index - (count * diff_len);
                if (repl_is_bigger)
                    memmove(buf + new_index + diff_len, buf + new_index, m_size - index + 1ull);
                else if (diff_len != 0)
                    memmove(buf + new_index, buf + new_index + diff_len, m_size - index + 1ull);
                memcpy(buf + new_index, s, repl_len);
            }
            set_size(repl_is_bigger ? m_size + diff_len * n_occurr : m_size - diff_len * n_occurr);
        }

        String replace(const char* n, const char* s, size_t times = String::npos) {
            String str{*this};
            str.ireplace(n, s, times);
            return str;
        }

        void reserve(size_t new_capacity) { grow_if_needed(new_capacity); }
    };

    String operator""_s(const char* source, size_t len);

    template <>
    struct Hash<String> {
        [[nodiscard]] size_t operator()(const String& key) const noexcept {
            return hash_array_representation(key.data(), key.size());
        }
    };
}; // namespace ARLib

using ARLib::String;
using ARLib::operator""_s;
