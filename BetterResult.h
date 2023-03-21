#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Macros.h"
#include "RefBox.h"
#include "StackTrace.h"
#include "String.h"
#include "UniquePtr.h"
#include "PrintInfo.h"
namespace ARLib::vnext {
class ErrorBase {
    public:
    virtual const String& error_string() const = 0;
    virtual ~ErrorBase()                               = default;
};
class Error : public ErrorBase {
    protected:
    String m_error_string;
    public:
    Error(Error&&) = default;
    Error(ConvertibleTo<String> auto val) : m_error_string{ move(val) } {}
    const String& error_string() const override { return m_error_string; }
    ~Error() {}
};
class BacktraceError final : public Error {
    BackTrace m_bt;
    public:
    BacktraceError(BacktraceError&&) = default;
    BacktraceError(ConvertibleTo<String> auto val) : Error{ move(val) }, m_bt{ capture_backtrace() } {
        m_error_string += '\n';
        m_error_string += PrintInfo<BackTrace>{ m_bt }.repr();
    }
    ~BacktraceError() {}
};
template <typename ResT>
class Result {
    constexpr static inline bool IsReference = IsLvalueReferenceV<ResT>;
    using Res                                = ConditionalT<IsReference, RefBox<RemoveReferenceT<ResT>>, ResT>;
    enum class CurrType : bool { Ok, Err };
    union {
        Res m_ok;
        UniquePtr<ErrorBase> m_err;
    };
    CurrType m_type : 1 { CurrType::Ok };

    public:
    Result()
    requires(DefaultConstructible<ResT> && !IsReference)
        : m_ok{}, m_type{ CurrType::Ok } {}
    template <typename T>
    Result(T&& val)
    requires(Constructible<ResT, T>)
        : m_ok{ Forward<T>(val) }, m_type{ CurrType::Ok } {}
    template <DerivedFrom<ErrorBase> ErrorType>
    Result(ErrorType&& val) : m_err{ new ErrorType{ move(val) } }, m_type{ CurrType::Err } {}
    CurrType type() const { return m_type; }
    bool is_error() const { return m_type == CurrType::Err; }
    bool is_ok() const { return m_type == CurrType::Ok; }
    template <DerivedFrom<ErrorBase> ErrorType = Error>
    ErrorType& error_value() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return *static_cast<ErrorType*>(m_err.get());
    }
    ResT& ok_value() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        if constexpr (IsReference) {
            return m_ok.get();
        } else {
            return m_ok;
        }
    }
    template <DerivedFrom<ErrorBase> ErrorType = Error>
    const ErrorType& error_value() const {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        return *static_cast<const ErrorType*>(m_err.get());
    }
    const ResT& ok_value() const {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        return m_ok;
    }
    template <DerivedFrom<ErrorBase> ErrorType = Error>
    auto to_error() {
        HARD_ASSERT(is_error(), "Tried to take error type from result with value.");
        ErrorType copy = move(static_cast<ErrorType*>(m_err.get()));
        m_err.reset();
        return copy;
    }
    auto to_ok() {
        HARD_ASSERT(is_ok(), "Tried to take ok type from result with error.");
        return move(m_ok);
    }
    explicit operator bool() const { return is_ok(); }
    ~Result() {
        if (m_type == CurrType::Ok) {
            m_ok.~Res();
        } else {
            m_err.~UniquePtr();
        }
    }
};
template <typename T>
requires(!DerivedFrom<T, vnext::Error>)
Result(T) -> Result<T>;
}    // namespace ARLib::vnext
namespace ARLib {
template <DerivedFrom<vnext::ErrorBase> ErrorType>
struct PrintInfo<ErrorType> {
    const ErrorType& m_error;
    PrintInfo(const ErrorType& error) : m_error{ error } {}
    String repr() const { return m_error.error_string(); }
};
template <typename T>
struct PrintInfo<vnext::Result<T>> {
    const vnext::Result<T>& m_result;
    PrintInfo(const vnext::Result<T>& result) : m_result{ result } {}
    String repr() const {
        if (m_result.is_ok()) {
            return "Result { "_s + print_conditional(m_result.ok_value()) + " }"_s;
        } else {
            return "Result { "_s + print_conditional(m_result.error_value()) + " }"_s;
        }
    }
};
}    // namespace ARLib