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
	auto testVals = GENERATE(take(100, pair_random<int>(INT32_MIN + 1,INT32_MAX)));
	// TODO:get this working properly. handle edge cases and stuff
	CHECK(true);
}
