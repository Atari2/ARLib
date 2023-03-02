#pragma once
#include "Variant.h"
#include "Vector.h"
#include "Optional.h"
#include "GenericView.h"
#include "WString.h"
#include "WStringView.h"
#include "String.h"
#include "StringView.h"
#include "arlib_osapi.h"
#include "Result.h"
namespace ARLib {
class OutputType {
    mutable Vector<uint8_t> m_data;
    mutable Optional<String> m_cached_string;
    mutable Optional<WString> m_cached_wstring;

    public:
    OutputType() = default;
    OutputType(const ReadOnlyView<uint8_t>& data_span) { m_data.insert(data_span.begin(), data_span.end()); }
    void append(const ReadOnlyView<uint8_t>& data_span) { m_data.insert(data_span.begin(), data_span.end()); }
    const auto& data() const { return m_data; }
    auto data_span() const { return ReadOnlyView<uint8_t>{ m_data.data(), m_data.size() }; }
    const auto& string() const {
        if (!m_cached_string.has_value()) {
            if (m_data.empty())
                m_cached_string = String{};
            else {
                if (m_data.last() == 0)
                    m_cached_string = String{ reinterpret_cast<const char*>(m_data.data()) };
                else {
                    String str{};
                    str.resize(m_data.size());
                    memcpy(str.rawptr(), m_data.data(), m_data.size());
                    m_cached_string = move(str);
                }
            }
        }
        return m_cached_string.value();
    }
    auto string_view() const { return StringView{ string() }; }
    const auto& wstring() const {
        if (!m_cached_wstring.has_value()) {
            if (m_data.empty())
                m_cached_wstring = WString{};
            else {
                bool pushed = false;
                if (m_data.last() != 0) {
                    m_data.push_back(0);
                    pushed = true;
                }
                m_cached_wstring =
                string_to_wstring(StringView{ reinterpret_cast<const char*>(m_data.data()), m_data.size() });
                if (pushed) m_data.pop();
            }
        }
        return m_cached_wstring.value();
    }
    auto wstring_view() const { return WStringView{ wstring() }; }
    operator String() const { return string(); }
    operator WString() const { return wstring(); }
    operator StringView() const { return string_view(); }
    operator WStringView() const { return wstring_view(); }
};
class EnvironString {
    Variant<String, WString> m_environ_string;

    public:
    EnvironString() : m_environ_string() {}
    EnvironString(Constructible<String> auto arg) : m_environ_string(String{ arg }) {}
    EnvironString(Constructible<WString> auto arg) : m_environ_string(WString{ arg }) {}
    const auto& string() const { return get<String>(m_environ_string); }
    const auto& wstring() const { return get<WString>(m_environ_string); }
    bool holds_wide() const { return m_environ_string.contains_type<WString>(); }
    bool holds_normal() const { return m_environ_string.contains_type<String>(); }
    bool operator==(const EnvironString& other) const {
        if (holds_wide() && other.holds_wide()) {
            return wstring() == other.wstring();
        } else if (holds_normal() && other.holds_normal()) {
            return string() == other.string();
        } else {
            return false;
        }
    }
    auto size() const {
        if (holds_wide())
            return wstring().size();
        else
            return string().size();
    }
};
template <>
struct Hash<EnvironString> {
    size_t operator()(const EnvironString& str) const {
        if (str.holds_normal()) {
            return Hash<String>{}(str.string());
        } else {
            return Hash<WString>{}(str.wstring());
        }
    }
};
class ArgumentString {
    Variant<String, StringView> m_argument;

    public:
    ArgumentString(const String& arg) : m_argument(arg) {}
    ArgumentString(StringView arg) : m_argument(arg) {}
    StringView argument() const {
        if (m_argument.contains_type<String>())
            return get<String>(m_argument).view();
        else
            return get<StringView>(m_argument);
    }
};
struct ProcessError {
    static inline String m_no_error_string{ "No error" };
    String error;
};
using ProcessResult = DiscardResult<ProcessError>;
template <typename CharT>
struct StringTAllocatedRAII {
    CharT* ptr;
    StringTAllocatedRAII(size_t sz) { ptr = new CharT[sz]; }
    ~StringTAllocatedRAII() { delete[] ptr; }
};
using CharPtr  = StringTAllocatedRAII<char>;
using WCharPtr = StringTAllocatedRAII<wchar_t>;
}    // namespace ARLib