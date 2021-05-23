#pragma once
#include "Concepts.h"
#include "Assertion.h"

namespace ARLib {
	template <DefaultConstructible T>
	class Optional {
		T* m_object = nullptr;
		bool m_exists = false;

		void assert_not_null_() {
			SOFT_ASSERT(m_object, "Null deref in Optional")
		}

		void evict_() {
			if (m_exists)
				delete m_object;
			m_exists = false;
		}
	public:
		Optional() = default;

		Optional(T&& val) requires MoveAssignable<T> : m_object(new T), m_exists(true) {
			*m_object = move(val);
		}

		Optional(const T& val) requires CopyAssignable<T> : m_object(new T), m_exists(true) {
			*m_object = val;
		}

		Optional<T>& operator=(const T& val) requires CopyAssignable<T> {
			evict_();
			m_object = new T;
			*m_object = val;
			m_exists = true;
			return *this;
		}

		Optional<T>& operator=(T&& val) requires CopyAssignable<T> {
			evict_();
			m_object = new T;
			*m_object = move(val);
			m_exists = true;
			return *this;
		}

		operator bool() {
			return m_exists;
		}
		operator T() {
			assert_not_null_();
			return *m_object;
		}

		bool empty() const { return m_exists; }
		bool has_value() const { return m_exists; };
		const T& value() const { assert_not_null_(); return *m_object; }
		T& value() { assert_not_null_(); return *m_object; }

		const T* operator->() const {
			assert_not_null_();
			return m_object;
		}

		T* operator->() {
			assert_not_null_();
			return m_object;
		}

		const T& operator*() const {
			assert_not_null_();
			return *m_object;
		}

		T& operator*() {
			assert_not_null_();
			return *m_object;
		}

		T value_or(T&& default_value) requires CopyConstructible<T> {
			if (!m_exists) return move(default_value);
			return value();
		}

		template <typename... Args>
		void emplate(Args... args) {
			m_object = new T(args...);
			m_exists = true;
		}

		T* detach() { 
			assert_not_null_(); 
			T* val = m_object; 
			m_object = nullptr;
			m_exists = false;
			return val;
		}

		~Optional() {
			evict_();
		}

	};
}

using ARLib::Optional;