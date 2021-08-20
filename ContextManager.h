#pragma once
#include "GeneratedEnums/ManageWhen.h"

namespace ARLib {

    namespace detail {
        constexpr auto DefaultValueRet = []() {
            return true;
        };
    }

    template <typename Functor, typename TestFunc = decltype(detail::DefaultValueRet)>
    class ContextManager {
        Functor m_func;
        TestFunc m_testfunc;
        ManageWhen m_when;

        public:
        explicit ContextManager(Functor func, ManageWhen when = ManageWhen::AtExit,
                       TestFunc testfunc = detail::DefaultValueRet) :
            m_func(func), m_when(when), m_testfunc(testfunc) {
            if (!!(m_when & ManageWhen::AtEnter)) {
                if (!!(m_when & ManageWhen::OnSuccess)) {
                    if (m_testfunc()) m_func();
                } else if (!!(m_when & ManageWhen::OnFailure)) {
                    if (!m_testfunc()) m_func();
                } else {
                    m_func();
                }
            }
        }
        ContextManager(const ContextManager&) = delete;
        ContextManager(ContextManager&&) = delete;
        ContextManager& operator=(const ContextManager&) = delete;
        ContextManager& operator=(ContextManager&&) = delete;

        ~ContextManager() {
            if (!!(m_when & ManageWhen::AtExit)) {
                if (!!(m_when & ManageWhen::OnSuccess)) {
                    if (m_testfunc()) m_func();
                } else if (!!(m_when & ManageWhen::OnFailure)) {
                    if (!m_testfunc()) m_func();
                } else {
                    m_func();
                }
            }
        }
    };
} // namespace ARLib

using ARLib::ContextManager;
using ARLib::ManageWhen;