#include "JSONParser.h"

#include "Algorithm.h"
#include "JSONObject.h"
#include "Optional.h"
#include "Pair.h"
namespace ARLib {
namespace JSON {

#define CHK_SIZE(c)                                                                                                    \
    if (current_index >= view.size()) {                                                                                \
        return ParseError{ "Expected " #c " but end of file was reached"_s, current_index };                           \
    }

#define CHK_CURR(c)                                                                                                    \
    CHK_SIZE(c)                                                                                                        \
    if (view[current_index] != c) {                                                                                    \
        return ParseError{ String::formatted("Invalid character, expected '%c' but got '%c'", c, view[current_index]), \
                           current_index };                                                                            \
    }

#define VERIFY_COMMA(c)                                                                                                \
    current_index = skip_whitespace(view, current_index);                                                              \
    if (view[current_index] != ',') {                                                                                  \
        if (view[current_index] != c) {                                                                                \
            return ParseError{ "Invalid character, expected a comma but got "_s + view[current_index],                 \
                               current_index };                                                                        \
        }                                                                                                              \
    } else {                                                                                                           \
        current_index++;                                                                                               \
    }

#define ADD_TO_OBJ_WITH(func)                                                                                          \
    auto value_or_error = func(view, current_index);                                                                   \
    if (value_or_error.is_error()) { return value_or_error.to_error(); }                                               \
    auto [new_index_val, value] = value_or_error.to_ok();                                                              \
    current_index               = skip_whitespace(view, new_index_val);                                                \
    obj.add(move(key), ValueObj::construct(move(value)));

#define ADD_TO_ARR_WITH(func)                                                                                          \
    auto value_or_error = func(view, current_index);                                                                   \
    if (value_or_error.is_error()) { return value_or_error.to_error(); }                                               \
    auto [new_index_val, value] = value_or_error.to_ok();                                                              \
    current_index               = skip_whitespace(view, new_index_val);                                                \
    arr.append(ValueObj::construct(move(value)));
    size_t skip_whitespace(StringView view, size_t current_index) {
        while (isspace(view[current_index++]))
            ;
        return current_index - 1;
    }
    // delimeters are comma, close square parens and close curly.
    Pair<size_t, String> eat_until_space_or_delim(StringView view, size_t current_index) {
        String container{};
        while (!isspace(view[current_index]) && view[current_index] != ',' && view[current_index] != ']' &&
               view[current_index] != '}') {
            container.append(view[current_index++]);
        }
        return Pair{ current_index, container };
    }
    Parsed<JString> parse_quoted_string(StringView view, size_t current_index) {
        CHK_CURR('"')
        auto check_c = [](char c) {
            constexpr char escaped[] = { 'n', 'r', 'v', 't', 'f' };
            constexpr char equiv[]   = { '\n', '\r', '\v', '\t', '\f' };
            auto found               = find(begin(escaped), end(escaped), c);
            if (found != npos_) { return equiv[found]; }
            return c;
        };
        current_index++;
        JString container{};
        bool at_end          = false;
        const auto view_size = view.size();
        while (!at_end && current_index < view_size) {
            const char c = view[current_index];
            if (c == '\\') {
                if (view[current_index + 1] == '\\') {
                    container.append(check_c(view[current_index + 2]));
                    current_index += 3;
                } else {
                    container.append(check_c(view[current_index + 1]));
                    current_index += 2;
                }

            } else if (c == '"') {
                current_index++;
                at_end = true;
            } else {
                container.append(c);
                current_index++;
            }
        }
        if (!at_end) return ParseError{ "Missing end of quotation on string"_s, current_index };
        return Pair{ current_index, container };
    }
    Pair<size_t, String> parse_non_delimited(StringView view, size_t current_index) {
        current_index            = skip_whitespace(view, current_index);
        auto [new_index, string] = eat_until_space_or_delim(view, current_index);
        current_index            = skip_whitespace(view, new_index);
        return Pair{ current_index, string };
    }
    Number parse_number(const String& raw_value) {
        // parse number doesn't handle scientific notation for integer values
        if (raw_value.contains('.') || raw_value.contains(',')) {
            return Number{ number_tag, StrToDouble(raw_value) };
        } else {
            return Number{ number_tag, StrToI64(raw_value) };
        }
    }
    Parsed<Array> parse_array(StringView view, size_t current_index) {
        CHK_CURR('[')
        Array arr{};
        current_index = skip_whitespace(view, current_index + 1);
        while (view[current_index] != ']') {
            current_index = skip_whitespace(view, current_index);
            switch (view[current_index]) {
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
                        auto [new_index_val, raw_value] = parse_non_delimited(view, current_index);
                        current_index                   = new_index_val;
                        if (raw_value == "null"_s) {
                            arr.append(ValueObj::construct(Null{ null_tag }));
                        } else if (raw_value == "true"_s) {
                            arr.append(ValueObj::construct(Bool{ bool_tag, true }));
                        } else if (raw_value == "false"_s) {
                            arr.append(ValueObj::construct(Bool{ bool_tag, false }));
                        } else {
                            arr.append(ValueObj::construct(parse_number(raw_value)));
                        }
                        break;
                    }
            }
            VERIFY_COMMA(']')
        }
        return Pair{ current_index + 1, move(arr) };
    }
    Parsed<Object> parse_object(StringView view, size_t current_index) {
        CHK_CURR('{')
        Object obj{};
        current_index = skip_whitespace(view, current_index + 1);
        while (view[current_index] != '}') {
            // we're in an object

            // parse key:
            current_index     = skip_whitespace(view, current_index);
            auto key_or_error = parse_quoted_string(view, current_index);
            if (key_or_error.is_error()) return key_or_error.to_error();
            auto [new_index, key] = key_or_error.to_ok();

            // parse divisor between key and value
            current_index = skip_whitespace(view, new_index);
            if (view[current_index] != ':')
                return ParseError{ "Invalid character, expected : but got "_s + view[current_index], current_index };
            current_index = skip_whitespace(view, current_index + 1);

            switch (view[current_index]) {
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
                        const ARLib::Array valid_values_for_number{ '0', '1', '2', '3', '4', '5', '6',
                                                                    '7', '8', '9', '-', '+', 'E', 'e' };
                        auto check_if_valid_number = [&](const String& str) {
                            // this is not real validation, it lets invalid values slip throught, but it's good enough for now
                            for (const char c : str) {
                                if (find(valid_values_for_number, c) == npos_) { return false; }
                            }
                            return true;
                        };
                        auto [new_index_val, raw_value] = parse_non_delimited(view, current_index);
                        current_index                   = new_index_val;
                        if (raw_value == "null"_s) {
                            obj.add(move(key), ValueObj::construct(Null{ null_tag }));
                        } else if (raw_value == "true"_s) {
                            obj.add(move(key), ValueObj::construct(Bool{ bool_tag, true }));
                        } else if (raw_value == "false"_s) {
                            obj.add(move(key), ValueObj::construct(Bool{ bool_tag, false }));
                        } else if (check_if_valid_number(raw_value)) {
                            obj.add(move(key), ValueObj::construct(parse_number(raw_value)));
                        } else {
                            return ParseError{ "Expected a valid json type but got "_s + raw_value, current_index };
                        }
                        break;
                    }
            }

            VERIFY_COMMA('}')
        }

        return Pair{ current_index + 1, move(obj) };
    }
    // FIXME: fix indentation
    String dump_array(const Array& arr, size_t indent) {
        String repr{ "[\n" };
        String indent_string{ indent, '\t' };
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array(val.get<Type::JArray>(), indent + 1));
                    break;
                case Type::JObject:
                    repr.append(dump_json(val.get<Type::JObject>(), indent + 1));
                    break;
                case Type::JNumber:
                    repr.append(indent_string + val.get<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append(indent_string + "null"_s);
                    break;
                case Type::JBool:
                    repr.append(indent_string + BoolToStr(val.get<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append(indent_string + "\""_s + val.get<Type::JString>() + '"');
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
        repr.append(indent_string + "]"_s);
        return repr;
    }
    String dump_array_compact(const Array& arr) {
        String repr{ "[" };
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array_compact(val.get<Type::JArray>()));
                    break;
                case Type::JObject:
                    repr.append(dump_json_compact(val.get<Type::JObject>()));
                    break;
                case Type::JNumber:
                    repr.append(val.get<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.get<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + val.get<Type::JString>() + '"');
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
    String dump_json_compact(const Object& obj) {
        String repr{ "{" };
        for (const auto& entry : obj) {
            const auto& val = *entry.value();
            const auto& key = entry.key();
            repr.append("\""_s + key + '"');
            repr.append(":"_s);
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array_compact(val.get<Type::JArray>()));
                    break;
                case Type::JObject:
                    repr.append(dump_json_compact(val.get<Type::JObject>()));
                    break;
                case Type::JNumber:
                    repr.append(val.get<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.get<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + val.get<Type::JString>() + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
        }
        repr.append("}");
        return repr;
    }
    String dump_json(const Object& obj, size_t indent) {
        String indent_string{ indent, '\t' };
        String prev_indent_string{ indent - 1, '\t' };
        String repr{ prev_indent_string + "{\n"_s };
        size_t i = 0;
        for (const auto& entry : obj) {
            const auto& val = *entry.value();
            const auto& key = entry.key();
            repr.append(indent_string);
            repr.append("\""_s + key + '"');
            repr.append(": "_s);
            switch (val.type()) {
                case Type::JArray:
                    repr.append(dump_array(val.get<Type::JArray>(), indent + 1));
                    break;
                case Type::JObject:
                    repr.append(dump_json(val.get<Type::JObject>(), indent + 1));
                    break;
                case Type::JNumber:
                    repr.append(val.get<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    repr.append("null"_s);
                    break;
                case Type::JBool:
                    repr.append(BoolToStr(val.get<Type::JBool>().value()));
                    break;
                case Type::JString:
                    repr.append("\""_s + val.get<Type::JString>() + '"');
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
    Parser::Parser(StringView view) : m_view(view) {}
    ParseResult Parser::parse_internal() {
        size_t current_index = skip_whitespace(m_view, 0);
        auto object_or_error = parse_object(m_view, current_index);
        if (object_or_error.is_error()) return object_or_error.to_error();
        auto [_, obj] = object_or_error.to_ok();
        return ParseResult{ Document{ move(obj) } };
    }
    ParseResult Parser::parse(StringView data) {
        Parser p{ data };
        return p.parse_internal();
    }
    FileParseResult Parser::from_file(StringView filename) {
        File f{ filename.extract_string() };
        auto maybe_error = f.open(OpenFileMode::Read);
        if (maybe_error) { return FileParseResult::from_error(maybe_error.to_error()); }
        auto read_res = f.read_all();
        if (read_res.is_error()) { return FileParseResult::from_error(read_res.to_error()); }
        auto val       = read_res.to_ok();
        auto parse_res = Parser::parse(val.view());
        if (parse_res.is_error()) { return FileParseResult::from_error(parse_res.to_error()); }
        return FileParseResult{ parse_res.to_ok() };
    }
}    // namespace JSON
}    // namespace ARLib
