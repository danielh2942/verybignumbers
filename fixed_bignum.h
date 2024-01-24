/*
 * File:      fixed_bignum.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <compare>
#ifndef FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E
#define FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E 1

#include <array>
#include <concepts>

#include <cmath>
#include <cstddef>
#include <cstdint>

/*
 * Fixed-size big number type, it does everything you'd expect.
 * this type does not utilize disk storage and is used to do
 * some stuff.
 *
 * This 
 */
template<std::size_t U>
struct FixedBigNum {

	constexpr FixedBigNum() : m_data{0}, m_signed{false}
	{}
	
	constexpr FixedBigNum(std::signed_integral auto x) : m_data{0}, m_signed{std::signbit(x)}
	{
		if(m_signed) {
			x *= -1;
		}
		std::size_t idx = 0;
		while(x > 0) {
			m_data[idx] = x % (INT32_MAX + 1);
			x /= (INT32_MAX + 1);
			idx++;
		}
	}

	constexpr FixedBigNum(std::unsigned_integral auto x) : m_data{0}, m_signed{false}
	{
		std::size_t idx = 0;
		while(x > 0) {
			m_data[idx] = x % (INT32_MAX + 1);
			x /= (INT32_MAX + 1);
			idx++;
		}
	}

	template<size_t T>
	constexpr FixedBigNum(FixedBigNum<T> const& x) : m_data{0}, m_signed{x.m_signed}
	{
		if constexpr(T > U) {
			// Truncate the number
			m_data = {x.m_data.begin(), x.m_data.begin() + U};
		} else {
			// Copy the number entirely
			m_data = {x.m_data.begin(), x.m_data.end()};
		}
	}

// Comparators

	constexpr std::partial_ordering operator<=>(FixedBigNum const& vs) const {
		if(auto signs = m_signed <=> vs.m_signed; signs != 0) {
			return signs;
		}

		if(!m_signed) {
			return vs.m_data <=> m_data;
		} else {
			return m_data <=> vs.m_data;
		}
	}

	constexpr bool operator==(FixedBigNum const& cmp) const {
		return std::is_eq(*this <=> cmp);
	}

// Arithmetic

	constexpr FixedBigNum& operator+=(FixedBigNum const& add) {
		return *this;		
	}

private:
	std::array<std::uint32_t, U> m_data;   // The number data itself
	bool						 m_signed; // The sign for the number
};

using int1024 = FixedBigNum<32>;

#endif // FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E
	
