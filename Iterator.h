#pragma once
namespace ARLib {
	template <typename T>
	class IteratorBase {
	protected:

		T* m_current;
		IteratorBase(T* ptr) : m_current(ptr) {}

		IteratorBase(const IteratorBase<T>& other) : m_current(other.m_current) {

		}

		IteratorBase(IteratorBase<T>&& other) : m_current(other.m_current) {
			other.m_current = nullptr;
		}
	public:
		using Type = T;
		virtual bool operator==(const IteratorBase<T>& other) {
			return m_current == other.m_current;
		}
		virtual bool operator!=(const IteratorBase<T>& other) {
			return m_current != other.m_current;
		}
		virtual bool operator<(const IteratorBase<T>& other) {
			return m_current < other.m_current;
		}
		virtual bool operator>(const IteratorBase<T>& other) {
			return m_current > other.m_current;
		}
		virtual size_t operator-(const IteratorBase<T>& other) {
			return m_current - other.m_current;
		}
	};

	// for some god forsaken reason
#define m_current IteratorBase<T>::m_current

	template <typename T>
	class Iterator final : public IteratorBase<T> {
	public:
		Iterator(T* start) : IteratorBase<T>(start) {}

		Iterator(const Iterator<T>& other) : IteratorBase<T>(other) {

		}

		Iterator(Iterator<T>&& other) noexcept : IteratorBase<T>(other) {
		}

		T& operator*() {
			return *m_current;
		}

		Iterator& operator=(const Iterator<T>& other) {
			m_current = other.m_current;
		}

		Iterator& operator=(Iterator<T>&& other) {
			m_current = other.m_current;
			other.m_current = nullptr;
		}

		Iterator<T>& operator++() {
			m_current++;
			return *this;
		}

		Iterator<T> operator++(int) {
			return { m_current++ };
		}

		Iterator<T>& operator+=(int offset) {
			m_current += offset;
			return *this;
		}

		Iterator<T> operator+(int offset) {
			return { m_current + offset };
		}

		Iterator<T>& operator--() {
			m_current--;
			return *this;
		}

		Iterator<T> operator--(int) {
			return { m_current-- };
		}

		Iterator<T>& operator-=(int offset) {
			m_current -= offset;
			return *this;
		}

		Iterator<T> operator-(int offset) {
			return { m_current - offset };
		}

		bool operator==(const Iterator<T>& other) {
			return m_current == other.m_current;
		}
		bool operator!=(const Iterator<T>& other) {
			return m_current != other.m_current;
		}
		bool operator<(const Iterator<T>& other) {
			return m_current < other.m_current;
		}
		bool operator>(const Iterator<T>& other) {
			return m_current > other.m_current;
		}
		size_t operator-(const Iterator<T>& other) {
			return m_current - other.m_current;
		}

	};

	template <typename T>
	class ConstIterator final : public IteratorBase<T> {
	public:
		ConstIterator(T* start) : IteratorBase<T>(start) {}

		ConstIterator(const ConstIterator<T>& other) : IteratorBase<T>(other) {

		}

		ConstIterator(ConstIterator<T>&& other) : IteratorBase<T>(other) {
			other.m_current = nullptr;
		}

		ConstIterator& operator=(const ConstIterator<T>& other) {
			m_current = other.m_current;
		}

		ConstIterator& operator=(ConstIterator<T>&& other) {
			m_current = other.m_current;
			other.m_current = nullptr;
		}

		const T& operator*() {
			return *m_current;
		}

		ConstIterator<T>& operator++() {
			m_current++;
			return *this;
		}

		ConstIterator<T> operator++(int) {
			return { m_current++ };
		}


		ConstIterator<T>& operator+=(int offset) {
			m_current += offset;
			return *this;
		}

		ConstIterator<T> operator+(int offset) {
			return { m_current + offset };
		}

		ConstIterator<T>& operator--() {
			m_current--;
			return *this;
		}

		ConstIterator<T> operator--(int) {
			return { m_current-- };
		}

		ConstIterator<T>& operator-=(int offset) {
			m_current -= offset;
			return *this;
		}

		ConstIterator<T> operator-(int offset) {
			return { m_current - offset };
		}

	};

	template <typename T>
	class ReverseIterator final : public IteratorBase<T> {
	public:
		ReverseIterator(T* end) : IteratorBase<T>(end) {}

		ReverseIterator(const ReverseIterator<T>& other) : IteratorBase<T>(other.m_current) {

		}

		ReverseIterator(ReverseIterator<T>&& other) : IteratorBase<T>(other) {
			other.m_current = nullptr;
		}

		T& operator*() {
			return *m_current;
		}

		ReverseIterator& operator=(const ReverseIterator<T>& other) {
			m_current = other.m_current;
		}

		ReverseIterator& operator=(ReverseIterator<T>&& other) {
			m_current = other.m_current;
			other.m_current = nullptr;
		}

		ReverseIterator<T>& operator++() {
			m_current--;
			return *this;
		}

		ReverseIterator<T> operator++(int) {
			return { m_current-- };
		}

		ReverseIterator<T>& operator+=(int offset) {
			m_current -= offset;
			return *this;
		}

		ReverseIterator<T> operator+(int offset) {
			return { m_current - offset };
		}

		ReverseIterator<T>& operator--() {
			m_current++;
			return *this;
		}

		ReverseIterator<T> operator--(int) {
			return { m_current++ };
		}

		ReverseIterator<T>& operator-=(int offset) {
			m_current += offset;
			return *this;
		}

		ReverseIterator<T> operator-(int offset) {
			return { m_current + offset };
		}
	};

	template <typename T>
	class ConstReverseIterator final : public IteratorBase<T> {
	public:
		ConstReverseIterator(T* end) : IteratorBase<T>(end) {}

		ConstReverseIterator(const ConstReverseIterator<T>& other) : IteratorBase<T>(other.m_current) {

		}

		ConstReverseIterator(ConstReverseIterator<T>&& other) : IteratorBase<T>(other) {
			other.m_current = nullptr;
		}

		const T& operator*() {
			return *m_current;
		}

		ConstReverseIterator& operator=(const ConstReverseIterator<T>& other) {
			m_current = other.m_current;
		}

		ConstReverseIterator& operator=(ConstReverseIterator<T>&& other) {
			m_current = other.m_current;
			other.m_current = nullptr;
		}

		ConstReverseIterator<T>& operator++() {
			m_current--;
			return *this;
		}

		ConstReverseIterator<T> operator++(int) {
			return { m_current-- };
		}

		ConstReverseIterator<T>& operator+=(int offset) {
			m_current -= offset;
			return *this;
		}

		ConstReverseIterator<T> operator+(int offset) {
			return { m_current - offset };
		}

		ConstReverseIterator<T>& operator--() {
			m_current++;
			return *this;
		}

		ConstReverseIterator<T> operator--(int) {
			return { m_current++ };
		}

		ConstReverseIterator<T>& operator-=(int offset) {
			m_current += offset;
			return *this;
		}

		ConstReverseIterator<T> operator-(int offset) {
			return { m_current + offset };
		}
	};
#undef m_current
}

using ARLib::Iterator;
using ARLib::ConstIterator;
using ARLib::ReverseIterator;
using ARLib::ConstReverseIterator;
