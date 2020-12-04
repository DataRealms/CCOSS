#include "RTETools.h"
#include "Vector.h"

namespace RTE {

	std::mt19937 g_RNG;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float GetMPP() { return c_MPP; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float GetPPM() { return c_PPM; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float GetLPP() { return c_LPP; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float GetPPL() { return c_PPL; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SeedRNG() {
		// Pre-Seed generation
		std::array<int, 624> seedData;
		std::random_device randomDevice;
		std::generate_n(seedData.data(), seedData.size(), std::ref(randomDevice));

		std::seed_seq sequence(std::begin(seedData), std::end(seedData));
		g_RNG.seed(sequence);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SeedRNG(unsigned int seed) {
		g_RNG.seed(seed);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LERP(float scaleStart, float scaleEnd, float startValue, float endValue, float progressScalar) {
		if (progressScalar <= scaleStart) {
			return startValue;
		} else if (progressScalar >= scaleEnd) {
			return endValue;
		}
		return startValue + ((progressScalar - scaleStart) * ((endValue - startValue) / (scaleEnd - scaleStart)));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseIn(float start, float end, float progressScalar) {
		if (progressScalar <= 0) {
			return start;
		} else if (progressScalar >= 1.0F) {
			return end;
		}
		float t = 1 - progressScalar;
		return (end - start) * (std::sin(-t * c_HalfPI) + 1) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseOut(float start, float end, float progressScalar) {
		if (progressScalar <= 0) {
			return start;
		} else if (progressScalar >= 1.0F) {
			return end;
		}
		return (end - start) * -std::sin(-progressScalar * c_HalfPI) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseInOut(float start, float end, float progressScalar) {
		return start * (2 * std::pow(progressScalar, 3) - 3 * std::pow(progressScalar, 2) + 1) + end * (3 * std::pow(progressScalar, 2) - 2 * std::pow(progressScalar, 3));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Clamp(float &value, float upperLimit, float lowerLimit) {
		// Straighten out the limits
		if (upperLimit < lowerLimit) {
			float temp = upperLimit;
			upperLimit = lowerLimit;
			lowerLimit = temp;
		}
		// Do the clamping
		if (value > upperLimit) {
			value = upperLimit;
			return true;
		} else if (value < lowerLimit) {
			value = lowerLimit;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Limit(float value, float upperLimit, float lowerLimit) {
		// Straighten out the limits
		if (upperLimit < lowerLimit) {
			float temp = upperLimit;
			upperLimit = lowerLimit;
			lowerLimit = temp;
		}

		// Do the clamping
		if (value > upperLimit) {
			return upperLimit;
		} else if (value < lowerLimit) {
			return lowerLimit;
		}
		return value;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(Vector &point, float left, float top, float right, float bottom) {
		return point.m_X >= left && point.m_X < right && point.m_Y >= top && point.m_Y < bottom;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(Vector &point, Vector &boxPos, float width, float height) {
		return point.m_X >= boxPos.m_X && point.m_X < (boxPos.m_X + width) && point.m_Y >= boxPos.m_Y && point.m_Y < (boxPos.m_Y + height);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float GetAllegroAngle(float angleDegrees) { return (angleDegrees / 360) * 256; }
}