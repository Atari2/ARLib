#include "StringProxy.h"
#include "String.h"
#include "StringView.h"

namespace ARLib {
    StringProxy::StringProxy(String& str) {
        m_buf = str.rawptr();
        m_size = str.size();
    }
    StringProxy::StringProxy(StringView view) {
        m_buf = view.rawptr();
        m_size = view.size();
    }
    StringProxy& StringProxy::operator=(String& str) {
        m_buf = str.rawptr();
        m_size = str.size();
        return *this;
    }
    StringProxy& StringProxy::operator=(StringView view) {
        m_buf = view.rawptr();
        m_size = view.size();
        return *this;
    }
} // namespace ARLib