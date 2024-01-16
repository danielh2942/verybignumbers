/*
 * File:      humanreadable.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <algorithm>
#include <compare>
#include <string_view>
#ifndef HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
#define HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5 1
#include <ostream>
#include <vector>
#include <cmath>
#include <cstddef>
#include <concepts>

/*
 * HUMAN READABLE NUMBER
 * basically, this is a base-10 number type that should only be used when printing numbers
 *
 * TODO:Make it store the number data in a **FILE** as this will grow RAPIDLY!
 * TODO:Make the number support decimal places
 *
 */
struct HumanReadableNum {

	HumanReadableNum(): m_data{'0'}, m_signed{false}
	{}

	HumanReadableNum(std::signed_integral auto x): m_data{}, m_signed(std::signbit(x)) {
		if(m_signed) x *= -1;
		while(x != 0) {
			m_data.emplace_back((x%10)+'0');
			x/=10;
		}
	}

	HumanReadableNum(std::unsigned_integral auto x): m_data{}, m_signed{false} {
		while(x != 0) {
			m_data.emplace_back((x%10)+'0');
			x/=10;
		}
	}

	HumanReadableNum(HumanReadableNum const& a): m_data{a.m_data}, m_signed{a.m_signed}
	{}

	HumanReadableNum(HumanReadableNum const&& a): m_data{std::move(a.m_data)}, m_signed{a.m_signed}
	{}

	friend std::ostream& operator<<(std::ostream& os, HumanReadableNum const& hrn) {
		if(hrn.m_signed) {
			os << '-';
		}
		for(auto it = hrn.m_data.rbegin(); it != hrn.m_data.rend(); it++) {
			os << *it;
		}
		return os;	
	}

// Comparison Operators
	friend bool operator==(HumanReadableNum const& lhs, HumanReadableNum const& rhs);
	friend std::strong_ordering operator<=>(HumanReadableNum const& lhs, HumanReadableNum const& rhs);

// Mathematical functions
	friend HumanReadableNum abs(const HumanReadableNum &a);

// Assignment Operators
	HumanReadableNum& operator=(std::integral auto x) {
		HumanReadableNum a{x};
		m_data.swap(a.m_data);
		m_signed = a.m_signed;
		return *this;
	}

// Arithmetic
	/*
	 * Add two HumanReadableNums together
	 */
	HumanReadableNum& operator+=(HumanReadableNum const& add) {
		if(m_signed != add.m_signed) {
			m_signed ^= true;
			operator-=(add);
			m_signed^=true;
			return *this;
		}
		char carry = 0;
		std::size_t limit = std::min(m_data.size(),add.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			carry += (m_data[idx] - '0') + (add.m_data[idx] - '0');
			m_data[idx] = '0' + (carry % 10);
			carry /= 10;
		}

		if ( m_data.size() > limit) {
			for(std::size_t idx = limit; (idx < m_data.size()) && (carry != 0); idx++) {
				carry += m_data[idx] & 0x0F;
				m_data[idx] = '0' + (carry % 10);
				carry /= 10;
			}
		} else if(add.m_data.size() > limit) {
			for(std::size_t idx = limit; (idx < add.m_data.size()); idx++) {
				carry += add.m_data[idx] & 0x0F;
				m_data.emplace_back('0' + (carry % 10));
				carry /= 10;
			}
		}
		if(carry) {
			m_data.emplace_back('1');
		}

		while((m_data.size() > 1) && (m_data[m_data.size() - 1] == '0')) m_data.pop_back();
		return *this;
	}

	HumanReadableNum& operator++() {
		*this += 1;
		return *this;
	}

	HumanReadableNum operator+(HumanReadableNum const& add) const {
		HumanReadableNum tmp{*this};
		tmp += add;
		return tmp;
	}

	HumanReadableNum& operator-=(HumanReadableNum const& t) {
		// Prevent -0
		if(&t == this) {
			m_data = {'0'};
			m_signed = false;
			return *this;
		}

		// LOGIC TABLE for subtraction

		/* - |  +  |  -  |
		 * _______________
		 * + | sub | add |
		 * _______________
		 * - | add | sub |
		 * _______________
		 */

		if(m_signed != t.m_signed) {
			// Flip the sign and use addition logic :D
			m_signed ^= true;
			operator+=(t);
			m_signed ^= true;
			return *this;
		}
		
		// if it reaches this, the sign definitely flips
		if(abs(*this) < abs(t)) {
			HumanReadableNum temp = t - *this;
			m_data.swap(temp.m_data);
			m_signed ^= true;
			return *this;
		}

		char temp = 0;
		char carry = 0;
		std::size_t limit = std::min(m_data.size(), t.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			temp = (m_data[idx] - 38) - carry;
			temp -= (t.m_data[idx] - '0');
			carry = temp < 10;
			m_data[idx] = (temp % 10) + '0';
		}

		// Addition and ting
		if(limit == m_data.size() && m_data.size() != t.m_data.size()) {
			for(std::size_t idx = limit; idx < t.m_data.size(); idx++) {
				temp = t.m_data[idx] - carry;
				if(temp < '0') {
					carry = 1;
					m_data.emplace_back('9');
				} else {
					carry = 0;
					m_data.emplace_back(temp);
				}
			}
		} else {
			for(std::size_t idx = limit; idx < m_data.size(); idx++) {
				temp = (m_data[idx] - 38) - carry;
				carry = temp < 10;
				m_data[idx] = (temp % 10) + '0';
			}
		}

		// Tidy up by eliminating leading zeroes.
		std::size_t idx = m_data.size() - 1;
		while(idx > 0 && m_data[idx] == '0') {
			m_data.pop_back();
			idx--;
		}

		// Handle -0 case
		if(m_data.size() == 1 && m_data[0] == '0') {
			m_signed = false;
		}

		return *this;
	}

	HumanReadableNum& operator--() {
		*this -= 1;
		return *this;
	}

	HumanReadableNum operator-(HumanReadableNum const& val) const {
		HumanReadableNum temp{*this};
		temp -= val;
		return temp;
	}

	// TODO:Implement Karatsuba for this type
	HumanReadableNum operator*(HumanReadableNum const& val) const {
		if((val == 0) || (*this == 0)) {
			return 0;
		}

		// This uses a Lookup table as multiplication of base 10 on a computer is annoying
		HumanReadableNum result = 0;
		HumanReadableNum temp = 0;
		for(std::size_t idx = 0; idx < val.m_data.size(); idx++) {
			if(val.m_data[idx] == '0') continue;
			while((idx + 1) >= temp.m_data.size()) {
				temp.m_data.emplace_back('0');
			}
			for(std::size_t idy = 0; idy < m_data.size(); idy++) {
				if((idx + idy + 1) >= temp.m_data.size()) temp.m_data.emplace_back('0');
				auto lutres = sc_multiLUT[((m_data[idy] - '0') * 10) + (val.m_data[idx] - '0')];
				temp.m_data[idx+idy] = lutres[0];
				temp.m_data[idx+idy+1] = lutres[1];
				result += temp;
				temp.m_data[idx+idy] = '0';
				temp.m_data[idx+idy+1] = '0';
			}
		}

		if(m_signed != val.m_signed) {
			result.m_signed = true;
		}

		return result;
	}

	HumanReadableNum& operator*=(HumanReadableNum const& v) {
		HumanReadableNum tmp = this->operator*(v);
		m_data.swap(tmp.m_data);
		m_signed = tmp.m_signed;
		return *this;
	}

private:
	char subtract_fast(char a, char b, char carry) {
		a -= '0';
		b -= '0';
		return (std::abs(a-b-carry)&0xFF) + '0';
	}

private:
	std::vector<char> m_data;	// String digit data, in reverse order
	bool			  m_signed; // If the number is signed and whatever

private:
	// Lookup table for multiplication.
	// These are all back to front
	static constexpr std::string_view sc_multiLUT[]  = {
			"00", "00", "00", "00", "00", "00", "00", "00", "00", "00",
			"00", "10", "20", "30", "40", "50", "60", "70", "80", "90",
			"00", "20", "40", "60", "80", "01", "21", "41", "61", "81",
			"00", "30", "60", "90", "21", "51", "81", "12", "42", "72", 
			"00", "40", "80", "21", "61", "02", "42", "82", "23", "63", 
			"00", "50", "01", "51", "02", "52", "03", "53", "04", "54",
			"00", "60", "21", "81", "42", "03", "63", "24", "84", "45",
			"00", "70", "41", "12", "82", "53", "24", "94", "65", "36", 
			"00", "80", "61", "42", "23", "04", "84", "65", "46", "27", 
			"00", "90", "81", "72", "63", "54", "45", "36", "27", "18"};
};

inline bool operator==(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	return std::is_eq(lhs<=>rhs);
}

inline HumanReadableNum abs(HumanReadableNum const &a) {
	HumanReadableNum tmp{a};
	tmp.m_signed = false;
	return tmp;
}

inline std::strong_ordering operator<=>(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	if(&lhs == &rhs) return std::strong_ordering::equal;
	if(auto sign = rhs.m_signed <=> lhs.m_signed; sign != 0) return sign;
	// Positive numbers
	if(!lhs.m_signed) {
		if(auto width = lhs.m_data.size() <=> rhs.m_data.size(); width != 0) {
			return width;
		}
		for(std::size_t idx = lhs.m_data.size(); idx > 0; idx--) {
			if(auto val = lhs.m_data[idx -1] <=> rhs.m_data[idx - 1]; val != 0) {
				return val;
			}
		}
	} else {
		if(auto width = rhs.m_data.size() <=> lhs.m_data.size(); width != 0) {
			return width;
		}
		for(std::size_t idx = rhs.m_data.size(); idx > 0; idx--) {
			if(auto val = rhs.m_data[idx-1] <=> lhs.m_data[idx-1]; val != 0) {
				return val;
			}
		}
	}
	return std::strong_ordering::equal;
}

#endif // HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
	
