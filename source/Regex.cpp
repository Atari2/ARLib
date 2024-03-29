#include "Regex.hpp"
#include "Array.hpp"
#include "Pair.hpp"
#include "Printer.hpp"
#include "Stack.hpp"
namespace ARLib {
constexpr static Array re_tok_chars{ '.', '(', ')', '[', ']', '|', '$', '^', '?', '*', '+', '{', '}' };
constexpr static Array re_esc_chars{ 's', 'w', 'W', 'd' };

constexpr static inline bool REGEX_DEBUG = false;
template <typename Func>
void regex_debug_print(Func&& func) {
    if constexpr (REGEX_DEBUG) { func(); }
}
#define REGEX_DEBUG(format, ...) regex_debug_print([&]() { Printer::print(format, __VA_ARGS__); })

static_assert(enum_size<RegexToken>() == re_tok_chars.size());
static_assert(from_enum(EscapedRegexToken::NumberChar) + 1 == re_esc_chars.size());
static Result<Regex::CountToken, RegexParseError>
parse_count_token(ARLib::Iterator<char>& it, ARLib::Iterator<char> end, size_t& index) {
    // get stringview from current up until }
    const char* start_ptr = it.operator->();
    while (it != end) {
        if (*it != '}') {
            ++it;
            ++index;
        } else {
            break;
        }
    }
    if (it == end) { return RegexParseError{ "Invalid count"_s, index }; }
    const char* end_ptr = it.operator->();
    StringView view{ start_ptr, end_ptr };
    Regex::CountToken tok{};

    if (view.index_of(',') != StringView::npos) {
        auto values = view.split(",");
        if (values.size() != 2) { return RegexParseError{ "Count has more than 2 values"_s, index }; }
        TRY_SET(min, StrViewToU64Decimal(values[0]).map_error([&](auto&& err) {
            return RegexParseError{ err.error_string().str(), index };
        }));
        TRY_SET(max, StrViewToU64Decimal(values[1]).map_error([&](auto&& err) {
            return RegexParseError{ err.error_string().str(), index };
        }));
        tok = Regex::CountToken{ .m_min = min, .m_max = max };
    } else {
        TRY_SET(count, StrViewToU64Decimal(view).map_error([&](auto&& err) {
            return RegexParseError{ err.error_string().str(), index };
        }));
        tok = Regex::CountToken{ .m_min = count, .m_max = count };
    }

    // move it back so the parser can eat the }
    --it;
    --index;
    return tok;
}
Result<Regex, RegexParseError> Regex::parse_regex(String&& regex) {
    Regex::ReTokVector tokens{ new Vector<Regex::RegexVariant> };
    struct RegexState {
        size_t in_group;
        size_t in_square;
        bool in_count;
        bool next_is_escaped;
    };
    auto it             = regex.begin();
    auto end            = regex.end();
    size_t index        = 0;
    size_t group_number = 0;

    RegexState state{};
    Stack<Regex::Group> group_stack{};
    Regex::CharGroup current_chargroup{};

    auto current_tokens = [&]() -> Regex::ReTokVector& {
        if (state.in_square != 0) return current_chargroup.m_char_group;
        return group_stack.size() == 0 ? tokens : group_stack.peek().m_group_regex;
    };

#define CHECK_VALIDITY(b, msg)                                                                                         \
    {                                                                                                                  \
        if (!(b)) { return RegexParseError{ String{ msg }, index }; }                                                  \
    }

    while (it != end) {
        const char cur = *it;
        if (state.next_is_escaped) {
            if (auto fit = find(re_esc_chars, cur); fit != npos_) {
                auto tok = to_enum<EscapedRegexToken>(fit);
                current_tokens()->emplace(tok);
            } else {
                current_tokens()->emplace(cur);
            }
            state.next_is_escaped = false;
        } else {
            if (cur == '\\') {
                state.next_is_escaped = true;
                ++it;
                ++index;
                continue;
            }
            if (auto fit = find(re_tok_chars, cur); fit != npos_) {
                auto tok = to_enum<RegexToken>(fit);
                switch (tok) {
                    case RegexToken::OpenCurly:
                        CHECK_VALIDITY(!state.in_count, "Invalid count");
                        if (state.in_square != 0) {
                            current_tokens()->emplace(cur);
                        } else {
                            state.in_count = true;
                        }
                        break;
                    case RegexToken::CloseCurly:
                        CHECK_VALIDITY(state.in_count, "Unmatched close count group");
                        if (state.in_square != 0) {
                            current_tokens()->emplace(cur);
                        } else {
                            state.in_count = false;
                        }
                        break;
                    case RegexToken::GroupOpen:
                        CHECK_VALIDITY(!state.in_count, "Invalid count");
                        if (state.in_square != 0) {
                            current_tokens()->emplace(cur);
                        } else {
                            state.in_group++;
                            group_stack.push({ .m_group_number = ++group_number,
                                               .m_group_regex  = UniquePtr{ new Vector<Regex::RegexVariant> } });
                        }
                        break;
                    case RegexToken::GroupClose:
                        CHECK_VALIDITY(!state.in_count, "Invalid count");
                        {
                            if (state.in_square != 0) {
                                current_tokens()->emplace(cur);
                            } else {
                                if (state.in_group == 0) {
                                    return RegexParseError{ "Unmatched group parenthesis"_s, index };
                                }
                                Regex::Group group{ move(group_stack.pop()) };
                                current_tokens()->emplace(move(group));
                                state.in_group--;
                            }
                        }
                        break;
                    case RegexToken::SquareOpen:
                        CHECK_VALIDITY(!state.in_count, "Invalid count");
                        current_chargroup.m_char_group = UniquePtr{ new Vector<Regex::RegexVariant> };
                        state.in_square++;
                        break;
                    case RegexToken::SquareClose:
                        CHECK_VALIDITY(!state.in_count, "Invalid count");
                        {
                            if (state.in_square == 0) {
                                return RegexParseError{ "Unmatched square parenthesis"_s, index };
                            }
                            state.in_square--;
                            current_tokens()->emplace(move(current_chargroup));
                        }
                        break;

                    default:
                        current_tokens()->emplace(tok);
                        break;
                }
            } else {
                if (state.in_count) {
                    // attempt at parsing {1,2}
                    TRY_SET(count_token, parse_count_token(it, end, index));
                    current_tokens()->emplace(count_token);
                } else {
                    current_tokens()->emplace(cur);
                }
            }
        }

        ++index;
        ++it;
    }
    if (group_stack.size() != 0) return RegexParseError{ "Unclosed group at end"_s, index };
    if (state.in_group != 0 || state.in_square != 0)
        return RegexParseError{ "Unclosed group or character group"_s, index };
    REGEX_DEBUG("PARSED_REGEX_TOKENS: {}", tokens);
    return Regex{ move(tokens) };
}
}    // namespace ARLib