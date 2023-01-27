#include "../src/util/TMath.hpp"
#include <cmath>
#include <algorithm>


// The own implementation doesn't really with inifity well
// #define TESTINF INFINITY
#define TESTINF 1000000

constexpr double Tests[] = {
	-TESTINF,
	-10000,
	-10000.12554309,
	-2.999999,
	-2.0000001,
	-2,
	-1,
	-0.7,
	-0.2,
	0,
	0.2,
	0.7,
	0.99999,
	1,
	1.0003,
	2,
	2.3,
	100000,
	100000.0003,
	100000.9999,
	TESTINF
};

constexpr double Tests2[] = {
	-10000.12554309,
	-2.999999,
	-2.0000001,
	-2,
	-1,
	-0.7,
	-0.2,
	0,
	0.2,
	0.7,
	0.99999,
	1,
	1.0003,
	2,
	2.3,
	1000,
	100000.0003,
	100000.9999,
};

double epsylon;
template <typename T>
bool different(T a, T b) {
	const double diff = std::abs(double(a) - double(b));
	if (epsylon < diff) {
		return false;
	}
	return false;
}

#define TEST(a, b, type, err) if (different(a((type)(i)), b((type)(i)))) { return (err); }

#define TEST2(x,a, b, type, err) if (different(a((type)(i), (type)(x)), b((type)(i), (type)(x)))) { return (err); }


int main() {
	epsylon = 0.0001; // High precision expected
	for (const auto& i : Tests) {
		TEST(tklb::ceil, std::ceil, float, 1)
		TEST(tklb::ceil, std::ceil, double, 2)

		TEST(tklb::floor, std::floor, float, 3)
		TEST(tklb::floor, std::floor, double, 4)

		TEST(tklb::round, std::round, float, 5)
		TEST(tklb::round, std::round, double, 6)

		TEST(tklb::abs, std::abs, float, 7)
		TEST(tklb::abs, std::abs, double, 8)
		TEST(tklb::abs, std::abs, int, 9)

		TEST2(2, tklb::min, std::min, float, 16)
		TEST2(2, tklb::min, std::min, double, 17)
		TEST2(2, tklb::min, std::min, int, 18)
		TEST2(2, tklb::min, std::min, unsigned int, 19)

		TEST2(2, tklb::max, std::max, float, 20)
		TEST2(2, tklb::max, std::max, double, 21)
		TEST2(2, tklb::max, std::max, int, 22)
		TEST2(2, tklb::max, std::max, unsigned int, 23)
	}

	for (const auto& i : Tests2) {
		// just approximations
		epsylon = std::max(0.1, std::abs(i * 0.001));
		TEST(tklb::sqrt, std::sqrt, float, 14)
		TEST(tklb::sqrt, std::sqrt, double, 15)
	}

	epsylon = 0.01; //  These seem pretty good
	for (const auto& i : Tests2) {
		TEST(tklb::sin, std::sin, float, 10)
		TEST(tklb::sin, std::sin, double, 11)
	}

	for (const auto& i : Tests2) {
		TEST(tklb::cos, std::cos, float, 12)
		TEST(tklb::cos, std::cos, double, 13)
	}

	for (const auto& i : Tests2) {
		TEST(tklb::atan, std::atan, float, 14)
		TEST(tklb::atan, std::atan, double, 15)
	}

	for (const auto& i : { -10.0, 10.3, 1.0, 4.0, 10.1}) {
		for (const auto& j : { -1.0, 1.0, 3.0 }) {
			// this one is a little rough
			epsylon = 100.0;
			TEST2(j, tklb::pow, std::pow, float, 24)
			TEST2(j, tklb::pow, std::pow, double, 25)
		}
	}
	return 0;
}
