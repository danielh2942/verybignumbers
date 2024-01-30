#include "fixed_bignum.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <compare>
#include <cstdint>
#include <string>

using TestFixed = FixedBigNum<2>;

TEST_CASE("Test constructor works as expected", "[fixbig_ctor]") {
	auto a = GENERATE(take(100, random<long>(INT64_MIN, INT64_MAX)));
	TestFixed b{a};
	std::stringstream ss;
	ss << a;
	std::string expected = ss.str();
	ss = {};
	ss << b;
	std::string result = ss.str();
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(expected == result);
}

TEST_CASE("Check Addition works", "[fixbig_add]") {
	auto testVals = GENERATE(take(100, pair_random<std::uint64_t>(0U, UINT64_MAX)));
	std::uint64_t a = testVals.first;
	std::uint64_t b = testVals.second;
	std::uint64_t res = a + b;
	TestFixed tv1{a};
	TestFixed tv2{b};
	auto result = tv1 + tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == res);
}

TEST_CASE("Check Subtraction works", "[fixbig_sub]") {
	auto testVals = GENERATE(take(1000, pair_random<std::uint64_t>(0U, UINT64_MAX)));
	std::uint64_t a = std::max(testVals.first, testVals.second);
	std::uint64_t b = std::min(testVals.second, testVals.first);
	std::uint64_t res = a - b;
	TestFixed tv1{a};
	TestFixed tv2{b};
	auto result = tv1 - tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == res);
}

TEST_CASE("Check spaceship operator works as expected", "[fixbig_spaceship]") {
	auto testVals = GENERATE(take(1000, pair_random<std::int64_t>(INT64_MIN,INT64_MAX)));
	std::int64_t a = testVals.first;
	std::int64_t b = testVals.second;
	std::partial_ordering expected = a <=> b;
	TestFixed tv1{a};
	TestFixed tv2{b};
	std::partial_ordering result = tv1<=>tv2;
	INFO("a = " << a << "  b = " << b << "\nExpected: " << comparisonString(expected) << " Got: " << comparisonString(result));
	CHECK(expected == result);
}

