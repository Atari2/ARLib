#include "cmath_compat.h"
#ifdef _MSC_VER
#include <emmintrin.h>
#endif
namespace ARLib {
	double sqrt(double arg)
	{
#ifdef _MSC_VER
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_sqrt_pd(_mm_load1_pd(&arg)));
		return mem_addr[0];
#else
		double b = 0.0;
		__asm__ volatile(
			"movq %1, %%xmm0 \n"
			"sqrtsd %%xmm0, %%xmm1 \n"
			"movq %%xmm1, %0 \n"
			: "=r"(b)
			: "g"(arg)
			: "xmm0", "xmm1", "memory"
			);
		return b;
#endif
	}
}