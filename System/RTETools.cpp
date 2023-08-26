#include "RTETools.h"
#include "Vector.h"

#include "allegro.h"
#include "allegro/internal/aintern.h"

namespace RTE {

	RandomGenerator g_RandomGenerator;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SeedRNG() {
		// Use a constant seed for determinism.
		static constexpr uint32_t constSeed = []() {
			// VERY IMPORTANT, DO NOT CHANGE THIS!...
			// ...it's the name of my childhood pet ;)
			std::string_view seedString = "Bubble";

			// Biggest prime in a int64_t, because we want all bits to potentially be set (so let us overflow).
			const uint64_t hugePrime = 18446744073709551557;

			uint64_t seedResult = 0;
			for (char c : seedString) {
				seedResult += static_cast<uint64_t>(c) * hugePrime;
			}
			return static_cast<uint32_t>(seedResult);
		}();

		g_RandomGenerator.Seed(constSeed);
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

	float NormalizeAngleBetween0And2PI(float angle) {
		while (angle < 0) {
			angle += c_TwoPI;
		}
		return (angle > c_TwoPI) ? fmodf(angle + c_TwoPI, c_TwoPI) : angle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float NormalizeAngleBetweenNegativePIAndPI(float angle) {
		while (angle < 0) {
			angle += c_TwoPI;
		}
		return (angle > c_PI) ? fmodf(angle + c_PI, c_TwoPI) - c_PI : angle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AngleWithinRange(float angleToCheck, float startAngle, float endAngle) {
		angleToCheck = NormalizeAngleBetween0And2PI(angleToCheck);
		startAngle = NormalizeAngleBetween0And2PI(startAngle);
		endAngle = NormalizeAngleBetween0And2PI(endAngle);

		return endAngle >= startAngle ? (angleToCheck >= startAngle && angleToCheck <= endAngle) : (angleToCheck >= startAngle || angleToCheck <= endAngle);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float ClampAngle(float angleToClamp, float startAngle, float endAngle) {
		angleToClamp = NormalizeAngleBetween0And2PI(angleToClamp);

		if (!AngleWithinRange(angleToClamp, startAngle, endAngle)) {
			startAngle = NormalizeAngleBetween0And2PI(startAngle);
			endAngle = NormalizeAngleBetween0And2PI(endAngle);

			float shortestDistanceToStartAngle = std::min(c_TwoPI - std::abs(angleToClamp - startAngle), std::abs(angleToClamp - startAngle));
			float shortestDistanceToEndAngle = std::min(c_TwoPI - std::abs(angleToClamp - endAngle), std::abs(angleToClamp - endAngle));
			angleToClamp = shortestDistanceToStartAngle < shortestDistanceToEndAngle ? startAngle : endAngle;
		}

		return angleToClamp;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(const Vector &point, float left, float top, float right, float bottom) {
		return point.m_X >= left && point.m_X < right && point.m_Y >= top && point.m_Y < bottom;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(const Vector &point, const Vector &boxPos, float width, float height) {
		return point.m_X >= boxPos.m_X && point.m_X < (boxPos.m_X + width) && point.m_Y >= boxPos.m_Y && point.m_Y < (boxPos.m_Y + height);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string RoundFloatToPrecision(float input, int precision, int roundingMode) {
		if (roundingMode == 0) {
			std::stringstream floatStream;
			floatStream << std::fixed << std::setprecision(precision) << input;
			return floatStream.str();
		} else {
			float precisionMagnitude = std::pow(10.0F, static_cast<float>(precision));
			RTEAssert(precisionMagnitude < std::numeric_limits<float>::max(), "Precision set greater than able to display (exponent too high)!");
			RTEAssert(precisionMagnitude > 0, "Negative precision will yield divide by zero error!");
			RTEAssert(input < (std::numeric_limits<float>::max() / precisionMagnitude), "Value will exceed numeric limits with precision " + std::to_string(precision));

			float roundingBuffer = input * precisionMagnitude;

			switch (roundingMode) {
				case 1:
					roundingBuffer = std::floor(roundingBuffer);
					break;
				case 2:
					roundingBuffer = std::ceil(roundingBuffer);
					break;
				case 3:
					roundingBuffer = std::ceil(roundingBuffer);
					if (int remainder = static_cast<int>(roundingBuffer) % 10; remainder > 0) {
						roundingBuffer = roundingBuffer - static_cast<float>(remainder) + (remainder <= 5 ? 5.0F : 10.0F);
					}
					break;
				default:
					RTEAbort("Error in RoundFloatToPrecision: INVALID ROUNDING MODE");
					break;
			}
			return RoundFloatToPrecision((roundingBuffer / precisionMagnitude), precision);
		}
	}

	unsigned long TrueAlphaBlender(unsigned long x, unsigned long y, unsigned long n) {
		// This is the original allegro alpha blender with added alpha component blending.
		unsigned long res;
		unsigned long green;
		unsigned long alphaX = geta32(x);
		unsigned long alphaY = geta32(y);
		unsigned long alpha = 0;

		n = geta32(x);

		if (n)
			n++;

		res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
		y &= 0xFF00;
		x &= 0xFF00;
		green = (x - y) * n / 256 + y;

		res &= 0xFF00FF;
		green &= 0xFF00;

		alpha = alphaX + (255 - alphaX) / 255.0f * alphaY;
		alpha <<= 24;

		return res | green | alpha;
	}

	void SetTrueAlphaBlender() {
		set_blender_mode_ex(_blender_black, _blender_black, _blender_black, TrueAlphaBlender, _blender_black, _blender_black, _blender_black, 0, 0, 0, 0);
	}
}
