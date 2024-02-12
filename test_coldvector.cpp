#include "cold_vector.h"
#include "test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Test ColdVector", "[coldvec]") {
	SECTION("Test Construction") {
		ColdVector<int> t{};
		CHECK(t.size() == 0);
	}

	SECTION("Check placement and access") {
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
}

TEST_CASE("Test Iterator", "[coldvec_iter]") {
	std::vector<int> a{1,2,3,4,5,6,7,8,9,10};
	ColdVector<int> testVec{};
	for(auto const& val : a) {
		testVec.emplace_back(val);
	}

	std::size_t idx = 0;
	for(auto itr = testVec.begin(); itr < testVec.end(); itr++) {
		CHECK(*itr == a[idx++]);
	}
}

TEST_CASE("Test Modification and access", "[coldvec_randaccess]") {
	std::vector<int> a{1,2,3,4,5,6,7,8,9,10};
	ColdVector<int> testVec{};
	for(auto const& val : a) {
		testVec.emplace_back(val);
	}

	for(auto& i : testVec) {
		i*=2;
	}

	std::vector<int> b{2,4,6,8,10,12,14,16,18,20};
	std::size_t idx = 0;
	for(auto const& v: b) {
		CHECK(testVec[idx++]  == v);
	}
}

