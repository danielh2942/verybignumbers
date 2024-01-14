/*
 * File:      test_helpers.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2023 Daniel Hannon 
 */

#include <random>
#ifndef TEST_HELPERS_H_583EA76E46D841E09555B7A66D76820F
#define TEST_HELPERS_H_583EA76E46D841E09555B7A66D76820F 1

#include <catch2/generators/catch_generators_all.hpp>

#include <utility>

template<typename T>
class RandomPairGenerator : public Catch::Generators::IGenerator<std::pair<T, T>> {
	std::minstd_rand m_rand;
	std::uniform_int_distribution<T> m_dist;
	std::pair<T, T> current;
public:

	RandomPairGenerator(T low, T high):
		m_rand{std::random_device{}()},
		m_dist(low, high)
	{
		static_cast<void>(next());
	}

	std::pair<T, T> const& get() const override {
		return current;
	}

	bool next() override {
		current = {m_dist(m_rand), m_dist(m_rand)};
		return true;
	}
};

template<typename T>
Catch::Generators::GeneratorWrapper<std::pair<T, T>> pair_random(T low, T high) {
	return Catch::Generators::GeneratorWrapper<std::pair<T, T>>(
			new RandomPairGenerator<T>(low,high)
	);
}


#endif // TEST_HELPERS_H_583EA76E46D841E09555B7A66D76820F
	
