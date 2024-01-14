#include "humanreadable.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdint>

TEST_CASE("Test constructor works as expected", "[constructor]") {
	auto a = GENERATE(take(100, random<int>(INT32_MIN, INT32_MAX)));
	HumanReadableNum b{a};
	std::stringstream ss;
	ss << a;
	std::string expected = ss.str();
	ss = {};
	ss << b;
	std::string result = ss.str();
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(expected == result);
}

TEST_CASE("Verify subtraction works as expected", "[subtraction]") {
	// Testing the interval [-2,500,000 2,500,000] prevents rollover
	auto testVals = GENERATE(take(100, pair_random<int>(-2500000,2500000)));
	HumanReadableNum tv1{testVals.first};
	HumanReadableNum tv2{testVals.second};
	int expected = testVals.first - testVals.second;
	HumanReadableNum result = tv1 - tv2;
	INFO("Testing: " << testVals.first << " - " << testVals.second);
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(result == expected);
}

TEST_CASE("Verify addition works as expected", "[addition]") {
	// Testing the interval [-2,500,000 2,500,000] prevents rollover
	auto testVals = GENERATE(take(100, pair_random<int>(-2500000,2500000)));
	HumanReadableNum tv1{testVals.first};
	HumanReadableNum tv2{testVals.second};
	int expected = testVals.first + testVals.second;
	HumanReadableNum result = tv1 + tv2;
	INFO("Testing: " << testVals.first << " + " << testVals.second);
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(result == expected);
}

TEST_CASE("Verify less than works", "[lt_operator]") {
	// Testing the interval [-2,500,000 2,500,000] prevents rollover
	auto testVals = GENERATE(take(100, pair_random<int>(-2500000,2500000)));
	HumanReadableNum tv1{testVals.first};
	HumanReadableNum tv2{testVals.second};
	bool expected = testVals.first < testVals.second;
	bool result = tv1 < tv2;
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(result == expected);
}
