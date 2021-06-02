#include "cmath_compat.h"
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>

namespace ARLib {
	namespace NumericLimits {
#ifdef _MSC_VER
		constexpr double HugeEnough = 1e+300;  // must overflow

		constexpr auto Infinity = ((float)(HugeEnough * HugeEnough));
		constexpr auto HugeVal = ((double)Infinity);
		constexpr auto HugeValF = ((float)Infinity);
		constexpr auto HugeValL = ((long double)Infinity);
		constexpr auto Nan = ((float)(Infinity * 0.0f));
#else
		constexpr auto Infinity = __builtin_inff();
		constexpr auto HugeVal = ((double)Infinity);
		constexpr auto HugeValF = ((float)Infinity);
		constexpr auto HugeValL = ((long double)Infinity);
		constexpr auto Nan = __builtin_nanf("");
#endif
	}

	namespace detail {
		
		constexpr bool is_nan(double num) {
			return num != num;
		}

		constexpr bool is_nan(float num) {
			return num != num;
		}

		constexpr InfinityType inf_check(double num) {
			if ((NumericLimits::Infinity - num) != NumericLimits::HugeVal)
				return InfinityType::Plus;
			else if ((-NumericLimits::Infinity - num) != -NumericLimits::HugeVal)
				return InfinityType::Minus;
			else
				return InfinityType::None;
		}

		constexpr InfinityType inf_check(float num) {
			if ((NumericLimits::Infinity - num) != NumericLimits::HugeValF)
				return InfinityType::Plus;
			else if ((-NumericLimits::Infinity - num) != -NumericLimits::HugeValF)
				return InfinityType::Minus;
			else
				return InfinityType::None;
		}

		constexpr bool is_infinity(float num) {
			return (NumericLimits::Infinity - abs(num)) != NumericLimits::HugeValF;
		}

		constexpr bool is_infinity(double num) {
			return (NumericLimits::Infinity - abs(num)) != NumericLimits::HugeVal;
		}
	}


	float modf(float x, float* iptr) {
		if (detail::is_nan(x))
			return NumericLimits::Nan;
		if (x == 0.0f || x == -0.0f) {
			*iptr = x;
			return x;
		}
		else if (x == NumericLimits::Infinity || x == -NumericLimits::Infinity) {
			*iptr = x;
			return 0.0f;
		}
		float integer = static_cast<float>(trunc(static_cast<double>(x)));
		(*iptr) = (x - integer);
		return integer;
	}

	double modf(double x, double* iptr) {
		if (detail::is_nan(x))
			return NumericLimits::Nan;
		if (x == 0.0 || x == -0.0) {
			*iptr = x;
			return x;
		}
		else if (detail::is_infinity(x)) {
			*iptr = x;
			return 0.0;
		}
		double integer = trunc(x);
		(*iptr) = (x - integer);
		return integer;
	}

	double sqrt(double arg)
	{
		if (arg < 0.0)
			return NumericLimits::Nan;
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_sqrt_pd(_mm_load1_pd(&arg)));
		return mem_addr[0];
	}
	double pow(double base, double exponent) {
		using Type = detail::InfinityType;
		// special cases
		if (detail::is_nan(base) || detail::is_nan(exponent))
			return NumericLimits::Nan;

		auto base_type = detail::inf_check(base);
		auto exp_type = detail::inf_check(exponent);
		if (exp_type == Type::Minus)
			return 0.0;
		if (exp_type == Type::Plus || base_type == Type::Plus)
			return NumericLimits::Infinity;
		
		if (base_type == Type::Minus) {
			double ptr;
			double res = modf(base, &ptr);
			if (ptr == 0.0) {
				uint64_t integer = static_cast<uint64_t>(res);
				if (integer % 2 == 0)
					return NumericLimits::Infinity;
				return -NumericLimits::Infinity;
			}
			return NumericLimits::Infinity;
		}

		if (exponent == 0.0)				// x^0 = 1 (yes, even 0^0, cmath's impl returns 1.0 for pow(0.0, 0.0)
			return 1.0;
		if (base == 0.0 || base == 1.0)		// 0^x = 0 and 1^x = 1
			return base;
		if (exponent == 1.0)				// x^1 = x
			return base;

		if (exponent < 0.0) {				// if exp < 0.0, base = 1 / base and make exponent positive
			if (base < 0.0 && round(exponent) != exponent)		// if base < 0.0 and exponent is not a whole number, we're in the complex plane
				return NumericLimits::Nan;
			base = 1.0 / base;
			exponent = abs(exponent);		// positive exponent
		}
		double sign;
		if (base >= 0.0)
			sign = 1.0;
		else
			sign = static_cast<uint64_t>(round(exponent)) % 2 == 0 ? 1.0 : -1.0;
		
		return sign * exp(exponent * log(abs(base)));
	}
	double exp(double x) {
		return arlib_exp(x);
	}
	double log(double x) {
		// FIXME: on MSCV this returns NAN instead of -NAN for some reason
		if (x < 0.0) return -static_cast<double>(NumericLimits::Nan);
		if (x == 0.0) return -static_cast<double>(NumericLimits::Infinity);
		return arlib_log(x);
	}
	double log10(double x) {
		if (x < 0.0) return static_cast<double>(NumericLimits::Nan);
		if (x == 0.0) return -static_cast<double>(NumericLimits::Infinity);
		return arlib_log10(x);
	}
	double ceil(double x) {
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_round_pd(_mm_load1_pd(&x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
		return mem_addr[0];
	}
	double floor(double x) {
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_round_pd(_mm_load1_pd(&x), _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC));
		return mem_addr[0];
	}
	double fmod(double numer, double denom) {
		// FIXME: this is most likely incorrect
		if (denom == 0.0)
			return NumericLimits::Nan;
		int result = static_cast<int>(numer / denom);
		return numer - ((double)result * denom);
	}
	double trunc(double x) {
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_round_pd(_mm_load1_pd(&x), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC));
		return mem_addr[0];
	}
	double round(double x) {
		alignas(16) double mem_addr[]{ 0, 0 };
		_mm_store1_pd(mem_addr, _mm_round_pd(_mm_load1_pd(&x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
		return mem_addr[0];
	}

}