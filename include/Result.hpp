#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "Macros.hpp"
#include "RefBox.hpp"
#include "StackTrace.hpp"
#include "String.hpp"
#include "UniquePtr.hpp"
#include "PrintInfo.hpp"
namespace ARLib {
class ErrorBase {
    public:
    bool operator==(const ErrorBase& other) const { return error_string() == other.error_string(); }
    virtual const String& error_string() const = 0;
    virtual ~ErrorBase()                       = default;
};
class Error : public ErrorBase {
    protected:
    String m_error_string;
    public:
    Error(Error&&) = default;
    Error(ConvertibleTo<String> auto val) : m_error_string{ move(val) } {}
    template <typename OtherError>
    requires DerivedFrom<OtherError, ErrorBase>
    Error(OtherError&& other) : m_error_string{ move(other.error_string()) } {}
    const String& error_string() const override { return m_error_string; }
    bool operator==(const Error& other) const { return m_error_string == other.m_error_string; }
    virtual ~Error() {}
};
#ifdef DEBUG
class BacktraceError final : public Error {
    BackTrace m_bt;
    public:
    BacktraceError(BacktraceError&&) = default;
    BacktraceError(ConvertibleTo<String> auto val) : Error{ move(val) }, m_bt{ capture_backtrace() } {
        m_error_string += '\n';
        m_error_string += PrintInfo<BackTrace>{ m_bt }.repr();
    }
    virtual ~BacktraceError() {}
};
#else
class BacktraceError final : public Error {
    public:
    BacktraceError(BacktraceError&&) = default;
    BacktraceError(ConvertibleTo<String> auto val) : Error{ move(val) } {
        m_error_string += '\n';
        m_error_string += "[no backtrace available in release mode]"_s;
    }
    virtual ~BacktraceError() {}
};
#endif
struct EmplaceOk {};
struct EmplaceErr {};
constexpr inline EmplaceOk emplace_ok{};
constexpr inline EmplaceErr emplace_error{};
enum class CurrType : bool { Ok, Err };
template <typename ResT, typename ErrorType = Error>
requires(DerivedFrom<ErrorType, ErrorBase>)
class Result {
    constexpr static inline bool IsReference = IsLvalueReferenceV<ResT>;
    using Res                                = ConditionalT<IsReference, RefBox<RemoveReferenceT<ResT>>, ResT>;
    union {
        Res m_ok;
        UniquePtr<ErrorBase> m_err;
    };
    CurrType m_type : 1;


    public:
    Result()
    requires(DefaultConstructible<ResT> && !IsReference)
        : m_ok{}, m_type{ CurrType::Ok } {}
    Result(ResT val)
    requires(IsReference)
        : m_ok{ val }, m_type{ CurrType::Ok } {}
    template <typename O>
    Result(O&& val, EmplaceOk)
    requires(Constructible<ResT, O> && Constructible<ErrorType, O>)
        : m_ok{ Forward<O>(val) }, m_type{ CurrType::Ok } {}
    template <typename O>
    Result(O&& val, EmplaceErr)
    requires(Constructible<ResT, O> && Constructible<ErrorType, O>)
        : m_err{ new ErrorType{ Forward<O>(val) } }, m_type{ CurrType::Err } {}
    template <typename T>
    Result(T&& val)
    requires(Constructible<ResT, T>)
        : m_ok{ Forward<T>(val) }, m_type{ CurrType::Ok } {}
    template <typename ET>
    Result(ET&& val)
    requires(Constructible<ErrorType, ET> && !DerivedFrom<ET, ErrorBase>)
        : m_err{ new ErrorType{ move(val) } }, m_type{ CurrType::Err } {}
    template <typename OtherET>
    Result(OtherET&& val)
    requires(DerivedFrom<OtherET, ErrorBase>)
        : m_err{ new OtherET{ move(val) } }, m_type{ CurrType::Err } {}
    template <typename OtherET>
    requires(Constructible<ErrorType, OtherET>)
    Result(Result<ResT, OtherET>&& other) {
        if (other.is_ok()) {
            new (&m_ok) Res{ move(other.to_ok()) };
        } else {
            new (&m_err) UniquePtr<ErrorBase>{ new ErrorType{ move(other.to_error()) } };
        }
        m_type = other.type();
    }
    Result(ErrorType&& val) : m_err{ new ErrorType{ move(val) } }, m_type{ CurrType::Err } {}
    Result(Result&& other) noexcept : m_type(other.m_type) {
        if (other.m_type == CurrType::Ok) {
            new (&m_ok) Res{ move(other.m_ok) };
        } else {
            new (&m_err) UniquePtr<ErrorBase>{ move(other.m_err) };
        }
    }
    Result& operator=(Result&& other) noexcept {
        if (m_type == CurrType::Ok) {
            m_ok.~Res();
        } else {
            if (m_err.exists()) ASSERT_NOT_REACHED("Result with error was not handled");
            m_err.~UniquePtr();
        }
        if (other.m_type == CurrType::Ok) {
            new (&m_ok) Res{ move(other.m_ok) };
        } else {
            new (&m_err) UniquePtr<ErrorBase>{ move(other.m_err) };
        }
        m_type = other.m_type;
        return *this;
    }
    CurrType type() const { return m_type; }
    bool is_error() const { return m_type == CurrType::Err; }
    bool is_ok() const { return m_type == CurrType::Ok; }
    ErrorType& error_value() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return *static_cast<ErrorType*>(m_err.get());
    }
    auto& ok_value() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        if constexpr (IsReference) {
            return m_ok.get();
        } else {
            return m_ok;
        }
    }
    const ErrorType& error_value() const {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return *static_cast<const ErrorType*>(m_err.get());
    }
    auto& ok_value() const {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        if constexpr (IsReference) {
            return m_ok.get();
        } else {
            return m_ok;
        }
    }
    auto to_error() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        ErrorType copy{ move(*static_cast<ErrorType*>(m_err.get())) };
        m_err.reset();
        return copy;
    }
    auto to_ok() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        if constexpr (IsReference) {
            return move(m_ok.get());
        } else {
            return move(m_ok);
        }
    }
    auto value_or(ResT&& default_value) {
        if (is_error()) {
            ignore_error();
            return default_value;
        } else {
            return to_ok();
        }
    }
    template <typename T>
    requires(ConvertibleTo<ResT, T>)
    Result<T, ErrorType> map() {
        if (is_error()) {
            return to_error();
        } else {
            return static_cast<T>(to_ok());
        }
    }
    template <typename Func>
    requires(CallableWith<Func, ResT>)
    Result<InvokeResultT<Func, ResT>, ErrorType> map(Func&& f) {
        if (is_error()) {
            return to_error();
        } else {
            return invoke(Forward<Func>(f), move(to_ok()));
        }
    }
    bool operator==(const Result& other) const {
        if (m_type == other.m_type) {
            if (m_type == CurrType::Ok) {
                return ok_value() == other.ok_value();
            } else {
                return error_value() == other.error_value();
            }
        }
        return false;
    }
    void ignore_error() {
        if (m_type == CurrType::Err) m_err.reset();
    }
    auto must() {
        if (is_error()) {
            ASSERT_NOT_REACHED_FMT(
            "%s::must() failed \"%s\"", TYPENAME_TO_STRING(*this), print_conditional(to_error()).data()
            );
        }
        return to_ok();
    }
    explicit operator bool() const { return is_ok(); }
    ~Result() {
        if (m_type == CurrType::Ok) {
            m_ok.~Res();
        } else {
            if (m_err.exists()) ASSERT_NOT_REACHED("Result with error was not handled");
            m_err.~UniquePtr();
        }
    }
};
template <typename T>
requires(!DerivedFrom<T, Error>)
Result(T) -> Result<T>;
struct DefaultOk {};
template <typename Err = Error>
using DiscardResult = Result<DefaultOk, Err>;
template <DerivedFrom<ErrorBase> ErrorType>
struct PrintInfo<ErrorType> {
    const ErrorType& m_error;
    PrintInfo(const ErrorType& error) : m_error{ error } {}
    String repr() const { return m_error.error_string(); }
};
template <typename T>
struct PrintInfo<Result<T>> {
    const Result<T>& m_result;
    PrintInfo(const Result<T>& result) : m_result{ result } {}
    String repr() const {
        if (m_result.is_ok()) {
            return "Result { "_s + print_conditional(m_result.ok_value()) + " }"_s;
        } else {
            return "Result { "_s + print_conditional(m_result.error_value()) + " }"_s;
        }
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
    if (temp_name.is_error()) { return { temp_name.to_error() }; }

#define CONCAT_TOKENS_IMPL(x, y) x##y
#define CONCAT_TOKENS(x, y)      CONCAT_TOKENS_IMPL(x, y)

#define TRY_SET(val, expression) TRY_SET_IMPL(val, CONCAT_TOKENS(__tr_, __COUNTER__), expression)

#define TRY_RET(expression)                                                                                            \
    { TRY_RET_IMPL(CONCAT_TOKENS(__tr_, __COUNTER__), expression) }

#define TRY(expression)                                                                                                \
    { TRY_IMPL(CONCAT_TOKENS(__tr_, __COUNTER__), expression) }

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