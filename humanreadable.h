/*
 * File:      humanreadable.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

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


// Arithmetic
	/*
	 * Add two HumanReadableNums together
	 * TODO: Negative number handling (This will come with subtraction)
	 */
	HumanReadableNum& operator+=(HumanReadableNum const& add) {
		if(m_signed != add.m_signed) {
			// TODO:Handle this logic!!!!
		}
		char carry = 0;
		std::size_t limit = std::min(m_data.size(),add.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			carry += m_data[idx]&0x0F + add.m_data[idx]&0x0F;
			m_data[idx] = '0' + (carry % 10);
			carry /= 10;
		}
		if(carry == 0) {
			return *this;
		}

		if(limit == m_data.size()) {
			m_data.emplace_back('1');
		} else if ( m_data.size() > limit) {
			for(std::size_t idx = limit; (idx < m_data.size()) && (carry != 0); idx++) {
				carry += m_data[idx] & 0x0F;
				m_data[idx] = '0' + (carry % 10);
				carry /= 10;
			}
		} else {
			for(std::size_t idx = limit; (idx < add.m_data.size()) && (carry != 0); idx++) {
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
		// TODO:Complete
		char carry = 0;
		char temp = 0;
		std::size_t limit = std::min(m_data.size(),t.m_data.size());
		for(std::size_t idx = 0; idx < limit; idx++) {
			temp = m_data[idx];
			m_data[idx] = subtract_fast(temp, t.m_data[idx], carry);
			carry = temp < t.m_data[idx];
		}
		if(carry == 0) {
			return *this;
		}

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
		return true;
	} else if(lhs.m_signed && !rhs.m_signed) {
		return false;
	} else if(!lhs.m_signed && !rhs.m_signed) {
		if(lhs.m_data.size() > rhs.m_data.size()) return true;
		if(rhs.m_data.size() > lhs.m_data.size()) return false;
		for(std::size_t idx = lhs.m_data.size() - 1; idx >= 1; idx--) {
			if(lhs.m_data[idx] < rhs.m_data[idx]) return false;
			if(lhs.m_data[idx] > rhs.m_data[idx]) return true;
		}
		return (lhs.m_data[0] > rhs.m_data[0]);
	}
	
	if (lhs.m_data.size() < rhs.m_data.size()) return true;
	if (lhs.m_data.size() > rhs.m_data.size()) return false;
	for(std::size_t idx = lhs.m_data.size() - 1; idx >= 1; idx--) {
		if(lhs.m_data[idx] > rhs.m_data[idx]) return false;
		if(lhs.m_data[idx] < rhs.m_data[idx]) return true;
	}
	return (lhs.m_data[0] < rhs.m_data[0]);
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

#endif // HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
	
