#include "Vector.h"

#pragma float_control(precise, on)

namespace RTE {

	const std::string Vector::c_ClassName = "Vector";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Vector::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "X") {
			reader >> m_X;
		} else if (propName == "Y") {
			reader >> m_Y;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Vector::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("X", m_X);
		writer.NewPropertyWithValue("Y", m_Y);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::SetMagnitude(const float newMag) {
		if (IsZero()) {
			SetXY(newMag, 0.0F);
		} else {
			*this *= (newMag / GetMagnitude());
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::CapMagnitude(const float capMag) {
		if (capMag == 0) { Reset(); }
		if (GetMagnitude() > capMag) { SetMagnitude(capMag); }
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::ClampMagnitude(float upperLimit, float lowerLimit) {
		if (upperLimit < lowerLimit) { std::swap(upperLimit, lowerLimit); }
		if (upperLimit == 0 && lowerLimit == 0) {
			Reset();
		} else if (GetMagnitude() > upperLimit) {
			SetMagnitude(upperLimit);
		} else if (GetMagnitude() < lowerLimit) {
			SetMagnitude(lowerLimit);
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Vector::GetAbsRadAngle() const {
		const float radAngle = -std::atan2(m_Y, m_X);
		return (radAngle < -c_HalfPI) ? (radAngle + c_TwoPI) : radAngle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector Vector::GetRadRotated(const float angle) {
		Vector returnVector = *this;
		const float adjustedAngle = -angle;
		returnVector.m_X = m_X * std::cos(adjustedAngle) - m_Y * std::sin(adjustedAngle);
		returnVector.m_Y = m_X * std::sin(adjustedAngle) + m_Y * std::cos(adjustedAngle);

		return returnVector;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector &Vector::operator=(const Vector &rhs) {
		if (*this != rhs) {
			m_X = rhs.m_X;
			m_Y = rhs.m_Y;
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::operator=(const std::deque<Vector> &rhs) {
		Reset();
		if (!rhs.empty()) {
			for (const Vector &vector : rhs) {
				*this += vector;
			}
			*this /= static_cast<float>(rhs.size());
		}
		return *this;
	}
}
