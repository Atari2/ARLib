#pragma once
#include "File.h"
#include "JSONObject.h"
#include "Pair.h"
#include "Result.h"
#include "StringView.h"
#include "Variant.h"
namespace ARLib {
namespace JSON {

    template <typename T>
    using Parsed = Result<Pair<size_t, T>, ParseError>;

    Parsed<Object> parse_object(StringView view, size_t current_index);
    size_t skip_whitespace(StringView view, size_t current_index);
    Pair<size_t, String> eat_until_space_or_delim(StringView view, size_t current_index);
    Parsed<JString> parse_quoted_string(StringView view, size_t current_index);
    Pair<size_t, String> parse_non_delimited(StringView view, size_t current_index);
    Number parse_number(const String& raw_value);
    Parsed<Array> parse_array(StringView view, size_t current_index);
    String dump_array(const Array& arr, size_t indent = 1);
    String dump_json(const Object& obj, size_t indent = 1);
    String dump_array_compact(const Array& arr);
    String dump_json_compact(const Object& obj);
    using ParseResult     = Result<Document, ParseError>;
    using FileParseResult = Result<Document, Variant<OpenFileError, ReadFileError, ParseError>>;

    template <typename T>
    using ParseResultT = Result<T, ParseError>;

    template <typename T>
    using FileParseResultT = Result<T, Variant<OpenFileError, ReadFileError, ParseError>>;

    template <typename T>
    using FileWriteResultT = Result<T, Variant<OpenFileError, WriteFileError, ParseError>>;
    class Parser {
        StringView m_view;

        Parser(StringView);

        ParseResult parse_internal();

        public:
        static ParseResult parse(StringView);
        static FileParseResult from_file(StringView);
    };
    template <typename T>
    concept Serializable = requires(const T& t) {
                               { T::deserialize(declval<StringView>()) } -> SameAs<ParseResultT<T>>;
                               { t.serialize() } -> SameAs<String>;
                           };
    template <Serializable T>
    class Serializer {
        public:
        static FileParseResultT<T> deserialize_from_file(StringView filename) {
            File f{ filename.extract_string() };
            auto maybe_error = f.open(OpenFileMode::Read);
            if (maybe_error) { return FileParseResultT<T>::from_error(maybe_error.extract()); }
            auto read_res = f.read_all();
            if (read_res.is_error()) { return FileParseResultT<T>::from_error(read_res.to_error()); }
            auto val             = read_res.to_ok();
            auto deserialize_res = T::deserialize(val.view());
            if (deserialize_res.is_error()) { return FileParseResultT<T>::from_error(deserialize_res.to_error()); }
            return deserialize_res.to_ok();
        }
        static FileWriteResultT<size_t> serialize_to_file(const T& object, StringView filename) {
            auto result = object.serialize();
            File f{ filename.extract_string() };
            auto maybe_error = f.open(OpenFileMode::Write);
            if (maybe_error) { return FileWriteResultT<size_t>::from_error(maybe_error.extract()); }
            auto write_res = f.write(result);
            if (write_res.is_error()) { return FileWriteResultT<size_t>::from_error(write_res.to_error()); }
            auto val = write_res.to_ok();
            return val;
        }
    };
}    // namespace JSON
inline JSON::Document operator""_json(const char* str, size_t len) {
    StringView view{ str, len };
    auto result = JSON::Parser::parse(view);
    HARD_ASSERT(result.is_ok(), "Parsing failed");
    return result.to_ok();
}
template <>
struct PrintInfo<JSON::Document> {
    const JSON::Document& m_document;
    PrintInfo(const JSON::Document& document) : m_document(document) {}
    String repr() const { return JSON::dump_json(m_document.object()); }
};
}    // namespace ARLib
