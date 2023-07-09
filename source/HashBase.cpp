#include "HashBase.hpp"
#include "Types.hpp"
#include "cstring_compat.hpp"
#include "Utility.hpp"
#ifdef ON_WINDOWS
    #include <intrin.h>
#endif
namespace ARLib {
#ifdef ON_WINDOWS
uint32_t hash_bswap(uint32_t value) {
    return _byteswap_ulong(value);
}
#endif

#ifdef COMPILER_GCC
#define FORCEINLINE_EXCEPT_GCC 
#else
#define FORCEINLINE_EXCEPT_GCC arlib_forceinline
#endif

static FORCEINLINE_EXCEPT_GCC size_t bitcast_copy(const char* p) {
    size_t result;
    memcpy(&result, p, sizeof(result));
    return result;
}
static FORCEINLINE_EXCEPT_GCC size_t xorshift(size_t val) {
    return val ^ (val >> 47);
}
static FORCEINLINE_EXCEPT_GCC size_t copy_remainder(const char* p, int n) {
    size_t result = 0;
    while (n > 0) { result = (result << BITS_PER_BYTE) + static_cast<uint8_t>(p[--n]); }
    return result;
}
[[nodiscard]] size_t murmur_hash_bytes(const char* const ptr, size_t count, size_t seed) {
    constexpr size_t align         = alignof(size_t) - 1;
    constexpr size_t step          = sizeof(size_t);
    static const size_t mult_magic = (0xC6A4A793_sz << 32_sz) + 0x5BD1E995_sz;
    const size_t len_aligned       = count & ~align;
    const char* const end          = ptr + len_aligned;
    size_t hash                    = seed ^ (count * mult_magic);
    for (const char* p = ptr; p != end; p += step) {
        const size_t data = xorshift(bitcast_copy(p) * mult_magic) * mult_magic;
        hash ^= data;
        hash *= mult_magic;
    }
    if (int rem = (count & align); rem != 0) {
        const size_t data = copy_remainder(end, rem);
        hash ^= data;
        hash *= mult_magic;
    }
    hash = xorshift(hash) * mult_magic;
    hash = xorshift(hash);
    return hash;
}
}    // namespace ARLib