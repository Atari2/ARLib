#pragma once
#define HASHBASE_INCLUDE
#include "HashCalc.h"
#include "cstring_compat.h"

namespace ARLib {
	inline constexpr uint32_t hash(int n) {
		return hash32(n);
	}

	inline constexpr uint32_t hash(char c) {
		return static_cast<uint32_t>(c);
	}

	inline constexpr uint32_t hash(size_t s) {
		return hashsize(s);
	}

	inline constexpr uint32_t hash(const void* s) {
		return hashptr(s);
	}

	inline constexpr uint32_t hash(const char* s) {
		return hashstr(s, strlen(s));
	}

	inline constexpr bool hash_equals(int n, int m) {
		return hash(n) == hash(m);
	}

	inline constexpr bool hash_equals(char c, char d) {
		return hash(c) == hash(d);
	}

	inline constexpr bool hash_equals(size_t s, size_t z) {
		return hash(s) == hash(z);
	}

	inline constexpr bool hash_equals(const void* s, const void* z) {
		return hash(s) == hash(z);
	}

	inline bool hash_equals(const char* s, const char* z) {
		return hash(s) == hash(z);
	}
}