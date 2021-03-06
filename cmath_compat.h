#pragma once

#include "Concepts.h"
#include "Types.h"

namespace ARLib {
    namespace detail {
        enum class InfinityType : uint8_t { None, Plus, Minus };
        constexpr bool is_nan(double num);
        constexpr bool is_nan(float num);
        constexpr InfinityType inf_check(double num);
        constexpr InfinityType inf_check(float num);
        constexpr bool is_infinity(double num);
        constexpr bool is_infinity(float num);
    } // namespace detail

    namespace NumericLimits {
#ifdef COMPILER_MSVC
        constexpr double HugeEnough = 1e+300; // must overflow

        constexpr auto InfinityD = (HugeEnough * HugeEnough);
        constexpr auto Infinity = ((float)(HugeEnough * HugeEnough));
        constexpr auto HugeVal = ((double)Infinity);
        constexpr auto HugeValF = ((float)Infinity);
        constexpr auto HugeValL = ((long double)Infinity);
        constexpr auto Nan = ((float)(Infinity * 0.0f));
        constexpr auto NanD = ((double)Nan);
#else
        constexpr auto InfinityD = __builtin_inf();
        constexpr auto Infinity = __builtin_inff();
        constexpr auto HugeVal = __builtin_huge_val();
        constexpr auto HugeValF = __builtin_huge_valf();
        constexpr auto HugeValL = __builtin_huge_vall();
        constexpr auto Nan = __builtin_nanf("");
        constexpr auto NanD = __builtin_nan("");
#endif
    } // namespace NumericLimits

    namespace NumericConstants {
        template <class T>
        struct InvalidN {};

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
        inline constexpr long double EV<long double> = static_cast<long double>(EV<double>);
        template <>
        inline constexpr long double Log2EV<long double> = static_cast<long double>(Log2EV<double>);
        template <>
        inline constexpr long double Log10EV<long double> = static_cast<long double>(Log10EV<double>);
        template <>
        inline constexpr long double PiV<long double> = static_cast<long double>(PiV<double>);
        template <>
        inline constexpr long double InvPiV<long double> = static_cast<long double>(InvPiV<double>);
        template <>
        inline constexpr long double InvSqrtPiV<long double> = static_cast<long double>(InvSqrtPiV<double>);
        template <>
        inline constexpr long double Ln2V<long double> = static_cast<long double>(Ln2V<double>);
        template <>
        inline constexpr long double Ln10V<long double> = static_cast<long double>(Ln10V<double>);
        template <>
        inline constexpr long double Sqrt2V<long double> = static_cast<long double>(Sqrt2V<double>);
        template <>
        inline constexpr long double Sqrt3V<long double> = static_cast<long double>(Sqrt3V<double>);
        template <>
        inline constexpr long double InvSqrt3V<long double> = static_cast<long double>(InvSqrt3V<double>);
        template <>
        inline constexpr long double EGammaV<long double> = static_cast<long double>(EGammaV<double>);
        template <>
        inline constexpr long double PhiV<long double> = static_cast<long double>(PhiV<double>);

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
    } // namespace NumericConstants
    float modf(float x, float* iptr);
    double modf(double x, double* iptr);
    double sqrt(double arg);
    constexpr int64_t constexpr_int_nonneg_pow(int64_t base, uint64_t exponent) {
        int64_t res = 1;
        for (uint64_t i = 0; i < exponent; i++) {
            res *= base;
        }
        return res;
    }
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

    constexpr auto abs(Numeric auto x) { return x < decltype(x){0} ? -x : x; }
} // namespace ARLib
