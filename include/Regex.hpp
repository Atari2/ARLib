#pragma once
#include "PrintInfo.hpp"
#include "Types.hpp"
#include "Concepts.hpp"
#include "String.hpp"
#include "EnumHelpers.hpp"
#include "Result.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "SSOVector.hpp"
#include "UniquePtr.hpp"
#include "Unimplemented.hpp"
namespace ARLib {

MAKE_FANCY_ENUM(
RegexToken, size_t, Dot, GroupOpen, GroupClose, SquareOpen, SquareClose, Or, StartString, EndString, Lazy, Asterisk,
Plus, OpenCurly, CloseCurly
);
MAKE_FANCY_ENUM(EscapedRegexToken, size_t, WhiteSpace, WordChar, NotWordChar, NumberChar);
struct RegexErrorInfo {
    String error_string{};
    size_t error_offset{};
};
class RegexParseError : public ErrorBase {
    RegexErrorInfo m_info;

    public:
    RegexParseError() = default;
    RegexParseError(String error, size_t offset) : m_info{ move(error), offset } {};
    RegexParseError(RegexErrorInfo info) : m_info(move(info)){};
    const RegexErrorInfo& info() const { return m_info; }
    const String& message() const { return m_info.error_string; }
    StringView error_string() const { return m_info.error_string; }
    size_t offset() const { return m_info.error_offset; }
};
class Regex {
    public:
    struct Group;
    struct CharGroup;
    struct CountToken;
    private:
    using RegexVariant = Variant<char, RegexToken, EscapedRegexToken, Group, CharGroup, CountToken>;
    public:
    using ReTokVector = UniquePtr<Vector<RegexVariant>>;
    struct Group {
        size_t m_group_number;
        ReTokVector m_group_regex{};
    };
    struct CharGroup {
        ReTokVector m_char_group{};
    };
    struct CountToken {
        size_t m_min;
        size_t m_max;
    };
    private:
    friend struct PrintInfo<Regex>;
    ReTokVector m_regex;

    static Result<Regex, RegexParseError> parse_regex(String&&);
    Regex(ReTokVector&& vec) : m_regex{ Forward<ReTokVector>(vec) } {}
    public:
    template <typename StringLike>
    requires Constructible<String, StringLike>
    Regex(StringLike&& regex) : m_regex{ regex } {}
    template <typename StringLike>
    requires Constructible<String, StringLike>
    static auto create(StringLike&& regex) {
        return parse_regex(String{ regex });
    }
    template <typename... Args>
    Unimplemented match(Args&&... args) {
        UnusedArgument arg{ args... };
        arg.remove_warning();
        return { "regex_match" };
    }
};
inline Regex operator""_re(const char* regex, size_t len) {
    return MUST(Regex::create(StringView{ regex, len }));
}
template <>
struct PrintInfo<Regex> {
    const Regex& m_regex;
    PrintInfo(const Regex& regex) : m_regex(regex) {}
    String repr() const { return print_conditional(m_regex.m_regex); }
};
template <>
struct PrintInfo<Regex::Group> {
    const Regex::Group& m_group;
    PrintInfo(const Regex::Group& group) : m_group(group) {}
    String repr() const {
        return "Group: { n: "_s + IntToStr(m_group.m_group_number) + ", tokens: "_s +
               print_conditional(m_group.m_group_regex) + " }"_s;
    }
};
template <>
struct PrintInfo<Regex::CharGroup> {
    const Regex::CharGroup& m_chargroup;
    PrintInfo(const Regex::CharGroup& chargroup) : m_chargroup(chargroup) {}
    String repr() const { return "CharGroup: { "_s + print_conditional(m_chargroup.m_char_group) + " }"_s; }
};
template <>
struct PrintInfo<Regex::CountToken> {
    const Regex::CountToken& m_count_token;
    PrintInfo(const Regex::CountToken& count_token) : m_count_token(count_token) {}
    String repr() const {
        return "CountToken: { "_s + IntToStr(m_count_token.m_min) + ","_s + IntToStr(m_count_token.m_max) + " }"_s;
    }
};
template <>
struct PrintInfo<RegexParseError> {
    const RegexParseError& m_error;
    PrintInfo(const RegexParseError& error) : m_error(error) {}
    String repr() const {
        return "Error encountered while parsing regex: "_s + m_error.message() + " at offset "_s +
               IntToStr(m_error.offset());
    }
};
}    // namespace ARLib