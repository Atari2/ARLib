#pragma once
#include "CharConv.h"
#include "Concepts.h"
#include "Optional.h"
#include "Pair.h"
#include "RefBox.h"
#include "Result.h"
#include "String.h"
#include "StringView.h"
#include "Variant.h"
#include "Vector.h"

namespace ARLib {

    struct NoValueTag {};

    using StringRef = RefBox<String>;
    using BoolRef = RefBox<bool>;
    using IntRef = RefBox<int>;

    template <typename T>
    concept OptionType = SameAs<T, IntRef> || SameAs<T, StringRef> || SameAs<T, NoValueTag> || SameAs<T, BoolRef>;

    class ArgParser {
        Vector<String> m_unmatched_arguments{};
        String m_program_name;
        Vector<StringView> m_arguments{};

        using ArgIter = decltype(m_arguments.begin());
        struct Option {
            enum class Type { String, Bool, Int, NoValue } type;
            StringView description;
            StringView value_name;

            Variant<NoValueTag, BoolRef, StringRef, IntRef> value;
            bool found;
            static constexpr inline size_t npos = static_cast<size_t>(-1);

            static constexpr inline Type map_t_to_type(const OptionType auto& val) {
                using T = RemoveCvRefT<decltype(val)>;
                if constexpr (SameAs<T, StringRef>) {
                    return Type::String;
                } else if constexpr (SameAs<T, BoolRef>) {
                    return Type::Bool;
                } else if constexpr (SameAs<T, IntRef>) {
                    return Type::Int;
                } else if constexpr (SameAs<T, NoValueTag>) {
                    return Type::NoValue;
                }
            }
            Option() : type{Type::NoValue}, description{}, value_name{}, value(NoValueTag{}), found{false} {}
            Option(StringView desc, StringView name, OptionType auto&& val) :
                type{map_t_to_type(val)}, description{desc}, value_name{name}, found{false} {
                value = move(val);
            }
            bool requires_value() const;
            bool assign(StringView arg_value);
            bool assign(bool arg_value);
            bool assign(int arg_value);
            bool has_default() const;
        };

        using OptT = Pair<StringView, Option>;
        Vector<OptT> m_options{};
        bool m_help_requested = false;
        uint8_t m_version_partial = 0;
        uint8_t m_version_edition = 0;
        size_t m_leftover_args_needed = 0;
        StringView m_usage_string{};
        mutable String m_help_string{};

        template <typename T>
        static constexpr bool inline dependant_false = false;

        String construct_help_string() const;

        public:
        struct ArgParserError {
            String error;
        };

        struct GetOptionError {
            StringView error;
        };

        template <typename T>
        using GetResult = Result<AddLvalueReferenceT<T>, GetOptionError>;
        using ParseResult = DiscardResult<ArgParserError>;

        static constexpr inline auto no_value = NoValueTag{};
        ArgParser(int argc, const char** argv);
        ArgParser(int argc, char** argv);
        void add_version(uint8_t version_partial, uint8_t version_edition);
        void allow_unmatched(size_t quantity = Option::npos);
        const Vector<String>& unmatched() const { return m_unmatched_arguments; }
        void add_usage_string(StringView usage_string);
        ParseResult parse();
        bool help_requested() const;
        ArgParser& add_option(StringView opt_name, StringView value_name, StringView description, String& value_ref);
        ArgParser& add_option(StringView opt_name, StringView value_name, StringView description, int& value_ref);
        ArgParser& add_option(StringView opt_name, StringView description, bool& value_ref);
        ArgParser& add_option(StringView opt_name, StringView description, NoValueTag);
        void print_help() const;
        const String& help_string() const;

        bool is_present(StringView opt_name) {
            auto it = m_options.find([&](const auto& kvp) { return kvp.first() == opt_name; });
            if (it == m_options.end()) return false;
            const auto& [_, value] = *it;
            return value.found;
        }

        template <typename T>
        requires OptionType<RefBox<RemoveCvRefT<T>>>
        auto get(StringView opt_name) {
            using Tp = RemoveCvRefT<T>;
            auto it = m_options.find([&](const auto& kvp) { return kvp.first() == opt_name; });
            if (it == m_options.end()) return GetResult<Tp>{GetOptionError{"Invalid option"}};
            auto& [_, value] = *it;
            if constexpr (SameAs<Tp, bool>) {
                if (value.type == Option::Type::Bool) {
                    return GetResult<Tp>{value.value.template get<BoolRef>().get()};
                } else if (value.type == Option::Type::NoValue) {
                    return GetResult<Tp>{value.found};
                } else {
                    return GetResult<Tp>{GetOptionError{"Requested type `bool` for option containing int or string"}};
                }
            } else if constexpr (SameAs<Tp, String>) {
                if (value.type == Option::Type::String) {
                    return GetResult<Tp>{value.value.template get<StringRef>().get()};
                } else {
                    return GetResult<Tp>{
                    GetOptionError{"Requested type `string` for option containing int, string or none"}};
                }
            } else if constexpr (SameAs<Tp, int>) {
                if (value.type == Option::Type::Int) {
                    return GetResult<Tp>{value.value.template get<BoolRef>().get()};
                } else {
                    return GetResult<Tp>{
                    GetOptionError{"Requested type `int` for option containing bool, string or none"}};
                }
            } else {
                static_assert(dependant_false<T>, "Invalid get() call");
            }
        }
    };

} // namespace ARLib
