#include "fixed_bignum.h"
#include <iostream>

template<size_t SIZE>
FixedBigNum<SIZE> fact(FixedBigNum<SIZE> const& a) {
	auto result = a;
	auto dec = a - 1;
	while(dec != 0) {
		result *= dec;
		dec--;
	}
	return result;
}

int main() {
	FixedBigNum<270> val{1000};
	auto res = fact(val);
	std::cout << res << std::endl;
	return 0;
}
