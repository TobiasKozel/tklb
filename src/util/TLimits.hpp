/**
 * @file TLimits.hpp
 * @author Tobias Kozel
 * @brief Simple version of limits
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_LIMITS
#define _TKLB_LIMITS

namespace tklb { namespace limits {
	template<typename T>
	struct max { };

	/**
	 * TODO this might not be protable/safe on every arch.
	 * Since the tests runs against the std this should be obious tho.
	 */
	template<>
	struct max<double> { static constexpr double value = 1.7976931348623158e+308; };
	template<>
	struct max<float> { static constexpr float value = 3.402823466e+38F; };

	template<>
	struct max<long long> { static constexpr long value = ((unsigned long long) ~0) >> 1; };
	template<>
	struct max<unsigned long long> { static constexpr unsigned long value = ~0; };

	template<>
	struct max<long> { static constexpr long value = ((unsigned long) ~0) >> 1; };
	template<>
	struct max<unsigned long> { static constexpr unsigned long value = ~0; };

	template<>
	struct max<int> { static constexpr int value = ((unsigned int) ~0) >> 1; };
	template<>
	struct max<unsigned int> { static constexpr unsigned int value = ~0; };

	template<>
	//(~((unsigned short) 0)) / 2
	struct max<short> { static constexpr short value = ((unsigned short) ~0) >> 1; };
	template<>
	struct max<unsigned short> { static constexpr unsigned short value = ~0; };

	template<>
	struct max<char> { static constexpr char value = ((unsigned char) ~0) >> 1; };
	template<>
	struct max<unsigned char> { static constexpr unsigned char value = ~0; };

	// TODO compiler throws a fit at intentional overflowing
	template<typename T>
	struct min { static constexpr T value = max<T>::value + 1; };
	template<>
	struct min<double> { static constexpr double value = 2.2250738585072014e-308; };
	template<>
	struct min<float> { static constexpr float value = 1.175494351e-38F; };

} }

#endif // _TKLB_LIMITS
