//////////////////////////////////////////////////////////////////////////////////////////
// File:            Material.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Material class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Material.h"
#include "DDTTools.h"
#include "FrameMan.h"

namespace RTE {

CONCRETECLASSINFO(Material, Entity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Material, effectively
//                  resetting the members of this abstraction level only.

void Material::Clear()
{
    id = 0;
    restitution = 0;
    friction = 0;
    stickiness = 0;
    strength = 0;
    volumeDensity = 0;
    pixelDensity = 0;
    gibImpulseLimitPerLitre = 0;
    gibWoundLimitPerLitre = 0;
    priority = 0;
    settleMaterial = 0;
    spawnMaterial = 0;
    isScrap = false;
    color.Reset();
    m_UseOwnColor = false;
    m_TextureFile.Reset();
    m_pTexture = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Material object ready for use.

int Material::Create()
{
    // Read all the properties
    if (Entity::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Arm to be identical to another, by deep copy.

int Material::Create(const Material &reference)
{
    Entity::Create(reference);

    id = reference.id;
    restitution = reference.restitution;
    friction = reference.friction;
    stickiness = reference.stickiness;
    strength = reference.strength;
    volumeDensity = reference.volumeDensity;
    pixelDensity = reference.pixelDensity;
    gibImpulseLimitPerLitre = reference.gibImpulseLimitPerLitre;
    gibWoundLimitPerLitre = reference.gibWoundLimitPerLitre;
    priority = reference.priority;
    settleMaterial = reference.settleMaterial;
    spawnMaterial = reference.spawnMaterial;
    isScrap = reference.isScrap;
    color = reference.color;
    m_UseOwnColor = reference.m_UseOwnColor;
    m_TextureFile = reference.m_TextureFile;
    m_pTexture = reference.m_pTexture;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Material::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Index")
    {
        reader >> id;
// TODO: Check for collisions here
    }
    else if (propName == "Restitution" || propName == "Bounce")
        reader >> restitution;
    else if (propName == "Friction")
        reader >> friction;
    else if (propName == "Stickiness")
        reader >> stickiness;
    else if (propName == "Strength" || propName == "StructuralIntegrity")
        reader >> strength;
    else if (propName == "DensityKGPerVolumeL")
    {
        reader >> volumeDensity;
        // Overrides the pixeldensity
        pixelDensity = volumeDensity * g_FrameMan.GetLPP();
    }
    else if (propName == "DensityKGPerPixel")
    {
        reader >> pixelDensity;
        // Overrides the volumedensity
        volumeDensity = pixelDensity * g_FrameMan.GetPPL();
    }
    else if (propName == "GibImpulseLimitPerVolumeL")
        reader >> gibImpulseLimitPerLitre;
    else if (propName == "GibWoundLimitPerVolumeL")
        reader >> gibWoundLimitPerLitre;
    else if (propName == "Priority")
        reader >> priority;
    else if (propName == "SettleMaterial")
        reader >> settleMaterial;
    else if (propName == "SpawnMaterial" || propName == "TransformsInto")
        reader >> spawnMaterial;
    else if (propName == "IsScrap")
        reader >> isScrap;
    else if (propName == "Color")
        reader >> color;
    else if (propName == "UseOwnColor")
        reader >> m_UseOwnColor;
    else if (propName == "TextureFile")
    {
        reader >> m_TextureFile;
        m_pTexture = m_TextureFile.GetAsBitmap();
    }
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Material with a Writer for
//                  later recreation with Create(Reader &reader);

int Material::Save(Writer &writer) const
{
    Entity::Save(writer);

    // Materials should never be altered, so no point in saving additional properties when it's a copy
    if (m_IsOriginalPreset)
    {
        writer.NewProperty("Restitution");
        writer << restitution;
        writer.NewProperty("Friction");
        writer << friction;
        writer.NewProperty("Stickiness");
        writer << stickiness;
        writer.NewProperty("StructuralIntegrity");
        writer << strength;
        writer.NewProperty("DensityKGPerVolumeL");
        writer << volumeDensity;
        writer.NewProperty("GibImpulseLimitPerVolumeL");
        writer << gibImpulseLimitPerLitre;
        writer.NewProperty("GibWoundLimitPerVolumeL");
        writer << gibWoundLimitPerLitre;
        writer.NewProperty("Priority");
        writer << priority;
        writer.NewProperty("SettleMaterial");
        writer << settleMaterial;
        writer.NewProperty("SpawnMaterial");
        writer << spawnMaterial;
        writer.NewProperty("IsScrap");
        writer << isScrap;
        writer.NewProperty("Color");
        writer << color;
        writer.NewProperty("UseOwnColor");
        writer << m_UseOwnColor;
        writer.NewProperty("TextureFile");
        writer << m_TextureFile;
    }

    return 0;
}

} // namespace RTE