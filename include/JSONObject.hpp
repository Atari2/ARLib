#pragma once

#include "CharConv.hpp"
#include "FlatMap.hpp"
#include "Map.hpp"
#include "PrintInfo.hpp"
#include "String.hpp"
#include "UniquePtr.hpp"
#include "Variant.hpp"
#include "Result.hpp"
#include "Path.hpp"
#include "File.hpp"

namespace ARLib {
namespace JSON {

    // FWD
    class Parser;

    // types
    class ValueObj;
    struct Value : public UniquePtr<ValueObj> {
        Value(ValueObj&& obj);
        Value(const Value& other);
        Value& operator=(const Value& other);
        Value(Value&& other) noexcept            = default;
        Value& operator=(Value&& other) noexcept = default;
        Value deepcopy() const;
    };
    struct Object : public FlatMap<String, Value> {
        using Parent = FlatMap<String, Value>;

        operator Value() &&;
        ValueObj& operator[](const String& key);
        const ValueObj& operator[](const String& key) const { return *(static_cast<const Parent&>(*this))[key]; }
    };
    struct Array : public Vector<Value> {
        using Parent = Vector<Value>;

        Array()                       = default;
        Array(Array&& other) noexcept = default;
        Array(const Array& other);
        Array& operator=(const Array& other);
        Array& operator=(Array&& other) noexcept = default;

        operator Value() &&;
        ValueObj& operator[](size_t index) { return *(static_cast<Parent*>(this)->operator[](index)); }
        const ValueObj& operator[](size_t index) const {
            return *(static_cast<const Parent*>(this)->operator[](index));
        }
    };
    struct JString : public String {
        JString() = default;
        JString(String&& other) noexcept : String(Forward<String>(other)) {}
        JString(const String& other) : String(other) {}
        operator Value() &&;
    };
    // Null detail impl
    namespace detail {
        struct NullTag {};
        struct BoolTag {};
        struct NumberTag {};
    }    // namespace detail
    constexpr inline auto null_tag   = detail::NullTag{};
    constexpr inline auto bool_tag   = detail::BoolTag{};
    constexpr inline auto number_tag = detail::NumberTag{};
    struct Null {
        constexpr Null(detail::NullTag) {}
        constexpr Null(nullptr_t) {}
        operator Value() &&;
        operator nullptr_t() const;
        constexpr bool operator==(nullptr_t) const { return true; }
    };
    class Bool {
        bool m_value;

        public:
        constexpr Bool(bool val) : m_value(val) {}
        constexpr Bool(detail::BoolTag, bool val) : m_value(val) {}
        constexpr bool value() const { return m_value; }
        operator bool() const { return m_value; }
        operator Value() &&;
    };
    class Number {
        union {
            double m_double_value;
            int64_t m_int_value;
        };
        enum class NumberType { Double, Integer } m_type;

        public:
        constexpr Number(double val) : m_double_value(val), m_type(NumberType::Double) {}
        constexpr Number(int64_t val) : m_int_value(val), m_type(NumberType::Integer) {}
        constexpr Number(int val) : m_int_value(val), m_type(NumberType::Integer) {}
        constexpr Number(detail::NumberTag, double val) : m_double_value(val), m_type(NumberType::Double) {}
        constexpr Number(detail::NumberTag, int64_t val) : m_int_value(val), m_type(NumberType::Integer) {}
        constexpr Number(detail::NumberTag, int val) : m_int_value(val), m_type(NumberType::Integer) {}
        constexpr double value_double() const {
            HARD_ASSERT(m_type == NumberType::Double, "Type must be double when this is called");
            return m_double_value;
        }
        constexpr int64_t value_integer() const {
            HARD_ASSERT(m_type == NumberType::Integer, "Type must be integer when this is called");
            return m_int_value;
        }
        operator double() const {
            HARD_ASSERT(m_type == NumberType::Double, "Type must be double when this is called");
            return m_double_value;
        }
        operator int64_t() const {
            HARD_ASSERT(m_type == NumberType::Integer, "Type must be integer when this is called");
            return m_int_value;
        }
        operator int() const {
            HARD_ASSERT(m_type == NumberType::Integer, "Type must be integer when this is called");
            return static_cast<int>(m_int_value);
        }
        operator Value() &&;
        String to_string() const {
            if (m_type == NumberType::Integer) {
                return IntToStr(m_int_value);
            } else {
                return DoubleToStr(m_double_value);
            }
        }
        bool operator==(int value) const {
            if (m_type == NumberType::Integer) {
                return m_int_value == value;
            } else {
                return false;
            }
        }
        bool operator==(int64_t value) const {
            if (m_type == NumberType::Integer) {
                return m_int_value == value;
            } else {
                return false;
            }
        }
        bool operator==(double value) const {
            if (m_type == NumberType::Double) {
                return m_double_value == value;
            } else {
                return false;
            }
        }
    };
    enum class Type : uint8_t;
}    // namespace JSON
MAKE_FANCY_ENUM(JSON::Type, uint8_t, JObject, JString, JNumber, JArray, JBool, JNull);
namespace JSON {
    template <typename Tp>
    concept JSONType = IsAnyOfV<Tp, Object, JString, Number, Array, Bool, Null>;

    template <typename Tp>
    concept JSONTypeExt =
    IsAnyOfV<Tp, Object, JString, Number, Array, Bool, Null, bool, double, int, int64_t, String, nullptr_t>;
    template <JSONType T>
    constexpr Type enum_from_type() {
        if constexpr (IsSameV<T, Object>) {
            return Type::JObject;
        } else if constexpr (IsSameV<T, JString>) {
            return Type::JString;
        } else if constexpr (IsSameV<T, Number>) {
            return Type::JNumber;
        } else if constexpr (IsSameV<T, Array>) {
            return Type::JArray;
        } else if constexpr (IsSameV<T, Bool>) {
            return Type::JBool;
        } else if constexpr (IsSameV<T, Null>) {
            return Type::JNull;
        } else {
            COMPTIME_ASSERT("Invalid type to construct a JSON Value from")
        }
    }
    class ConversionError : public ErrorBase {
        String m_info;

        public:
        ConversionError() = default;
        ConversionError(Type to, Type actual) :
            m_info{ "Invalid JSON conversion attempted, tried to convert a "_s + enum_to_str(actual) + " to a " +
                    enum_to_str(to) } {};
        const String& message() const { return m_info; }
        const String& error_string() const { return m_info; }
    };
    using SerializeResult = DiscardResult<FileError>;
    class ValueObj {
        friend Parser;
        friend PrintInfo<ValueObj>;

        using JSONVariant = Variant<Object, JString, Number, Array, Bool, Null>;

        using JSONTypeArray = TypeArray<Object, JString, Number, Array, Bool, Null>;

        JSONVariant m_internal_value{ null_tag };
        Type m_type{ Type::JNull };
        template <JSONType T>
        ValueObj(T&& value, Type type) : m_internal_value(move(value)), m_type(type) {}
        template <JSONTypeExt T>
        static consteval Type map_t_to_enum() {
            if constexpr (SameAs<T, Object>) {
                return Type::JObject;
            } else if constexpr (IsAnyOfV<T, JString, String>) {
                return Type::JString;
            } else if constexpr (IsAnyOfV<T, Number, double, int, int64_t>) {
                return Type::JNumber;
            } else if constexpr (SameAs<T, Array>) {
                return Type::JArray;
            } else if constexpr (IsAnyOfV<T, Bool, bool>) {
                return Type::JBool;
            } else if constexpr (IsAnyOfV<T, Null, nullptr_t>) {
                return Type::JNull;
            } else {
                COMPTIME_ASSERT("Invalid enum value passed to JSON operator=");
            }
        }
        template <JSONTypeExt T>
        constexpr static auto map_type_to_json_type() {
            if constexpr (SameAs<T, String>) {
                JString* ptr{ nullptr };
                return *ptr;
            } else if constexpr (IsAnyOfV<T, double, int, int64_t>) {
                Number* ptr{ nullptr };
                return *ptr;
            } else if constexpr (SameAs<T, bool>) {
                Bool* ptr{ nullptr };
                return *ptr;
            } else if constexpr (SameAs<T, nullptr_t>) {
                Null* ptr{ nullptr };
                return *ptr;
            } else {
                COMPTIME_ASSERT("Invalid value passed to map_type_to_json_type");
            }
        }

        public:
        ValueObj(const ValueObj&)                = default;
        ValueObj(ValueObj&&) noexcept            = default;
        ValueObj& operator=(const ValueObj&)     = default;
        ValueObj& operator=(ValueObj&&) noexcept = default;
        template <JSONTypeExt T>
        static Value construct(T&& value) {
            if constexpr (IsAnyOfV<T, Object, JString, Number, Array, Bool, Null>) {
                return Value{
                    ValueObj{Forward<T>(value), enum_from_type<T>()}
                };
            } else {
                using ActualJSONT = RemoveCvRefT<decltype(map_type_to_json_type<T>())>;
                return Value{
                    ValueObj{ActualJSONT{ Forward<T>(value) }, map_t_to_enum<T>()}
                };
            }
        }
        SerializeResult serialize_to_file(File& f) const;
        Type type() const { return m_type; }
        template <JSONTypeExt T>
        ValueObj& operator=(const T& value) {
            m_internal_value = value;
            m_type           = map_t_to_enum<T>();
            return *this;
        }
        template <JSONTypeExt T>
        ValueObj& operator=(T&& value) {
            m_internal_value = move(value);
            m_type           = map_t_to_enum<T>();
            return *this;
        }
        template <JSONTypeExt T>
        bool operator==(const T& value) const {
            constexpr auto val = map_t_to_enum<T>();
            if (val != m_type) return false;
            return as<val>() == value;
        }
        template <JSONTypeExt T>
        operator T() const {
            constexpr auto val = map_t_to_enum<T>();
            if (val != m_type) ASSERT_NOT_REACHED("Invalid type requested");
            return as<val>();
        }
        template <JSONTypeExt T>
        ConditionalT<JSONType<T>, const T&, T> as() const {
            constexpr auto val = map_t_to_enum<T>();
            if constexpr (!JSONType<T>) { return static_cast<T>(as<val>()); }
            return static_cast<const T&>(as<val>());
        }
        template <JSONTypeExt T>
        ConditionalT<JSONType<T>, const T&, T> as() {
            constexpr auto val = map_t_to_enum<T>();
            if constexpr (!JSONType<T>) { return static_cast<T>(as<val>()); }
            return static_cast<const T&>(as<val>());
        }
        template <JSONTypeExt T, bool IsConst>
        constexpr static auto& _try_as_return_value() {
            if constexpr (JSONType<T>) {
                using Ret = Result<
                ConditionalT<IsConst, AddConstT<AddLvalueReferenceT<T>>, AddLvalueReferenceT<T>>, ConversionError>;
                Ret* ptr{ nullptr };
                return *ptr;
            } else {
                using Ret = Result<T, ConversionError>;
                Ret* ptr{ nullptr };
                return *ptr;
            }
        }
        template <JSONTypeExt T, bool IsConst>
        using TryAsRet = RemoveCvRefT<decltype(ValueObj::_try_as_return_value<T, IsConst>())>;
        template <JSONTypeExt T>
        TryAsRet<T, true> try_as() const {
            constexpr auto val = map_t_to_enum<T>();
            auto t             = try_as<val>();
            if constexpr (JSONType<T>) {
                return t;
            } else {
                return t.map<T>();
            }
        }
        template <JSONTypeExt T>
        TryAsRet<T, false> try_as() {
            constexpr auto val = map_t_to_enum<T>();
            auto t             = try_as<val>();
            if constexpr (JSONType<T>) {
                return t;
            } else {
                return t.map<T>();
            }
        }
        template <Type T>
        const auto& as() const {
            // Value, Object, String, Number, Array, Bool, Null
            if constexpr (T == Type::JObject) {
                return ARLib::get<Object>(m_internal_value);
            } else if constexpr (T == Type::JString) {
                return ARLib::get<JString>(m_internal_value);
            } else if constexpr (T == Type::JNumber) {
                return ARLib::get<Number>(m_internal_value);
            } else if constexpr (T == Type::JArray) {
                return ARLib::get<Array>(m_internal_value);
            } else if constexpr (T == Type::JBool) {
                return ARLib::get<Bool>(m_internal_value);
            } else if constexpr (T == Type::JNull) {
                return ARLib::get<Null>(m_internal_value);
            } else {
                COMPTIME_ASSERT("Invalid enum value passed to JSON:Value::get<T>");
            }
        }
        template <Type T>
        auto& as() {
            // Value, Object, String, Number, Array, Bool, Null
            if constexpr (T == Type::JObject) {
                return ARLib::get<Object>(m_internal_value);
            } else if constexpr (T == Type::JString) {
                return ARLib::get<JString>(m_internal_value);
            } else if constexpr (T == Type::JNumber) {
                return ARLib::get<Number>(m_internal_value);
            } else if constexpr (T == Type::JArray) {
                return ARLib::get<Array>(m_internal_value);
            } else if constexpr (T == Type::JBool) {
                return ARLib::get<Bool>(m_internal_value);
            } else if constexpr (T == Type::JNull) {
                return ARLib::get<Null>(m_internal_value);
            } else {
                COMPTIME_ASSERT("Invalid enum value passed to JSON:Value::get<T>");
            }
        }
        template <Type T>
        auto try_as() {
            constexpr size_t index = static_cast<size_t>(from_enum(T));
            using JType            = typename JSONTypeArray::At<index>;
            using Ret              = Result<AddLvalueReferenceT<JType>, ConversionError>;
            if (m_type != T)
                return Ret{
                    ConversionError{T, m_type}
                };
            return Ret{ as<T>() };
        }
        template <Type T>
        auto try_as() const {
            constexpr size_t index = static_cast<size_t>(from_enum(T));
            using JType            = typename JSONTypeArray::At<index>;
            using Ret              = Result<AddConstT<AddLvalueReferenceT<JType>>, ConversionError>;
            if (m_type != T)
                return Ret{
                    ConversionError{T, m_type}
                };
            return Ret{ as<T>() };
        }
        const auto& operator[](const String& s) const { return as<Type::JObject>()[s]; }
        auto& operator[](const String& s) { return as<Type::JObject>()[s]; }
        const auto& operator[](size_t i) const { return as<Type::JArray>()[i]; }
        auto& operator[](size_t i) { return as<Type::JArray>()[i]; }
    };
    template <JSONTypeExt T>
    Value make(T&& val) {
        return ValueObj::construct(Forward<T>(val));
    }
    class Document {
        Value m_value;

        public:
        Document(Value&& v) : m_value(move(v)) {}
        Document(const Document& other)                = default;
        Document& operator=(const Document& other)     = default;
        Document& operator=(Document&& other) noexcept = default;
        Document(Document&& other) noexcept            = default;
        const ValueObj& root() const { return *m_value; }
        ValueObj& root() { return *m_value; }
        SerializeResult serialize_to_file(const Path& filename) const;
    };
    struct ErrorInfo {
        String error_string{};
        size_t error_offset{};
    };
    class ParseError : public ErrorBase {
        ErrorInfo m_info;

        public:
        ParseError() = default;
        ParseError(String error, size_t offset) : m_info{ move(error), offset } {};
        ParseError(ErrorInfo info) : m_info(move(info)){};
        const ErrorInfo& info() const { return m_info; }
        const String& message() const { return m_info.error_string; }
        const String& error_string() const { return m_info.error_string; }
        size_t offset() const { return m_info.error_offset; }
    };
}    // namespace JSON
template <>
struct PrintInfo<JSON::Value> {
    const JSON::Value& m_value;
    PrintInfo(const JSON::Value& value) : m_value(value) {}
    String repr() const { return PrintInfo<UniquePtr<JSON::ValueObj>>{ m_value }.repr(); }
};
template <>
struct PrintInfo<JSON::Null> {
    const JSON::Null& m_null;
    PrintInfo(const JSON::Null& null) : m_null(null) {}
    String repr() const { return "JSON::Null"_s; }
};
template <>
struct PrintInfo<JSON::Bool> {
    const JSON::Bool& m_bool;
    PrintInfo(const JSON::Bool& bol) : m_bool(bol) {}
    String repr() const { return BoolToStr(m_bool.value()); }
};
template <>
struct PrintInfo<JSON::Array> {
    const JSON::Array& m_array;
    PrintInfo(const JSON::Array& array_) : m_array(array_) {}
    String repr() const { return PrintInfo<Vector<JSON::Value>>{ m_array }.repr(); }
};
template <>
struct PrintInfo<JSON::Object> {
    const JSON::Object& m_object;
    PrintInfo(const JSON::Object& obj) : m_object(obj) {}
    String repr() const { return PrintInfo<FlatMap<String, JSON::Value>>{ m_object }.repr(); }
};
template <>
struct PrintInfo<JSON::Number> {
    const JSON::Number& m_number;
    PrintInfo(const JSON::Number& num) : m_number(num) {}
    String repr() const { return m_number.to_string(); }
};
template <>
struct PrintInfo<JSON::JString> {
    const JSON::JString& m_string;
    PrintInfo(const JSON::JString& string) : m_string(string) {}
    String repr() const { return m_string; }
};
template <>
struct PrintInfo<JSON::ValueObj> {
    const JSON::ValueObj& m_value;
    PrintInfo(const JSON::ValueObj& value) : m_value(value) {}
    String repr() const {
        switch (m_value.type()) {
            case JSON::Type::JArray:
                return PrintInfo<JSON::Array>{ m_value.as<JSON::Type::JArray>() }.repr();
            case JSON::Type::JString:
                return PrintInfo<JSON::JString>{ m_value.as<JSON::Type::JString>() }.repr();
            case JSON::Type::JObject:
                return PrintInfo<JSON::Object>{ m_value.as<JSON::Type::JObject>() }.repr();
            case JSON::Type::JBool:
                return PrintInfo<JSON::Bool>{ m_value.as<JSON::Type::JBool>() }.repr();
            case JSON::Type::JNull:
                return PrintInfo<JSON::Null>{ m_value.as<JSON::Type::JNull>() }.repr();
            case JSON::Type::JNumber:
                return PrintInfo<JSON::Number>{ m_value.as<JSON::Type::JNumber>() }.repr();
            default:
                ASSERT_NOT_REACHED("Invalid JSON type.");
        }
        return "Invalid JSON Value"_s;
    }
};
template <>
struct PrintInfo<JSON::ConversionError> {
    const JSON::ConversionError& m_error;
    PrintInfo(const JSON::ConversionError& error) : m_error(error) {}
    String repr() const { return m_error.message(); }
};
template <>
struct PrintInfo<JSON::ParseError> {
    const JSON::ParseError& m_error;
    PrintInfo(const JSON::ParseError& error) : m_error(error) {}
    String repr() const {
        return "Error encountered while parsing json: "_s + m_error.message() + " at offset "_s +
               IntToStr(m_error.offset());
    }
};
}    // namespace ARLib
