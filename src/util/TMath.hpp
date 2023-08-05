/**
 * @file TMath.hpp
 * @author Tobias Kozel
 * @brief Wraps all cmath functions needed in tklb.
 *        Also provides more or less sane fallbacks without the std.
 * @version 0.1
 * @date 2023-01-07
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_MATH
#define _TKLB_MATH

#ifndef TKLB_NO_STDLIB
	#include <cmath>
#endif

#include "./TTraits.hpp"


namespace tklb {
	template <typename T>
	constexpr T PI = 3.14159265358979323846;

	template <typename T>
	constexpr bool isPowerof2(T v) {
		static_assert(!traits::IsFloat<T>::value, "isPowerof2 only works with integers");
		return v && ((v & (v - 1)) == 0);
	}

	template <typename T>
	constexpr T min(const T& v1, const T& v2) {
		return v1 < v2 ? v1 : v2;
	}

	template <typename T>
	constexpr T max(const T& v1, const T& v2) {
		return v1 < v2 ? v2 : v1;
	}

	template <typename T>
	constexpr T clamp(const T& v, const T& _min, const T& _max) {
		return min(_max, max(v, _min));
	}

	template <typename T>
	constexpr T abs(const T& v) {
		return T(0) <= v ? v : -v;
	}

	template <typename T, typename T2>
	constexpr T lerp(const T& v1, const T& v2, const T2& t) {
		return v1 + t * (v2 - v1);
	}

	template <typename T>
	constexpr T floor(const T& v) {
		#ifdef TKLB_NO_STDLIB
			const auto trucated = (long) v;
			if (trucated == v) { return v; }
			return T(0) < v ? (long) v : ((long) v) - T(1);
		#else
			return std::floor(v);
		#endif
	}

	template <typename T>
	constexpr T ceil(const T& v) {
		#ifdef TKLB_NO_STDLIB
			const auto trucated = (long) v;
			if (trucated == v) { return v; }
			return T(0) < v ? ((long) v) + T(1) : (long) v;
		#else
			return std::ceil(v);
		#endif
	}

	template <typename T>
	constexpr T round(const T& v) {
		#ifdef TKLB_NO_STDLIB
			const auto trucated = (long) v;
			if (trucated == v) { return v; }
			return (long) ((T(0) < v) ? (v + T(0.5)) : (v - T(0.5)));
		#else
			return std::round(v);
		#endif
	}

	template <typename T>
	constexpr T cos(const T& v) {
		static_assert(traits::IsFloat<T>::value, "cos only works with float/double");
		#ifdef TKLB_NO_STDLIB
			// https://stackoverflow.com/a/28050328
			constexpr T tp = T(1) / (T(2) * PI<T>);
			T x = v * tp;
			x -= T(0.25) + floor(x + T(0.25));
			x *= T(16) * (abs(x) - T(0.5));
			x += T(0.225) * x * (abs(x) - T(1)); // Optional
			return x;
		#else
			return std::cos(v);
		#endif
	}

	template <typename T>
	constexpr T sin(const T& v) {
		static_assert(traits::IsFloat<T>::value, "sin only works with float/double");
		#ifdef TKLB_NO_STDLIB
			return cos(v - T(0.5) * PI<T>);
		#else
			return std::sin(v);
		#endif
	}

	template <typename T>
	constexpr T atan(const T& v) {
		static_assert(traits::IsFloat<T>::value, "atan only works with float/double");
		#ifdef TKLB_NO_STDLIB
			// https://stackoverflow.com/a/42542593
			constexpr T A =  0.0776509570923569;
			constexpr T B = -0.287434475393028;
			constexpr T C =  ((PI<T> / T(4)) - A - B);
			const T v2 = v * v;
  			return ((A * v2 + B)* v2 + C) * v;
		#else
			return std::atan(v);
		#endif
	}

	template <typename T>
	constexpr T pow(const T& x, const T& y) {
		#ifdef TKLB_NO_STDLIB
			// https://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/
			// this isn't really up to spec
			union {
				double d;
				int x[2];
			} u = { (double) x };
			u.x[1] = (int(y) * (u.x[1] - 1072632447) + 1072632447);
			u.x[0] = 0;
			return u.d;
		#else
			return std::pow(x, y);
		#endif
	}

	/**
	 * @brief This beeing constexpr allows the SPCAP to be prepared at compile time
	 *        for the predefined speaker setups.
	 * @param x
	 * @return constexpr T
	 */
	template <typename T>
	constexpr T sqrt(const T& x) {
		#ifdef TKLB_NO_STDLIB
			// https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi sqrt3
			union {
				float x;
				int i;
			} u = { (float) x };
			u.x = x;
			u.i = (1<<29) + (u.i >> 1) - (1<<22);
			return u.x;
		#else
			// TODO apparently not all std lib versions have constexpr sqrt
			return std::sqrt(x);
		#endif
	}
} // namespace

#endif // _TKLB_MATH
