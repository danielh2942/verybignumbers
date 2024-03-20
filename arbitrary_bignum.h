/*
 * File:      arbitrary_bignum.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <bit>
#include <iterator>
#ifndef ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9
#define ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9 1
#include "cold_vector.h"
#include "util.h"

#include <utility>
#include <compare>
#include <iomanip>
#include <iostream>
#include <concepts>
#include <algorithm>
#include <type_traits>

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
			carry += ((std::uint64_t)(m_data[idx] & 0xFFFFFFFF) + (std::uint64_t)(add.m_data[idx] & 0xFFFFFFFF));
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
			ArbitraryBigNum temp = sub - *this;
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

		shrink_number();

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

	ArbitraryBigNum operator*(ArbitraryBigNum const& val) const {
		if ((val == 0) || (*this == 0)) {
			return 0;
		}

		std::uint64_t buff = 0;
		ArbitraryBigNum result{0};
		for(std::size_t idx = 0; idx < val.m_data.size(); idx++) {
			for(std::size_t idy = 0; idy < m_data.size(); idy++) {
				if((idx + idy) >= result.m_data.size()) result.m_data.emplace_back(0);
				buff += ((std::uint64_t)m_data[idy] & 0xFFFFFFFF) * ((std::uint64_t)val.m_data[idx]);
				buff += ((std::uint64_t)result.m_data[idx + idy] & 0xFFFFFFFF);
				result.m_data[idx + idy] = (buff % sc_modVal) & 0xFFFFFFFF;
				buff /= sc_modVal;
			}
			while(buff != 0) {
				result.m_data.emplace_back(buff % sc_modVal);
				buff /= sc_modVal;
			}
		}

		if(m_signed != val.m_signed) {
			result.m_signed = true;
		}

		result.shrink_number();	

		return result;
	}

	ArbitraryBigNum& operator*=(ArbitraryBigNum const& val) {
		ArbitraryBigNum temp = this->operator*(val);
		m_data.swap(temp.m_data);
		m_signed = temp.m_signed;
		return *this;
	}

	// Cut out allocations for small numbers and whatever
	ArbitraryBigNum& operator*=(width32int auto val) {
		if constexpr(!std::is_unsigned_v<decltype(val)>) {
			if(std::signbit(val)) {
				m_signed ^= true;
				val *= -1;
			}
		}
		std::uint64_t buff = 0;
		for(auto idx = 0; idx < m_data.size(); idx++) {
			buff += ((std::uint64_t)m_data[idx]&0xFFFFFFFF) * ((std::uint64_t)val & 0xFFFFFFFF);
			m_data[idx] = buff % sc_modVal;
			buff /= sc_modVal;
		}
		while(buff != 0) {
			m_data.emplace_back(buff % sc_modVal);
			buff /= sc_modVal;
		}
		return *this;
	}

	ArbitraryBigNum operator*(width32int auto val) const {
		ArbitraryBigNum temp{*this};
		temp *= val;
		return temp;
	}

	ArbitraryBigNum operator/(ArbitraryBigNum const& val) const {
		return simple_divide(val).first;
	}

	ArbitraryBigNum& operator/=(ArbitraryBigNum const& val) {
		auto temp = *this / val;
		m_data.swap(temp.m_data);
		m_signed = temp.m_signed;
		return *this;
	}

	ArbitraryBigNum& operator/=(width32int auto val) {
		if constexpr(std::is_signed_v<decltype(val)>) {
			if(std::signbit(val)) {
				m_signed ^= true;
			}
		}
		std::uint64_t carry = 0;
		for(int idx = m_data.size() - 1; idx > 0; idx--) {
			carry += m_data[idx] & 0xFFFFFFFF;
			m_data[idx] = carry / val & 0xFFFFFFFF;
			carry %= val;
			carry *= sc_modVal;
		}
		carry += (m_data[0] & 0xFFFFFFFF);
		m_data[0] = carry / val;
		return *this;
	}

	ArbitraryBigNum operator/(width32int auto val) const {
		ArbitraryBigNum temp{*this};
		temp /= val;
		return temp;
	}

	ArbitraryBigNum operator%(ArbitraryBigNum const& val) const {
		return simple_divide(val).second;
	}

	ArbitraryBigNum& operator%=(ArbitraryBigNum const& v) {
		auto temp = *this % v;
		m_data.swap(temp.m_data);
		m_signed = temp.m_signed;
		return *this;
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
			for(std::size_t idx = 0; idx <= word_offset; idx++) m_data.emplace_back(0);
			if((m_data.size() == 2) && (word_offset == 0)) {
				buff = (std::uint64_t)(m_data[0]&0xFFFFFFFF);
				m_data[1] = (buff >> (32 - bit_offset)) &0xFFFFFFFF;
				m_data[0] = (buff << bit_offset) & 0xFFFFFFFF;
			} else {
				for(int idx = m_data.size() - 2; idx > word_offset; idx--) {
					buff = (m_data[idx - word_offset] & 0xFFFFFFFF) << bit_offset;
					buff ^= (m_data[(idx - 1) - word_offset] & 0xFFFFFFFF) >> (32 - bit_offset);
					m_data[idx] = buff &0xFFFFFFFF;
				}
			}
			for(std::size_t idx = 0; idx < word_offset; idx++) {
				m_data[idx] = 0;
			}
			while((m_data.size() > 1) && (m_data[m_data.size() - 1] == 0)) {
				m_data.pop_back();
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

	// Right shift
	// Does nothing for non-standard numbers :DDDDDD
	ArbitraryBigNum& operator>>=(std::size_t offset) {
		if constexpr(MAX_VAL == UINT32_MAX) {
			auto word_offset = offset >> 5;
			auto bit_offset = offset & 0x1F;
			if(offset == 0) return *this;
			std::uint64_t buff;
			if(m_data.size() == 1) {
				if(word_offset > 1) {
					m_data[0] = 0;
				} else {
					m_data[0] >>= bit_offset;
				}
				return  *this;
			}
			if(m_data.size() == 1) {
				buff = ((std::uint64_t)m_data[0]) & 0xFFFFFFFF;
			} else {
				buff = ((std::uint64_t)m_data[m_data.size() - 1] << 32) ^ (m_data[m_data.size() - 2] & 0xFFFFFFFF);
			}
			for(auto idx = m_data.size() - word_offset - 1; idx > 0; idx--) {
				m_data[idx] = buff >> bit_offset;
				buff <<= 32;
				buff ^= m_data[idx + word_offset - 1];
			}
			m_data[0] = buff >> bit_offset;

			for(auto idx = 0; idx < word_offset; idx++) m_data.pop_back();
		}
		return *this;
	}

	ArbitraryBigNum operator>>(std::size_t offset) const {
		ArbitraryBigNum temp{*this};
		temp >>= offset;
		return temp;
	}

// Comparison Operators
	/**
	 * This compares absolute values in a way which does not perform any copy construction
	 * operations in order to save compute time, memory, etc.
	 */
	friend std::strong_ordering absolute_compare(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs) {
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

	friend bool operator==(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs) {
		return std::is_eq(lhs <=> rhs);
	}

	friend std::strong_ordering operator<=>(ArbitraryBigNum const& lhs, ArbitraryBigNum const& rhs) {
		if(&lhs == &rhs) return std::strong_ordering::equal;
		if(auto sign = rhs.m_signed <=> lhs.m_signed; sign != 0) return sign;
		if(!lhs.m_signed) {
			return absolute_compare(lhs, rhs);
		} else {
			return absolute_compare(rhs, lhs);
		}
	}

	friend bool operator==(ArbitraryBigNum const& lhs, width32int auto const rhs) {
		return std::is_eq(lhs <=> rhs);
	}

	friend std::strong_ordering operator<=>(ArbitraryBigNum const& lhs, width32int auto rhs) {
		if constexpr(std::is_signed_v<decltype(rhs)>) {
			if(std::signbit(rhs)) {
				if(!lhs.m_signed) return std::strong_ordering::greater;
				rhs *= -1;
			} else {
				if(lhs.m_signed) return std::strong_ordering::less;
			}
		} else {
			if(lhs.m_signed) return std::strong_ordering::less;
		}

		if(lhs.m_data.size() > 1) return std::strong_ordering::greater;

		return lhs.m_data[0] <=> static_cast<std::uint32_t>(rhs);
	}

	friend std::ostream& operator<<(std::ostream& os, ArbitraryBigNum const& abg) {
		if constexpr(MAX_VAL == ARBITRARY_PRINTABLE) {
			if(abg.m_signed) {
				os << '-';
			}
			os << abg.m_data[abg.m_data.size() - 1];
			for(int itr = abg.m_data.size() - 2; itr >= 0; itr--) {
				os << std::setw(9) << std::setfill('0') << abg.m_data[itr];
			}
		} else {
			ArbitraryBigNum<ARBITRARY_PRINTABLE> result{0};
			for(int idx = abg.m_data.size() - 1; idx >= 0; idx--) {
				result *= sc_modVal;
				result += abg.m_data[idx];
			}
			if(abg.m_signed) {
				result *= -1;
			}
			os << result;
		}
		return os;
	}

	friend bool signbit(ArbitraryBigNum const& num) {
		return num.m_signed;
	}

	friend ArbitraryBigNum abs(ArbitraryBigNum const& num) {
		auto temp = ArbitraryBigNum{num};
		temp.m_signed = false;
		return temp;
	}

private:
	// Remove leading zeroes from the number :D
	void shrink_number() {
		while((m_data.size() != 1) 
			  && (m_data[m_data.size() - 1] == 0)) {
			m_data.pop_back();
		}
	}

	std::pair<ArbitraryBigNum, ArbitraryBigNum> simple_divide(ArbitraryBigNum const& val) const {
		auto res = std::pair<ArbitraryBigNum, ArbitraryBigNum>{0,*this};
		if((*this == 0) || (val == 0)) return res;
		auto check = absolute_compare(*this, val); 
		if(check == std::weak_ordering::less) {
			return res;
		}
		if(check == std::strong_ordering::equal) {
			return {1,0};
		}

		if constexpr(MAX_VAL == UINT32_MAX) {
			// I can do bitshifts and stuff :DDDD
			std::size_t len_divisor = val.m_data.size() - 1;
			std::size_t len_dividend = m_data.size() - 1;

			if(len_divisor > len_dividend) return res;

			int dividend_pop_bits = (32 - std::countl_zero(m_data[len_dividend])) + (len_dividend * 32);
			int divisor_pop_bits = (32 - std::countl_zero(val.m_data[len_divisor])) + (len_divisor * 32);

			int operations = (dividend_pop_bits - divisor_pop_bits);

			if(operations < 0) return res;

			ArbitraryBigNum divisor = val << operations;
			while((absolute_compare(divisor, val) != std::strong_ordering::less) && (operations >= 0)) {
				res.first<<=1;
				if(res.second >= divisor) {
					res.second -= divisor;
					res.first++;
				}
				divisor >>= 1;
				operations--;
			}
		}

		return res;
	}

private:
	static constexpr std::uint64_t	sc_modVal = MAX_VAL + 1; // Modulo and divide value to be used
	ColdVector<std::uint32_t>		m_data;					 // Digit Data in reversed Order.
	bool							m_signed;				 // If the number carries a sign
};

#endif // ARBITRARY_BIGNUM_H_00E681C94204436A9C4EC4EFAA0DE0F9
	
