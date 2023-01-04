#ifndef _TKLB_MATH
#define _TKLB_MATH

#ifndef TKLB_NO_STDLIB
	#include <cmath>
#endif


namespace tklb {
	constexpr double PI = 3.14159265358979323846;

	template <typename T>
	inline constexpr  T min(const T& v1, const T& v2) {
		return v1 < v2 ? v1 : v2;
	}

	template <typename T>
	inline constexpr T max(const T& v1, const T& v2) {
		return v1 < v2 ? v2 : v1;
	}

	template <typename T>
	inline constexpr T clamp(const T& v, const T& _min, const T& _max) {
		return min(_max, max(v, _min));
	}

	template <typename T>
	inline constexpr T abs(const T& v) {
		return T(0) <= v ? v : -v;
	}

	template <typename T, typename T2>
	inline constexpr T lerp(const T& v1, const T& v2, const T2& t) {
		return v1 + t * (v2 - v1);
	}

	template <typename T>
	inline constexpr T round(T v) {
		return v;
	}
	template <>
	inline constexpr float round(float v) {
		return (long) (v + float(0.5));
	}
	template <>
	inline constexpr double round(double v) {
		return (long) (v + double(0.5));
	}

	template <typename T>
	inline T pow (T x, T y)
	{
		#ifdef TKLB_NO_STDLIB
			TKLB_ASSERT(false) // TODO
			return 0;
		#else
			std::pow(x, y);
		#endif
	}


	inline float sqrt(const float& x) {
		#ifdef TKLB_NO_STDLIB
			// https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi sqrt3
			// ! absolutely untested
			union {
				int i;
				float x;
			} u;

			u.x = x;
			u.i = (1<<29) + (u.i >> 1) - (1<<22);
			return u.x;
		#else
			return std::sqrt(x);
		#endif
	}
} // namespace

#endif // _TKLB_MATH
