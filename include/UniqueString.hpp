#pragma once
#include "String.hpp"
#include "Types.hpp"
#include "SharedPtr.hpp"
#include "PrintInfo.hpp"
#include "HashBase.hpp"
namespace ARLib {
class UniqueString {
    SharedPtr<String> m_ref;

    static SharedPtr<String> construct(const String& s);
    friend Hash<UniqueString>;
    public:
    explicit UniqueString(const String& str) : m_ref(construct(str)) {}
    explicit UniqueString(const char* ptr) : m_ref(construct(String{ ptr })) {}
    UniqueString(const UniqueString& other)            = default;
    UniqueString(UniqueString&& other)                 = default;
    UniqueString& operator=(const UniqueString& other) = default;
    UniqueString& operator=(UniqueString&& other)      = default;
    UniqueString& operator=(const String& other) {
        m_ref = construct(other);
        return *this;
    }
    UniqueString& operator=(String&& other) {
        m_ref = construct(other);
        return *this;
    }
    bool operator==(const UniqueString& other) const { return m_ref == other.m_ref; }
    bool operator==(const String& other) const { return *m_ref == other; }
    bool operator!=(const UniqueString& other) const { return m_ref != other.m_ref; }
    bool operator!=(const String& other) const { return *m_ref != other; }
    friend bool operator==(const String& lhs, const UniqueString& rhs) { return rhs == lhs; }
    friend bool operator!=(const String& lhs, const UniqueString& rhs) { return rhs != lhs; }
    const String* operator->() { return m_ref.get(); }
    const String* operator->() const { return m_ref.get(); }
    const String& operator*() { return *m_ref; }
    const String& operator*() const { return *m_ref; }
    explicit operator String() const { return String{ *m_ref.get() }; }
};
template <>
struct Hash<UniqueString> {
    [[nodiscard]] size_t operator()(const UniqueString& str) const noexcept {
        return Hash<String>{}(*str.m_ref);
    }
};
template <>
struct PrintInfo<UniqueString> {
    const UniqueString& m_string;
    PrintInfo(const UniqueString& string) : m_string(string) {}
    String repr() const { return *m_string; }
};
}    // namespace ARLib
