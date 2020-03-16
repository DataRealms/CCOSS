#include "Vector.h"

#pragma intrinsic (sin, cos)
#pragma float_control(precise, on)

namespace RTE {

	const std::string Vector::m_ClassName = "Vector";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Vector::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "X") {
			reader >> m_X;
		} else if (propName == "Y") {
			reader >> m_Y;
		} else {
			// See if the base class(es) can find a match instead
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
		if (m_X == 0) {
			return m_Y > 0 ? -c_HalfPI : (m_Y < 0 ? c_HalfPI : 0);
		}
		if (m_Y == 0) {
			return m_X > 0 ? 0 : (m_X < 0 ? c_PI : 0);
		}
		// TODO: Confirm that this is correct!")
		float rawAngle = -atan(m_Y / m_X);
		if (m_X < 0) { rawAngle += c_PI; }

		return rawAngle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Vector::GetAbsDegAngle() const {
		if (m_X == 0) {
			return m_Y > 0 ? -90 : (m_Y < 0 ? 90 : 0);
		}
		if (m_Y == 0) {
			return m_X > 0 ? 0 : (m_X < 0 ? 180 : 0);
		}

		float rawAngle = -(atan(m_Y / m_X) / c_PI) * 180;
		if (m_X < 0) { rawAngle += 180; }

		return rawAngle;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::RadRotate(float angle) {
		angle = -angle;
		float tempX = m_X * cos(angle) - m_Y * sin(angle);
		float tempY = m_X * sin(angle) + m_Y * cos(angle);
		m_X = tempX;
		m_Y = tempY;

		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::DegRotate(float angle) {
		angle = -angle;

		// Convert to radians.
		angle /= 180;
		angle *= c_PI;

		float tempX = m_X * cos(angle) - m_Y * sin(angle);
		float tempY = m_X * sin(angle) + m_Y * cos(angle);
		m_X = tempX;
		m_Y = tempY;

		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::AbsRotateTo(const Vector &refVector) {
		float rawAngle;
		if (refVector.m_X == 0) {
			rawAngle = refVector.m_Y > 0 ? -c_HalfPI : (refVector.m_Y < 0 ? c_HalfPI : 0);
		} else if (refVector.m_Y == 0) {
			rawAngle = refVector.m_X > 0 ? 0 : (refVector.m_X < 0 ? c_PI : 0);
		} else {
			rawAngle = -atan(refVector.m_Y / refVector.m_X);
			if (refVector.m_X < 0) { rawAngle += c_PI; }
		}
		rawAngle = -rawAngle;

		m_X = GetMagnitude();
		m_Y = 0.0;

		float tempX = m_X * cos(rawAngle) - m_Y * sin(rawAngle);
		float tempY = m_X * sin(rawAngle) + m_Y * cos(rawAngle);
		m_X = tempX;
		m_Y = tempY;

		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::operator=(const Vector &rhs) {
		if (*this == rhs) {
			return *this;
		}
		m_X = rhs.m_X;
		m_Y = rhs.m_Y;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector & Vector::operator=(const deque<Vector> &rhs) {
		Clear();
		if (rhs.empty()) { return *this; }
		for (deque<Vector>::const_iterator itr = rhs.begin(); itr != rhs.end(); ++itr) {
			*this += *itr;
		}
		*this /= rhs.size();
		return *this;
	}
}