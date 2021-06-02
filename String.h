#pragma once
#include "Vector.h"
#include "Algorithm.h"
#include "Assertion.h"
#include "HashCalc.h"
#include "Ordering.h"
#include "Types.h"
#include "Iterator.h"
#include "cstring_compat.h"

namespace ARLib {

    class String {
        static constexpr size_t SMALL_STRING_CAP = 24;
        union {
            char m_local_buf[SMALL_STRING_CAP + 1] = { 0 };
            size_t m_allocated_capacity;
        };
        char* m_data_buf = nullptr;
        size_t m_size = 0;

        char* data_internal() {
            return m_data_buf;
        }

        const char* data_internal() const {
            return m_data_buf;
        }

        char* local_data_internal() {
            return PointerTraits<char*>::pointer_to(*m_local_buf);
        }

        const char* local_data_internal() const {
            return PointerTraits<const char*>::pointer_to(*m_local_buf);
        }

        bool is_local() const {
            return data_internal() == local_data_internal();
        }

        void grow_internal(size_t new_capacity) {
            if (is_local()) {
                // grow outside of locality, copy buffer and change active member of union
                m_data_buf = new char[new_capacity];
                memmove(m_data_buf, m_local_buf, new_capacity + 1);
                m_allocated_capacity = new_capacity;
            }
            else {
                if (new_capacity < m_allocated_capacity * 2)
                    new_capacity = m_allocated_capacity * 2;
                m_allocated_capacity = new_capacity;
                char* new_buf = new char[m_allocated_capacity];
                memmove(new_buf, m_data_buf, m_size + 1);
                delete[] m_data_buf;
                m_data_buf = new_buf;
            }
        }

        void grow_if_needed(size_t newsize) {
            if (is_local()) {
                if (newsize > SMALL_STRING_CAP)
                    grow_internal(newsize + 1);
            }
            else {
                if (newsize > m_allocated_capacity - 1)
                    grow_internal(newsize + 1);
            }
        }

        const char* get_buf_internal() const { 
            if (is_local()) {
                return local_data_internal();
            }
            return data_internal();
        }

        char* get_buf_internal() {
            if (is_local()) {
                return local_data_internal();
            }
            return data_internal();
        }
        static const char* get_buf_internal(const String& other) {
            return other.get_buf_internal();
        }
        Vector<size_t> all_indexes(const char* any) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            indexes.reserve(size);
            for (int i = 0; i < size; i++) {
                indexes.push_back(index_of(any[i]));
            }
            return indexes;
        }
        Vector<size_t> all_last_indexes(const char* any) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            indexes.reserve(size);
            for (int i = 0; i < size; i++) {
                indexes.push_back(last_index_of(any[i]));
            }
            return indexes;
        }
        Vector<size_t> all_not_indexes(const char* any) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            indexes.reserve(size);
            for (int i = 0; i < size; i++) {
                indexes.push_back(index_not_of(any[i]));
            }
            return indexes;
        }
        Vector<size_t> all_last_not_indexes(const char* any) const {
            auto size = strlen(any);
            Vector<size_t> indexes{};
            indexes.reserve(size);
            for (int i = 0; i < size; i++) {
                indexes.push_back(last_index_not_of(any[i]));
            }
            return indexes;
        }

    public:
        static constexpr auto npos = static_cast<size_t>(-1);

        // constructors, destructor equality operators
        String() : m_data_buf(local_data_internal()) {};
        String(size_t size) {
            grow_if_needed(size);
        }
        String(const char* begin, const char* end) {
            m_size = end - begin;
            bool local = m_size <= SMALL_STRING_CAP;
            if (local) {
                strncpy(m_local_buf, begin, m_size);
                m_data_buf = local_data_internal();
            }
            else {
                m_allocated_capacity = m_size + 1;
                m_data_buf = new char[m_allocated_capacity];
                strncpy(m_data_buf, begin, m_size);
                m_data_buf[m_size] = '\0';
            }
        }
        String(const char* other, size_t size) : m_size(size) {
            bool local = m_size <= SMALL_STRING_CAP;
            auto real_len = strlen(other);
            if (local) {
                if (m_size > real_len) {
                    strcpy(m_local_buf, other);
                }
                else {
                    strncpy(m_local_buf, other, m_size);
                    m_local_buf[m_size] = '\0';
                }
                m_data_buf = local_data_internal();
            }
            else {
                // we always allocate the requested size, even if it's waaay bigger than actual size, we also always base
                // m_small_string_opt on requested size and not actual size, this way the user can force a big String to be
                // constructed even if the passed String would fit in SMALL_STRING_CAP
                m_allocated_capacity = size;
                m_data_buf = new char[m_size + 1];
                if (m_size > real_len) {
                    strcpy(m_data_buf, other);
                }
                else {
                    strncpy(m_data_buf, other, m_size);
                    m_data_buf[m_size] = '\0';
                }
            }
        }
        String(const char* other) {
            auto len = strlen(other);
            m_size = len;
            bool local = m_size <= SMALL_STRING_CAP;
            if (local) {
                strcpy(m_local_buf, other);
                m_data_buf = local_data_internal();
            }
            else {
                m_allocated_capacity= m_size + 1;
                m_data_buf = new char[m_size + 1];
                strcpy(m_data_buf, other);
            }
        }
        String(const String& other) noexcept : m_size(other.m_size) {
            if (other.is_local()) {
                strcpy(m_local_buf, other.m_local_buf);
                m_data_buf = local_data_internal();
            }
            else {
                m_allocated_capacity = m_size + 1;
                m_data_buf = new char[m_size + 1];
                strcpy(m_data_buf, other.m_data_buf);
            }
        }
        String(String&& other) noexcept : m_size(other.m_size) {
            if (other.is_local()) {
                strcpy(m_local_buf, other.m_local_buf);
                m_data_buf = local_data_internal();
            }
            else {
                m_allocated_capacity = other.m_size;
                m_data_buf = other.m_data_buf;
                other.m_allocated_capacity = 0;
            }
            other.m_data_buf = nullptr;
            other.m_size = 0;
        }
        /*
        String(StringView other) : m_size(other.length()) {
            m_small_string_opt = m_size <= SMALL_STRING_CAP;
            if (m_small_string_opt) {
                strcpy(m_small_string_buf, other.data());
            }
            else {
                m_big_string_buf = new char[m_size + 1];
                strcpy(m_big_string_buf, other.data());
            }
        }
        */
        String& operator=(const String& other) {
            if (this != &other) {
                m_size = other.m_size;
                if (other.is_local()) {
                    strcpy(m_local_buf, other.m_local_buf);
                    m_data_buf = local_data_internal();
                }
                else {
                    delete[] m_data_buf;
                    m_data_buf = new char[m_size + 1];
                    m_allocated_capacity = m_size + 1;
                    strcpy(m_data_buf, other.m_data_buf);
                }
            }
            return *this;
        }
        String& operator=(String&& other) noexcept {
            if (this != &other) {
                m_size = other.m_size;
                if (other.is_local()) {
                    strcpy(m_local_buf, other.m_local_buf);
                    m_data_buf = local_data_internal();
                }
                else {
                    delete[] m_data_buf;
                    m_data_buf = other.m_data_buf;
                    m_allocated_capacity = other.m_allocated_capacity;
                    other.m_data_buf = nullptr;
                    other.m_size = 0;
                }
            }
            return *this;
        }
        ~String() {
            m_size = 0;
            if (!is_local()) {
                delete[] m_data_buf;
                m_data_buf = nullptr;
            }
        }

        // comparison operators
        [[nodiscard]] bool operator==(const String& other) const {
            if (other.m_size == m_size) {
                // if m_small_string_opt is true for *this, it's also true for other,
                // due to the fact that they share length, therefor we don't need to check if other.m_small_string_opt is true
                if (is_local()) {
                    return strcmp(m_local_buf, other.m_local_buf) == 0;
                }
                else {
                    return strcmp(m_data_buf, other.m_data_buf) == 0;
                }
            }
            return false;
        }
        [[nodiscard]] bool operator!=(const String& other) const { return !(*this == other); }
        [[nodiscard]] bool operator<(const String& other) const {
            size_t smaller_len = m_size > other.m_size ? other.m_size : m_size;
            bool local = is_local();
            bool other_local = other.is_local();
            if (local && other_local) {
                return strncmp(m_local_buf, other.m_local_buf, smaller_len) < 0;
            }
            else if (local && !other_local) {
                return strncmp(m_local_buf, other.m_data_buf, smaller_len) < 0;
            }
            else if (!local && other_local) {
                return strncmp(m_data_buf, other.m_local_buf, smaller_len) < 0;
            }
            else {
                return strncmp(m_data_buf, other.m_data_buf, smaller_len) < 0;
            }
        }
        [[nodiscard]] bool operator>(const String& other) const { return !(*this < other) && !(*this == other); }
        [[nodiscard]] bool operator<=(const String& other) const { return (*this < other || *this == other); }
        [[nodiscard]] bool operator>=(const String& other) const { return (*this > other || *this == other); }
        [[nodiscard]] Ordering operator<=>(const String & other) const {
            if (*this == other) return equal;
            else if (*this < other) return less;
            else return greater;
        }


        [[nodiscard]] size_t length() const { return m_size; }
        [[nodiscard]] size_t capacity() const { return is_local() ? SMALL_STRING_CAP : m_allocated_capacity ;}
        [[nodiscard]] const char* data() const { return get_buf_internal(); }
        [[nodiscard]] bool is_empty() const { return m_size == 0; }

        // starts/ends with
        [[nodiscard]] bool starts_with(const String& other) const {
            if (other.m_size > m_size)
                return false;
            if (other.m_size == m_size)
                return other == *this;
            auto res = strncmp(get_buf_internal(other), get_buf_internal(), other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool starts_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > m_size)
                return false;
            if (m_size == o_len)
                return strcmp(other, get_buf_internal()) == 0;
            auto res = strncmp(other, get_buf_internal(), o_len);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const String& other) const {
            if (other.m_size > m_size)
                return false;
            if (other.m_size == m_size)
                return other == *this;
            auto ptrdiff = m_size - other.m_size;
            const char* buf = get_buf_internal(other);
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, buf, other.m_size);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > m_size)
                return false;
            if (m_size == o_len)
                return strcmp(other, get_buf_internal()) == 0;
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
            get_buf_internal()[++m_size] = '\0';
        }
        void concat(const String& other) {
            auto new_size = m_size + other.m_size;
            grow_if_needed(new_size);
            strcat(get_buf_internal(), other.get_buf_internal());
        }
        void concat(const char* other) {
            String sother(other);
            concat(sother);
        }


        String operator+(const String& other) const {
            String new_str(*this);
            new_str.concat(other);
            return new_str;
        }
        String& operator+=(const String& other) { this->concat(other); return *this; }

        // reverse 
        String reversed() {
            String reversed{};
            for (auto rc = this->rbegin(); rc != this->rend(); rc--)
                reversed.append(*rc);
            return reversed;
        }

        // iterator support
        [[nodiscard]] Iterator<char> begin() { return { get_buf_internal() }; }
        [[nodiscard]] Iterator<char> rbegin() { return end() - 1; }
        [[nodiscard]] Iterator<char> end() { return { get_buf_internal() + m_size }; }
        [[nodiscard]] Iterator<char> rend() { return begin() - 1; }
        [[nodiscard]] char front() const { return get_buf_internal()[0]; }
        [[nodiscard]] char back() const { return get_buf_internal()[m_size - 1]; }
        
        // indexing access
        [[nodiscard]] char at(size_t index) const {
            SOFT_ASSERT_FMT((index >= m_size || index < 0), "Index of %llu was out of bounds of String with size %llu", index, m_size)
            return get_buf_internal()[index];
        }
        [[nodiscard]] char& operator[](size_t index) { return get_buf_internal()[index]; }
        [[nodiscard]] char operator[](size_t index) const { return get_buf_internal()[index]; }

        // various char checks
        [[nodiscard]] size_t index_of(char c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = 0; i < m_size; i++) {
                if (buf[i] == c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(char c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (ptrdiff_t i = m_size - 1; i >= 0; i--) {
                if (buf[i] == c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t index_not_of(char c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = 0; i < m_size; i++) {
                if (buf[i] != c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(char c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = m_size - 1; ; i--) {
                if (buf[i] != c)
                    return i;
                if (i == 0)
                    break;
            }
            return npos;
        }
        [[nodiscard]] size_t index_of(const char* c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > m_size)
                return npos;
            if (o_len == m_size && strcmp(buf, c) == 0)
                return 0;
            for (size_t i = 0; i < m_size; i++) {
                if (strncmp(buf + i, c, o_len) == 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(const char* c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > m_size)
                return npos;
            if (o_len == m_size && strcmp(buf, c) == 0)
                return 0;
            for (ptrdiff_t i = m_size - o_len; i >= 0; i--) {
                if (strncmp(buf + i, c, o_len) == 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t index_not_of(const char* c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > m_size)
                return npos;
            if (o_len == m_size && strcmp(buf, c) != 0)
                return 0;
            for (size_t i = 0; i < m_size; i++) {
                if (strncmp(buf + i, c, o_len) != 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(const char* c) const {
            if (m_size == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > m_size)
                return npos;
            if (o_len == m_size && strcmp(buf, c) != 0)
                return 0;
            for (ptrdiff_t i = m_size - o_len; i >= 0; i--) {
                if (strncmp(buf + i, c, o_len) != 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t index_of_any(const char* any) const {
            auto indexes = all_indexes(any);
            return *min(indexes);
        }
        [[nodiscard]] size_t last_index_of_any(const char* any) const {
            auto indexes = all_last_indexes(any);
            return *max(indexes);
        }
        [[nodiscard]] size_t index_not_of_any(const char* any) const {
            auto indexes = all_not_indexes(any);
            return *min(indexes);
        }
        [[nodiscard]] size_t last_index_not_of_any(const char* any) const {
            auto indexes = all_last_not_indexes(any);
            return *max(indexes);
        }
        [[nodiscard]] bool contains(const char* other) const { return index_of(other) != npos; }
        [[nodiscard]] bool contains(char c) const { return index_of(c) != npos; }
        
        // trim
        void irtrim() {
            if (is_local()) {
                int count = 0;
                while (isspace(m_local_buf[count++]))
                    ;
                count--;
                if (count != 0) {
                    m_size -= count;
                    memmove(m_local_buf, m_local_buf + count, m_size);
                }
            }
            else {
                int count = 0;
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
                    }
                    else {
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
            }
            else {
                while (isspace(m_data_buf[m_size - 1]) && m_size > 0)
                    m_size--;
                m_data_buf[m_size] = '\0';
                // if the m_size is now small enough, let's swap to small String
                if (m_size <= SMALL_STRING_CAP) {
                    strcpy(m_local_buf, m_data_buf);
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

        void reserve(size_t new_capacity) {
            grow_if_needed(new_capacity);
        }
    };

    // TODO: make stringview usable
    class StringView {
        const char* m_start = nullptr;
        const char* m_end = nullptr;

    public:
        static constexpr auto npos = String::npos;
        StringView(const char* buf, size_t size) : m_start(buf) { m_end = buf + size; }
        StringView(const char* buf) : m_start(buf) { m_end = buf + strlen(buf); }
        StringView(const String& ref) : m_start(ref.data()) { m_end = m_start + ref.length(); }
        [[nodiscard]] const size_t length() const { return m_end - m_start; }
        [[nodiscard]] const char* data() const { return m_start; }
        [[nodiscard]] const char* end() const { return m_end; }
        [[nodiscard]] String extract_string() const { return String(m_start, length()); }
    };

    String operator""s(const char* source, size_t len) {
        return String{ source, len + 1};
    }

    inline uint32_t hash(const String& s) {
        return hashstr(s.data(), s.length());
    }
    inline bool hash_equals(const String& a, const String& b) {
        return hash(a) == hash(b);
    }
};

using ARLib::String;
using ARLib::StringView;
using ARLib::operator""s;