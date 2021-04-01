#include "Material.h"
#include "Constants.h"

namespace RTE {

	ConcreteClassInfo(Material, Entity, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Material::Clear() {
		m_Index = 0;
		m_Priority = 0;
		m_Integrity = 0.0F;
		m_Restitution = 0.0F;
		m_Friction = 0.0F;
		m_Stickiness = 0.0F;	
		m_VolumeDensity = 0.0F;
		m_PixelDensity = 0.0F;
		m_GibImpulseLimitPerLiter = 0.0F;
		m_GibWoundLimitPerLiter = 0.0F;	
		m_SettleMaterialIndex = 0;
		m_SpawnMaterialIndex = 0;
		m_IsScrap = false;
		m_Color.Reset();
		m_UseOwnColor = false;
		m_TextureFile.Reset();
		m_TerrainTexture = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Material::Create(const Material &reference) {
		Entity::Create(reference);

		m_Index = reference.m_Index;
		m_Priority = reference.m_Priority;
		m_Integrity = reference.m_Integrity;
		m_Restitution = reference.m_Restitution;
		m_Friction = reference.m_Friction;
		m_Stickiness = reference.m_Stickiness;
		m_VolumeDensity = reference.m_VolumeDensity;
		m_PixelDensity = reference.m_PixelDensity;
		m_GibImpulseLimitPerLiter = reference.m_GibImpulseLimitPerLiter;
		m_GibWoundLimitPerLiter = reference.m_GibWoundLimitPerLiter;
		m_SettleMaterialIndex = reference.m_SettleMaterialIndex;
		m_SpawnMaterialIndex = reference.m_SpawnMaterialIndex;
		m_IsScrap = reference.m_IsScrap;
		m_Color = reference.m_Color;
		m_UseOwnColor = reference.m_UseOwnColor;
		m_TextureFile = reference.m_TextureFile;
		m_TerrainTexture = reference.m_TerrainTexture;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Material::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Index") {
			// TODO: Check for index collisions here
			reader >> m_Index;	
		} else if (propName == "Priority") {
			reader >> m_Priority;
		} else if (propName == "Integrity" || propName == "StructuralIntegrity") {
			reader >> m_Integrity;
		} else if (propName == "Restitution" || propName == "Bounce") {
			reader >> m_Restitution;
		} else if (propName == "Friction") {
			reader >> m_Friction;
		} else if (propName == "Stickiness") {
			reader >> m_Stickiness;
		} else if (propName == "DensityKGPerVolumeL") {
			reader >> m_VolumeDensity;
			// Overrides the pixel density
			m_PixelDensity = m_VolumeDensity * c_LPP;
		} else if (propName == "DensityKGPerPixel") {
			reader >> m_PixelDensity;
			// Overrides the volume density
			m_VolumeDensity = m_PixelDensity * c_PPL;
		} else if (propName == "GibImpulseLimitPerVolumeL") {
			reader >> m_GibImpulseLimitPerLiter;
		} else if (propName == "GibWoundLimitPerVolumeL") {
			reader >> m_GibWoundLimitPerLiter;
		} else if (propName == "SettleMaterial") {
			reader >> m_SettleMaterialIndex;
		} else if (propName == "SpawnMaterial" || propName == "TransformsInto") {
			reader >> m_SpawnMaterialIndex;
		} else if (propName == "IsScrap") {
			reader >> m_IsScrap;
		} else if (propName == "Color") {
			reader >> m_Color;
		} else if (propName == "UseOwnColor") {
			reader >> m_UseOwnColor;
		} else if (propName == "TextureFile") {
			reader >> m_TextureFile;
			m_TerrainTexture = m_TextureFile.GetAsBitmap();
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Material::Save(Writer &writer) const {
		Entity::Save(writer);
		// Materials should never be altered, so no point in saving additional properties when it's a copy
		if (m_IsOriginalPreset) {
			writer.NewProperty("Priority");
			writer << m_Priority;
			writer.NewProperty("StructuralIntegrity");
			writer << m_Integrity;
			writer.NewProperty("Restitution");
			writer << m_Restitution;
			writer.NewProperty("Friction");
			writer << m_Friction;
			writer.NewProperty("Stickiness");
			writer << m_Stickiness;
			writer.NewProperty("DensityKGPerVolumeL");
			writer << m_VolumeDensity;
			writer.NewProperty("GibImpulseLimitPerVolumeL");
			writer << m_GibImpulseLimitPerLiter;
			writer.NewProperty("GibWoundLimitPerVolumeL");
			writer << m_GibWoundLimitPerLiter;
			writer.NewProperty("SettleMaterial");
			writer << m_SettleMaterialIndex;
			writer.NewProperty("SpawnMaterial");
			writer << m_SpawnMaterialIndex;
			writer.NewProperty("IsScrap");
			writer << m_IsScrap;
			writer.NewProperty("Color");
			writer << m_Color;
			writer.NewProperty("UseOwnColor");
			writer << m_UseOwnColor;
			writer.NewProperty("TextureFile");
			writer << m_TextureFile;
		}
		return 0;
	}
}