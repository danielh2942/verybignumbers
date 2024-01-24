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
// Friend operators
	friend FixedBigNum abs(FixedBigNum const&);

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
		if(m_signed != add.m_signed) {
			m_signed ^= true;
			operator-=(add);
			m_signed ^= true;
			return *this;
		}
		
		std::uint64_t carry = 0U;
		for(std::size_t idx = 0; idx < U; idx++) {
			carry += (m_data[idx] & 0xFFFFFFFF)	+ (add.m_data[idx] & 0xFFFFFFFF);
			m_data[idx] = carry & 0xFFFFFFFF;
			carry >>= 32;
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
		bool all_zeroes = true;
		for(auto idx = 0; idx < U; idx++) {
			buff = (m_data[idx]&0xFFFFFFFF) - (buff & 0x1);
			buff -= (sub.m_data[idx] & 0xFFFFFFFF);
			m_data[idx] = buff & 0xFFFFFFFF;
			all_zeroes &= (m_data[idx] == 0);
			buff >>= 32;
		}

		if(all_zeroes) {
			m_signed = false;
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

	constexpr FixedBigNum operator*(FixedBigNum const& mult) const {
		// TODO:Replace with karatsuba
		FixedBigNum temp{0};
		for(std::size_t idx = 0; idx < U; idx++) {
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

		return temp;
	}

	constexpr FixedBigNum& operator*=(FixedBigNum const& mult) {
		
		auto temp = operator*(mult);
		m_data.swap(temp.m_data);
		m_signed = temp.m_signed;

		return *this;
	}

// Bitshift operators
	constexpr FixedBigNum& operator<<=(std::size_t const& val) {
		auto word_offset = val >> 5;
		auto bit_offset = val & 0x1F;

		std::uint64_t buff;

		/* Start with most significant shift right and work backwards
		 */
		if constexpr(U < 2) {
			static_assert(false, "TODO: support for U < 2");
		}
		for(std::size_t idx = U - 2; idx > word_offset; idx--) {
			buff = (m_data[idx - word_offset] & 0xFFFFFFFF) << bit_offset;
			buff ^= (m_data[idx - word_offset - 1] & 0xFFFFFFFF) >> (32 - bit_offset);
			m_data[idx] = buff & 0xFFFFFFFF;
		}

		for(std::size_t idx = 0; idx < word_offset; idx++) {
			m_data[idx] = 0;
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
		if(word_offset >= U) {
			for(auto& v: m_data) {
				v = 0;
			}
		}
		std::uint64_t buff = m_data[word_offset+1];
		for(auto idx = 0; (idx + word_offset) < U; idx++) {
			buff <<= (32-bit_offset);
			buff ^= m_data[idx + word_offset] >> bit_offset;
			m_data[idx] = buff;
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

private:
	std::array<std::uint32_t, U> m_data;   // The number data itself
	bool						 m_signed; // The sign for the number
};

template<size_t U>
FixedBigNum<U> abs(FixedBigNum<U> const& num) {
	FixedBigNum<U> temp{num};
	temp.m_signed = false;
	return temp;
}

using int1024 = FixedBigNum<32>;

#endif // FIXED_BIGNUM_H_48E15CF0647345CD87782509952C8E4E
	
