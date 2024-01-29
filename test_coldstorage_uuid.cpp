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
#include <cstdint>

int main() {
	std::cout << "UUID Example: " << ColdStorage::generateUUID() << std::endl;
	ColdStorage test{true};
	std::cout << "Testing round trip rw" << std::endl;
	test.write_to(0,"Hello",6);
	test.write_to(7,"My Name Jeff",13);
	auto strBuff = std::array<char,13>{};
	test.read_at(&strBuff,7,13);
	std::array<char, 6> buff{};
	test.read_at(&buff,0,6);
	for(auto const& v: strBuff) {
		std::cout << v;
	}
	std::cout << std::endl;
	
	test.read_at(&buff, 0, 6);
	for(auto const& v: buff) {
		std::cout << v;
	}
	std::cout << std::endl;

	std::cout << "Testing non string data" << std::endl;
	std::array<std::uint32_t,7> out = {2233,454345,213213,3243242,876546,99654,334534};
	test.write_to(0, out, 28);
	std::array<std::uint32_t,7> in{};
	test.read_at(&in, 0, 28);
	for(auto idx = 0; idx < 7; idx++) {
		if(in[idx] != out[idx]) {
			std::cout << "Mismatch! Expected: " << out[idx] << " Got: " << in[idx] << std::endl;
		}
	}
	test.read_at(&strBuff, 7, 13);
	std::cout << strBuff.begin();
	return 0;
}
