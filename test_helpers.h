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


class RandomPairGenerator : public Catch::Generators::IGenerator<std::pair<unsigned int, unsigned int>> {
	std::minstd_rand m_rand;
	std::uniform_int_distribution<unsigned int> m_dist;
	std::pair<unsigned int, unsigned int> current;
public:

	RandomPairGenerator(unsigned int low, unsigned int high):
		m_rand{std::random_device{}()},
		m_dist(low, high)
	{
		static_cast<void>(next());
	}

	std::pair<unsigned int, unsigned int> const& get() const override;

	bool next() override {
		current = {m_dist(m_rand), m_dist(m_rand)};
		return true;
	}
};

inline std::pair<unsigned int, unsigned int> const& RandomPairGenerator::get() const {
	return current;
}

inline Catch::Generators::GeneratorWrapper<std::pair<unsigned int, unsigned int>> pair_random(unsigned int low, unsigned int high) {
	return Catch::Generators::GeneratorWrapper<std::pair<unsigned int, unsigned int>>(
			new RandomPairGenerator(low,high)
	);
}


#endif // TEST_HELPERS_H_583EA76E46D841E09555B7A66D76820F
	
