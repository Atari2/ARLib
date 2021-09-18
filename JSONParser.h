#pragma once
#include "File.h"
#include "JSONObject.h"
#include "PrintInfo.h"
#include "Result.h"
#include "StringView.h"
#include "Variant.h"
#include "Pair.h" 

namespace ARLib {

    namespace JSON {
        // TODO: better errors.

        template <typename T>
        using Parsed = Optional<Pair<size_t, T>>;

        Parsed<Object> parse_object(StringView view, size_t current_index);
        size_t skip_whitespace(StringView view, size_t current_index);
        Pair<size_t, String> eat_until_space(StringView view, size_t current_index);
        Parsed<JString> parse_quoted_string(StringView view, size_t current_index);
        Pair<size_t, String> parse_non_delimited(StringView view, size_t current_index);
        Parsed<Array> parse_array(StringView view, size_t current_index);

        struct ParseError {
            ParseError() = default;
            static constexpr inline StringView error = "JSON String couldn't be parsed correctly"_sv;
        };

        using ParseResult = Result<Object, ParseError>;
        using FileParseResult = Result<Object, Variant<ReadFileError, ParseError>>;

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
            // this is a workaround, as returning String{m_error.error} was bugging out on msvc
            // e.g. size() was 40 at compile time but 88 at runtime
            // was working fine on GCC/Clang
            return "JSON String couldn't be parsed correctly"_s;
        }
    };
} // namespace ARLib