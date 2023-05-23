#include "FlatSet.hpp"
#include <immintrin.h>
#ifdef COMPILER_MSVC
    #include <intrin.h>
#endif
namespace ARLib {
namespace internal {
    uint32_t trailing_zeros(uint32_t val) {
#ifdef COMPILER_MSVC
        unsigned long result = 0;
        _BitScanForward(&result, val);
        return result;
#else
        return static_cast<uint32_t>(__builtin_ctz(val));
#endif
    }
    BitMask<uint32_t> match(int8_t hash, const MetadataBlock& block) {
        auto ctrl  = _mm_loadu_si128(reinterpret_cast<const __m128i*>(block.data()));
        auto match = _mm_set1_epi8(static_cast<char>(hash));
        return BitMask{ static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl))) };
    }
    BitMask<uint32_t> match_empty(const MetadataBlock& block) {
        auto ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(block.data()));
        return BitMask{ static_cast<uint32_t>(_mm_movemask_epi8(_mm_sign_epi8(ctrl, ctrl))) };
    }
    BitMask<uint32_t> match_non_empty(const MetadataBlock& block) {
        auto all_neg_ones = _mm_set1_epi8(-1);
        auto ctrl         = _mm_loadu_si128(reinterpret_cast<const __m128i*>(block.data()));
        return BitMask{ static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(ctrl, all_neg_ones))) };
    }
}    // namespace internal
}    // namespace ARLib