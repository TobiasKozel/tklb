#ifndef _TKLB_MATH
#define _TKLB_MATH

#ifndef KTLB_NO_STDLIB
	#include <cmath>
#endif


namespace tklb {

	constexpr double PI = 3.14159265358979323846;

	template<typename T>
	inline T powerOf2(const T&& v) {
		T p = 1;
		while (p < v) { p *= 2; }
		return p;
	}

	template <typename T>
	inline T min(const T& v1, const T& v2) {
		return v1 < v2 ? v1 : v2;
	}

	template <typename T>
	inline T max(const T& v1, const T& v2) {
		return v1 < v2 ? v2 : v1;
	}

	template <typename T>
	inline T clamp(const T& v, const T& _min, const T& _max) {
		return min(_max, max(v, _min));
	}

	template <typename T>
	inline T abs(const T& v) {
		return T(0) <= v ? v : -v;
	}

	template <typename T, typename T2>
	inline T lerp(const T& v1, const T& v2, const T2& t) {
		return v1 + t * (v2 - v1);
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
