/*
 * File:      coldstorage.cpp
 * Author:    Daniel Hannon
 *
 * Copyright: 2024 Daniel Hannon
 */

#include "coldstorage.h"
#include <random>
#include <string>
#include <uuid_v4.h>

std::string ColdStorage::generateUUID() {
	static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	auto uuid = uuidGenerator.getUUID();
	std::string out = uuid.str();
	return out;
}
