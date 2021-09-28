#pragma once

#include "CharConv.h"
#include "HashMap.h"
#include "Map.h"
#include "PrintInfo.h"
#include "String.h"
#include "UniquePtr.h"
#include "Variant.h"

namespace ARLib {
    namespace JSON {

        // FWD
        class Parser;

        // types
        class ValueObj;
        using Value = UniquePtr<ValueObj>;

        struct Object : public HashMap<String, Value> {

            using Parent = HashMap<String, Value>;

            operator Value() &&;
            ValueObj& operator[](const String& key) { return *(static_cast<Parent*>(this)->operator[](key)); }
            const ValueObj& operator[](const String& key) const {
                return *(static_cast<const Parent*>(this)->operator[](key));
            }
        };
        struct Array : public Vector<Value> {

            using Parent = Vector<Value>;

            operator Value() &&;
            ValueObj& operator[](size_t index) { return *(static_cast<Parent*>(this)->operator[](index)); }
            const ValueObj& operator[](size_t index) const {
                return *(static_cast<const Parent*>(this)->operator[](index));
            }
        };

        struct JString : public String {
            operator Value() &&;
        };

        // Null detail impl
        namespace detail {
            struct NullTag {};
            struct BoolTag {};
            struct NumberTag {};
        } // namespace detail
        constexpr inline auto null_tag = detail::NullTag{};
        constexpr inline auto bool_tag = detail::BoolTag{};
        constexpr inline auto number_tag = detail::NumberTag{};
        struct Null {
            constexpr Null(detail::NullTag) {}
            constexpr Null(nullptr_t) {}
            operator Value() &&;
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
            double m_value;

            public:
            constexpr Number(double val) : m_value(val) {}
            constexpr Number(detail::NumberTag, double val) : m_value(val) {}
            constexpr double value() const { return m_value; }
            operator double() const { return m_value; }
            operator Value() &&;
        };

        enum class Type { JObject, JString, JNumber, JArray, JBool, JNull };

        template <typename Tp>
        concept JSONType = IsAnyOfV<Tp, Object, JString, Number, Array, Bool, Null>;

        template <typename Tp>
        concept JSONTypeExt = IsAnyOfV<Tp, Object, JString, Number, Array, Bool, Null, bool, double, String, nullptr_t>;

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

        class ValueObj {
            friend Parser;
            friend PrintInfo<ValueObj>;

            using JSONVariant = Variant<Object, JString, Number, Array, Bool, Null>;
            JSONVariant m_internal_value{null_tag};
            Type m_type{Type::JNull};

            template <JSONType T>
            ValueObj(T&& value, Type type) : m_internal_value(move(value)), m_type(type) {}

            template <JSONTypeExt T>
            static consteval Type map_t_to_enum() {
                if constexpr (SameAs<T, Object>) {
                    return Type::JObject;
                } else if constexpr (SameAs<T, JString> || SameAs<T, String>) {
                    return Type::JString;
                } else if constexpr (SameAs<T, Number> || SameAs<T, double>) {
                    return Type::JNumber;
                } else if constexpr (SameAs<T, Array>) {
                    return Type::JArray;
                } else if constexpr (SameAs<T, Bool> || SameAs<T, bool>) {
                    return Type::JBool;
                } else if constexpr (SameAs<T, Null> || SameAs<T, nullptr_t>) {
                    return Type::JNull;
                } else {
                    COMPTIME_ASSERT("Invalid enum value passed to JSON operator=");
                }
            }

            public:
            template <JSONType T>
            static Value construct(T&& value) {
                return UniquePtr{ValueObj{Forward<T>(value), enum_from_type<T>()}};
            }

            Type type() const { return m_type; }

            template <JSONTypeExt T>
            ValueObj& operator=(const T& value) {
                m_internal_value = value;
                m_type = map_t_to_enum<T>();
                return *this;
            }

            template <JSONTypeExt T>
            ValueObj& operator=(T&& value) {
                m_internal_value = move(value);
                m_type = map_t_to_enum<T>();
                return *this;
            }

            template <JSONTypeExt T>
            bool operator==(const T& value) const {
                constexpr auto val = map_t_to_enum<T>();
                if (val != m_type) return false;
                return get<val>() == value;
            }

            template <JSONTypeExt T>
            bool operator!=(const T& value) const {
                return !(*this == value);
            }

            template <Type T>
            const auto& get() const {
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
            auto& get() {
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
        };

        class Document {
            Object m_object;

            public:
            Document(Object&& obj) : m_object(move(obj)) {}
            const Object& object() const { return m_object; }
            Object& object() { return m_object; }

            ValueObj& operator[](const String& key) { return m_object[key]; }
            const ValueObj& operator[](const String& key) const { return m_object[key]; }
        };
    } // namespace JSON

#ifndef COMPILER_CLANG
    // until I figure out what's wrong with this on Clang...
    // this is the solution.

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
        String repr() const { return PrintInfo<Vector<JSON::Value>>{static_cast<Vector<JSON::Value>>(m_array)}.repr(); }
    };

    template <>
    struct PrintInfo<JSON::Object> {
        const JSON::Object& m_object;
        PrintInfo(const JSON::Object& obj) : m_object(obj) {}
        String repr() const {
            return PrintInfo<HashMap<String, JSON::Value>>{static_cast<HashMap<String, JSON::Value>>(m_object)}.repr();
        }
    };
#endif

    template <>
    struct PrintInfo<JSON::ValueObj> {
        const JSON::ValueObj& m_value;
        PrintInfo(const JSON::ValueObj& value) : m_value(value) {}
        String repr() const {
            switch (m_value.type()) {
            case JSON::Type::JArray:
                return PrintInfo<Vector<JSON::Value>>{m_value.get<JSON::Type::JArray>()}.repr();
            case JSON::Type::JString:
                return m_value.get<JSON::Type::JString>();
            case JSON::Type::JObject:
                return PrintInfo<HashMap<String, JSON::Value>>{m_value.get<JSON::Type::JObject>()}.repr();
            case JSON::Type::JBool:
                return BoolToStr(m_value.get<JSON::Type::JBool>().value());
            case JSON::Type::JNull:
                return "null"_s;
            case JSON::Type::JNumber:
                return DoubleToStr(m_value.get<JSON::Type::JNumber>());
            default:
                HARD_ASSERT(false, "Invalid JSON type.");
            }
            return "Invalid JSON Value"_s;
        }
    };
} // namespace ARLib