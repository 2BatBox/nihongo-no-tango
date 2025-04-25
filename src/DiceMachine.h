#pragma once

#include <cstdlib>
#include <cstdint>

/**
 * Provides an independent stream of pseudo-random numbers.
 */
class DiceMachine {
	uint16_t m_seed[3];
public:

	explicit DiceMachine(const uint64_t seed) {
		m_seed[2] = uint16_t((seed >> uint64_t(0)) & uint64_t(0xFFFF));
		m_seed[1] = uint16_t((seed >> uint64_t(16)) & uint64_t(0xFFFF));
		m_seed[0] = uint16_t((seed >> uint64_t(32)) & uint64_t(0xFFFF));
	}

	/**
	 * @param prob - must be in [0, 1] interval.
	 * @return The probability of returning true is @prob.
	 */
	bool pass(double prob) {
		return erand48(m_seed) < prob;
	}

	double range_double(double min, double max) {
		const double range = max - min;
		return min + erand48(m_seed) * range;
	}

	double drand48() {
		return erand48(m_seed);
	}

	long int lrand48() {
		return jrand48(m_seed);
	}

};
