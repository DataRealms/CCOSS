#include "Vector.h"

#pragma float_control(precise, on)

namespace RTE {

	const std::string Vector::c_ClassName = "Vector";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Vector::ReadProperty(std::string propName, Reader &reader) {
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

		writer.NewProperty("X");
		writer << m_X;
		writer.NewProperty("Y");
		writer << m_Y;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::SetMagnitude(float newMag) {
		if (IsZero()) {
			SetXY(newMag, 0.0F);
		} else {
			*this *= (newMag / GetMagnitude());
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::CapMagnitude(float capMag) {
		if (capMag == 0) { Reset(); }
		if (GetMagnitude() > capMag) { SetMagnitude(capMag); }
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Vector::GetAbsRadAngle() const {
		const float radAngle = -std::atan2(m_Y, m_X);
		return (radAngle < -c_HalfPI) ? (radAngle + c_TwoPI) : radAngle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::RadRotate(float angle) {
		angle = -angle;
		const float tempX = m_X * std::cos(angle) - m_Y * std::sin(angle);
		const float tempY = m_X * std::sin(angle) + m_Y * std::cos(angle);
		m_X = tempX;
		m_Y = tempY;

		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::operator=(const Vector &rhs) {
		if (*this != rhs) {
			m_X = rhs.m_X;
			m_Y = rhs.m_Y;
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::operator=(const std::deque<Vector> &rhs) {
		Clear();
		if (!rhs.empty()) {
			for (const Vector &vector : rhs) {
				*this += vector;
			}
			*this /= static_cast<float>(rhs.size());
		}
		return *this;
	}
}