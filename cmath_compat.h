#pragma once

#include "Types.h"
namespace ARLib {
	namespace detail {
		enum class InfinityType : uint8_t {
			None,
			Plus,
			Minus
		};
		constexpr bool is_nan(double num);
		constexpr bool is_nan(float num);
		constexpr InfinityType inf_check(double num);
		constexpr InfinityType inf_check(float num);
		constexpr bool is_infinity(double num);
		constexpr bool is_infinity(float num);
	}

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

	namespace NumericConstants {
		template <class T>
		struct InvalidN {

		};

		template <class T>
		inline constexpr T EV = InvalidN<T>{};
		template <class T>
		inline constexpr T Log2EV = InvalidN<T>{};
		template <class T>
		inline constexpr T Log10EV = InvalidN<T>{};
		template <class T>
		inline constexpr T PiV = InvalidN<T>{};
		template <class T>
		inline constexpr T InvPiV = InvalidN<T>{};
		template <class T>
		inline constexpr T InvSqrtPiV = InvalidN<T>{};
		template <class T>
		inline constexpr T Ln2V = InvalidN<T>{};
		template <class T>
		inline constexpr T Ln10V = InvalidN<T>{};
		template <class T>
		inline constexpr T Sqrt2V = InvalidN<T>{};
		template <class T>
		inline constexpr T Sqrt3V = InvalidN<T>{};
		template <class T>
		inline constexpr T InvSqrt3V = InvalidN<T>{};
		template <class T>
		inline constexpr T EGammaV = InvalidN<T>{};
		template <class T>
		inline constexpr T PhiV = InvalidN<T>{};

		template <>
		inline constexpr double EV<double> = 2.718281828459045;
		template <>
		inline constexpr double Log2EV<double> = 1.4426950408889634;
		template <>
		inline constexpr double Log10EV<double> = 0.4342944819032518;
		template <>
		inline constexpr double PiV<double> = 3.141592653589793;
		template <>
		inline constexpr double InvPiV<double> = 0.3183098861837907;
		template <>
		inline constexpr double InvSqrtPiV<double> = 0.5641895835477563;
		template <>
		inline constexpr double Ln2V<double> = 0.6931471805599453;
		template <>
		inline constexpr double Ln10V<double> = 2.302585092994046;
		template <>
		inline constexpr double Sqrt2V<double> = 1.4142135623730951;
		template <>
		inline constexpr double Sqrt3V<double> = 1.7320508075688772;
		template <>
		inline constexpr double InvSqrt3V<double> = 0.5773502691896257;
		template <>
		inline constexpr double EGammaV<double> = 0.5772156649015329;
		template <>
		inline constexpr double PhiV<double> = 1.618033988749895;

		template <>
		inline constexpr float EV<float> = static_cast<float>(EV<double>);
		template <>
		inline constexpr float Log2EV<float> = static_cast<float>(Log2EV<double>);
		template <>
		inline constexpr float Log10EV<float> = static_cast<float>(Log10EV<double>);
		template <>
		inline constexpr float PiV<float> = static_cast<float>(PiV<double>);
		template <>
		inline constexpr float InvPiV<float> = static_cast<float>(InvPiV<double>);
		template <>
		inline constexpr float InvSqrtPiV<float> = static_cast<float>(InvSqrtPiV<double>);
		template <>
		inline constexpr float Ln2V<float> = static_cast<float>(Ln2V<double>);
		template <>
		inline constexpr float Ln10V<float> = static_cast<float>(Ln10V<double>);
		template <>
		inline constexpr float Sqrt2V<float> = static_cast<float>(Sqrt2V<double>);
		template <>
		inline constexpr float Sqrt3V<float> = static_cast<float>(Sqrt3V<double>);
		template <>
		inline constexpr float InvSqrt3V<float> = static_cast<float>(InvSqrt3V<double>);
		template <>
		inline constexpr float EGammaV<float> = static_cast<float>(EGammaV<double>);
		template <>
		inline constexpr float PhiV<float> = static_cast<float>(PhiV<double>);

		template <>
		inline constexpr long double EV<long double> = EV<double>;
		template <>
		inline constexpr long double Log2EV<long double> = Log2EV<double>;
		template <>
		inline constexpr long double Log10EV<long double> = Log10EV<double>;
		template <>
		inline constexpr long double PiV<long double> = PiV<double>;
		template <>
		inline constexpr long double InvPiV<long double> = InvPiV<double>;
		template <>
		inline constexpr long double InvSqrtPiV<long double> = InvSqrtPiV<double>;
		template <>
		inline constexpr long double Ln2V<long double> = Ln2V<double>;
		template <>
		inline constexpr long double Ln10V<long double> = Ln10V<double>;
		template <>
		inline constexpr long double Sqrt2V<long double> = Sqrt2V<double>;
		template <>
		inline constexpr long double Sqrt3V<long double> = Sqrt3V<double>;
		template <>
		inline constexpr long double InvSqrt3V<long double> = InvSqrt3V<double>;
		template <>
		inline constexpr long double EGammaV<long double> = EGammaV<double>;
		template <>
		inline constexpr long double PhiV<long double> = PhiV<double>;

		inline constexpr double e = EV<double>;
		inline constexpr double log2e = Log2EV<double>;
		inline constexpr double log10e = Log10EV<double>;
		inline constexpr double pi = PiV<double>;
		inline constexpr double inv_pi = InvPiV<double>;
		inline constexpr double inv_sqrtpi = InvSqrtPiV<double>;
		inline constexpr double ln2 = Ln2V<double>;
		inline constexpr double ln10 = Ln10V<double>;
		inline constexpr double sqrt2 = Sqrt2V<double>;
		inline constexpr double sqrt3 = Sqrt3V<double>;
		inline constexpr double inv_sqrt3 = InvSqrt3V<double>;
		inline constexpr double egamma = EGammaV<double>;
		inline constexpr double phi = PhiV<double>;
	}
	float modf(float x, float* iptr);
	double modf(double x, double* iptr);
	double sqrt(double arg);
	double pow(double base, double exponent);
	extern "C" {
		double arlib_exp(double x);
		double arlib_log(double x);
		double arlib_log10(double x);
	}
	double exp(double x);
	double log(double x);
	double log10(double x);
	double ceil(double x);
	double floor(double x);
	double fmod(double numer, double denom);
	double trunc(double x);
	double round(double x);

	constexpr double abs(double x) {
		return x < 0.0 ? -x : x;
	}
	constexpr int abs(int x) {
		return x < 0 ? -x : x;
	}
	constexpr float abs(float x) {
		return x < 0.0f ? -x : x;
	}
}
