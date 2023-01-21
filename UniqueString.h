#pragma once
#include "String.h"
#include "Types.h"
#include "SharedPtr.h"
#include "PrintInfo.h"
namespace ARLib {
class UniqueString {
    SharedPtr<String> m_ref;

    static SharedPtr<String> construct(String s);

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
    SharedPtr<String>& operator->() { return m_ref; }
    const SharedPtr<String>& operator->() const { return m_ref; }
    ~UniqueString();
};
template <>
struct PrintInfo<UniqueString> {
    const UniqueString& m_string;
    PrintInfo(const UniqueString& string) : m_string(string) {}
    String repr() const { return *m_string.operator->().get(); }
};
}    // namespace ARLib
