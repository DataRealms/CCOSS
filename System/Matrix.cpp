#include "Matrix.h"

namespace RTE {

	const std::string Matrix::c_ClassName = "Matrix";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Matrix::Clear() {
		m_Rotation = 0;
		m_Flipped[X] = false;
		m_Flipped[Y] = false;
		m_ElementsUpdated = false;
		m_Elements[0][0] = 1.0;
		m_Elements[0][1] = 0.0;
		m_Elements[1][0] = 0.0;
		m_Elements[1][1] = 1.0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Matrix::Create() {
		// Read all the properties
		if (Serializable::Create() < 0) {
			return -1;
		}
		m_ElementsUpdated = false;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Matrix::Create(float angle) {
		m_Rotation = angle;
		m_ElementsUpdated = true;

		// Inverse angle to make CCW positive direction.
		const float CosAngle = std::cos(-angle);
		const float SinAngle = std::sin(-angle);
		m_Elements[0][0] = CosAngle;
		m_Elements[0][1] = -SinAngle;
		m_Elements[1][0] = SinAngle;
		m_Elements[1][1] = CosAngle;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Matrix::Create(const Matrix &reference) {
		m_Rotation = reference.m_Rotation;
		m_Flipped[X] = reference.m_Flipped[X];
		m_Flipped[Y] = reference.m_Flipped[Y];
		m_ElementsUpdated = false;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Matrix::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "AngleDegrees") {
			SetDegAngle(std::stof(reader.ReadPropValue()));
		} else if (propName == "AngleRadians") {
			reader >> m_Rotation;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Matrix::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("AngleDegrees", GetDegAngle());

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Matrix::GetRadAngleTo(float otherAngle) const {
		// Rotate this' angle with the other angle so that the sought after difference angle is between the resulting angle and the x-axis
		float difference = otherAngle - GetRadAngle();

		// "Normalize" difference to range [-PI,PI)
		while (difference < -c_PI) { difference += c_TwoPI; }
		while (difference >= c_PI) { difference -= c_TwoPI; }

		// difference has the signed answer
		return difference;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Matrix::GetDegAngleTo(float otherAngle) const {
		// Rotate this' angle with the other angle so that the sought after difference angle is between the resulting angle and the x-axis
		float difference = otherAngle - GetDegAngle();

		// "Normalize" difference to range [-180,180)
		while (difference < -180) { difference += 360; }
		while (difference >= 180) { difference -= 360; }

		// difference has the signed answer
		return difference;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Matrix & Matrix::operator=(const Matrix &rhs) {
		if (*this == rhs) {
			return *this;
		}
		m_Rotation = rhs.m_Rotation;
		m_Flipped[X] = rhs.m_Flipped[X];
		m_Flipped[Y] = rhs.m_Flipped[Y];
		m_ElementsUpdated = false;

		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector Matrix::operator*(const Vector &rhs) {
		if (!m_ElementsUpdated) { UpdateElements(); }

		Vector retVec = rhs;
		// Apply flipping as set.
		retVec.m_X = m_Flipped[X] ? -retVec.m_X : retVec.m_X;
		retVec.m_Y = m_Flipped[Y] ? -retVec.m_Y : retVec.m_Y;

		// Do the matrix multiplication.
		retVec.SetXY(m_Elements[0][0] * retVec.m_X + m_Elements[0][1] * retVec.m_Y, m_Elements[1][0] * retVec.m_X + m_Elements[1][1] * retVec.m_Y);

		return retVec;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector Matrix::operator/(const Vector &rhs) {
		if (!m_ElementsUpdated) { UpdateElements(); }

		Vector retVec = rhs;
		// Apply flipping as set.
		retVec.m_X = m_Flipped[X] ? -retVec.m_X : retVec.m_X;
		retVec.m_Y = m_Flipped[Y] ? -retVec.m_Y : retVec.m_Y;

		// Do the matrix multiplication.
		retVec.SetXY(m_Elements[0][0] * retVec.m_X + m_Elements[1][0] * retVec.m_Y, m_Elements[0][1] * retVec.m_X + m_Elements[1][1] * retVec.m_Y);

		return retVec;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Matrix Matrix::operator-() {
		m_Rotation = -m_Rotation;

		if (m_ElementsUpdated) {
			// Swap two of the elements to reverse the rotation direction
			float temp = m_Elements[0][1];
			m_Elements[0][1] = m_Elements[1][0];
			m_Elements[1][0] = temp;
		}
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Matrix::UpdateElements() {
		// Negative angle to Account for upside-down coordinate system.
		const float CosAngle = std::cos(-m_Rotation);
		const float SinAngle = std::sin(-m_Rotation);
		m_Elements[0][0] = CosAngle;
		m_Elements[0][1] = -SinAngle;
		m_Elements[1][0] = SinAngle;
		m_Elements[1][1] = CosAngle;

		m_ElementsUpdated = true;
	}
}