#pragma once
#include "File.hpp"
#include "JSONObject.hpp"
#include "Pair.hpp"
#include "Result.hpp"
#include "StringView.hpp"
#include "Variant.hpp"
namespace ARLib {
namespace JSON {

    template <typename T>
    using Parsed = Result<T, ParseError>;
    class ParseState {
        StringView view;
        size_t current_index;
        size_t depth;
        public:
        constexpr static size_t depth_limit = 1000;
        constexpr ParseState(StringView v) : view(v), current_index(0), depth(0) {}
        constexpr bool enter() {
            if (depth == depth_limit) return false;
            depth++;
            return true;
        }
        constexpr bool exit() {
            if (depth == 0) return false;
            depth--;
            return true;
        }
        constexpr char current() const { return view[current_index]; }
        constexpr char peek(size_t off) const { return view[current_index + off]; }
        constexpr bool invalid_index() const { return current_index >= view.size(); }
        constexpr void advance(size_t off = 1) { current_index += off; }
        constexpr bool advance_check(size_t off = 1) { return (current_index += off) < view.size(); }
        constexpr size_t index() const { return current_index; }
        constexpr bool at_end() const { return current_index == view.size(); }
    };
    String escape_string(const String& str);
    Parsed<Object> parse_object(ParseState& state);
    void skip_whitespace(ParseState& state);
    Parsed<String> eat_until_space_or_delim(ParseState& state);
    Parsed<JString> parse_quoted_string(ParseState& state);
    Parsed<String> parse_non_delimited(ParseState& state);
    Parsed<Number> parse_number(const String& raw_value);
    Parsed<Array> parse_array(ParseState& state);
    String dump_array(const Array& arr, size_t indent = 1);
    String dump_object(const Object& obj, size_t indent = 1);
    String dump_array_compact(const Array& arr);
    String dump_object_compact(const Object& obj);
    String dump_json(const ValueObj& val, size_t index = 1);
    String dump_json_compact(const ValueObj& val);
    using ParseResult = Result<Document, ParseError>;

    class Parser {
        StringView m_view;

        Parser(StringView);

        ParseResult parse_internal();

        public:
        static ParseResult parse(StringView);
        static ParseResult from_file(const Path&);
    };
    template <typename T>
    concept Serializable = requires(const T& t) {
                               { T::deserialize(declval<StringView>()) } -> SameAs<Parsed<T>>;
                               { t.serialize() } -> SameAs<String>;
                           };
    template <Serializable T>
    class Serializer {
        public:
        static Parsed<T> deserialize_from_file(const Path& filename) {
            File f{ filename };
            auto maybe_error = f.open(OpenFileMode::Read);
            if (maybe_error.is_error()) { return maybe_error.to_error(); }
            auto read_res = f.read_all();
            if (read_res.is_error()) { return read_res.to_error(); }
            auto val             = read_res.to_ok();
            auto deserialize_res = T::deserialize(val.view());
            if (deserialize_res.is_error()) { return deserialize_res.to_error(); }
            return deserialize_res.to_ok();
        }
        static Parsed<size_t> serialize_to_file(const T& object, const Path& filename) {
            auto result = object.serialize();
            File f{ filename };
            auto maybe_error = f.open(OpenFileMode::Write);
            if (maybe_error.is_error()) {
                return ParseError{ maybe_error.to_error()->error_string(), 0 };
            }
            auto write_res = f.write(result);
            if (write_res.is_error()) { return write_res; }
            auto val = write_res.to_ok();
            return val;
        }
    };
}    // namespace JSON
inline JSON::Document operator""_json(const char* str, size_t len) {
    StringView view{ str, len };
    auto val = MUST(JSON::Parser::parse(view));
    return val;
}
template <>
struct PrintInfo<JSON::Document> {
    const JSON::Document& m_document;
    PrintInfo(const JSON::Document& document) : m_document(document) {}
    String repr() const { return JSON::dump_json(m_document.root()); }
};
}    // namespace ARLib
