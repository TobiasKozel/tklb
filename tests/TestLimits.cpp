#include "../src/util/TLimits.hpp"
#include <limits>
#include <iostream>

template <typename T>
bool compareMax() {
	const auto a = std::numeric_limits<T>::max();
	const auto b = tklb::limits::max<T>::value;
	if (a != b) {
		std::cout << "Max Mismatch: " << a << "\t" << b << "\n";
		return false;
	}
	// TODO
	// const auto c = std::numeric_limits<T>::min();
	// const auto d = tklb::limits::min<T>::value;
	// if (c != d) {
	// 	std::cout << "Min Mismatch: " << c << "\t" << d << "\n";
	// 	return false;
	// }
	return true;
}

int main() {
	if (!compareMax<long long>()) { return 1; }
	if (!compareMax<unsigned long long>()) { return 2; }

	if (!compareMax<long>()) { return 3; }
	if (!compareMax<unsigned long>()) { return 4; }

	if (!compareMax<int>()) { return 5; }
	if (!compareMax<unsigned int>()) { return 6; }

	if (!compareMax<short>()) { return 7; }
	if (!compareMax<unsigned short>()) { return 8; }

	if (!compareMax<char>()) { return 9; }
	if (!compareMax<unsigned char>()) { return 10; }

	if (!compareMax<float>()) { return 11; }
	if (!compareMax<double>()) { return 12; }

	return 0;
}
