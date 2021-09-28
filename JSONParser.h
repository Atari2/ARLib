#pragma once
#include "File.h"
#include "JSONObject.h"
#include "Pair.h"
#include "PrintInfo.h"
#include "Result.h"
#include "StringView.h"
#include "Variant.h"

namespace ARLib {

    namespace JSON {

        class ParseError;

        template <typename T>
        using Parsed = Result<Pair<size_t, T>, ParseError>;

        Parsed<Object> parse_object(StringView view, size_t current_index);
        size_t skip_whitespace(StringView view, size_t current_index);
        Pair<size_t, String> eat_until_space_or_delim(StringView view, size_t current_index);
        Parsed<JString> parse_quoted_string(StringView view, size_t current_index);
        Pair<size_t, String> parse_non_delimited(StringView view, size_t current_index);
        Parsed<Array> parse_array(StringView view, size_t current_index);
        String dump_array(const Array& arr, size_t indent = 1);
        String dump_json(const Object& obj, size_t indent = 1);

        struct ErrorInfo {
            String error_string{};
            size_t error_offset{};
        };

        class ParseError {
            ErrorInfo m_info;

            public:
            ParseError() = default;
            ParseError(String error, size_t offset) : m_info{move(error), offset} {};
            ParseError(ErrorInfo info) : m_info(move(info)){};
            const ErrorInfo& info() const { return m_info; }
            const String& message() const { return m_info.error_string; }
            size_t offset() const { return m_info.error_offset; }
        };

        using ParseResult = Result<Document, ParseError>;
        using FileParseResult = Result<Document, Variant<ReadFileError, ParseError>>;

        class Parser {
            StringView m_view;

            Parser(StringView);

            ParseResult parse_internal();

            public:
            static ParseResult parse(StringView);
            static FileParseResult from_file(StringView);
        };
    } // namespace JSON

    template <>
    struct PrintInfo<JSON::ParseError> {
        const JSON::ParseError& m_error;
        explicit PrintInfo(const JSON::ParseError& error) : m_error(error) {}
        String repr() const {
            return "Error encountered while parsing json: "_s + m_error.message() + " at offset "_s +
                   IntToStr(m_error.offset());
        }
    };

    template <>
    struct PrintInfo<JSON::Document> {
        const JSON::Document& m_document;
        PrintInfo(const JSON::Document& document) : m_document(document) {}
        String repr() const { return JSON::dump_json(m_document.object()); }
    };
} // namespace ARLib