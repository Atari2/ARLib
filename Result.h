#pragma once
#include "Concepts.h"
#include "Macros.h"
#include "Utility.h"

namespace ARLib {
    struct DefaultErr {};

    struct DefaultOk {};

    enum class CurrType : bool { Ok, Error };

    template <typename T_ok, typename T_err = DefaultErr>
    class Result {
        CurrType m_type;
        union {
            T_ok m_ok;
            T_err m_err;
        };

        Result(const Result& other) : m_type(other.m_type) {
            if (other.is_error()) {
                m_err = other.m_err;
            } else {
                m_ok = other.m_ok;
            }
        }
        Result(Result&& other) noexcept : m_type(other.m_type) {
            if (other.is_error()) {
                m_err = move(other.m_err);
            } else {
                m_ok = move(other.m_ok);
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
                    m_ok.~T_ok();
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
                    m_ok.~T_ok();
                    m_err = other.m_err;
                } else {
                    m_err.~T_err();
                    m_ok = other.ok;
                }
            }
            return *this;
        }

        public:
        Result(T_ok&& ok) : m_type(CurrType::Ok), m_ok(move(ok)) {}
        Result(T_err&& err) : m_type(CurrType::Error), m_err(move(err)) {}

        template <typename... Args>
        static Result from_error(Args... args) {
            T_err err{args...};
            Result res{Forward<T_err>(err)};
            return res;
        }

        template <typename... Args>
        static Result from_ok(Args... args) {
            T_ok ok{args...};
            Result res{Forward<T_ok>(ok)};
            return res;
        }

        template <typename... Args>
        static Result from(Args... args) {
            if constexpr (Constructible<T_ok, Args...>) {
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
        T_err&& to_error() {
            if (is_error()) return move(m_err);
            unreachable
        }
        T_ok&& to_ok() {
            if (is_ok()) return move(m_ok);
            unreachable
        }

        operator bool() const { return is_ok(); }

        ~Result() { 
            if (m_type == CurrType::Ok) {
                m_ok.~T_ok();
            } else {
                m_err.~T_err();
            }
        }
    };

    template <typename T_err>
    using DiscardResult = class Result<DefaultOk, T_err>;

} // namespace ARLib

using ARLib::DiscardResult;
using ARLib::Result;