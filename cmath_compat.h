#pragma once

#include "Concepts.h"
#include "Types.h"
namespace ARLib {
namespace detail {
    enum class InfinityType : uint8_t { None, Plus, Minus };
    bool is_nan(double num);
    bool is_nan(float num);
    bool is_nan(long double num);
    InfinityType inf_check(double num);
    InfinityType inf_check(float num);
    InfinityType inf_check(long double num);
    bool is_infinity(double num);
    bool is_infinity(float num);
    bool is_infinity(long double num);
}    // namespace detail
namespace NumericLimits {
#ifdef COMPILER_MSVC
    constexpr double HugeEnough = 1e+300;    // must overflow

    constexpr auto InfinityD = (HugeEnough * HugeEnough);
    constexpr auto Infinity  = ((float)(HugeEnough * HugeEnough));
    constexpr auto HugeVal   = ((double)Infinity);
    constexpr auto HugeValF  = ((float)Infinity);
    constexpr auto HugeValL  = ((long double)Infinity);
    constexpr auto Nan       = ((float)(Infinity * 0.0f));
    constexpr auto NanD      = ((double)Nan);
#else
    constexpr auto InfinityD = __builtin_inf();
    constexpr auto Infinity  = __builtin_inff();
    constexpr auto HugeVal   = __builtin_huge_val();
    constexpr auto HugeValF  = __builtin_huge_valf();
    constexpr auto HugeValL  = __builtin_huge_vall();
    constexpr auto Nan       = __builtin_nanf("");
    constexpr auto NanD      = __builtin_nan("");
#endif
}    // namespace NumericLimits
namespace NumericConstants {
    template <class T>
    struct InvalidN {};

    template <class T>
    constexpr inline T EV = InvalidN<T>{};
    template <class T>
    constexpr inline T Log2EV = InvalidN<T>{};
    template <class T>
    constexpr inline T Log10EV = InvalidN<T>{};
    template <class T>
    constexpr inline T PiV = InvalidN<T>{};
    template <class T>
    constexpr inline T InvPiV = InvalidN<T>{};
    template <class T>
    constexpr inline T InvSqrtPiV = InvalidN<T>{};
    template <class T>
    constexpr inline T Ln2V = InvalidN<T>{};
    template <class T>
    constexpr inline T Ln10V = InvalidN<T>{};
    template <class T>
    constexpr inline T Sqrt2V = InvalidN<T>{};
    template <class T>
    constexpr inline T Sqrt3V = InvalidN<T>{};
    template <class T>
    constexpr inline T InvSqrt3V = InvalidN<T>{};
    template <class T>
    constexpr inline T EGammaV = InvalidN<T>{};
    template <class T>
    constexpr inline T PhiV = InvalidN<T>{};

    template <>
    constexpr inline double EV<double> = 2.718281828459045;
    template <>
    constexpr inline double Log2EV<double> = 1.4426950408889634;
    template <>
    constexpr inline double Log10EV<double> = 0.4342944819032518;
    template <>
    constexpr inline double PiV<double> = 3.141592653589793;
    template <>
    constexpr inline double InvPiV<double> = 0.3183098861837907;
    template <>
    constexpr inline double InvSqrtPiV<double> = 0.5641895835477563;
    template <>
    constexpr inline double Ln2V<double> = 0.6931471805599453;
    template <>
    constexpr inline double Ln10V<double> = 2.302585092994046;
    template <>
    constexpr inline double Sqrt2V<double> = 1.4142135623730951;
    template <>
    constexpr inline double Sqrt3V<double> = 1.7320508075688772;
    template <>
    constexpr inline double InvSqrt3V<double> = 0.5773502691896257;
    template <>
    constexpr inline double EGammaV<double> = 0.5772156649015329;
    template <>
    constexpr inline double PhiV<double> = 1.618033988749895;

    template <>
    constexpr inline float EV<float> = static_cast<float>(EV<double>);
    template <>
    constexpr inline float Log2EV<float> = static_cast<float>(Log2EV<double>);
    template <>
    constexpr inline float Log10EV<float> = static_cast<float>(Log10EV<double>);
    template <>
    constexpr inline float PiV<float> = static_cast<float>(PiV<double>);
    template <>
    constexpr inline float InvPiV<float> = static_cast<float>(InvPiV<double>);
    template <>
    constexpr inline float InvSqrtPiV<float> = static_cast<float>(InvSqrtPiV<double>);
    template <>
    constexpr inline float Ln2V<float> = static_cast<float>(Ln2V<double>);
    template <>
    constexpr inline float Ln10V<float> = static_cast<float>(Ln10V<double>);
    template <>
    constexpr inline float Sqrt2V<float> = static_cast<float>(Sqrt2V<double>);
    template <>
    constexpr inline float Sqrt3V<float> = static_cast<float>(Sqrt3V<double>);
    template <>
    constexpr inline float InvSqrt3V<float> = static_cast<float>(InvSqrt3V<double>);
    template <>
    constexpr inline float EGammaV<float> = static_cast<float>(EGammaV<double>);
    template <>
    constexpr inline float PhiV<float> = static_cast<float>(PhiV<double>);

    template <>
    constexpr inline long double EV<long double> = static_cast<long double>(EV<double>);
    template <>
    constexpr inline long double Log2EV<long double> = static_cast<long double>(Log2EV<double>);
    template <>
    constexpr inline long double Log10EV<long double> = static_cast<long double>(Log10EV<double>);
    template <>
    constexpr inline long double PiV<long double> = static_cast<long double>(PiV<double>);
    template <>
    constexpr inline long double InvPiV<long double> = static_cast<long double>(InvPiV<double>);
    template <>
    constexpr inline long double InvSqrtPiV<long double> = static_cast<long double>(InvSqrtPiV<double>);
    template <>
    constexpr inline long double Ln2V<long double> = static_cast<long double>(Ln2V<double>);
    template <>
    constexpr inline long double Ln10V<long double> = static_cast<long double>(Ln10V<double>);
    template <>
    constexpr inline long double Sqrt2V<long double> = static_cast<long double>(Sqrt2V<double>);
    template <>
    constexpr inline long double Sqrt3V<long double> = static_cast<long double>(Sqrt3V<double>);
    template <>
    constexpr inline long double InvSqrt3V<long double> = static_cast<long double>(InvSqrt3V<double>);
    template <>
    constexpr inline long double EGammaV<long double> = static_cast<long double>(EGammaV<double>);
    template <>
    constexpr inline long double PhiV<long double> = static_cast<long double>(PhiV<double>);

    constexpr inline double e          = EV<double>;
    constexpr inline double log2e      = Log2EV<double>;
    constexpr inline double log10e     = Log10EV<double>;
    constexpr inline double pi         = PiV<double>;
    constexpr inline double inv_pi     = InvPiV<double>;
    constexpr inline double inv_sqrtpi = InvSqrtPiV<double>;
    constexpr inline double ln2        = Ln2V<double>;
    constexpr inline double ln10       = Ln10V<double>;
    constexpr inline double sqrt2      = Sqrt2V<double>;
    constexpr inline double sqrt3      = Sqrt3V<double>;
    constexpr inline double inv_sqrt3  = InvSqrt3V<double>;
    constexpr inline double egamma     = EGammaV<double>;
    constexpr inline double phi        = PhiV<double>;
}    // namespace NumericConstants
float modf(float x, float* iptr);
double modf(double x, double* iptr);
double sqrt(double arg);
constexpr int64_t constexpr_int_nonneg_pow(int64_t base, uint64_t exponent) {
    int64_t res = 1;
    for (uint64_t i = 0; i < exponent; i++) { res *= base; }
    return res;
}
double pow(double base, double exponent);
extern "C"
{
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
constexpr auto abs(Numeric auto x) {
    return x < decltype(x){ 0 } ? -x : x;
}
}    // namespace ARLib
