/*
 * File:      testnum.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2023 Daniel Hannon 
 */

#ifndef TESTNUM_H_A9029654B9334CB6AF4DC1861248C6DD
#define TESTNUM_H_A9029654B9334CB6AF4DC1861248C6DD 1
#include <algorithm>
#include <compare>
#include <cstddef>
#include <iostream>
#include <array>
#include <cstdint>
#include <ostream>
#include <utility>

#include "helpers.h"

// TestNum is a proof-of-concept for large integer types
// It replicates a uint32 using only characters to test algorithms
// for numbers that are greater than the largest integer type
// (which this very much is not)
struct TestNum {
public:
	TestNum(): m_data{0,0,0,0}
	{}

	TestNum(std::uint32_t val): m_data{0,0,0,0}
	{
		m_data[0] = val;
		m_data[1] = val >> 8;
		m_data[2] = val >> 16;
		m_data[3] = val >> 24;
	}

	TestNum(TestNum const& t) : m_data{t.m_data}
	{}

	TestNum(TestNum&& t) : m_data{std::move(t.m_data)}
	{}

	// Output the number as a uint32 because it's equivalent to that anyway
	std::uint32_t to_uint32() const {
		std::uint32_t outp = 0;
		outp ^= m_data[0] & 0x000000FF;
		outp ^= (m_data[1] << 8) & 0x0000FF00;
		outp ^= (m_data[2] << 16) & 0x00FF0000;
		outp ^= (m_data[3] << 24) & 0xFF000000;
		return outp;
	}

// Comparators

	friend bool operator==(TestNum const& lhs, TestNum const& rhs);
	friend bool operator!=(TestNum const& lhs, TestNum const& rhs);
	friend std::weak_ordering operator<=>(TestNum const& lhs, TestNum const& rhs);


// Assignment Operator
	TestNum& operator=(TestNum const& t) {
		if(this == &t) {
			return *this;
		}

		m_data = t.m_data;
		return *this;
	}

	TestNum& operator=(std::uint32_t const& val) {
		m_data[0] = val;
		m_data[1] = val >> 8;
		m_data[2] = val >> 16;
		m_data[3] = val >> 24;
		return *this;
	}

// Arithmetic Operators
	TestNum operator+(TestNum const& val) const {
		TestNum res{*this};
		res += val;
		return res;
	}

	TestNum& operator++() {
		*this += 1;
		return *this;
	}

	TestNum& operator+=(TestNum const& t) {
		std::uint16_t buff = 0;
		for(std::size_t i = 0; i < 4; i++) {
			buff += (m_data[i]&0x00FF) + (t.m_data[i]&0x00FF);
			m_data[i] = (buff & 0x00FF);
			buff = (buff >> 8) & 0x00FF;
		}
		return *this;
	} 

	TestNum operator-(TestNum const& val) const {
		TestNum res{*this};
		res -= val;
		return res;
	}

	TestNum& operator--() {
		auto tmp = *this - 1;
		m_data.swap(tmp.m_data);
		return *this;
	}

	TestNum& operator-=(TestNum const& t) {
		std::uint16_t tmp = 0;	
		std::uint16_t buff = 0;
		for(std::size_t i = 0; i < 4; i++) {
			tmp = m_data[i] & 0x00FF;
			buff = tmp - (t.m_data[i]&0x00FF) - buff;
			m_data[i] = buff & 0x00FF;
			// Underflow can occur.
			buff = (buff > tmp);
		}
		return *this;
	}

	TestNum operator*(TestNum const& t) const {
		if(t == TestNum{0}) {
			return 0;
		}
		TestNum outp{0};

		std::uint16_t buff = 0;
		std::uint16_t tmp = 0;

		// This is n^2 because I'm not bothered making this more efficient yet
		for(std::size_t i = 0; i < 4; i++) {
			// avoid pointless operations
			if(t.m_data[i] == 0) continue;
			for(std::size_t j = 0; j + i < 4; j++) {
				tmp = (m_data[j] & 0xFF);
				tmp *= (t.m_data[i] & 0xFF);
				buff += tmp;
				buff += (outp.m_data[j + i] & 0xFF);
				outp.m_data[j + i] = (buff& 0xFF);
				buff = buff >> 8;
			}
			buff = 0;
		}

		return outp;
	}

	TestNum& operator*=(TestNum const& t) {
		auto tmp = TestNum{*this};
		tmp = tmp * t;
		m_data.swap(tmp.m_data);
		return *this;
	}

	TestNum operator/(TestNum const& t) const {
		auto tmp = this->simple_divide(t);
		return tmp.first;
	}

	TestNum& operator/=(TestNum const& t) {
		auto tmp = this->operator/(t);
		m_data.swap(tmp.m_data);
		return *this;
	}

	TestNum operator%(TestNum const& t) const {
		auto tmp = this->simple_divide(t);
		return tmp.second;
	}

	TestNum& operator%=(TestNum const& t) {
		auto tmp = *this % t;
		m_data.swap(tmp.m_data);
		return *this;
	}

// Bitwise operators
	// Shift left operator for the TestNum type, it's proof-of-concept so idk has to be done and whatever
	TestNum operator<<(std::size_t const& val) const {
		TestNum temp{0};
		
		auto byte_offset = val >> 3;
		auto bit_offset  = val & 0x7;

		std::uint16_t buff = 0;
	
		if(byte_offset >= 4) {
			return 0;
		}

		for(auto i = 0; (i + byte_offset) < 4; i++) {
			buff = buff >> 8;
			buff ^= ((m_data[i] & 0xFF) << bit_offset);
			temp.m_data[byte_offset + i] = (buff & 0xFF);
		}

		return temp;
	}

	// Shift right operator does what you'd expect :D
	TestNum operator>>(std::size_t const& val) const {
		TestNum temp{0};

		auto byte_offset = val >> 3;
		auto bit_offset = val & 0x7;

		std::uint16_t buff = 0;

		if(byte_offset >= 4) {
			return 0;
		}

		for(auto i = 0; (i + byte_offset) < 4; i++) {
			buff = buff << (8 - bit_offset);
			buff ^= ((m_data[3 - i] & 0xFF) >> bit_offset);
			temp.m_data[3 - (byte_offset + i)] = buff;
			buff = m_data[3-i] & 0xFF;
		}

		return temp;
	}

// Bitwise Operators
	TestNum operator&(TestNum const& test) const {
		TestNum val{*this};
		for(std::size_t idx = 0; idx < 4; idx++) {
			val.m_data[idx] &= test.m_data[idx];
		}
		return val;
	}

	TestNum& operator&=(TestNum const& test) {
		auto val = operator&(test);
		m_data.swap(val.m_data);
		return *this;
	}

	TestNum operator^(TestNum const& mask) const {
		TestNum val{*this};
		for(std::size_t idx = 0; idx < 4; idx++) {
			val.m_data[idx] ^= mask.m_data[idx];
		}
		return val;
	}

	TestNum& operator^=(TestNum const& mask) {
		auto val = operator^(mask);
		m_data.swap(val.m_data);
		return *this;
	}

	TestNum operator|(TestNum const& cmp) const {
		TestNum val{*this};
		for(std::size_t idx = 0; idx < 4; idx++) {
			val.m_data[idx] |= cmp.m_data[idx];
		}
		return val;
	}

	TestNum& operator|=(TestNum const& cmp) {
		auto val = operator|(cmp);
		m_data.swap(val.m_data);
		return *this;
	}

	TestNum operator~() const {
		TestNum val{*this};
		for(auto & c : val.m_data) {
			c = ~c;
		}
		return val;
	}

// Ostream
	friend std::ostream & operator<<(std::ostream & os, TestNum const& v);

// Test Code
	std::size_t get_most_populated() const;

	std::pair<TestNum,TestNum> split_at(std::size_t idx) const;

	static TestNum karatsuba_multiplication(TestNum const& v1, TestNum const& v2);

private:
	// Internal divide maybe make this public?
	std::pair<TestNum, TestNum> divide_by(TestNum const& val);

	std::pair<TestNum, TestNum> simple_divide(TestNum const& val) const;

	std::array<std::uint8_t,4> m_data; // The data stored within
};

//Equivalance operator
inline bool operator==(TestNum const& lhs, TestNum const& rhs) {
	return std::is_eq(lhs <=> rhs);
}

inline bool operator!=(TestNum const& lhs, TestNum const& rhs) {
	return !(lhs==rhs);
}

inline std::weak_ordering operator<=>(TestNum const& lhs, TestNum const& rhs) {
	if(&lhs == &rhs) return std::weak_ordering::equivalent;
	for(auto idx = 4; idx > 0; idx--) {
		if(auto cmp = lhs.m_data[idx - 1] <=> rhs.m_data[idx - 1]; cmp != 0) {
			return cmp;
		}
	}
	return std::weak_ordering::equivalent;
}

inline std::ostream& operator<<(std::ostream & os, TestNum const& v) {
	os << v.to_uint32();
	return os;
}

inline std::size_t TestNum::get_most_populated() const {
	if(m_data[3] != 0) return 3;
	if(m_data[2] != 0) return 2;
	if(m_data[1] != 0) return 1;
	return 0;
}

inline std::pair<TestNum,TestNum> TestNum::split_at(std::size_t idx) const {
	std::cout << "Splitting at idx: " << idx << std::endl;
	TestNum left = 0;
	TestNum right = 0;
	
	int j = 0;
	for(int i = idx; i <= 3; i++) { 
		left.m_data[j] = m_data[i];
		j++;
	}

	for(int i = 0; i < idx; i++) {
		right.m_data[i] = m_data[i];
	}
	std::cout << "Left: " << left << ", Right: " << right << std::endl;
	return {left, right};
}

// Multiplication using the Karatsuba Multiplication formula, this has an O(n^1.56) as opposed to 
// standard multiplication which is O(n^2). It's not particularly useful for this class but,
// it's a good testbench for a larger number as that is algorithm is very much portable
inline TestNum TestNum::karatsuba_multiplication(TestNum const& v1,TestNum const& v2) {
	if((v1.get_most_populated() == 0) || (v2.get_most_populated() == 0)) {
		return v1 * v2;
	}

	std::size_t split_point = std::max(v1.get_most_populated(),v2.get_most_populated());
	split_point = (split_point >> 1) + (split_point != 0);
	if(split_point == 0) {
		return v1 * v2;
	}

	auto p1 = v1.split_at(split_point);
	auto p2 = v2.split_at(split_point);

	auto r0 = TestNum::karatsuba_multiplication(p1.second, p2.second);
	auto r1 = TestNum::karatsuba_multiplication((p1.first + p1.second), (p2.first + p2.second));
	auto r2 = TestNum::karatsuba_multiplication(p1.first, p2.first);

	std::cout << "Split Point: " << split_point << ", r0: " << r0 << ", r1: " << r1 << ", r2: " << r2 << std::endl;

	// The 16 and 8 represent byte offsets as the base here is 256
	// and I can just bitshift instead of doing a multiplication which makes this cleaner
	auto comp1 = (r2 << (split_point * 16));
	auto comp2 = ((r1 - r2) - r0) << (split_point * 8);
	std::cout << "Sum is: " << comp1 << " + " << comp2 << " + " << r0 << std::endl;
	return comp1 + comp2 + r0;
}



// This is going to utilize Knuths Algorithm D
// TODO:Complete
inline std::pair<TestNum, TestNum> TestNum::divide_by(TestNum const& divisor) {
	static constexpr std::uint16_t base = 256;
	if(divisor == TestNum{0} || *this == TestNum{0}) {
		return {0,0};
	}

	if(*this == divisor) {
		return {1,0};
	}

	/*
	 * Algorithm D overview
	 *
	 * D0:
	 *   Let U be the dividend of m+n digits stored in an array of m+n+1 digits with LSD at U[0] and Most Significant at U[m+n-1]
	 *   Let V be the divisor of n digits stored in a second array of n elements with n > 1 most Significant non zero digit at V[n-1] and least Significant digit at V[0]
	 *   Let B be the base of the "digits" in this case we are using base 256
	 * D1:  
	 *  Normalize by multiplying U and V by some D (since this is a base2 computer we can just do bitshifts for this :D)
	 */
	// Get number of bitshifts :D
	std::size_t m = 0;
	std::size_t n = 0;

	for(n = 3 ;n != 0;n--) {
		if(divisor.m_data[n] != 0) break;
	}

	for(m = 3; m != 0; m--) {
		if (m_data[m] != 0) break;
	}

	if(m < n) {
		return {0,*this};
	}

	// Algorithm D requires that the divisor be at least two digits in length
	// Otherwise we just do schoolboy division which is fine.
	if(n == 0) {
		std::cout << "Divisor is less than 256 using Schoolboy division" << std::endl;
		// Schoolboy division 
		std::uint32_t dividend = 0;
		std::uint32_t remainder = 0;
		std::uint16_t buffer = 0;
		for(int idx = 3; idx >= 0; idx--) {
			buffer = buffer << 8;
			dividend = dividend << 8;
			buffer ^= (m_data[idx] & 0xFF);
			dividend ^= ((buffer / divisor.m_data[0]) & 0xFF);
			buffer = buffer % divisor.m_data[0];
		}
		remainder  = buffer & 0x000000FF;
		return {dividend,remainder};
	}

	// Normalize data
	auto v = divisor << clz_uint8(divisor.m_data[n]);
	auto u = *this << clz_uint8(divisor.m_data[n]);

	m -= n;
	n++;
	m++;
	for(auto j = m; j >= 0; j--) {
		std::uint16_t q_p = (u.m_data[n + j] << 16) ^ (u.m_data[n + j - 1] & 0x00FF) / v.m_data[n - 1];
		std::uint16_t r_p = (u.m_data[n + j] << 16) ^ (u.m_data[n + j - 1] & 0x00FF) % v.m_data[n - 1];
		do {
			if ((q_p == base) || 
				((q_p * u.m_data[n-2]) > ((r_p << 16 ) ^ (u.m_data[n - 2 + j] & 0x00FF)))) {
				q_p--;
				r_p += v.m_data[n-1];
			}
		} while(r_p < base);

	}


	return {std::move(u),0};
}

// this just does standard long division whatever
inline std::pair<TestNum, TestNum> TestNum::simple_divide(TestNum const& t) const {
	// Division by zero is undefined in the C++ standard, so 
	// I think short circuiting the division is the best approach.
	
	if(t == 0) return {0,0};
	
	TestNum remainder{*this};
	TestNum qty{0};

	std::size_t len_divisor = t.get_most_populated();
	std::size_t len_dividend = get_most_populated();
	
	// divisor bigger so remainder is just the number
	if(len_divisor > len_dividend) return {qty, remainder};

	int dividend_pop_bits = (8 - clz_uint8(m_data[len_dividend])) + (len_dividend * 8);
	int divisor_pop_bits = (8 - clz_uint8(t.m_data[len_divisor])) + (len_divisor * 8);
	int operations = dividend_pop_bits - divisor_pop_bits;

	//int operations = (((3 - len_divisor) * 8) + clz_uint8(t.m_data[len_divisor])) - (((3 - len_dividend) * 8) + clz_uint8(t.m_data[len_dividend]));
	if(operations < 0) return {0, remainder};
	// This may produce undefined behavior but it won't matter because the result will be discarded
	TestNum divisor = t << operations;
	while((divisor >= t) && (operations >= 0)) {
		qty = qty << 1;
		if(remainder >= divisor) {
			remainder -= divisor;
			qty += 1;
		}
		divisor = divisor >> 1;
		operations--;
	}
	

	return {qty, remainder};
}

#endif // TESTNUM_H_A9029654B9334CB6AF4DC1861248C6DD
	
