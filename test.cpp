#include "testnum.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <cstdlib>
#include <cstdint>

TEST_CASE("uint32 round trip conversion works", "[constructor]") {
	auto a = GENERATE(take(100, random<unsigned int>(0, UINT32_MAX)));
	TestNum b{a};
	REQUIRE(b.to_uint32() == a);
}

TEST_CASE("Check Addition works", "[addition]") {
	auto testVals = GENERATE(take(100, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a + b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 + tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check Subtraction works", "[subtraction]") {
	auto testVals = GENERATE(take(1000, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a - b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 - tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check Multiplication works", "[multiplication]") {
	auto testVals = GENERATE(take(100, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a * b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 * tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check Division works", "[division]") {
	auto testVals = GENERATE(take(100, filter([](std::pair<unsigned int, unsigned int> const& i) {return (i.second != 0);}, pair_random<unsigned int>(0U, UINT32_MAX))));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a / b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 / tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check Modulus works", "[modulus]") {
	auto testVals = GENERATE(take(100, filter([](std::pair<unsigned int, unsigned int> const& i) {return (i.second != 0);}, pair_random<unsigned int>(0U, UINT32_MAX))));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a % b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 % tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check AND works", "[bitand]") {
	auto testVals = GENERATE(take(100, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a & b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 & tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check OR works", "[bitor]") {
	auto testVals = GENERATE(take(100, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a | b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 | tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check XOR works", "[bitxor]") {
	auto testVals = GENERATE(take(100, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;
	std::uint32_t res = a ^ b;
	TestNum tv1{a};
	TestNum tv2{b};
	auto result = tv1 ^ tv2;
	INFO("a = " << a << " b = " << b);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check NOT works", "[bitnot]") {
	std::uint32_t a = GENERATE(take(100, random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t res = ~a;
	TestNum tv1{a};		
	auto result = ~tv1;
	INFO("a = " << a );
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check Bitshift left works", "[bitshiftleft]") {
	auto testVals = GENERATE(take(1000, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t displacement = testVals.second % 32;
	auto res = a << displacement;
	TestNum tv1{a};
	auto result = tv1 << displacement;
	INFO("a = " << a << " Offset = " << displacement);
	CHECK(result.to_uint32() == res);

}

TEST_CASE("Check Bitshift right works", "[bitshiftright]") {
	auto testVals = GENERATE(take(1000, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t displacement = testVals.second % 32;
	std::uint32_t res = a >> displacement;
	TestNum tv1{a};
	auto result = tv1 >> displacement;
	INFO("a = " << a << " Offset = " << displacement);
	CHECK(result.to_uint32() == res);
}

TEST_CASE("Check LT Comparison operator",  "[ltcomp]") {
	auto testVals = GENERATE(take(1000, pair_random<unsigned int>(0U, UINT32_MAX)));
	std::uint32_t a = testVals.first;
	std::uint32_t b = testVals.second;

	TestNum tv1{a};
	TestNum tv2{b};

	INFO("a = " << a << " b = " << b << " tv1 = " << tv1.to_uint32() << " tv2 = " << tv2.to_uint32());

	CHECK((tv1 < tv2) == (a < b));
}

