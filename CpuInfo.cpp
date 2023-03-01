#include "CpuInfo.h"
#include "Compat.h"
#ifdef WINDOWS
    #include <immintrin.h>
    #include <intrin.h>

    #define bit_OSXSAVE (1 << 27)
    #define bit_AVX     (1 << 28)
    #define bit_FMA     (1 << 12)
    #define bit_AVX2    (1 << 5)
#else
    #include <cpuid.h>
    #define cpuid         __cpuid
    #define cpuid_count   __cpuid_count
    #define get_cpuid_max __get_cpuid_max

    #define xgetbv(feat, a, d) __asm__ volatile("xgetbv" : "=a"(a), "=d"(d) : "c"(feat))
#endif
namespace ARLib {

CPUInfo cpuinfo{};

#ifdef WINDOWS
static void cpuid(uint32_t level, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx) {
    int info[4];
    __cpuid(info, level);
    eax = info[0];
    ebx = info[1];
    ecx = info[2];
    edx = info[3];
}
static void cpuid_count(uint32_t level, uint32_t count, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx) {
    int info[4];
    __cpuidex(info, level, count);
    eax = info[0];
    ebx = info[1];
    ecx = info[2];
    edx = info[3];
}
static void xgetbv(uint32_t feat, uint32_t& a, uint32_t& d) {
    uint64_t ret = _xgetbv(feat);
    a            = static_cast<uint32_t>(ret & 0xFFFFFFFF);
    d            = ret >> 32;
}
static uint32_t get_cpuid_max(uint32_t ext, uint32_t* sig) {
    int info[4];
    __cpuid(info, ext);
    if (sig) *sig = info[1];
    return info[0];
}
#endif

#ifndef _XCR_XFEATURE_ENABLED_MASK
    #define _XCR_XFEATURE_ENABLED_MASK 0
#endif

#define XSTATE_FP     0x1
#define XSTATE_SSE    0x2
#define XSTATE_YMM    0x4
#define XSTATE_OPMASK 0x20
#define XSTATE_ZMM    0x40
#define XSTATE_HI_ZMM 0x80
CPUInfo::CPUInfo() noexcept {
#ifdef COMPILER_CLANG
    [[maybe_unused]] uint32_t cpuid_max = static_cast<uint32_t>(get_cpuid_max(0u, nullptr));
#else
    [[maybe_unused]] uint32_t cpuid_max = get_cpuid_max(0u, nullptr);
#endif
#ifndef __SSE__
    if (cpuid_max < 1) return;    // comments in cpuid.h say x86_64 always has cpuid
#endif

    uint32_t dummy1{ 0 }, dummy2{ 0 };
    uint32_t l1ecx{ 0 }, l1edx{ 0 };

    cpuid(1, dummy1, dummy2, l1ecx, l1edx);

    uint32_t osxsave = 0;
#ifndef __AVX__
    if (l1ecx & bit_OSXSAVE)
#endif
    {
        xgetbv(_XCR_XFEATURE_ENABLED_MASK, osxsave, dummy1);
    }

#ifndef __AVX__
    if (XSTATE_YMM & ~osxsave) l1ecx &= ~(bit_AVX | bit_FMA);
#endif

    cpuinfo[0] = l1ecx;
    cpuinfo[1] = l1edx;

#ifndef __AVX2__
    if (cpuid_max >= 7)
#endif
    {
        uint32_t l7ebx{ 0 }, l7ecx{ 0 }, l7edx{ 0 };
        cpuid_count(7, 0, dummy1, l7ebx, l7ecx, l7edx);

#ifndef __AVX__
        if (XSTATE_YMM & ~osxsave) l7ebx &= ~bit_AVX2;
#endif

        cpuinfo[2] = l7ebx;
    }
}
}    // namespace ARLib
