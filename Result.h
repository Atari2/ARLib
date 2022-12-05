#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Macros.h"
#include "PrintInfo.h"
#include "Utility.h"
#include "RefBox.h"
namespace ARLib {
struct DefaultErr {};
struct DefaultOk {};

enum class CurrType : bool { Ok, Error };
template <typename T_ok, typename T_err = DefaultErr>
class Result {
    CurrType m_type;
    constexpr static inline bool IsReference = IsLvalueReferenceV<T_ok>;
    using T_ok_f = ConditionalT<IsLvalueReferenceV<T_ok>, RefBox<RemoveReferenceT<T_ok>>, T_ok>;
    union {
        T_ok_f m_ok;
        T_err m_err;
    };
    Result(const Result& other) : m_type(other.m_type) {
        if (other.is_error()) {
            new (&m_err) T_err{ other.m_err };
        } else {
            new (&m_ok) T_ok_f{ other.m_ok };
        }
    }
    Result(Result&& other) noexcept : m_type(other.m_type) {
        if (other.is_error()) {
            new (&m_err) T_err{ move(other.m_err) };
        } else {
            new (&m_ok) T_ok_f{ move(other.m_ok) };
        }
    }
    Result& operator=(Result&& other) noexcept {
        if (other.is_error() && is_error()) {
            m_err = move(other.m_err);
        } else if (other.is_ok() && is_ok()) {
            m_ok = move(other.m_ok);
        } else {
            m_type = other.m_type;
            if (other.is_ok()) {
                m_ok.~T_ok_f();
                m_err = move(other.m_err);
            } else {
                m_err.~T_err();
                m_ok = move(other.ok);
            }
        }
        return *this;
    }
    Result& operator=(const Result& other) {
        if (other.is_error() && is_error()) {
            m_err = other.m_err;
        } else if (other.is_ok() && is_ok()) {
            m_ok = other.m_ok;
        } else {
            m_type = other.m_type;
            if (other.is_ok()) {
                m_ok.~T_ok_f();
                m_err = other.m_err;
            } else {
                m_err.~T_err();
                m_ok = other.ok;
            }
        }
        return *this;
    }

    public:
    Result(T_ok&& ok)
    requires(!IsReference)
        : m_type(CurrType::Ok), m_ok(move(ok)) {}
    Result(T_ok& ok)
    requires(IsReference)
        : m_type(CurrType::Ok), m_ok(ok) {}
    Result(T_err&& err) : m_type(CurrType::Error), m_err(move(err)) {}
    template <typename... Args>
    static Result from_error(Args... args) {
        T_err err{ args... };
        Result res{ Forward<T_err>(err) };
        return res;
    }
    template <typename... Args>
    static Result from_ok(Args... args) {
        T_ok_f ok{ args... };
        Result res{ Forward<T_ok_f>(ok) };
        return res;
    }
    template <typename... Args>
    static Result from(Args... args) {
        if constexpr (Constructible<T_ok_f, Args...>) {
            return from_ok(args...);
        } else if constexpr (Constructible<T_err, Args...>) {
            return from_error(args...);
        } else {
            COMPTIME_ASSERT("Invalid types for constructor of T_ok or T_err");
        }
    }
    CurrType type() const { return m_type; }
    bool is_error() const { return m_type == CurrType::Error; }
    bool is_ok() const { return m_type == CurrType::Ok; }
    T_err& error_value() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return m_err;
    }
    T_ok& ok_value() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        if constexpr (IsReference) {
            return m_ok.get();
        } else {
            return m_ok;
        }
    }
    const T_err& error_value() const {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return m_err;
    }
    const T_ok& ok_value() const {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        return m_ok;
    }
    T_err to_error() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return move(m_err);
    }
    T_ok to_ok() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        return move(m_ok);
    }
    explicit operator bool() const { return is_ok(); }
    ~Result() {
        if (m_type == CurrType::Ok) {
            m_ok.~T_ok_f();
        } else {
            m_err.~T_err();
        }
    }
};

template <typename T_err>
using DiscardResult = class Result<DefaultOk, T_err>;
template <typename T_ok>
using DiscardError = class Result<T_ok, DefaultErr>;
template <>
struct PrintInfo<DefaultOk> {
    const DefaultOk& m_ok;
    explicit PrintInfo(const DefaultOk& ok) : m_ok(ok) {}
    String repr() const { return "DefaultOk"_s; }
};
template <>
struct PrintInfo<DefaultErr> {
    const DefaultErr& m_ok;
    explicit PrintInfo(const DefaultErr& ok) : m_ok(ok) {}
    String repr() const { return "DefaultErr"_s; }
};
template <Printable T, Printable U>
struct PrintInfo<Result<T, U>> {
    const Result<T, U>& m_result;
    explicit PrintInfo(const Result<T, U>& result) : m_result(result) {}
    String repr() const {
        return "Result { "_s +
               (m_result.is_ok() ? PrintInfo<T>{ m_result.ok_value() }.repr() : PrintInfo<U>{ m_result.error_value() }.repr()) +
               " }"_s;
    }
};
}    // namespace ARLib
