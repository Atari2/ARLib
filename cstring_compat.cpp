#include "cstring_compat.h"
#include "CpuInfo.h"

namespace ARLib {
    typedef intptr_t word;
#define wsize sizeof(word)
#define wmask (wsize - 1)

    void* memcpy_vectorized(void* dst0, const void* src0, size_t num) {
        char* dst = static_cast<char*>(dst0);
        const char* src = static_cast<const char*>(src0);
        size_t rem = num % 32;
        for (size_t offset = 0; offset < (num - rem); offset += 32) {
            __m256i buffer = _mm256_loadu_si256((const __m256i*)(src + offset));
            _mm256_storeu_si256((__m256i*)(dst + offset), buffer);
        }
        dst += (num - rem);
        src += (num - rem);
        for (size_t i = 0; i < rem; i++) {
            *dst++ = *src++;
        }
        return dst;
    }

    void* memset_vectorized(void* dst0, uint8_t val, size_t size) {
        uint8_t* dst = static_cast<uint8_t*>(dst0);
        size_t rem = size % 32;
        for (size_t offset = 0; offset < (size - rem); offset += 32) {
            __m256i buffer = _mm256_set1_epi8(val);
            _mm256_storeu_si256((__m256i*)(dst + offset), buffer);
        }
        dst += (size - rem);
        for (size_t i = 0; i < rem; i++) {
            *dst++ = val;
        }
        return dst0;
    }

    int memcmp(void* dst, const void* src, size_t num) { return __builtin_memcmp(dst, src, num); }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4311 4302)
#endif
    // stolen from an old stdlib implementation
    // assumes that sizeof(char) == 1... should probably use uint8_t but eh
    void* memmove(void* dst0, const void* src0, size_t length) {
        uint8_t* dst = static_cast<uint8_t*>(dst0);
        const uint8_t* src = static_cast<const uint8_t*>(src0);
        if (length == 0 || dst == src) return dst;
        if ((uintptr_t)dst < (uintptr_t)src) {
            // copy forward
            size_t t = (size_t)src;
            if ((t | (size_t)dst) & wmask) {
                if ((t ^ (size_t)dst) & wmask || length < wsize)
                    t = length;
                else
                    t = wsize - (t & wmask);
                length -= t;
                do {
                    *dst++ = *src++;
                } while (--t);
            }
            t = length / wsize;
            if (t) {
                do {
                    *(word*)dst = *(word*)src;
                    src += wsize;
                    dst += wsize;
                } while (--t);
            }
            t = length & wmask;
            if (t) {
                do {
                    *dst++ = *src++;
                } while (--t);
            }
        } else {
            src += length;
            dst += length;
            size_t t = (size_t)src;
            if ((t | (size_t)dst) & wmask) {
                if ((t ^ (size_t)dst) & wmask || length <= wsize)
                    t = length;
                else
                    t &= wmask;
                length -= t;
                do {
                    *--dst = *--src;
                } while (--t);
            }
            t = length / wsize;
            if (t) {
                do {
                    src -= wsize;
                    dst -= wsize;
                    *(word*)dst = *(const word*)src;
                } while (--t);
            }
            t = length & wmask;
            if (t) {
                do {
                    *--dst = *--src;
                } while (--t);
            }
        }
        return dst0;
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    void* memset(void* ptr, uint8_t value, size_t size) {
        if (size >= 64 && cpuinfo.avx2()) { // check avx2 support
            return memset_vectorized(ptr, value, size);
        }
        uint8_t* dst = static_cast<uint8_t*>(ptr);
        for (size_t i = 0; i < size; i++)
            dst[i] = value;
        return ptr;
    }
} // namespace ARLib
#undef word
#undef wmask
