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

        using Object = HashMap<String, Value>;
        using Array = Vector<Value>;
        using Number = double;

        // Null detail impl
        namespace detail {
            struct NullTag {};
            struct BoolTag {};
        } // namespace detail
        constexpr inline auto null_tag = detail::NullTag{};
        constexpr inline auto bool_tag = detail::BoolTag{};
        struct Null {
            constexpr Null(detail::NullTag) {}
        };
        class Bool {
            bool m_value;

            public:
            constexpr Bool(detail::BoolTag, bool val) : m_value(val) {}
            constexpr bool value() const { return m_value; }
        };

        enum class Type { JObject, JString, JNumber, JArray, JBool, JNull };

        template <typename Tp>
        concept JSONType = IsAnyOfV<Tp, Object, String, Number, Array, Bool, Null>;

        template <JSONType T>
        constexpr Type enum_from_type() {
            if constexpr (IsSameV<T, Object>) {
                return Type::JObject;
            } else if constexpr (IsSameV<T, String>) {
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

            using JSONVariant = Variant<Object, String, Number, Array, Bool, Null>;
            JSONVariant m_internal_value{null_tag};
            Type m_type{Type::JNull};

            template <JSONType T>
            ValueObj(T&& value, Type type) : m_internal_value(move(value)), m_type(type) {}

            public:
            template <JSONType T>
            static Value construct(T&& value) {
                return UniquePtr{ValueObj{Forward<T>(value), enum_from_type<T>()}};
            }

            Type type() const { return m_type; }

            template <Type T>
            const auto& get() const {
                // Value, Object, String, Number, Array, Bool, Null
                if constexpr (T == Type::JObject) {
                    return ARLib::get<Object>(m_internal_value);
                } else if constexpr (T == Type::JString) {
                    return ARLib::get<String>(m_internal_value);
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
                    return ARLib::get<String>(m_internal_value);
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

            ValueObj& operator[](const String& key) { return *m_object[key]; }
            const ValueObj& operator[](const String& key) const { return *m_object[key]; }
        };
    } // namespace JSON

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
    struct PrintInfo<JSON::ValueObj> {
        const JSON::ValueObj& m_value;
        PrintInfo(const JSON::ValueObj& value) : m_value(value) {}
        String repr() const { return PrintInfo<JSON::ValueObj::JSONVariant>{m_value.m_internal_value}.repr(); }
    };
} // namespace ARLib