/*
 * File:      util.cpp
 * Author:    Daniel Hannon
 *
 * Copyright: 2024 Daniel Hannon
 */

#include "util.h"

#include <uuid_v4.h>

#include <random>

std::string get_uuid() {
	static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	auto uuid = uuidGenerator.getUUID();
	std::string out = uuid.str();
	return out;
}
