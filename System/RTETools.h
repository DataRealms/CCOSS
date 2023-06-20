#ifndef _RTETOOLS_
#define _RTETOOLS_

// Header file for global utility methods.

#include "RTEError.h"
#include "Constants.h"

namespace RTE {

	class Vector;

	extern std::mt19937 g_RNG; //!< The random number generator used for all random functions.

#pragma region Random Numbers
	/// <summary>
	/// Seed the mt19937 random number generator. mt19937 is the standard mersenne_twister_engine.
	/// </summary>
	void SeedRNG();

	/// <summary>
	/// Function template which returns a uniformly distributed random number in the range [-1, 1].
	/// </summary>
	/// <returns>Uniformly distributed random number in the range [-1, 1].</returns>
	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNormalNum() {
		return std::uniform_real_distribution<floatType>(floatType(-1.0), std::nextafter(floatType(1.0), std::numeric_limits<floatType>::max()))(g_RNG);
	}

	/// <summary>
	/// Function template specialization for int types which returns a uniformly distributed random number in the range [-1, 1].
	/// </summary>
	/// <returns>Uniformly distributed random number in the range [-1, 1].</returns>
	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNormalNum() {
		return std::uniform_int_distribution<intType>(intType(-1), intType(1))(g_RNG);
	}

	/// <summary>
	/// Function template which returns a uniformly distributed random number in the range [0, 1].
	/// </summary>
	/// <returns>Uniformly distributed random number in the range [0, 1].</returns>
	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum() {
		return std::uniform_real_distribution<floatType>(floatType(0.0), std::nextafter(floatType(1.0), std::numeric_limits<floatType>::max()))(g_RNG);
	}

	/// <summary>
	/// Function template specialization for int types which returns a uniformly distributed random number in the range [0, 1].
	/// </summary>
	/// <returns>Uniformly distributed random number in the range [0, 1].</returns>
	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum() {
		return std::uniform_int_distribution<intType>(intType(0), intType(1))(g_RNG);
	}

	/// <summary>
	/// Function template which returns a uniformly distributed random number in the range [min, max].
	/// </summary>
	/// <param name="min">Lower boundary of the range to pick a number from.</param>
	/// <param name="max">Upper boundary of the range to pick a number from.</param>
	/// <returns>Uniformly distributed random number in the range [min, max].</returns>
	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum(floatType min, floatType max) {
		if (max < min) { std::swap(min, max); }
		return (std::uniform_real_distribution<floatType>(floatType(0.0), std::nextafter(max - min, std::numeric_limits<floatType>::max()))(g_RNG) + min);
	}

	/// <summary>
	/// Function template specialization for int types which returns a uniformly distributed random number in the range [min, max].
	/// </summary>
	/// <param name="min">Lower boundary of the range to pick a number from.</param>
	/// <param name="max">Upper boundary of the range to pick a number from.</param>
	/// <returns>Uniformly distributed random number in the range [min, max].</returns>
	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum(intType min, intType max) {
		if (max < min) { std::swap(min, max); }
		return (std::uniform_int_distribution<intType>(intType(0), max - min)(g_RNG) + min);
	}
#pragma endregion

#pragma region Interpolation
	/// <summary>
	/// Simple Linear Interpolation, with an added bonus: scaleStart and scaleEnd let you define your scale, where 0 and 1 would be standard scale.
	/// This scale is used to normalize your progressScalar value and LERP accordingly.
	/// </summary>
	/// <param name="scaleStart">The start of the scale to LERP along.</param>
	/// <param name="scaleEnd">The end of the scale to LERP along.</param>
	/// <param name="startValue">The start value of your LERP.</param>
	/// <param name="endValue">The end value of your LERP.</param>
	/// <param name="progressScalar">How far your LERP has progressed. Automatically normalized through use of scaleStart and scaleEnd.</param>
	/// <returns>Interpolated value.</returns>
	float LERP(float scaleStart, float scaleEnd, float startValue, float endValue, float progressScalar);

	/// <summary>
	/// Nonlinear ease-in interpolation. Starts slow.
	/// </summary>
	/// <param name="start">Start value.</param>
	/// <param name="end">End value.</param>
	/// <param name="progressScalar">Normalized positive progress scalar (0 - 1.0).</param>
	/// <returns>Interpolated value.</returns>
	float EaseIn(float start, float end, float progressScalar);

	/// <summary>
	/// Nonlinear ease-out interpolation. Slows down toward the end.
	/// </summary>
	/// <param name="start">Start value.</param>
	/// <param name="end">End value.</param>
	/// <param name="progressScalar">Normalized positive progress scalar (0 - 1.0).</param>
	/// <returns>Interpolated value.</returns>
	float EaseOut(float start, float end, float progressScalar);

	/// <summary>
	/// Nonlinear ease-in-out interpolation. Slows down in the start and end.
	/// </summary>
	/// <param name="start">Start value.</param>
	/// <param name="end">End value.</param>
	/// <param name="progressScalar">Normalized positive progress scalar (0 - 1.0).</param>
	/// <returns>Interpolated value.</returns>
	float EaseInOut(float start, float end, float progressScalar);
#pragma endregion

#pragma region Clamping
	/// <summary>
	/// Clamps a value between two limit values.
	/// </summary>
	/// <param name="value">Value to clamp.</param>
	/// <param name="upperLimit">Upper limit of value.</param>
	/// <param name="lowerLimit">Lower limit of value.</param>
	/// <returns>True if either limit is currently reached, False if not.</returns>
	bool Clamp(float &value, float upperLimit, float lowerLimit);

	/// <summary>
	/// Clamps a value between two limit values.
	/// </summary>
	/// <param name="value">Value to clamp.</param>
	/// <param name="upperLimit">Upper limit of value.</param>
	/// <param name="lowerLimit">Lower limit of value.</param>
	/// <returns>Upper/Lower limit value if limit is currently reached, value between limits if not.</returns>
	float Limit(float value, float upperLimit, float lowerLimit);
#pragma endregion

#pragma region Rounding
	/// <summary>
	/// Rounds a float to a set fixed point precision (digits after decimal point) with option to always ceil or always floor the remainder.
	/// </summary>
	/// <param name="inputFloat">The input float to round.</param>
	/// <param name="precision">The precision to round to, i.e. the number of digits after the decimal points.</param>
	/// <param name="roundingMode">Method of rounding to use. 0 for system default, 1 for floored remainder, 2 for ceiled remainder.</param>
	/// <returns>A string of the float, rounded and displayed to chosen precision.</returns>
	std::string RoundFloatToPrecision(float input, int precision, int roundingMode = 0);

	/// <summary>
	/// Rounds an integer to the specified nearest multiple.
	/// For example, if the arguments are 63 and 5, the returned value will be 65.
	/// </summary>
	/// <param name="num">The number to round to the nearest multiple.</param>
	/// <param name="multiple">The multiple to round to.</param>
	/// <returns>An integer rounded to the specified nearest multiple.</returns>
	inline int RoundToNearestMultiple(int num, int multiple) { return static_cast<int>(std::round(static_cast<float>(num) / static_cast<float>(multiple)) * static_cast<float>(multiple)); }
#pragma endregion

#pragma region Angle Helpers
	/// <summary>
	/// Returns a copy of the angle normalized so it's between 0 and 2PI.
	/// </summary>
	/// <param name="angle">The angle to normalize, in radians.</param>
	/// <returns>The angle, normalized so it's between 0 and 2PI</returns>
	float NormalizeAngleBetween0And2PI(float angle);

	/// <summary>
	/// Returns a copy of the angle normalized so it's between -PI and PI.
	/// </summary>
	/// <param name="angle">The angle to normalize, in radians.</param>
	/// <returns>The angle, normalized so it's between -PI and PI</returns>
	float NormalizeAngleBetweenNegativePIAndPI(float angle);

	/// <summary>
	/// Returns whether or not the angle to check is between the start and end angles. Note that, because of how angles work (when normalized), the start angle may be greater than the end angle.
	/// </summary>
	/// <param name="angleToCheck">The angle to check, in radians.</param>
	/// <param name="startAngle">The starting angle for the range.</param>
	/// <param name="endAngle">The ending angle for the range.</param>
	/// <returns>Whether or not the angle to check is between the start and end angle.</returns>
	bool AngleWithinRange(float angleToCheck, float startAngle, float endAngle);

	/// <summary>
	/// Clamps the passed in angle between the specified lower and upper limits, in a CCW direction.
	/// </summary>
	/// <param name="angleToClamp">The angle to clamp.</param>
	/// <param name="startAngle">The lower limit for clamping.</param>
	/// <param name="endAngle">The upper limit for clamping.</param>
	/// <returns>The angle, clamped between the start and end angle.</returns>
	float ClampAngle(float angleToClamp, float startAngle, float endAngle);
#pragma endregion

#pragma region Detection
	/// <summary>
	/// Tells whether a point is within a specified box.
	/// </summary>
	/// <param name="point">Vector position of the point we're checking.</param>
	/// <param name="boxPos">Vector position of the box.</param>
	/// <param name="width">Width of the box.</param>
	/// <param name="height">Height of the box.</param>
	/// <returns>True if point is inside box bounds.</returns>
	bool WithinBox(Vector &point, Vector &boxPos, float width, float height);

	/// <summary>
	/// Tells whether a point is within a specified box.
	/// </summary>
	/// <param name="point">Vector position of the point we're checking.</param>
	/// <param name="left">Position of box left plane (X start).</param>
	/// <param name="top">Position of box top plane (Y start).</param>
	/// <param name="right">Position of box right plane (X end).</param>
	/// <param name="bottom">Position of box bottom plane (Y end).</param>
	/// <returns>True if point is inside box bounds.</returns>
	bool WithinBox(Vector &point, float left, float top, float right, float bottom);
#pragma endregion

#pragma region Conversion
	/// <summary>
	/// Returns a corrected angle value that can be used with Allegro fixed point math routines where 256 equals 360 degrees.
	/// </summary>
	/// <param name="angleDegrees">The angle value to correct. In degrees.</param>
	/// <returns>A float with the represented angle as full rotations being 256.</returns>
	inline float GetAllegroAngle(float angleDegrees) { return (angleDegrees / 360) * 256; }

	/// <summary>
	/// Returns the given angle converted from degrees to radians.
	/// </summary>
	/// <param name="angleDegrees">The angle in degrees to be converted.</param>
	/// <returns>The converted angle in radians.</returns>
	inline float DegreesToRadians(float angleDegrees) { return angleDegrees / 180.0F * c_PI; }

	/// <summary>
	/// Returns the given angle converted from radians to degrees.
	/// </summary>
	/// <param name="angleRadians">The angle in radians to be converted.</param>
	/// <returns>The converted angle in degrees.</returns>
	inline float RadiansToDegrees(float angleRadians) { return angleRadians / c_PI * 180.0F; }
#pragma endregion

#pragma region Comparison
	/// <summary>
	/// Checks whether two strings are equal when the casing is disregarded.
	/// </summary>
	/// <param name="strA">First string.</param>
	/// <param name="strB">Second string.</param>
	/// <returns>Whether the two strings are equal case insensitively.</returns>
	inline bool StringsEqualCaseInsensitive(const std::string_view &strA, const std::string_view &strB) { return std::equal(strA.begin(), strA.end(), strB.begin(), strB.end(), [](char strAChar, char strBChar) { return std::tolower(strAChar) == std::tolower(strBChar); }); }
#pragma endregion

#pragma region Misc
	/// <summary>
	/// Convenience method that takes in a double pointer array and returns a std::vector with its contents, because pointers-to-pointers are the devil. The passed in array is deleted in the process so no need to delete it manually.
	/// </summary>
	/// <param name="arrayOfType">The double pointer to convert to a std::vector.</param>
	/// <param name="arraySize">The size of the double pointer array.</param>
	template <typename Type> std::vector<Type *> ConvertDoublePointerToVectorOfPointers(Type **arrayOfType, size_t arraySize) {
		std::unique_ptr<Type *[]> doublePointerArray = std::unique_ptr<Type *[]>(arrayOfType);
		std::vector<Type *> outputVector;
		for (size_t i = 0; i < arraySize; ++i) {
			outputVector.emplace_back(doublePointerArray[i]);
		}
		return outputVector;
	}

	/// <summary>
	/// Returns the sign of the given input value.
	/// </summary>
	/// <returns>The sign as an integer -1, 0 or +1.</returns>
	template <typename Type> int Sign(const Type &value) {
		return (Type(0) < value) - (Type(0) > value);
	}
#pragma endregion
}
#endif