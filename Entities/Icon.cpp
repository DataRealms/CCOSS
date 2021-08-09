#include "Icon.h"

namespace RTE {

	ConcreteClassInfo(Icon, Entity, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Icon::Clear() {
		m_BitmapFile.Reset();
		m_FrameCount = 0;
		m_BitmapsIndexed.clear();
		m_BitmapsTrueColor.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Icon::Create() {
		if (m_BitmapsIndexed.empty() || m_BitmapsTrueColor.empty()) {
			if (m_BitmapFile.GetDataPath().empty()) { m_BitmapFile.SetDataPath("Base.rte/GUIs/DefaultIcon.png"); }

			m_BitmapFile.GetAsAnimation(m_BitmapsIndexed, m_FrameCount, COLORCONV_REDUCE_TO_256);
			m_BitmapFile.GetAsAnimation(m_BitmapsTrueColor, m_FrameCount, COLORCONV_8_TO_32);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Icon::Create(const Icon &reference) {
		Entity::Create(reference);

		m_BitmapFile = reference.m_BitmapFile;
		m_FrameCount = reference.m_FrameCount;
		m_BitmapsIndexed = reference.m_BitmapsIndexed;
		m_BitmapsTrueColor = reference.m_BitmapsTrueColor;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Icon::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "BitmapFile") {
			reader >> m_BitmapFile;
		} else if (propName == "FrameCount") {
			reader >> m_FrameCount;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Icon::Save(Writer &writer) const {
		Entity::Save(writer);
		writer.NewProperty("BitmapFile");
		writer << m_BitmapFile;
		writer.NewProperty("FrameCount");
		writer << m_FrameCount;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Icon::Destroy(bool notInherited) {
		if (!notInherited) { Entity::Destroy(); }
		Clear();
	}
}