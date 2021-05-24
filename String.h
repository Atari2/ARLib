#pragma once
#include "Vector.h"
#include "Algorithm.h"
#include "Assertion.h"
#include "HashCalc.h"
#include "Ordering.h"
#include "Types.h"
#include "cstring_compat.h"

namespace ARLib {
    class String {
        static constexpr int max_small = 24;
        bool small_string_opt = false;
        char small_string_buf[max_small + 1] = { 0 };
        char* big_string_buf = nullptr;
        size_t len = 0;

        const char* get_buf_internal() const { return small_string_opt ? small_string_buf : big_string_buf; }
        static const char* get_buf_internal(const String& other) {
            return other.small_string_opt ? other.small_string_buf : other.big_string_buf;
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
        static constexpr auto npos = 0xffffffffffffffff;
        String() : len(0), small_string_opt(true) {};
        String(const char* other, size_t size) : len(size) {
            small_string_opt = len <= max_small;
            auto real_len = strlen(other);
            if (small_string_opt) {
                if (len > real_len) {
                    strcpy(small_string_buf, other);
                }
                else {
                    strncpy(small_string_buf, other, len);
                    small_string_buf[len] = '\0';
                }
            }
            else {
                // we always allocate the requested size, even if it's waaay bigger than actual size, we also always base
                // small_string_opt on requested size and not actual size, this way the user can force a big String to be
                // constructed even if the passed String would fit in max_small
                big_string_buf = new char[len + 1];
                if (len > real_len) {
                    strcpy(big_string_buf, other);
                }
                else {
                    strncpy(big_string_buf, other, len);
                    big_string_buf[len] = '\0';
                }
            }
        }
        String(const char* other) : len(strlen(other)) {
            small_string_opt = len <= max_small;
            if (small_string_opt) {
                strcpy(small_string_buf, other);
            }
            else {
                big_string_buf = new char[len + 1];
                strcpy(big_string_buf, other);
            }
        }
        String(const String& other) noexcept : len(other.len), small_string_opt(other.small_string_opt)  {
            if (small_string_opt) {
                strcpy(small_string_buf, other.small_string_buf);
            }
            else {
                big_string_buf = new char[len + 1];
                strcpy(big_string_buf, other.big_string_buf);
            }
        }
        String(String&& other) noexcept : len(other.len), small_string_opt(other.small_string_opt)  {
            if (small_string_opt) {
                strcpy(small_string_buf, other.small_string_buf);
            }
            else {
                big_string_buf = other.big_string_buf;
            }
            other.big_string_buf = nullptr;
            other.len = 0;
        }
        /*
        String(StringView other) : len(other.length()) {
            small_string_opt = len <= max_small;
            if (small_string_opt) {
                strcpy(small_string_buf, other.data());
            }
            else {
                big_string_buf = new char[len + 1];
                strcpy(big_string_buf, other.data());
            }
        }
        */
        String& operator=(const String& other) {
            if (this != &other) {
                len = other.len;
                small_string_opt = other.small_string_opt;
                if (small_string_opt) {
                    strcpy(small_string_buf, other.small_string_buf);
                }
                else {
                    delete[] big_string_buf;
                    big_string_buf = new char[len + 1];
                    strcpy(big_string_buf, other.big_string_buf);
                }
            }
            return *this;
        }
        String& operator=(String&& other) noexcept {
            if (this != &other) {
                len = other.len;
                small_string_opt = other.small_string_opt;
                if (small_string_opt) {
                    strcpy(small_string_buf, other.small_string_buf);
                }
                else {
                    delete[] big_string_buf;
                    big_string_buf = other.big_string_buf;
                    other.big_string_buf = nullptr;
                    other.len = 0;
                }
            }
            return *this;
        }
        ~String() {
            if (!small_string_opt) {
                delete[] big_string_buf;
            }
        }
        [[nodiscard]] bool operator==(const String& other) const {
            if (other.len == len) {
                // if small_string_opt is true for *this, it's also true for other,
                // due to the fact that they share length, therefor we don't need to check if other.small_string_opt is true
                if (small_string_opt) {
                    return strcmp(small_string_buf, other.small_string_buf) == 0;
                }
                else {
                    return strcmp(big_string_buf, other.big_string_buf) == 0;
                }
            }
            return false;
        }
        [[nodiscard]] bool operator!=(const String& other) const { return !(*this == other); }
        [[nodiscard]] bool operator<(const String& other) const {
            size_t smaller_len = len > other.len ? other.len : len;
            if (small_string_opt && other.small_string_opt) {
                return strncmp(small_string_buf, other.small_string_buf, smaller_len) < 0;
            }
            else if (small_string_opt && !other.small_string_opt) {
                return strncmp(small_string_buf, other.big_string_buf, smaller_len) < 0;
            }
            else if (!small_string_opt && other.small_string_opt) {
                return strncmp(big_string_buf, other.small_string_buf, smaller_len) < 0;
            }
            else {
                return strncmp(big_string_buf, other.big_string_buf, smaller_len) < 0;
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
        [[nodiscard]] const size_t length() const { return len; }
        [[nodiscard]] const char* data() const { return get_buf_internal(); }
        [[nodiscard]] bool is_empty() const { return len == 0; }
        [[nodiscard]] bool starts_with(const String& other) const {
            if (other.len > len)
                return false;
            if (other.len == len)
                return other == *this;
            auto res = strncmp(get_buf_internal(other), get_buf_internal(), other.len);
            return res == 0;
        }
        [[nodiscard]] bool starts_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > len)
                return false;
            if (len == o_len)
                return strcmp(other, get_buf_internal()) == 0;
            auto res = strncmp(other, get_buf_internal(), o_len);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const String& other) const {
            if (other.len > len)
                return false;
            if (other.len == len)
                return other == *this;
            auto ptrdiff = len - other.len;
            const char* buf = get_buf_internal(other);
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, buf, other.len);
            return res == 0;
        }
        [[nodiscard]] bool ends_with(const char* other) const {
            auto o_len = strlen(other);
            if (o_len > len)
                return false;
            if (len == o_len)
                return strcmp(other, get_buf_internal()) == 0;
            auto ptrdiff = len - o_len;
            const char* my_buf = get_buf_internal();
            auto res = strncmp(my_buf + ptrdiff, other, o_len);
            return res == 0;
        }
        void concat(const String& other) {
            auto new_len = len + other.len;
            auto new_optimized = new_len <= max_small;
            if (small_string_opt && new_optimized) {
                // if the sum of the 2 is *still* under the small String optimization, then we have the guarantee that
                // it'll fit completely
                strcat(small_string_buf, other.small_string_buf);
            }
            else if (small_string_opt && !new_optimized) {
                // if the old one was small, and new one can't fit, we'll make it fit
                const char* ptr = get_buf_internal(other);
                small_string_opt = false;
                len = new_len;
                big_string_buf = new char[len + 1];
                strcpy(big_string_buf, small_string_buf);
                strcat(big_string_buf, ptr);
                small_string_buf[0] = '\0';
            }
            else {
                // old one wasn't small to begin with, just enlarge to fit.
                const char* ptr = get_buf_internal(other);
                len = new_len;
                char* new_buf = new char[len + 1];
                strcpy(new_buf, big_string_buf);
                strcat(new_buf, ptr);
                delete[] big_string_buf;
                big_string_buf = new_buf;
            }
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
        String operator+=(const String& other) const { return *this + other; }
        [[nodiscard]] const char* begin() const { return get_buf_internal(); }
        [[nodiscard]] const char* rbegin() const { return end(); }
        [[nodiscard]] const char* end() const { return get_buf_internal() + len; }
        [[nodiscard]] const char* rend() const { return begin(); }
        [[nodiscard]] char front() const { return get_buf_internal()[0]; }
        [[nodiscard]] char back() const { return get_buf_internal()[len - 1]; }
        // bounds checking
        [[nodiscard]] char at(size_t index) const {
            SOFT_ASSERT_FMT((index >= len || index < 0), "Index of %llu was out of bounds of String with size %llu", index, len)
            return get_buf_internal()[index];
        }
        [[nodiscard]] char operator[](size_t index) const { return get_buf_internal()[index]; }

        [[nodiscard]] size_t index_of(char c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = 0; i < len; i++) {
                if (buf[i] == c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(char c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            for (ptrdiff_t i = len - 1; i >= 0; i--) {
                if (buf[i] == c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t index_not_of(char c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = 0; i < len; i++) {
                if (buf[i] != c)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(char c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            for (size_t i = len - 1; ; i--) {
                if (buf[i] != c)
                    return i;
                if (i == 0)
                    break;
            }
            return npos;
        }
        [[nodiscard]] size_t index_of(const char* c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > len)
                return npos;
            if (o_len == len && strcmp(buf, c) == 0)
                return 0;
            for (size_t i = 0; i < len; i++) {
                if (strncmp(buf + i, c, o_len) == 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_of(const char* c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > len)
                return npos;
            if (o_len == len && strcmp(buf, c) == 0)
                return 0;
            for (ptrdiff_t i = len - o_len; i >= 0; i--) {
                if (strncmp(buf + i, c, o_len) == 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t index_not_of(const char* c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > len)
                return npos;
            if (o_len == len && strcmp(buf, c) != 0)
                return 0;
            for (size_t i = 0; i < len; i++) {
                if (strncmp(buf + i, c, o_len) != 0)
                    return i;
            }
            return npos;
        }
        [[nodiscard]] size_t last_index_not_of(const char* c) const {
            if (len == 0) return npos;
            const char* buf = get_buf_internal();
            auto o_len = strlen(c);
            if (o_len > len)
                return npos;
            if (o_len == len && strcmp(buf, c) != 0)
                return 0;
            for (ptrdiff_t i = len - o_len; i >= 0; i--) {
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
        void irtrim() {
            if (small_string_opt) {
                int count = 0;
                while (isspace(small_string_buf[count++]))
                    ;
                count--;
                if (count != 0) {
                    len -= count;
                    memmove(small_string_buf, small_string_buf + count, len);
                }
            }
            else {
                int count = 0;
                while (isspace(big_string_buf[count++]))
                    ;
                count--;
                if (count != 0) {
                    len -= count;
                    // if the len is now small enough, let's swap to small String
                    if (len <= max_small) {
                        small_string_opt = true;
                        memmove(small_string_buf, big_string_buf + count, len);
                        delete[] big_string_buf;
                        big_string_buf = nullptr;
                    }
                    else {
                        memmove(big_string_buf, big_string_buf + count, len);
                    }
                }
            }
        }
        void iltrim() {
            if (small_string_opt) {
                while (isspace(small_string_buf[len - 1]) && len > 0)
                    len--;
                small_string_buf[len] = '\0';
            }
            else {
                while (isspace(big_string_buf[len - 1]) && len > 0)
                    len--;
                big_string_buf[len] = '\0';
                // if the len is now small enough, let's swap to small String
                if (len <= max_small) {
                    small_string_opt = true;
                    strcpy(small_string_buf, big_string_buf);
                    delete[] big_string_buf;
                    big_string_buf = nullptr;
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
        void iupper() {
            char* buf = small_string_opt ? small_string_buf : big_string_buf;
            for (size_t i = 0; i < len; i++) {
                buf[i] = toupper(buf[i]);
            }
        }
        void ilower() {
            char* buf = small_string_opt ? small_string_buf : big_string_buf;
            for (size_t i = 0; i < len; i++) {
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
    };
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
        return hashptr(&s);
    }
    inline bool hash_equals(const String& a, const String& b) {
        return hashptr(&a) == hashptr(&b);
    }
};

using ARLib::String;
using ARLib::StringView;
using ARLib::operator""s;