#include "../src/util/TTraits.hpp"
#include <type_traits>

class TestClass { };
struct TestStruct { };
class TestNoMatchClass { };

template <typename T>
bool compareArithmetic() {
	return tklb::traits::IsArithmetic<T>::value == std::is_arithmetic<T>::value;
}

template <typename T>
bool compareIsFloat() {
	return tklb::traits::IsFloat<T>::value == std::is_floating_point<T>::value;
}

template <typename A, typename B>
bool compareIsSame() {
	if (tklb::traits::IsSame<A, B>::value != std::is_same<A, B>::value) {
		return false;
	}
	if (tklb::traits::IsSame<B, A>::value != std::is_same<B, A>::value) {
		return false;
	}
	if (tklb::traits::IsSame<A, TestNoMatchClass>::value != std::is_same<A, TestNoMatchClass>::value) {
		return false;
	}
	if (tklb::traits::IsSame<B, TestNoMatchClass>::value != std::is_same<B, TestNoMatchClass>::value) {
		return false;
	}
	return true;
}

int main() {
	{
		if (!compareArithmetic<float>()) {
			return 1;
		}
		if (!compareArithmetic<double>()) {
			return 2;
		}
		if (!compareArithmetic<int>()) {
			return 3;
		}
		if (!compareArithmetic<unsigned int>()) {
			return 4;
		}
		if (!compareArithmetic<char>()) {
			return 5;
		}
		if (!compareArithmetic<unsigned char>()) {
			return 6;
		}
		if (!compareArithmetic<short>()) {
			return 7;
		}
		if (!compareArithmetic<unsigned short>()) {
			return 8;
		}
		if (!compareArithmetic<long>()) {
			return 9;
		}
		if (!compareArithmetic<unsigned long>()) {
			return 10;
		}
		if (!compareArithmetic<long long>()) {
			return 11;
		}
		if (!compareArithmetic<unsigned long long>()) {
			return 12;
		}

		if (!compareArithmetic<void*>()) {
			return 13;
		}
		if (!compareArithmetic<float*>()) {
			return 14;
		}
		if (!compareArithmetic<TestClass>()) {
			return 15;
		}
		if (!compareArithmetic<TestStruct>()) {
			return 16;
		}

		if (!compareArithmetic<int&>()) {
			return 17; // References don't count
		}
		if (!compareArithmetic<int&&>()) {
			return 18;
		}
		if (!compareArithmetic<const int>()) {
			return 19; // Const still qualifies as number
		}
	}
	{
		if (!compareIsFloat<float>()) {
			return 20;
		}
		if (!compareIsFloat<double>()) {
			return 21;
		}
		if (!compareIsFloat<int>()) {
			return 22;
		}
		if (!compareIsFloat<unsigned int>()) {
			return 24;
		}
		if (!compareIsFloat<TestClass>()) {
			return 25;
		}
		if (!compareIsFloat<const int>()) {
			return 26;
		}
		if (!compareIsFloat<const float>()) {
			return 27;
		}
	}

	{
		if (!compareIsSame<int, int>()) {
			return 28;
		}
		if (!compareIsSame<const int, int>()) {
			return 28;
		}
		if (!compareIsSame<const int, const int>()) {
			return 28;
		}
		if (!compareIsSame<TestClass, TestClass>()) {
			return 28;
		}
		if (!compareIsSame<const TestClass, const TestClass>()) {
			return 28;
		}
		if (!compareIsSame<TestClass&, TestClass&>()) {
			return 28;
		}
		if (!compareIsSame<const TestClass&&, const TestClass&&>()) {
			return 28;
		}
	}
	return 0;
}
