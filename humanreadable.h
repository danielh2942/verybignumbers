/*
 * File:      humanreadable.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <algorithm>
#include <string_view>
#ifndef HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
#define HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5 1
#include <ostream>
#include <vector>
#include <cmath>
#include <cstddef>

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

	HumanReadableNum(int x): m_data{}, m_signed(std::signbit(x)) {
		if(m_signed) x *= -1;
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
	friend bool operator!=(HumanReadableNum const& lhs, HumanReadableNum const& rhs);
	friend bool operator<(HumanReadableNum const& lhs, HumanReadableNum const& rhs);
	friend bool operator>(HumanReadableNum const& lhs, HumanReadableNum const& rhs);
	friend bool operator<=(HumanReadableNum const& lhs, HumanReadableNum const& rhs);
	friend bool operator>=(HumanReadableNum const& lhs, HumanReadableNum const& rhs);

// Mathematical functions
	friend HumanReadableNum abs(const HumanReadableNum &a);

// Assignment Operators
	HumanReadableNum& operator=(int x) {
		HumanReadableNum a{x};
		m_data.swap(a.m_data);
		m_signed = a.m_signed;
		return *this;
	}

// Arithmetic
	/*
	 * Add two HumanReadableNums together
	 * TODO: Negative number handling (This will come with subtraction)
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
		if(t == *this) {
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

		HumanReadableNum result = 0;
		
		char carry = 0;
		char tmp = 0;
		for(std::size_t idx = 0; idx < val.m_data.size(); idx++) {
			if(val.m_data[idx] == '0') continue;
			while(idx > result.m_data.size()) result.m_data.push_back('0');
			for(std::size_t idy = 0; idy < m_data.size(); idy++) {
				tmp = (m_data[idy] - '0');
				tmp *= (val.m_data[idx] - '0');
				tmp += carry;
				if((idx+idy) == result.m_data.size()) {
					result.m_data.emplace_back((tmp % 10) + '0');
				} else {
					// Bring the carry up the number and whatever
					for(std::size_t idz = idx+idy; idz < result.m_data.size(); idz++) {
						tmp += (result.m_data[idz] - '0');
						result.m_data[idz] = (tmp % 10) + '0';
						tmp /= 10;
					}
					if(tmp != 0) {
						result.m_data.emplace_back(tmp + '0');
					}
					tmp = 0;
				}
				carry = tmp / 10;
			}
			if(carry != 0) {
				for(std::size_t idy = m_data.size(); idy < result.m_data.size(); idy++) {
					tmp = result.m_data[idy] - '0';
					tmp += carry;
					result.m_data[idy] = (tmp%10) + '0';
					carry = tmp / 10;
				}
				if(carry != 0) {
					result.m_data.emplace_back(carry + '0');
					carry = 0;
				}
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
	static constexpr std::string_view sc_multiLUT[]  = {
			"0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
			"0", "2", "4", "6", "8", "10", "12", "14", "16", "18",
			"0", "3", "6", "9", "12", "15", "18", "21", "24", "27", 
			"0", "4", "8", "12", "16", "20", "24", "28", "32", "36", 
			"0", "5", "10", "15", "20", "25", "30", "35", "40", "45",
			"0", "6", "12", "18", "24", "30", "36", "42", "48", "54",
			"0", "7", "14", "21", "28", "35", "42", "49", "56", "63", 
			"0", "8", "16", "24", "32", "40", "48", "56", "64", "72", 
			"0", "9", "18", "27", "36", "45", "54", "63", "72", "81"};
};

inline bool operator==(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	if(&lhs == &rhs) return true;

	// TODO:Add handles to avoid -0
	if(lhs.m_signed != rhs.m_signed) return false;

	if(lhs.m_data.size() != rhs.m_data.size()) return false;

	for(std::size_t idx = 0; idx < lhs.m_data.size(); idx++) {
		if(lhs.m_data[idx] != rhs.m_data[idx]) return false;
	}

	return true;
}

inline bool operator!=(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	return !(lhs == rhs);
}

inline bool operator<(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	if(!lhs.m_signed && rhs.m_signed) {
		return false;
	} else if(lhs.m_signed && !rhs.m_signed) {
		return true;
	} else if(!lhs.m_signed && !rhs.m_signed) {
		if(lhs.m_data.size() > rhs.m_data.size()) return false;
		if(rhs.m_data.size() > lhs.m_data.size()) return true;
		for(std::size_t idx = lhs.m_data.size() - 1; idx >= 1; idx--) {
			if(lhs.m_data[idx] < rhs.m_data[idx]) return true;
			if(lhs.m_data[idx] > rhs.m_data[idx]) return false;
		}
		return (lhs.m_data[0] < rhs.m_data[0]);
	}
	
	if (lhs.m_data.size() > rhs.m_data.size()) return true;
	if (lhs.m_data.size() < rhs.m_data.size()) return false;
	for(std::size_t idx = lhs.m_data.size() - 1; idx >= 1; idx--) {
		if(lhs.m_data[idx] < rhs.m_data[idx]) return false;
		if(lhs.m_data[idx] > rhs.m_data[idx]) return true;
	}
	return (lhs.m_data[0] > rhs.m_data[0]);
}

inline bool operator<=(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	return (lhs<rhs) || (lhs==rhs);
}

inline bool operator>(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	return rhs < lhs;
}

inline bool operator>=(HumanReadableNum const& lhs, HumanReadableNum const& rhs) {
	return (lhs > rhs) || (lhs == rhs);
}

inline HumanReadableNum abs(HumanReadableNum const &a) {
	HumanReadableNum tmp{a};
	tmp.m_signed = false;
	return tmp;
}

#endif // HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
	
