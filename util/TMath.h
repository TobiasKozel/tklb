#ifndef TKLB_MATH
#define TKLB_MATH


namespace tklb {
template<typename T>
inline T powerOf2(const T& v) {
	T p = 1;
	while (p < v) { p *= 2; }
	return p;
}
} // namespace

#endif // TKLB_MATH
