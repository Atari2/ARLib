#pragma once
#include "CharConv.hpp"
#include "Concepts.hpp"
#include "Optional.hpp"
#include "Pair.hpp"
#include "RefBox.hpp"
#include "Result.hpp"
#include "String.hpp"
#include "StringView.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
namespace ARLib {
struct NoValueTag {};
using StringRef    = RefBox<String>;
using BoolRef      = RefBox<bool>;
using IntRef       = RefBox<int>;
using UintRef      = RefBox<unsigned int>;
using RealRef      = RefBox<double>;
using StringVecRef = RefBox<Vector<String>>;
using IntVecRef    = RefBox<Vector<int>>;
using UintVecRef   = RefBox<Vector<unsigned int>>;
using RealVecRef   = RefBox<Vector<double>>;

template <typename T>
concept OptionType =
SameAs<T, IntRef> || SameAs<T, UintRef> || SameAs<T, RealRef> || SameAs<T, StringRef> || SameAs<T, NoValueTag> ||
SameAs<T, BoolRef> || SameAs<T, StringVecRef> || SameAs<T, IntVecRef> || SameAs<T, UintVecRef> || SameAs<T, RealVecRef>;
class ArgParser {
    Vector<String> m_unmatched_arguments{};
    String m_program_name;
    Vector<StringView> m_arguments{};

    using ArgIter = decltype(m_arguments.begin());
    struct Option {
        enum class Type {
            String,
            Bool,
            Int,
            Uint,
            Real,
            StringVector,
            IntVector,
            UintVector,
            RealVector,
            NoValue
        } type;
        StringView description;
        StringView value_name;

        Variant<
        NoValueTag, BoolRef, StringRef, IntRef, UintRef, RealRef, StringVecRef, IntVecRef, UintVecRef, RealVecRef>
        value;
        bool found;
        constexpr static inline size_t npos = static_cast<size_t>(-1);
        constexpr static inline Type map_t_to_type(const OptionType auto& val) {
            using T = RemoveCvRefT<decltype(val)>;
            if constexpr (SameAs<T, StringRef>) {
                return Type::String;
            } else if constexpr (SameAs<T, BoolRef>) {
                return Type::Bool;
            } else if constexpr (SameAs<T, IntRef>) {
                return Type::Int;
            } else if constexpr (SameAs<T, UintRef>) {
                return Type::Uint;
            } else if constexpr (SameAs<T, RealRef>) {
                return Type::Real;
            } else if constexpr (SameAs<T, StringVecRef>) {
                return Type::StringVector;
            } else if constexpr (SameAs<T, IntVecRef>) {
                return Type::IntVector;
            } else if constexpr (SameAs<T, UintVecRef>) {
                return Type::UintVector;
            } else if constexpr (SameAs<T, RealVecRef>) {
                return Type::RealVector;
            } else if constexpr (SameAs<T, NoValueTag>) {
                return Type::NoValue;
            }
        }
        Option() : type{ Type::NoValue }, description{}, value_name{}, value(NoValueTag{}), found{ false } {}
        Option(StringView desc, StringView name, OptionType auto&& val) :
            type{ map_t_to_type(val) }, description{ desc }, value_name{ name }, found{ false } {
            value = move(val);
        }
        bool requires_value() const;
        bool assign(StringView arg_value);
        bool assign(bool arg_value);
        bool assign(Vector<String>&& arg_value);
        bool assign(Vector<int>&& arg_value);
        bool assign(Vector<unsigned int>&& arg_value);
        bool assign(Vector<double>&& arg_value);
        bool assign(SignedIntegral auto arg_value) {
            if (type == Type::Int) {
                value.get<IntRef>().get() = static_cast<int>(arg_value);
            } else {
                return false;
            }
            return true;
        }
        bool assign(UnsignedIntegral auto arg_value) {
            if (type == Type::Uint) {
                value.get<UintRef>().get() = static_cast<unsigned int>(arg_value);
            } else {
                return false;
            }
            return true;
        }
        bool assign(FloatingPoint auto arg_value) {
            if (type == Type::Real) {
                value.get<RealRef>().get() = static_cast<double>(arg_value);
            } else {
                return false;
            }
            return true;
        }
        bool has_default() const;
    };
    using OptT = Pair<StringView, Option>;
    Vector<OptT> m_options{};
    bool m_help_requested         = false;
    uint8_t m_version_partial     = 0;
    uint8_t m_version_edition     = 0;
    size_t m_leftover_args_needed = 0;
    StringView m_usage_string{};
    mutable String m_help_string{};

    template <typename T>
    constexpr static inline bool dependant_false = false;

    String construct_help_string() const;

    public:
    template <typename T>
    using GetResult   = Result<AddLvalueReferenceT<T>>;
    using ParseResult = DiscardResult<>;

    constexpr static inline auto no_value = NoValueTag{};
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
    ArgParser& add_option(StringView opt_name, StringView value_name, StringView description, unsigned int& value_ref);
    ArgParser& add_option(StringView opt_name, StringView value_name, StringView description, double& value_ref);
    ArgParser& add_option(StringView opt_name, StringView value_name, StringView description, Vector<int>& value_ref);
    ArgParser&
    add_option(StringView opt_name, StringView value_name, StringView description, Vector<unsigned int>& value_ref);
    ArgParser&
    add_option(StringView opt_name, StringView value_name, StringView description, Vector<String>& value_ref);
    ArgParser&
    add_option(StringView opt_name, StringView value_name, StringView description, Vector<double>& value_ref);
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
        auto it  = m_options.find([&](const auto& kvp) { return kvp.first() == opt_name; });
        if (it == m_options.end()) return GetResult<Tp>{ "Invalid option"_s };
        auto& [_, value] = *it;
        if constexpr (SameAs<Tp, bool>) {
            if (value.type == Option::Type::Bool) {
                return GetResult<Tp>{ value.value.template get<BoolRef>().get() };
            } else if (value.type == Option::Type::NoValue) {
                return GetResult<Tp>{ value.found };
            } else {
                return GetResult<Tp>{ "Requested type `bool` for option containing int or string"_s };
            }
        } else if constexpr (SameAs<Tp, String>) {
            if (value.type == Option::Type::String) {
                return GetResult<Tp>{ value.value.template get<StringRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `string` for option containing int, string or none"_s };
            }
        } else if constexpr (SignedIntegral<Tp>) {
            if (value.type == Option::Type::Int) {
                return GetResult<Tp>{ value.value.template get<IntRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `int` for option containing bool, string or none"_s };
            }
        } else if constexpr (UnsignedIntegral<Tp>) {
            if (value.type == Option::Type::Uint) {
                return GetResult<Tp>{ value.value.template get<UintRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `uint` for option containing bool, string or none"_s };
            }
        } else if constexpr (FloatingPoint<Tp>) {
            if (value.type == Option::Type::Real) {
                return GetResult<Tp>{ value.value.template get<RealRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `real` for option containing bool, string or none"_s };
            }
        } else if constexpr (SameAs<Vector<int>, Tp>) {
            if (value.type == Option::Type::IntVector) {
                return GetResult<Tp>{ value.value.template get<IntVecRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `vector<int>` for option containing bool, string or none"_s };
            }
        } else if constexpr (SameAs<Vector<unsigned int>, Tp>) {
            if (value.type == Option::Type::UintVector) {
                return GetResult<Tp>{ value.value.template get<UintVecRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `vector<unsigned int>` for option containing bool, string or none"_s };
            }
        } else if constexpr (SameAs<Vector<double>, Tp>) {
            if (value.type == Option::Type::RealVector) {
                return GetResult<Tp>{ value.value.template get<RealVecRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `vector<double>` for option containing bool, string or none"_s };
            }
        } else if constexpr (SameAs<Vector<String>, Tp>) {
            if (value.type == Option::Type::StringVector) {
                return GetResult<Tp>{ value.value.template get<StringVecRef>().get() };
            } else {
                return GetResult<Tp>{ "Requested type `vector<string>` for option containing bool, string or none"_s };
            }
        } else {
            static_assert(dependant_false<T>, "Invalid get() call");
        }
    }
};
}    // namespace ARLib
