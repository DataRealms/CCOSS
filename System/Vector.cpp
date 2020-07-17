#include "Vector.h"

#pragma intrinsic (sin, cos, atan2)
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
		Vector temp(*this);
		SetXY(newMag, 0);
		AbsRotateTo(temp);
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
		float radAngle = -std::atan2f(m_Y, m_X);
		return (radAngle < -c_HalfPI) ? (radAngle + c_TwoPI) : radAngle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::RadRotate(float angle) {
		angle = -angle;
		float tempX = m_X * std::cosf(angle) - m_Y * std::sinf(angle);
		float tempY = m_X * std::sinf(angle) + m_Y * std::cosf(angle);
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
			*this /= rhs.size();
		}
		return *this;
	}
}