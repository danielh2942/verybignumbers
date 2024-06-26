/*
 * File:      fixed_bignum.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */
#ifndef FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E
#define FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E 1

#include "humanreadable.h"
#include "arbitrary_bignum.h"
#include <ostream>

#include <bit>
#include <compare>

#include <array>
#include <concepts>

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <iostream>

/*
 * Fixed-size big number type, it does everything you'd expect.
 * this type does not utilize disk storage and is used to do
 * some stuff.
 *
 * This 
 */
template<std::size_t U>
struct FixedBigNum {

	constexpr FixedBigNum() : m_data{0}, m_signed{false}, m_maxDigit{0}
	{}
	
	constexpr FixedBigNum(std::signed_integral auto x) : m_data{0}, m_signed{std::signbit(x)}, m_maxDigit{0}
	{
		if(m_signed) {
			x *= -1;
		}
		if(x > UINT32_MAX) {
			m_maxDigit = 1;
		}
		std::size_t idx = 0;
		while(x > 0) {
			m_data[idx] = x % ((std::uint64_t)UINT32_MAX + 1);
			x /= ((std::uint64_t)UINT32_MAX + 1);
			idx++;
		}
	}

	constexpr FixedBigNum(std::unsigned_integral auto x) : m_data{0}, m_signed{false}, m_maxDigit{0}
	{
		std::size_t idx = 0;
		if(x > UINT32_MAX) {
			m_maxDigit = 1;
		}
		while(x > 0) {
			m_data[idx] = x % ((std::uint64_t)UINT32_MAX + 1);
			x /= ((std::uint64_t)UINT32_MAX + 1);
			idx++;
		}
	}

	template<size_t T>
	constexpr FixedBigNum(FixedBigNum<T> const& x) : m_data{0}, m_signed{x.m_signed}, m_maxDigit{x.m_maxDigit}
	{
		if constexpr(T > U) {
			// Truncate the number
			m_data = {x.m_data.begin(), x.m_data.begin() + U};
		} else {
			// Copy the number entirely
			m_data = {x.m_data.begin(), x.m_data.end()};
		}
	}
// Friend operators
	friend FixedBigNum abs(FixedBigNum const&);

	friend bool signbit(FixedBigNum const&);

// Comparators

	constexpr std::partial_ordering operator<=>(FixedBigNum const& vs) const {
		if(auto signs = vs.m_signed <=> m_signed; signs != 0) {
			return signs;
		}

		if(!m_signed) {
			for(auto idx = U; idx > 0; idx--) {
				if(auto res = m_data[idx - 1] <=> vs.m_data[idx - 1]; res != 0) return res;
			} 
		} else {
			for(auto idx = U; idx > 0; idx--) {
				if(auto res = vs.m_data[idx - 1] <=> m_data[idx - 1]; res != 0) return res;
			}
		}
		return std::partial_ordering::equivalent;
	}

	constexpr bool operator==(FixedBigNum const& cmp) const {
		return std::is_eq(*this <=> cmp);
	}

// Arithmetic
	constexpr FixedBigNum& operator+=(FixedBigNum const& add) {
		if(m_signed != add.m_signed) {
			m_signed ^= true;
			operator-=(add);
			m_signed ^= true;
			return *this;
		}
		
		std::uint64_t carry = 0U;
		m_maxDigit = std::max(m_maxDigit, add.m_maxDigit) + 1;
		for(std::size_t idx = 0; idx < m_maxDigit; idx++) {
			carry += ((std::uint64_t)(m_data[idx] & 0xFFFFFFFF)) + (std::uint64_t)(add.m_data[idx] & 0xFFFFFFFF);
			m_data[idx] = carry & 0xFFFFFFFF;
			carry >>= 32;
		}

		if((m_maxDigit < U) && (carry != 0)) {
			m_data[m_maxDigit] = carry & 0xFFFFFFFF;
		}

		while(!m_data[m_maxDigit]) {
			m_maxDigit--;
		}
		return *this;		
	}

	constexpr FixedBigNum operator+(FixedBigNum const& add) const {
		FixedBigNum temp{*this};
		temp += add;
		return temp;
	}

	constexpr FixedBigNum& operator++() {
		*this += 1;
		return *this;
	}

	constexpr FixedBigNum& operator++(int) {
		*this += 1;
		return *this;
	}

	constexpr FixedBigNum& operator-=(FixedBigNum const& sub) {
		if(&sub == this) {
			m_data.fill(0);
			m_signed = false;
			return *this;
		}

		if(m_signed != sub.m_signed) {
			m_signed ^= true;
			operator+=(sub);
			m_signed ^= true;
			return *this;
		}

		if(abs(*this) < abs(sub)) {
			FixedBigNum temp = sub - *this;
			m_data.swap(temp.m_data);
			m_signed ^= true;
			return *this;
		}

		std::uint64_t buff = 0;
		std::uint64_t tmp = 0;
		bool all_zeroes = true;
		m_maxDigit = std::max(m_maxDigit, sub.m_maxDigit) + 1;
		for(auto idx = 0; idx < m_maxDigit; idx++) {
			tmp = m_data[idx] & 0xFFFFFFFF;
			buff = tmp - (sub.m_data[idx]&0xFFFFFFFF) - buff;
			m_data[idx] = buff & 0xFFFFFFFF;
			all_zeroes &= (buff == 0);
			buff = (buff > tmp);
		}

		if(all_zeroes) {
			m_signed = false;
		}

		while(!m_data[m_maxDigit]) {
			m_maxDigit--;
		}

		return *this;
	}

	constexpr FixedBigNum operator-(FixedBigNum const& sub) const {
		FixedBigNum temp{*this};
		temp-=sub;
		return temp;
	}

	constexpr FixedBigNum& operator--() {
		*this -= 1;
		return *this;
	}

	constexpr FixedBigNum& operator--(int) {
		*this -= 1;
		return *this;
	}

	constexpr FixedBigNum operator*(FixedBigNum const& mult) const {
		if(m_maxDigit == 0 && m_data[0] == 0) return {0};
		if(U <= 80000) {
			FixedBigNum temp{0};
			temp.m_maxDigit = std::min(U - 1, (m_maxDigit + 1) * (mult.m_maxDigit + 1)) + 1;
			for(std::size_t idx = 0; idx < temp.m_maxDigit; idx++) {
				std::uint64_t carry = 0;
				for(std::size_t idy = 0; (idx + idy) < U; idy++) {
					carry += ((std::uint64_t)(m_data[idx] & 0xFFFFFFFF)) * ((std::uint64_t)(mult.m_data[idy] & 0xFFFFFFFF));
					carry += (temp.m_data[idx+idy]) & 0xFFFFFFFF;
					temp.m_data[idx+idy] = carry & 0xFFFFFFFF;
					carry >>= 32;
				}
			}

			if(m_signed != mult.m_signed) {
				temp.m_signed = true;
			}

			while(!temp.m_data[temp.m_maxDigit]) {
				temp.m_maxDigit--;
			}

			return temp;
		} else {
			std::array<std::uint64_t, U> convulution{0};
			FixedBigNum result{0};
			result.m_maxDigit = std::min(U - 1, (m_maxDigit + 1) * (mult.m_maxDigit + 1)) + 1;

			for(std::size_t idx = 0; idx <= m_maxDigit; idx++) {
				if(m_data[idx] == 0) continue;
				for(std::size_t idy = 0; (idx + idy) < U; idy++) {
					convulution[idx + idy] += ((std::uint64_t)m_data[idx] & 0xFFFFFFFF) * ((std::uint64_t)mult.m_data[idy] & 0xFFFFFFFF);
				}
			}

			std::uint64_t carry = 0;
			for(std::size_t idx = 0; idx < U; idx++) {
				carry += convulution[idx];
				result.m_data[idx] = (carry & 0xFFFFFFFF);
				carry = carry >> 32;
			}
			
			while((result.m_data[result.m_maxDigit] == 0) && (result.m_maxDigit != 0)) {
				result.m_maxDigit--;
			}

			if(m_signed != mult.m_signed) {
				result.m_signed = true;
			}

			return result;
		}
	}

	constexpr FixedBigNum& operator*=(FixedBigNum const& mult) {
		
		auto temp = operator*(mult);
		m_data.swap(temp.m_data);
		m_maxDigit = temp.m_maxDigit;
		m_signed = temp.m_signed;

		return *this;
	}

	constexpr FixedBigNum& operator/=(FixedBigNum const& div) {
		auto tmp = simple_divide(div);
		m_data.swap(tmp.first.m_data);
		m_signed = tmp.first.m_signed;
		return *this;
	}

	constexpr FixedBigNum operator/(FixedBigNum const& div) const {
		auto tmp = simple_divide(div);
		return tmp.first;
	}

	constexpr FixedBigNum& operator%=(FixedBigNum const& div) {
		auto tmp = simple_divide(div);
		m_data.swap(tmp.second.m_data);
		m_signed = tmp.second.m_signed;
		return *this;
	}

	constexpr FixedBigNum operator%(FixedBigNum const& div) const {
		auto tmp = simple_divide(div);
		return tmp.second;
	}

// Bitshift operators
	constexpr FixedBigNum& operator<<=(std::size_t const& val) {
		auto word_offset = val >> 5;
		auto bit_offset = val & 0x1F;
		if(val == 0) return *this;
		std::uint64_t buff;

		/* Start with most significant shift right and work backwards
		 */
		if constexpr(U == 1) {
			if(word_offset > 0) m_data[0] = 0;
			m_data[0] <<= bit_offset;
		} else if constexpr(U == 2) {
			if(word_offset == 1) {
				m_data[1] = m_data[0] << bit_offset;
				m_data[0] = 0;
			} else if(word_offset >= 2) {
				m_data[0] = 0;
				m_data[1] = 0;
			} else {
				m_data[1] = (m_data[1] << bit_offset) ^ (m_data[0] >> (32 - bit_offset));
				m_data[0] = m_data[0] << bit_offset;
			}
		} else {
			for(int idx = U - 2; idx > word_offset; idx--) {
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

	constexpr FixedBigNum operator<<(std::size_t const& val) const {
		FixedBigNum temp{*this};
		temp<<=val;
		return temp;
	}

	constexpr FixedBigNum& operator>>=(std::size_t const& val) {
		auto word_offset = val >> 5;
		auto bit_offset = val & 0x1F;
		if(val == 0) return *this;
		if(word_offset >= U) {
			for(auto& v: m_data) {
				v = 0;
			}
			return *this;
		}

		if constexpr(U == 1) {
			m_data[0] = m_data[0] >> bit_offset;
		} else if constexpr(U == 2){
			std::uint64_t temp = (((std::uint64_t)m_data[1]) << 32) ^ m_data[0];
			temp >>= val;
			m_data[0] = temp & 0xFFFFFFFF;
			m_data[1] = (temp >> 32) & 0xFFFFFFFF;
		} else {
			std::uint64_t buff = m_data[word_offset+1];
			for(auto idx = 0; (idx + word_offset) < U; idx++) {
				buff <<= (32-bit_offset);
				buff ^= m_data[idx + word_offset] >> bit_offset;
				m_data[idx] = buff & 0xFFFFFFFF;
			}

			for(auto idx = U - word_offset; idx < U; idx++) {
				m_data[idx] = 0;
			}
		}

		return *this;
	}

	constexpr FixedBigNum operator>>(std::size_t const& amount) const {
		FixedBigNum temp{*this};
		temp>>=amount;
		return temp;
	}

// Bitwise Operators
	constexpr FixedBigNum& operator&=(FixedBigNum const& other) {
		for(auto idx = 0; idx < U; idx++) {
			m_data[idx] &= other.m_data[idx];
		}
	}

	constexpr FixedBigNum operator&(FixedBigNum const& other) const {
		FixedBigNum temp{*this};
		temp&=other;
		return temp;
	}

	constexpr FixedBigNum& operator^=(FixedBigNum const& other) {
		for(auto idx = 0; idx < U; idx++) {
			m_data[idx] ^= other.m_data[idx];
		}
		return *this;
	}

	constexpr FixedBigNum operator^(FixedBigNum const& other) const {
		FixedBigNum temp{*this};
		temp^=other;
		return temp;
	}

	constexpr FixedBigNum& operator|=(FixedBigNum const& other) {
		for(auto idx = 0; idx < U; idx++) {
			m_data[idx] |= other.m_data[idx];
		}
		return *this;
	}

	constexpr FixedBigNum operator|(FixedBigNum const& other) const {
		FixedBigNum temp{*this};
		temp|=other;
		return temp;
	}

	constexpr FixedBigNum operator~() const {
		FixedBigNum temp{*this};
		for(auto & v : temp.m_data) {
			v = ~v;
		}
		return temp;
	}

	friend std::ostream & operator<<(std::ostream & os, FixedBigNum const& bigNum) {
		//const HumanReadableNum HrU32MaxAdd1{0x100000000};
		std::uint64_t HrU32MaxAdd1 = 0x100000000;
		if constexpr(U == 1) {
			if(bigNum.m_signed) {
				os << '-';
			}
			os << bigNum.m_data[0];
		} else if constexpr(U == 2) {
			if(bigNum.m_signed) {
				os << '-';
			}
			std::uint64_t val = (bigNum.m_data[0] & 0xFFFFFFFF) ^ ((std::uint64_t)(bigNum.m_data[1] & 0xFFFFFFFF) << 32);
			os << val;
		} else {
			ArbitraryBigNum<ARBITRARY_PRINTABLE> tmp{0};
			//HumanReadableNum tmp{0};
			for(auto idx = bigNum.m_maxDigit + 1; idx > 0; idx--) {
				tmp *= HrU32MaxAdd1;
				tmp += bigNum.m_data[idx - 1];
			}
			if(bigNum.m_signed) {
				os << '-';
			}
			os << tmp;
		}
		return os;
	}

	friend FixedBigNum abs(FixedBigNum const& num) {
		FixedBigNum tmp{num};
		tmp.m_signed = false;
		return tmp;
	}

	friend bool signbit(FixedBigNum const& num) {
		return num.m_signed;
	}

private:
	constexpr std::size_t get_most_populated() const {
		for(std::size_t idx = U - 1; idx > 0; idx--) {
			if(m_data[idx]) return idx;
		}
		return 0;
	}

	// TODO:Test This!!!!!!!!!
	constexpr std::pair<FixedBigNum,FixedBigNum> simple_divide(FixedBigNum const& div) const {
		if((*this == 0) || (div == 0)) {
			return {0,0};
		}
		std::pair<FixedBigNum,FixedBigNum> result {0,*this};

		std::size_t len_divisor = div.get_most_populated();
		std::size_t len_dividend = get_most_populated();

		if(len_divisor > len_dividend) {
			return result;
		}

		int dividend_pop_bits = (32 - std::countl_zero(m_data[len_dividend])) + (len_dividend * 32);
		int divisor_pop_bits = (32 - std::countl_zero(div.m_data[len_divisor])) + (len_divisor * 32);

		int operations = (dividend_pop_bits - divisor_pop_bits);

		if(operations < 0) return result;

		FixedBigNum divisor = div << operations;
		while((abs(divisor) >= abs(div)) && (operations >= 0)) {
			result.first <<= 1;
			if(result.second >= divisor) {
				result.second -= divisor;
				result.first++;
			}
			divisor >>= 1;
			operations--;
		} 

		return result;
	}

private:
	std::array<std::uint32_t, U> m_data;     // The number data itself
	bool						 m_signed;	 // The sign for the number
	std::size_t					 m_maxDigit; // The Maximum Occupied digit
};

using int1024 = FixedBigNum<32>;

#endif // FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E
	
