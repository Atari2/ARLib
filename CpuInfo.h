#pragma once
#include "Types.h"

// 95% of the work in this and in CpuInfo.cpp was done by Alcaro (https://github.com/Alcaro)
// along with MSVC compatibility, so huge thanks to him for this, would've taken me ages if it wasn't for his prior work

namespace ARLib {

#ifdef _MSC_VER
#ifdef _M_AMD64
#define __x86_64__ 1
#define __SSE__ 1
#define __SSE2__ 1
#endif
#ifdef __AVX__ // conveniently, msvc's AVX names match the GCC names
#define __SSE3__ 1
#define __SSSE3__ 1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#define __POPCNT__ 1 // implied by SSE4.2; PCLMUL and AES are not implied by anything
#endif
	// AVX2 implies nothing except itself and AVX
#ifdef __AVX512F__
#define __FMA__ 1
#endif
#endif
	class CPUInfo {
		uint32_t cpuinfo[3] = { 0 };
	public:
		CPUInfo();
		constexpr uint32_t operator[](size_t index) {
			if (index > 2) return 0xFFFF;
			return cpuinfo[index];
		}
		constexpr bool mmx() {
#ifdef __MMX__
			return true;
#else
			return (cpuinfo[1] & 0x00800000);
#endif
		}
		constexpr bool sse() {
#ifdef __SSE__
			return true;
#else
			return (cpuinfo[1] & 0x02000000);
#endif
		}
		constexpr bool sse2() {
#ifdef __SSE2__
			return true;
#else
			return (cpuinfo[1] & 0x04000000);
#endif
		}

		constexpr bool lzcnt() {
#ifdef __LZCNT__
			return true;
#else
			return (cpuinfo[0] & 0x00000020);
#endif
		}

		constexpr bool popcnt() {
#ifdef __POPCNT__
			return true;
#else
			return (cpuinfo[0] & 0x00800000);
#endif
		}

		constexpr bool aes() {
#ifdef __AES__
			return true;
#else
			return (cpuinfo[0] & 0x02000000);
#endif
		}

		constexpr bool pclmul() {
#ifdef __PCLMUL__
			return true;
#else
			return (cpuinfo[0] & 0x00000002);
#endif
		}

		constexpr bool sse3() {
#ifdef __SSE3__
			return true;
#else
			return (cpuinfo[0] & 0x00000001);
#endif
		}

		constexpr bool ssse3() {
#ifdef __SSSE3__
			return true;
#else
			return (cpuinfo[0] & 0x00000200);
#endif
		}

		constexpr bool sse4_1() {
#ifdef __SSE4_1__
			return true;
#else
			return (cpuinfo[0] & 0x00080000);
#endif
		}

		constexpr bool sse4_2() {
#ifdef __SSE4_2__
			return true;
#else
			return (cpuinfo[0] & 0x00100000);
#endif
		}

		constexpr bool avx() {
#ifdef __AVX__
			return true;
#else
			return (cpuinfo[0] & 0x10000000);
#endif
		}

		constexpr bool fma() {
#ifdef __FMA__
			return true;
#else
			return (cpuinfo[0] & 0x00001000);
#endif
		}

		constexpr bool rdrnd() {
#ifdef __RDRND__
			return true;
#else
			return (cpuinfo[0] & 0x40000000);
#endif
		}

		constexpr bool f16c() {
#ifdef __F16C__
			return true;
#else
			return (cpuinfo[0] & 0x20000000);
#endif
		}

		constexpr bool bmi() {
#ifdef __BMI__
			return true;
#else
			return (cpuinfo[2] & 0x00000008);
#endif
		}

		constexpr bool fsgsbase() {
#ifdef __FSGSBASE__
			return true;
#else
			return (cpuinfo[2] & 0x00000001);
#endif
		}

		constexpr bool avx2() {
#ifdef __AVX2__
			return true;
#else
			return (cpuinfo[2] & 0x00000020);
#endif
		}

		constexpr bool bmi2() {
#ifdef __BMI2__
			return true;
#else
			return (cpuinfo[2] & 0x00000100);
#endif
		}

		constexpr bool rdseed() {
#ifdef __RDSEED__
			return true;
#else
			return (cpuinfo[2] & 0x00040000);
#endif
		}

		constexpr bool adx() {
#ifdef __ADX__
			return true;
#else
			return (cpuinfo[2] & 0x00080000);
#endif
		}

		constexpr bool clflushopt() {
#ifdef __CLFLUSHOPT__
			return true;
#else
			return (cpuinfo[2] & 0x00800000);
#endif
		}
	};

	static inline CPUInfo cpuinfo{};
}

using ARLib::cpuinfo;