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
#define TRY_SET_IMPL(val, temp_name, expression)                                                                       \
    auto temp_name = (expression);                                                                                     \
    static_assert(!SameAsCvRef<decltype(temp_name.to_ok()), DefaultOk>, "Don't use TRY_SET with DiscardResult");       \
    if (temp_name.is_error()) { return { temp_name.to_error() }; }                                                     \
    auto val = temp_name.to_ok();

#define TRY_RET_IMPL(temp_name, expression)                                                                            \
    auto temp_name = (expression);                                                                                     \
    static_assert(!SameAsCvRef<decltype(temp_name.to_ok()), DefaultOk>, "Don't use TRY_RET with DiscardResult");       \
    if (temp_name.is_error()) { return { temp_name.to_error() }; }                                                     \
    return temp_name.to_ok();

#define TRY_IMPL(temp_name, expression)                                                                                \
    auto temp_name = (expression);                                                                                     \
    static_assert(SameAsCvRef<decltype(temp_name.to_ok()), DefaultOk>, "Only use TRY with DiscardResult");             \
    if (temp_name.is_error()) { return { temp_name.to_error() }; }

#define CONCAT_TOKENS_IMPL(x, y) x##y
#define CONCAT_TOKENS(x, y)      CONCAT_TOKENS_IMPL(x, y)

#define TRY_SET(val, expression) TRY_SET_IMPL(val, CONCAT_TOKENS(__tr_, __COUNTER__), expression)

#define TRY_RET(expression) TRY_RET_IMPL(CONCAT_TOKENS(__tr_, __COUNTER__), expression)

#define TRY(expression) TRY_IMPL(CONCAT_TOKENS(__tr_, __COUNTER__), expression)

#define MUST(expression)                                                                                               \
    [](auto&& tr) {                                                                                                    \
        static_assert(!IsLvalueReferenceV<decltype(tr.to_ok())>, "to_ok() must not return an lvalue reference");       \
        if (tr.is_error()) {                                                                                           \
            ASSERT_NOT_REACHED_FMT(                                                                                    \
            "MUST(" STRINGIFY(expression) ") failed \"%s\"", print_conditional(tr.to_error()).data()                   \
            );                                                                                                         \
        }                                                                                                              \
        if constexpr (!SameAsCvRef<decltype(tr.to_ok()), DefaultOk>) { return tr.to_ok(); }                            \
    }((expression))

}    // namespace ARLib
