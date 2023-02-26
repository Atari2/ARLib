#include "cstring_compat.h"
#include "CpuInfo.h"
#include <immintrin.h>
#ifdef COMPILER_MSVC
    #include <intrin.h>
#endif
namespace ARLib {
typedef intptr_t word;
constexpr auto wsize                  = sizeof(word);
[[maybe_unused]] constexpr auto wmask = (wsize - 1);

#ifdef COMPILER_MSVC
forceinline uint32_t ctz_msvc(uint32_t value) {
    unsigned long trailing_zero = 0;
    if (_BitScanForward(&trailing_zero, value)) {
        return trailing_zero;
    } else {
        return sizeof(uint32_t) * BITS_PER_BYTE;
    }
}
#endif
size_t first_zero_bit(uint32_t value) {
#ifdef COMPILER_MSVC
    if (cpuinfo.bmi()) {
        return static_cast<size_t>(_tzcnt_u32(value));
    } else {
        return static_cast<size_t>(ctz_msvc(value));
    }
#else
    return static_cast<size_t>(__builtin_ctz(value));
#endif
}
size_t zerobyteidx(__m256i value) {
    int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_setzero_si256(), value));
    if (mask == 0) return sizeof(__m256i);
    return first_zero_bit(static_cast<uint32_t>(mask));
}
size_t strlen_vectorized(const char* src) {
    auto* ptr = reinterpret_cast<const __m256i*>(src);
    size_t sz = 0;
    for (;;) {
        __m256i partial = _mm256_loadu_si256(ptr);
        ptr += 1;
        auto result = zerobyteidx(partial);
        if (result < sizeof(__m256i)) {
            sz += result;
            break;
        }
        sz += result;
    }
    return sz;
}
void* memcpy_vectorized(void* dst0, const void* src0, size_t num) {
    char* dst       = static_cast<char*>(dst0);
    const char* src = static_cast<const char*>(src0);
    size_t rem      = num % 32;
    for (size_t offset = 0; offset < (num - rem); offset += 32) {
#ifdef COMPILER_MSVC
        __m256i buffer = _mm256_loadu_si256((const __m256i*)(src + offset));
        _mm256_storeu_si256((__m256i*)(dst + offset), buffer);
#else
        __m256i buffer = _mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(src + offset));
        _mm256_storeu_si256(reinterpret_cast<__m256i_u*>(dst + offset), buffer);
#endif
    }
    dst += (num - rem);
    src += (num - rem);
    for (size_t i = 0; i < rem; i++) { *dst++ = *src++; }
    return dst;
}
int memcmp(const void* dst, const void* src, size_t num) {
    return __builtin_memcmp(dst, src, num);
}
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4311 4302)
#endif
#if not HAS_BUILTIN(__builtin_memmove)
// stolen from an old stdlib implementation
// assumes that sizeof(char) == 1... should probably use uint8_t but eh
void* memmove(void* dst0, const void* src0, size_t length) {
    auto* dst       = static_cast<uint8_t*>(dst0);
    const auto* src = static_cast<const uint8_t*>(src0);
    if (length == 0 || dst == src) return dst;
    if (reinterpret_cast<uintptr_t>(dst) < reinterpret_cast<uintptr_t>(src)) {
        // copy forward
        auto t = reinterpret_cast<size_t>(src);
        if ((t | reinterpret_cast<size_t>(dst)) & wmask) {
            if ((t ^ reinterpret_cast<size_t>(dst)) & wmask || length < wsize)
                t = length;
            else
                t = wsize - (t & wmask);
            length -= t;
            do { *dst++ = *src++; } while (--t);
        }
        t = length / wsize;
        if (t) {
            do {
                *reinterpret_cast<word*>(dst) = *reinterpret_cast<const word*>(src);
                src += wsize;
                dst += wsize;
            } while (--t);
        }
        t = length & wmask;
        if (t) {
            do { *dst++ = *src++; } while (--t);
        }
    } else {
        src += length;
        dst += length;
        auto t = reinterpret_cast<size_t>(src);
        if ((t | reinterpret_cast<size_t>(dst)) & wmask) {
            if ((t ^ reinterpret_cast<size_t>(dst)) & wmask || length <= wsize)
                t = length;
            else
                t &= wmask;
            length -= t;
            do { *--dst = *--src; } while (--t);
        }
        t = length / wsize;
        if (t) {
            do {
                src -= wsize;
                dst -= wsize;
                *reinterpret_cast<word*>(dst) = *reinterpret_cast<const word*>(src);
            } while (--t);
        }
        t = length & wmask;
        if (t) {
            do { *--dst = *--src; } while (--t);
        }
    }
    return dst0;
}
#endif
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
}    // namespace ARLib
#undef word
#undef wmask
