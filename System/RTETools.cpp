#include "RTETools.h"
#include "Vector.h"

namespace RTE {

	std::mt19937 g_RNG;

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

	std::string RoundFloatToPrecision(float input, int precision, int roundingMode) {
		if (roundingMode == 0) {
			std::stringstream floatStream;
			floatStream << std::fixed << std::setprecision(precision) << input;
			return floatStream.str();
		} else {
			RTEAssert(powf(10.0F, static_cast<float>(precision)) < std::numeric_limits<float>::max(), "PRECISION SET GREATER THAN EXPONENT MAX!");
			RTEAssert(powf(10.0F, static_cast<float>(precision)) > 0, "NEGATIVE PRECISION WILL YEILD DIVIDE BY ZERO ERROR!");
			RTEAssert(input < (std::numeric_limits<float>::max() / powf(10.0F, static_cast<float>(precision))), "VALUE WILL EXCEED NUMERIC LIMITS WITH PRECISION " + std::to_string(precision));
			if (roundingMode == 1) {
				float roundingBuffer = floor(input * powf(10.0F, static_cast<float>(precision))) / powf(10.0F, static_cast<float>(precision));
				return RoundFloatToPrecision(roundingBuffer, precision);
			} else if (roundingMode == 2) {
				float roundingBuffer = ceil(input * powf(10.0F, static_cast<float>(precision))) / powf(10.0F, static_cast<float>(precision));
				return RoundFloatToPrecision(roundingBuffer, precision);
			} else {
				RTEAssert(false, "Error in RoundFloatToPrecision: INVALID ROUNDING MODE");
			}
		}
	}
	
}
