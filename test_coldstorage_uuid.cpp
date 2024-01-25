/*
 * File:		test_coldstorage_uuid.cpp
 * Author:		Daniel Hannon
 *
 * Copyright:	2023 Daniel Hannon
 *
 * Brief: Proof that I can generate UUIDs
 */

#include "coldstorage.h"
#include <iostream>
#include <array>

int main() {
	std::cout << "UUID Example: " << ColdStorage::generateUUID() << std::endl;
	ColdStorage test{};
	std::array<char, 6> buff{};
	std::cout << "Testing round trip rw" << std::endl;
	test.write_to(0,"Hello",6);
	buff = test.read_at<std::array<char,6>>(0,6);
	test.write_to(7,"My Name Jeff",13);
	auto tmp = test.read_at<std::array<char,13>>(7,13);
	for(auto const& v: buff) {
		std::cout << v;
	}
	std::cout << std::endl;
	for(auto const& v: tmp) {
		std::cout << v;
	}
	std::cout << std::endl;

	buff = test.read_at<std::array<char,6>>(0, 6);
	for(auto const& v: buff) {
		std::cout << v;
	}
	std::cout << std::endl;
	return 0;
}
