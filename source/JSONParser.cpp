#include "JSONParser.hpp"

#include "Algorithm.hpp"
#include "JSONObject.hpp"
#include "Optional.hpp"
#include "Pair.hpp"
namespace ARLib {
namespace JSON {

#define STATE_ENTER()                                                                                                  \
    if (!state.enter()) return ParseError {                                                                            \
            "Reached depth limit of "_s + IntToStr(state.depth_limit), state.index()                                   \
        }

#define STATE_EXIT()                                                                                                   \
    if (!state.exit()) return ParseError {                                                                             \
            "Trying to exit from a depth of 0 "_s, state.index()                                                       \
        }

#define CHK_SIZE(c)                                                                                                    \
    if (state.invalid_index()) { return ParseError{ "Expected " #c " but end of file was reached"_s, state.index() }; }

#define CHK_CURR(c)                                                                                                    \
    CHK_SIZE(c)                                                                                                        \
    if (state.current() != c) {                                                                                        \
        return ParseError{ String::formatted("Invalid character, expected " #c " but got '%c'", state.current()),      \
                           state.index() };                                                                            \
    }

#define VERIFY_COMMA(c)                                                                                                \
    skip_whitespace(state);                                                                                            \
    CHK_SIZE(c)                                                                                                        \
    if (auto cur = state.current(); cur != ',') {                                                                      \
        if (cur != c) {                                                                                                \
            return ParseError{ String::formatted("Invalid character, expected ',' or " #c " but got '%c'", cur),       \
                               state.index() };                                                                        \
        }                                                                                                              \
    } else {                                                                                                           \
        state.advance();                                                                                               \
    }

#define ADD_TO_OBJ_WITH(func)                                                                                          \
    auto value_or_error = func(state);                                                                                 \
    if (value_or_error.is_error()) { return value_or_error.to_error(); }                                               \
    auto value = value_or_error.to_ok();                                                                               \
    skip_whitespace(state);                                                                                            \
    obj.insert(move(key), ValueObj::construct(move(value)));

#define ADD_TO_ARR_WITH(func)                                                                                          \
    auto value_or_error = func(state);                                                                                 \
    if (value_or_error.is_error()) { return value_or_error.to_error(); }                                               \
    auto value = value_or_error.to_ok();                                                                               \
    skip_whitespace(state);                                                                                            \
    arr.append(ValueObj::construct(move(value)));
    void skip_whitespace(ParseState& state) {
        while (isspace(state.current())) {
            if (!state.advance_check()) { break; };
        }
    }
    String escape_string(const String& str) {
        return str.replace(R"(")"_sv, R"(\")"_sv);
    }
    // delimeters are comma, close square parens and close curly.
    Parsed<String> eat_until_space_or_delim(ParseState& state) {
        STATE_ENTER();
        String container{};
        while (!state.invalid_index() && !isspace(state.current()) && state.current() != ',' &&
               state.current() != ']' && state.current() != '}') {
            container.append(state.current());
            state.advance();
        }
        STATE_EXIT();
        return container;
    }
    Parsed<JString> parse_quoted_string(ParseState& state) {
        STATE_ENTER();
        CHK_CURR('"')
        auto check_c = [](char c) {
            constexpr char escaped[] = { 'n', 'r', 'v', 't', 'f' };
            constexpr char equiv[]   = { '\n', '\r', '\v', '\t', '\f' };
            auto found               = find(begin(escaped), end(escaped), c);
            if (found != npos_) { return equiv[found]; }
            return c;
        };
        state.advance();
        JString container{};
        bool at_end = false;
        while (!at_end && !state.invalid_index()) {
            const char c = state.current();
            if (c == '\\') {
                if (state.peek(1) == '\\') {
                    container.append(check_c(state.peek(2)));
                    state.advance(3);
                } else {
                    container.append(check_c(state.peek(1)));
                    state.advance(2);
                }

            } else if (c == '"') {
                state.advance();
                at_end = true;
            } else {
                container.append(c);
                state.advance();
            }
        }
        if (!at_end) return ParseError{ "Missing end of quotation on string"_s, state.index() };
        STATE_EXIT();
        return container;
    }
    Parsed<String> parse_non_delimited(ParseState& state) {
        STATE_ENTER();
        skip_whitespace(state);
        auto result = eat_until_space_or_delim(state);
        if (result.is_error()) return result.to_error();
        auto string = result.to_ok();
        skip_whitespace(state);
        STATE_EXIT();
        return string;
    }
    Parsed<Number> parse_number(const String& raw_value) {
        if (raw_value.contains('.') || raw_value.contains('E') || raw_value.contains('e')) {
            if (auto res = StrToDouble(raw_value); res.is_error())
                return ParseError{ res.to_error()->error_string(), 0 };
            else
                return res.to_ok();
        } else {
            if (auto res = StrToI64(raw_value); res.is_error())
                return ParseError{ res.to_error()->error_string(), 0 };
            else
                return res.to_ok();
        }
    }
    Parsed<Array> parse_array(ParseState& state) {
        STATE_ENTER();
        CHK_CURR('[') Array arr{};
        state.advance();
        skip_whitespace(state);
        while (state.current() != ']') {
            skip_whitespace(state);
            switch (state.current()) {
                case '{':
                    {
                        ADD_TO_ARR_WITH(parse_object);
                        break;
                    }
                case '[':
                    {
                        ADD_TO_ARR_WITH(parse_array);
                        break;
                    }
                case '"':
                    {
                        ADD_TO_ARR_WITH(parse_quoted_string);
                        break;
                    }
                default:
                    {
                        auto result = parse_non_delimited(state);
                        if (result.is_error()) return result.to_error();
                        auto raw_value = result.to_ok();
                        if (raw_value == "null"_s) {
                            arr.append(ValueObj::construct(Null{ null_tag }));
                        } else if (raw_value == "true"_s) {
                            arr.append(ValueObj::construct(Bool{ bool_tag, true }));
                        } else if (raw_value == "false"_s) {
                            arr.append(ValueObj::construct(Bool{ bool_tag, false }));
                        } else {
                            TRY_SET(number, parse_number(raw_value));
                            arr.append(ValueObj::construct(move(number)));
                        }
                        break;
                    }
            }
            VERIFY_COMMA(']')
        }
        state.advance();
        STATE_EXIT();
        return arr;
    }
    Parsed<Object> parse_object(ParseState& state) {
        STATE_ENTER();
        CHK_CURR('{')
        Object obj{};
        state.advance();
        skip_whitespace(state);
        while (state.current() != '}') {
            // we're in an object

            // parse key:
            skip_whitespace(state);
            auto key_or_error = parse_quoted_string(state);
            if (key_or_error.is_error()) return key_or_error.to_error();
            auto key = key_or_error.to_ok();
            // parse divisor between key and value
            skip_whitespace(state);
            if (state.current() != ':')
                return ParseError{ "Invalid character, expected : but got "_s + state.current(), state.index() };
            state.advance();
            skip_whitespace(state);

            switch (state.current()) {
                case '[':
                    {
                        ADD_TO_OBJ_WITH(parse_array)
                        break;
                    }
                case '"':
                    {
                        ADD_TO_OBJ_WITH(parse_quoted_string)
                        break;
                    }
                case '{':
                    {
                        ADD_TO_OBJ_WITH(parse_object)
                        break;
                    }
                default:
                    {
                        const ARLib::Array valid_values_for_number{ '0', '1', '2', '3', '4', '5', '6', '7',
                                                                    '8', '9', '-', '+', 'E', 'e', '.' };
                        auto check_if_valid_number = [&](const String& str) {
                            // this is not real validation, it lets invalid values slip throught, but it's good enough for now
                            for (const char c : str) {
                                if (find(valid_values_for_number, c) == npos_) { return false; }
                            }
                            return true;
                        };
                        auto result = parse_non_delimited(state);
                        if (result.is_error()) return result.to_error();
                        auto raw_value = result.to_ok();
                        if (raw_value == "null"_s) {
                            obj.insert(move(key), ValueObj::construct(Null{ null_tag }));
                        } else if (raw_value == "true"_s) {
                            obj.insert(move(key), ValueObj::construct(Bool{ bool_tag, true }));
                        } else if (raw_value == "false"_s) {
                            obj.insert(move(key), ValueObj::construct(Bool{ bool_tag, false }));
                        } else if (check_if_valid_number(raw_value)) {
                            TRY_SET(value, parse_number(raw_value));
                            obj.insert(move(key), ValueObj::construct(move(value)));
                        } else {
                            return ParseError{ "Expected a valid json type but got "_s + raw_value, state.index() };
                        }
                        break;
                    }
            }

            VERIFY_COMMA('}')
        }
        state.advance();
        STATE_EXIT();
        return obj;
    }
    // FIXME: fix indentation
    String dump_array(const Array& arr, size_t indent) {
        if (arr.size() == 0) return "[]"_s;
        String prev_indent_string{ indent - 1, '\t' };
        String indent_string{ indent, '\t' };
        String repr{ prev_indent_string + "[\n" };
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array(val.as<Type::JArray>(), indent + 1));
                    break;
                case Type::JObject:
                    repr.append(dump_object(val.as<Type::JObject>(), indent + 1));
                    break;
                case Type::JNumber:
                    repr.append(indent_string + val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append(indent_string + "null"_s);
                    break;
                case Type::JBool:
                    repr.append(indent_string + BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append(indent_string + "\""_s + escape_string(val.as<Type::JString>()) + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < arr.size()) {
                repr.append(",\n");
            } else {
                repr.append('\n');
            }
        }
        repr.append(prev_indent_string + "]"_s);
        return repr;
    }
    String dump_array_compact(const Array& arr) {
        String repr{ "[" };
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array_compact(val.as<Type::JArray>()));
                    break;
                case Type::JObject:
                    repr.append(dump_object_compact(val.as<Type::JObject>()));
                    break;
                case Type::JNumber:
                    repr.append(val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + escape_string(val.as<Type::JString>()) + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < arr.size()) { repr.append(","); }
        }
        repr.append(']');
        return repr;
    }
    String dump_object_compact(const Object& obj) {
        String repr{ "{" };
        for (const auto& entry : obj) {
            const auto& val = *entry.val();
            const auto& key = entry.key();
            repr.append("\""_s + escape_string(key) + '"');
            repr.append(":"_s);
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array_compact(val.as<Type::JArray>()));
                    break;
                case Type::JObject:
                    repr.append(dump_object_compact(val.as<Type::JObject>()));
                    break;
                case Type::JNumber:
                    repr.append(val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + escape_string(val.as<Type::JString>()) + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
        }
        repr.append("}");
        return repr;
    }
    String dump_object(const Object& obj, size_t indent) {
        if (obj.size() == 0) return "{}"_s;
        String indent_string{ indent, '\t' };
        String prev_indent_string{ indent - 1, '\t' };
        String repr{ prev_indent_string + "{\n"_s };
        size_t i = 0;
        for (const auto& entry : obj) {
            const auto& val = *entry.val();
            const auto& key = entry.key();
            repr.append(indent_string);
            repr.append("\""_s + escape_string(key) + '"');
            repr.append(": "_s);
            switch (val.type()) {
                case Type::JArray:
                    {
                        String arrrepr = dump_array(val.as<Type::JArray>(), indent + 1);
                        arrrepr.iltrim();
                        repr.append(move(arrrepr));
                    }
                    break;
                case Type::JObject:
                    {
                        String objrepr = dump_object(val.as<Type::JObject>(), indent + 1);
                        objrepr.iltrim();
                        repr.append(move(objrepr));
                    }
                    break;
                case Type::JNumber:
                    repr.append(val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + escape_string(val.as<Type::JString>()) + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < obj.size()) {
                repr.append(",\n");
            } else {
                repr.append('\n');
            }
        }
        repr.append(prev_indent_string + "}"_s);
        return repr;
    }
    String dump_json(const ValueObj& val, size_t index) {
        switch (val.type()) {
            case JSON::Type::JArray:
                return dump_array(val.as<Type::JArray>(), index);
            case JSON::Type::JBool:
                return BoolToStr(val.as<Type::JBool>().value());
            case JSON::Type::JNull:
                return "null"_s;
            case JSON::Type::JNumber:
                return val.as<Type::JNumber>().to_string();
            case JSON::Type::JObject:
                return dump_object(val.as<Type::JObject>(), index);
            case JSON::Type::JString:
                return "\""_s + escape_string(val.as<Type::JString>()) + "\""_s;
        }
        return "{}"_s;
    }
    String dump_json_compact(const ValueObj& val) {
        switch (val.type()) {
            case JSON::Type::JArray:
                return dump_array_compact(val.as<Type::JArray>());
            case JSON::Type::JBool:
                return BoolToStr(val.as<Type::JBool>().value());
            case JSON::Type::JNull:
                return "null"_s;
            case JSON::Type::JNumber:
                return val.as<Type::JNumber>().to_string();
            case JSON::Type::JObject:
                return dump_object_compact(val.as<Type::JObject>());
            case JSON::Type::JString:
                return "\""_s + escape_string(val.as<Type::JString>()) + "\""_s;
        }
        return "{}"_s;
    }
#define CHECK_STATE_AT_END()                                                                                           \
    skip_whitespace(state);                                                                                            \
    if (!state.at_end()) { return ParseError{ "End of json reached but end of buffer not reached"_s, state.index() }; }
    Parser::Parser(StringView view) : m_view(view) {}
    ParseResult Parser::parse_internal() {
        ParseState state{ m_view };
        skip_whitespace(state);
        auto c = state.current();
        switch (c) {
            case '{':
                {
                    TRY_SET(obj, parse_object(state));
                    CHECK_STATE_AT_END();
                    return ParseResult{ Document{ move(obj) } };
                }
            case '[':
                {
                    TRY_SET(arr, parse_array(state));
                    CHECK_STATE_AT_END();
                    return ParseResult{ Document{ move(arr) } };
                }
            case '"':
                {
                    TRY_SET(str, parse_quoted_string(state));
                    CHECK_STATE_AT_END();
                    return ParseResult{ Document{ move(str) } };
                }
            default:
                {
                    const ARLib::Array valid_values_for_number{ '0', '1', '2', '3', '4', '5', '6', '7',
                                                                '8', '9', '-', '+', 'E', 'e', '.' };
                    auto check_if_valid_number = [&](const String& str) {
                        // this is not real validation, it lets invalid values slip throught, but it's good enough for now
                        for (const char ch : str) {
                            if (find(valid_values_for_number, ch) == npos_) { return false; }
                        }
                        return true;
                    };
                    auto result = parse_non_delimited(state);
                    CHECK_STATE_AT_END();
                    if (result.is_error()) return result.to_error();
                    auto raw_value = result.to_ok();
                    if (raw_value == "null"_s) {
                        return ParseResult{ Document{ Null{ null_tag } } };
                    } else if (raw_value == "true"_s) {
                        return ParseResult{ Document{ Bool{ bool_tag, true } } };
                    } else if (raw_value == "false"_s) {
                        return ParseResult{ Document{ Bool{ bool_tag, false } } };
                    } else if (check_if_valid_number(raw_value)) {
                        TRY_SET(value, parse_number(raw_value));
                        return ParseResult{ Document{ ValueObj::construct(move(value)) } };
                    } else {
                        return ParseError{ "Expected a valid json type but got "_s + raw_value, state.index() };
                    }
                }
        }
        return ParseError{ "Expected a valid json type but got "_s + c, state.index() };
    }
    ParseResult Parser::parse(StringView data) {
        Parser p{ data };
        return p.parse_internal();
    }
    ParseResult Parser::from_file(const Path& filename) {
        File f{ filename };
        if (auto err = f.open(OpenFileMode::Read); err.is_error()) {
            return ParseError{ err.to_error()->error_string(), 0 };
        }
        auto val_or_err = f.read_all();
        if (val_or_err.is_error()) { return ParseError{ val_or_err.to_error()->error_string(), 0 };
        }
        auto val = val_or_err.to_ok();
        TRY_RET(Parser::parse(val.view()));
    }
}    // namespace JSON
}    // namespace ARLib
