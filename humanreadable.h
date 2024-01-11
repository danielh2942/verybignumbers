/*
 * File:      humanreadable.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <cstddef>
#ifndef HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
#define HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5 1
#include <ostream>
#include <vector>
#include <cmath>

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
		char tmp = 0;
		char buff = 0;
		
		return *this;
	}

private:
	std::vector<char> m_data;	// String digit data, in reverse order
	bool			  m_signed; // If the number is signed and whatever
};

#endif // HUMANREADABLE_H_2ADA56A63CC34EC5B844EF7F7C7178B5
	
