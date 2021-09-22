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

        // FIXME: when used with hashmap, it leaks huge amounts of memory for no reason.
        using Object = HashMap<String, Value>;
        using Array = Vector<Value>;
        using JString = String;
        using Number = double;
        struct Null;
        struct Bool;

        // Null detail impl
        namespace detail {
            struct NullTag {};
            struct BoolTag {};
        } // namespace detail
        constexpr inline auto null_tag = detail::NullTag{};
        constexpr inline auto bool_tag = detail::BoolTag{};
        struct Null {
            Null(detail::NullTag) {}
        };
        struct Bool {
            bool m_value;
            Bool(detail::BoolTag, bool val) : m_value(val) {}
            bool value() const { return m_value; }
        };

        enum class Type { Object, String, Number, Array, Bool, Null };

        template <typename Tp>
        concept JSONType = IsAnyOfV<Tp, Object, JString, Number, Array, Bool, Null>;

        template <JSONType T>
        constexpr Type enum_from_type() {
             if constexpr (IsSameV<T, Object>) {
                return Type::Object;
            } else if constexpr (IsSameV<T, JString>) {
                return Type::String;
            } else if constexpr (IsSameV<T, Number>) {
                return Type::Number;
            } else if constexpr (IsSameV<T, Array>) {
                return Type::Array;
            } else if constexpr (IsSameV<T, Bool>) {
                return Type::Bool;
            } else if constexpr (IsSameV<T, Null>) {
                return Type::Null;
            } else {
                COMPTIME_ASSERT("Invalid type to construct a JSON Value from")
            }
        }

        class ValueObj {
            friend Parser;
            friend PrintInfo<ValueObj>;

            using JSONVariant = Variant<Object, JString, Number, Array, Bool, Null>;
            JSONVariant m_internal_value{null_tag};
            Type m_type{Type::Null};

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
                if constexpr (T == Type::Object) {
                    return m_internal_value.template get<Object>();
                } else if constexpr (T == Type::String) {
                    return m_internal_value.template get<JString>();
                } else if constexpr (T == Type::Number) {
                    return m_internal_value.template get<Number>();
                } else if constexpr (T == Type::Array) {
                    return m_internal_value.template get<Array>();
                } else if constexpr (T == Type::Bool) {
                    return m_internal_value.template get<Bool>();
                } else if constexpr (T == Type::Null) {
                    return m_internal_value.template get<Null>();
                } else {
                    COMPTIME_ASSERT("Invalid enum value passed to JSON:Value::get<T>");
                }
            }

            template <Type T>
            auto& get() {
                // Value, Object, String, Number, Array, Bool, Null
                if constexpr (T == Type::Object) {
                    return m_internal_value.template get<Object>();
                } else if constexpr (T == Type::String) {
                    return m_internal_value.template get<JString>();
                } else if constexpr (T == Type::Number) {
                    return m_internal_value.template get<Number>();
                } else if constexpr (T == Type::Array) {
                    return m_internal_value.template get<Array>();
                } else if constexpr (T == Type::Bool) {
                    return m_internal_value.template get<Bool>();
                } else if constexpr (T == Type::Null) {
                    return m_internal_value.template get<Null>();
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
        String repr() const { return BoolToStr(m_bool.m_value); }
    };

    template <>
    struct PrintInfo<JSON::ValueObj> {
        const JSON::ValueObj& m_value;
        PrintInfo(const JSON::ValueObj& value) : m_value(value) {}
        String repr() const { return PrintInfo<JSON::ValueObj::JSONVariant>{m_value.m_internal_value}.repr(); }
    };
} // namespace ARLib