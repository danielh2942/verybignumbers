#include "arbitrary_bignum.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <cstdint>

TEST_CASE("Test ArbitraryBigNum constructor works as expected", "[arbbig_ctor]") {
	auto a = GENERATE(take(100, random<std::int64_t>(INT64_MIN, INT64_MAX)));
	ArbitraryBigNum b{a};
	std::stringstream ss;
	ss << a;
	std::string expected = ss.str();
	ss = {};
	ss << b;
	std::string result = ss.str();
	INFO("Expected = " << expected << " Result = " << result);
	CHECK(expected == result);
}

TEST_CASE("Check ArbitraryBigNum Addition works", "[arbbig_add]") {
	auto testVals = GENERATE(take(100, pair_random<std::uint64_t>(0U, INT64_MAX)));
	std::uint64_t a = testVals.first;
	std::uint64_t b = testVals.second;
	std::uint64_t res = a + b;
	ArbitraryBigNum tv1{a};
	ArbitraryBigNum tv2{b};
	auto result = tv1 + tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == res);
}

TEST_CASE("Check ArbitraryBigNum Subtraction works", "[arbbig_sub]") {
	auto testVals = GENERATE(take(1000, pair_random<std::uint64_t>(0U, UINT64_MAX)));
	std::uint64_t a = std::max(testVals.first, testVals.second);
	std::uint64_t b = std::min(testVals.second, testVals.first);
	std::uint64_t res = a - b;
	ArbitraryBigNum tv1{a};
	ArbitraryBigNum tv2{b};
	auto result = tv1 - tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == res);
}

TEST_CASE("Check ArbitraryBigNum spaceship operator works as expected", "[arbbig_spaceship]") {
	auto testVals = GENERATE(take(1000, pair_random<std::int64_t>(INT64_MIN,INT64_MAX)));
	std::int64_t a = testVals.first;
	std::int64_t b = testVals.second;
	std::partial_ordering expected = a <=> b;
	ArbitraryBigNum tv1{a};
	ArbitraryBigNum tv2{b};
	std::partial_ordering result = tv1<=>tv2;
	INFO("a = " << a << "  b = " << b << "\nExpected: " << comparisonString(expected) << " Got: " << comparisonString(result));
	CHECK(expected == result);
}

TEST_CASE("Check ArbitraryBigNum bitshift left works as expected", "[arbbig_leftshift]") {
	auto testVals = GENERATE(take(1000, pair_random<std::uint64_t>(0U,UINT32_MAX)));
	std::uint64_t a = testVals.first;
	std::uint64_t displacement = testVals.second % 32;
	auto res = a << displacement;
	ArbitraryBigNum tv1{a};
	auto result = tv1 << displacement;
	INFO("a = " << a << " Offset = " << displacement);
	CHECK(result == res);
}

TEST_CASE("Check ArbitraryBigNum bitshift right works as expected", "[arbbig_rightshift]") {
	auto testVals = GENERATE(take(1000, pair_random<std::uint64_t>(0U,UINT64_MAX)));
	std::uint64_t a = testVals.first;
	std::uint64_t displacement = testVals.second % 64;
	auto res = a >> displacement;
	ArbitraryBigNum tv1{a};
	auto result = tv1 >> displacement;
	INFO("a = " << a << " Offset = " << displacement);
	CHECK(result == res);
}

TEST_CASE("Check ArbitraryBigNum division operator works as expected", "[arbbig_div]") {
	auto testVals = GENERATE(take(1000, pair_random<std::int64_t>(INT64_MIN, INT64_MAX)));
	std::uint64_t a = std::max(testVals.first,testVals.second);
	std::uint64_t b = std::min(testVals.first, testVals.second);
	std::uint64_t expected = a / b;
	ArbitraryBigNum tv1{a};
	ArbitraryBigNum tv2{b};
	auto result = tv1 / tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == expected);
}

TEST_CASE("Check ArbitraryBigNum modulo operator works as expected", "[arbbig_mod]") {
	auto testVals = GENERATE(take(1000, pair_random<std::int64_t>(INT64_MIN, INT64_MAX)));
	std::uint64_t a = std::max(testVals.first,testVals.second);
	std::uint64_t b = std::min(testVals.first, testVals.second);
	std::uint64_t expected = a % b;
	ArbitraryBigNum tv1{a};
	ArbitraryBigNum tv2{b};
	auto result = tv1 % tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result == expected);
}

