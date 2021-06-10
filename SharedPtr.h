#pragma once
#include "TypeTraits.h"
namespace ARLib {
	template <typename T>
	class SharedPtr {
		T* m_storage = nullptr;
		size_t* m_count = nullptr;

		bool decrease_instance_count_() {
			if (m_count == nullptr)
				return false;
			(*m_count)--;
			if (*m_count == 0) {
				delete m_count;
				m_count = nullptr;
				return true;
			}
			return false;
		}

	public:
		constexpr SharedPtr() = default;
		SharedPtr(SharedPtr&&) = delete;
		SharedPtr& operator=(SharedPtr&&) = delete;

		SharedPtr(T* ptr) : m_storage(ptr), m_count(new size_t{ 1 }) {
		}
		SharedPtr(T&& storage) {
			m_storage = new T{ move(storage) };
			m_count = new size_t{ 1 };
		}
		SharedPtr(const SharedPtr& other) {
			m_storage = other.m_storage;
			m_count = other.m_count;
			(*m_count)++;
		}

		// FIXME: find a way to SFINAE away this constructor so I don't have to add Arg0, Arg1
		// Just having "typename... Args" makes this constructor match instead of above one when trying to copy construct...
		template<typename Arg0, typename Arg1, typename... Args>
		SharedPtr(Arg0&& arg0, Arg1&& arg1, Args&&... args) {
			m_storage = new T{ Forward<Arg0>(arg0), Forward<Arg1>(arg1), Forward<Args>(args)...};
			m_count = new size_t{ 1 };
		}

		SharedPtr& operator=(const SharedPtr& other) {
			reset();
			m_storage = other.m_storage;
			m_count = other.m_count;
			(*m_count)++;
		}
		T* release() {
			T* ptr = m_storage;
			decrease_instance_count_();
			m_count = nullptr;
			m_storage = nullptr;
			return ptr;
		}

		void reset() {
			if (decrease_instance_count_()) {
				delete m_storage;
			}
			m_storage = nullptr;
		}

		void share_with(SharedPtr& other) {
			other.m_storage = m_storage;
			other.m_count = m_count;
			(*m_count)++;
		}

		T* get() {
			return m_storage;
		}

		bool exists() {
			return m_storage != nullptr;
		}

		T* operator->() {
			return m_storage;
		}

		T& operator*() {
			return *m_storage;
		}

		~SharedPtr() {
			if (decrease_instance_count_()) {
				delete m_storage;
			}
		}

	};
}

using ARLib::SharedPtr;