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
		return (end - start) * (std::sinf(-t * c_HalfPI) + 1) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseOut(float start, float end, float progressScalar) {
		if (progressScalar <= 0) {
			return start;
		} else if (progressScalar >= 1.0F) {
			return end;
		}
		return (end - start) * -std::sinf(-progressScalar * c_HalfPI) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseInOut(float start, float end, float progressScalar) {
		return start * (2 * std::powf(progressScalar, 3) - 3 * std::powf(progressScalar, 2) + 1) + end * (3 * std::powf(progressScalar, 2) - 2 * std::powf(progressScalar, 3));
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void OpenBrowserToURL(std::string goToURL) {
		system(std::string("start ").append(goToURL).c_str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ASCIIFileContainsString(std::string filePath, std::string findString) {
		// Open the script file so we can check it out
		std::ifstream *file = new std::ifstream(filePath.c_str());
		if (!file->good()) {
			return false;
		}
		char rawLine[1024];
		std::string line;
		std::string::size_type pos = 0;
		std::string::size_type endPos = 0;
		std::string::size_type commentPos = std::string::npos;
		bool blockCommented = false;

		while (!file->eof()) {
			// Go through the script file, line by line
			file->getline(rawLine, 1024);
			line = rawLine;
			pos = endPos = 0;

			// Check for block comments
			if ((commentPos = line.find("/*", 0) != std::string::npos) && !blockCommented) { blockCommented = true; }

			// Find the end of the block comment
			if (((commentPos = line.find("*/", commentPos == std::string::npos ? 0 : commentPos)) != std::string::npos) && blockCommented) {
				blockCommented = false;
				pos = commentPos;
			}
			// Process the line as usual
			if (!blockCommented) {
				// See if this line is commented out anywhere
				commentPos = line.find("//", 0);
				// Find the string
				do {
					pos = line.find(findString.c_str(), pos);
					if (pos != std::string::npos && pos < commentPos) {
						// Found it!
						delete file;
						file = 0;
						return true;
					}
				} while (pos != std::string::npos && pos < commentPos);
			}
		}
		// Didn't find the search string
		delete file;
		file = 0;
		return false;
	}
}