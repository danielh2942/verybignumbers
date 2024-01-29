#include "cold_vector.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <cstddef>

TEST_CASE("Test construction", "[vec_ctor]") {
	ColdVector<int> t{};
	CHECK(t.size() == 0);
}

TEST_CASE("Check placement and access", "[vec_insert]") {
	ColdVector<int, 1> temp{};
	temp.emplace_back(1);
	temp.emplace_back(2);
	temp.emplace_back(3);
	temp.emplace_back(4);
	temp.emplace_back(5);
	CHECK(temp.size() == 5);
	CHECK(temp[0] == 1);
	CHECK(temp[1] == 2);
	CHECK(temp[2] == 3);
	CHECK(temp[3] == 4);
	CHECK(temp[4] == 5);
}
