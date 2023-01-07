#ifndef _TKLB_MATH
#define _TKLB_MATH

#ifndef TKLB_NO_STDLIB
	#include <cmath>
#else
	#include "./TAssert.h"
#endif


namespace tklb {
	constexpr double PI = 3.14159265358979323846;

	template <typename T>
	constexpr bool isPowerof2(T v) {
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
	constexpr T round(T v) {
		return v;
	}
	template <>
	constexpr float round(float v) {
		return (long) (v + float(0.5));
	}
	template <>
	constexpr double round(double v) {
		return (long) (v + double(0.5));
	}

	template <typename T>
	constexpr T pow(T x, T y) {
		#ifdef TKLB_NO_STDLIB
			TKLB_ASSERT(false) // TODO
			return 0;
		#else
			return std::pow(x, y);
		#endif
	}

	/**
	 * @brief This beeing constexpr allows the SPCAP to be prepared at compile time
	 *        for the predefined speaker setups.
	 * @param x
	 * @return constexpr float
	 */
	#ifdef TKLB_NO_STDLIB
	// TODO get a constexpr version going
	float sqrt(const float& x) {
		// ! absolutely untested
		// https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi sqrt3
		union {
			int i;
			float x;
		} u;
		u.x = x;
		u.i = (1<<29) + (u.i >> 1) - (1<<22);
		return u.x;
	}
	#else
	constexpr float sqrt(const float& x) {
		// TODO apparently not all std versions have constexpr sqrt
		return std::sqrt(x);
	}
	#endif
} // namespace

#endif // _TKLB_MATH
