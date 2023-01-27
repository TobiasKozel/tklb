/**
 * @file TTraits.hpp
 * @author Tobias Kozel
 * @brief Reimplementation of basic type traits, mostly for educational purposes
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_TRAITS
#define _TKLB_TRAITS

namespace tklb { namespace traits {
	/**
	 * @brief Value container type used to store result oll operation below.
	 */
	template<bool v>
	struct Value { static constexpr bool value = v; };

	/**
	 * @brief True if both types are exaclty the same, does not remove references etc/
	 */
	template<typename A, typename B>
	struct IsSame : Value<false> { };
	template<typename T>
	struct IsSame<T, T> : Value<true> { };

	/**
	 * @brief Super simple version of std::is_arithmetic which only works on basic types
	 *        which is sufficient for the use case.
	 */
	template<typename T>
	struct IsArithmetic : Value<false> { };
	template<>
	struct IsArithmetic<double> : Value<true> { };
	template<>
	struct IsArithmetic<float> : Value<true> { };
	template<>
	struct IsArithmetic<long long> : Value<true> { };
	template<>
	struct IsArithmetic<unsigned long long> : Value<true> { };
	template<>
	struct IsArithmetic<long> : Value<true> { };
	template<>
	struct IsArithmetic<unsigned long> : Value<true> { };
	template<>
	struct IsArithmetic<int> : Value<true> { };
	template<>
	struct IsArithmetic<unsigned int> : Value<true> { };
	template<>
	struct IsArithmetic<short> : Value<true> { };
	template<>
	struct IsArithmetic<unsigned short> : Value<true> { };
	template<>
	struct IsArithmetic<char> : Value<true> { };
	template<>
	struct IsArithmetic<unsigned char> : Value<true> { };
	template<>
	struct IsArithmetic<const double> : Value<true> { };
	template<>
	struct IsArithmetic<const float> : Value<true> { };
	template<>
	struct IsArithmetic<const long long> : Value<true> { };
	template<>
	struct IsArithmetic<const unsigned long long> : Value<true> { };
	template<>
	struct IsArithmetic<const long> : Value<true> { };
	template<>
	struct IsArithmetic<const unsigned long> : Value<true> { };
	template<>
	struct IsArithmetic<const int> : Value<true> { };
	template<>
	struct IsArithmetic<const unsigned int> : Value<true> { };
	template<>
	struct IsArithmetic<const short> : Value<true> { };
	template<>
	struct IsArithmetic<const unsigned short> : Value<true> { };
	template<>
	struct IsArithmetic<const char> : Value<true> { };
	template<>
	struct IsArithmetic<const unsigned char> : Value<true> { };

	/**
	 * @brief True if T is an unsigned non floating point interger type
	 */
	template<typename T>
	struct IsUnsigned : Value<false> { };
	template<>
	struct IsUnsigned<unsigned long long> : Value<true> { };
	template<>
	struct IsUnsigned<unsigned long> : Value<true> { };
	template<>
	struct IsUnsigned<unsigned int> : Value<true> { };
	template<>
	struct IsUnsigned<unsigned short> : Value<true> { };
	template<>
	struct IsUnsigned<unsigned char> : Value<true> { };

	/**
	 * @brief True if T is double or float
	 * @tparam T
	 */
	template<typename T>
	struct IsFloat : Value<false> { };
	template<>
	struct IsFloat<double> : Value<true> { };
	template<>
	struct IsFloat<const double> : Value<true> { };
	template<>
	struct IsFloat<float> : Value<true> { };
	template<>
	struct IsFloat<const float> : Value<true> { };

	/**
	 * @brief Removes reference from type
	 *
	 * @tparam T
	 */
	template<typename T>
	struct removeReference { typedef T type; };

	template<typename T>
	struct removeReference<T&> { typedef T type; };

	template<typename T>
	struct removeReference<T&&> { typedef T type; };

	/**
	 * @brief Reimplementation od the std::move
	 */
	template <class T>
	constexpr typename removeReference<T>::type&& move(T&& v) {
		return static_cast<typename removeReference<T>::type&&>(v);
	}
} } // tklb::tratis

#endif // _TKLB_TRAITS
