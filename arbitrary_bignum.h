/*
 * File:      arbitrary_bignum.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#ifndef ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9
#define ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9 1
#include "cold_vector.h"

#include <utility>
#include <compare>
#include <iomanip>
#include <iostream>
#include <concepts>
#include <algorithm>

#include <cmath>
#include <cstddef>
#include <cstdint>

// The Maximum value for an Arbitrarily printable number.
inline constexpr std::size_t ARBITRARY_PRINTABLE = 999999999;

/**
 *	ArbitraryBigNum is a type that makes large numbers with no fixed width.
 *	This stores everything on disk however.
 */
template<std::size_t MAX_VAL = UINT32_MAX>
struct ArbitraryBigNum {
	static_assert(MAX_VAL != 0, "You cant have a base 0 number");
	static_assert(MAX_VAL <= UINT32_MAX, "The base is too large");
	ArbitraryBigNum(): m_data{0}, m_signed{false}
	{}

	ArbitraryBigNum(std::signed_integral auto x): m_data{}, m_signed{std::signbit(x)}
	{
		if(x == 0) m_data.emplace_back(0);
		if(m_signed) x *= -1;
		while(x != 0) {
			m_data.emplace_back(x % sc_modVal);
			x /= sc_modVal;
		}
	}

	ArbitraryBigNum(std::unsigned_integral auto x): m_data{}, m_signed{false}
	{
		if(x == 0) m_data.emplace_back(0);
		while(x != 0) {
			m_data.emplace_back(x % sc_modVal);
			x /= sc_modVal;
		}	
	}

	ArbitraryBigNum(ArbitraryBigNum const& a): m_data{a.m_data}, m_signed{a.m_signed}
	{}

	ArbitraryBigNum(ArbitraryBigNum const&& a): m_data{std::move(a.m_data)}, m_signed{a.m_signed}
	{}

// Assignment Operators
	ArbitraryBigNum& operator=(std::integral auto x) {
		ArbitraryBigNum a{x};
		m_data.swap(a.m_data);
		m_signed = a.m_signed;
		return *this;	
	}

	ArbitraryBigNum& operator=(ArbitraryBigNum const& x) {
		m_data = {x.m_data};
		m_signed = x.m_signed;
		return *this;
	}

// Arithmetic
	ArbitraryBigNum& operator+=(ArbitraryBigNum const& add) {
		if(m_signed != add.m_signed) {
			m_signed ^= true;
			operator-=(add);
			m_signed ^= true;
			return *this;
		}
		std::uint64_t carry = 0;
		std::size_t limit = std::min(m_data.size(), add.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			carry += ((m_data[idx] & 0xFFFFFFFF)  + (add.m_data[idx] % 0xFFFFFFFF));
			m_data[idx] = carry % sc_modVal;
			carry /= sc_modVal;
		}

		if(m_data.size() > limit) {
			for(std::size_t idx = limit; (idx < m_data.size()) && (carry != 0); idx++) {
				carry += m_data[idx] & 0xFFFFFFFF;
				m_data[idx] = carry % sc_modVal;
				carry /= sc_modVal;
			}
		} else if(add.m_data.size() > limit) {
			for(std::size_t idx = limit; (idx < add.m_data.size()); idx++) {
				carry += add.m_data[idx] & 0xFFFFFFFF;
				m_data.emplace_back(carry % sc_modVal);
				carry /= sc_modVal;
			}
		}
		if(carry != 0) {
			while(carry != 0) {
				m_data.emplace_back(carry % sc_modVal);
				carry /= sc_modVal;
			}
		}
		return *this;
	}

	ArbitraryBigNum& operator++() {
		*this += 1;
		return *this;
	}

	ArbitraryBigNum& operator++(int) {
		*this += 1;
		return * this;
	}

	ArbitraryBigNum operator+(ArbitraryBigNum const& add) const {
		ArbitraryBigNum tmp{*this};
		tmp += add;
		return tmp;
	}

	ArbitraryBigNum& operator-=(ArbitraryBigNum const& sub) {
		// Prevent -0
		if(&sub == this) {
			m_data = {0};
			m_signed = false;
			return *this;
		}

		if(m_signed != sub.m_signed) {
			// Flip Sign and Add
			m_signed ^= true;
			operator+=(sub);
			m_signed ^= true;
			return *this;
		}

		// If it reaches here the sign definitely flips
		if(absolute_compare(*this, sub) == std::strong_ordering::less) {
			ArbitraryBigNum temp{sub};
			sub -= *this;
			m_data.swap(temp.m_data);
			m_signed ^= true;
			return *this;
		}

		std::uint64_t temp = 0;
		std::uint64_t carry = 0;
		std::size_t limit = std::min(m_data.size(), sub.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			// As sc_modVal is unreachable in any base
			// if it dips below it then it has clearly underflowed and as
			// a result we then have a carry flag set :D
			temp = sc_modVal + (m_data[idx] & 0xFFFFFFFF) - carry;
			temp -= sub.m_data[idx] & 0xFFFFFFFF;
			carry = temp < sc_modVal;
			m_data[idx] = (temp % sc_modVal) & 0xFFFFFFFF;
		}

		if(limit == m_data.size() && (m_data.size() != sub.m_data.size())) {
			for(std::size_t idx = limit; idx < sub.m_data.size(); idx++) {
				temp = (sc_modVal + sub.m_data[idx]) - carry;
				carry = temp < sc_modVal;
				m_data.emplace_back(temp % sc_modVal);
			}
		} else {
			for(std::size_t idx = limit; (idx < m_data.size()) && (carry != 0); idx++) {
				temp = sc_modVal + (m_data[idx] & 0xFFFFFFFF) - carry;
				carry = temp < sc_modVal;
				m_data[idx] = temp % sc_modVal;
			}
		}

		for(std::size_t idx = m_data.size() - 1; idx > 0 && (m_data[idx] == 0); idx--) {
			m_data.pop_back();
		}

		if(m_data.size() == 1 && m_data[0] == 0) {
			m_signed = false;
		}

		return *this;
	}

	ArbitraryBigNum& operator--() {
		*this -= 1;
		return *this;
	}

	ArbitraryBigNum& operator--(int) {
		*this -= 1;
		return *this;
	}

	ArbitraryBigNum operator-(ArbitraryBigNum const& val) const {
		ArbitraryBigNum temp{*this};
		temp -= val;
		return temp;
	}

// Masking and bitshifts and whatever
	// Left Shift Assignment operator
	// IMPORTANT: It only works if you use UINT32_MAX as MAX_VAL
	// Otherwise it does nothing.
	ArbitraryBigNum& operator<<=(std::size_t offset) {
		if constexpr(MAX_VAL == UINT32_MAX) {
			auto word_offset = offset >> 5;
			auto bit_offset = offset & 0x1F;
			if(offset == 0) return *this;
			std::uint64_t buff;
			for(std::size_t idx = m_data.size() - 2; idx > word_offset; idx--) {
				buff = (m_data[idx - word_offset] & 0xFFFFFFFF) << bit_offset;
				buff ^= (m_data[(idx - 1) - word_offset] & 0xFFFFFFFF) >> (32 - bit_offset);
				m_data[idx] = buff & 0xFFFFFFFF;
			}

			for(std::size_t idx = 0; idx < word_offset; idx++) {
				m_data[idx] = 0;
			}
		}
		return *this;
	}

	// Left Shift
	// IMPORTANT: Does nothing if MAX_VAL is not UINT32_MAX
	ArbitraryBigNum operator<<(std::size_t offset) const {
		ArbitraryBigNum temp{*this};
		temp <<= offset;
		return temp;
	}

// Comparison Operators
	friend std::strong_ordering absolute_compare(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs);

	friend bool operator==(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs);
	friend std::strong_ordering operator<=>(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs);

	template<std::size_t MAX>
	friend std::ostream& operator<<(std::ostream& os, ArbitraryBigNum<MAX> const& abg) {
		if constexpr(MAX == ARBITRARY_PRINTABLE) {
			if(abg.m_signed) {
				os << '-';
			}
			for(auto itr = abg.m_data.rbegin(); itr > (abg.m_data.rend() - 1); itr++) {
				os << std::setfill('0') << std::setw(9) << *itr;
			}
			os << abg.m_data[0];
		}
		return os;
	}

private:
	static constexpr std::uint64_t	sc_modVal = MAX_VAL + 1; // Modulo and divide value to be used
	ColdVector<std::uint32_t>		m_data;					 // Digit Data in reversed Order.
	bool							m_signed;				 // If the number carries a sign
};

/**
 * This compares absolute values in a way which does not perform any copy construction
 * operations in order to save compute time, memory, etc.
 */
template<std::size_t V>
inline std::strong_ordering absolute_compare(ArbitraryBigNum<V> const& lhs, ArbitraryBigNum<V> const& rhs) {
	if(auto width = lhs.m_data.size() <=> rhs.m_data.size(); width != 0) {
		return width;
	}
	for(std::size_t idx = lhs.m_data.size(); idx > 0; idx--) {
		if(auto val = lhs.m_data[idx - 1] <=> rhs.m_data[idx - 1]; val != 0) {
			return val;
		}
	}
	return std::strong_ordering::equal;
}

template<std::size_t V>
inline bool operator==(ArbitraryBigNum<V> const& lhs, ArbitraryBigNum<V> const& rhs) {
	return std::is_eq(lhs <=> rhs);
}

template<std::size_t V>
inline std::strong_ordering operator<=>(ArbitraryBigNum<V> const& lhs, ArbitraryBigNum<V> const& rhs) {
	if(&lhs == &rhs) return std::strong_ordering::equal;
	if(auto sign = rhs.m_signed <=> lhs.m_signed; sign != 0) return sign;
	if(!lhs.m_signed) {
		return absolute_compare(lhs, rhs);
	} else {
		return absolute_compare(rhs, lhs);
	}
}

#endif // ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9
	
