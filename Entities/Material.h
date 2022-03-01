#ifndef _RTEMATERIAL_
#define _RTEMATERIAL_

#include "Entity.h"
#include "ContentFile.h"
#include "Color.h"

namespace RTE {

	/// <summary>
	/// Represents a material and holds all the relevant data.
	/// </summary>
	class Material : public Entity {

	public:

		EntityAllocation(Material);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Material object in system memory. Create() should be called before using the object.
		/// </summary>
		Material() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a Material object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Material object which is passed in by reference.</param>
		Material(const Material &reference) { if (this != &reference) { Clear(); Create(reference); } }

		/// <summary>
		/// Creates a Material to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Material to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Material &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire Material, including its inherited members, to it's default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the texture bitmap of this Material, if any is associated with it.
		/// </summary>
		/// <returns>Pointer to the texture bitmap of this Material.</returns>
		BITMAP * GetTexture() const { return m_TerrainTexture; }

		/// <summary>
		/// Gets the index of this Material in the material palette.
		/// </summary>
		/// <returns>The index of this Material in the material palette. 0 - 255.</returns>
		unsigned char GetIndex() const { return m_Index; }

		/// <summary>
		/// Sets the index of this Material in the material palette to the next specified value.
		/// </summary>
		/// <param name="newIndex">The new index of this Material in the material palette. 0 - 255.</param>
		void SetIndex(unsigned char newIndex) { m_Index = newIndex; }

		/// <summary>
		/// Gets the drawing priority of this Material. The higher the number, the higher chances that a pixel of this material will be drawn on top of others.
		/// </summary>
		/// <returns>The drawing priority of this Material.</returns>
		int GetPriority() const { return m_Priority; }

		/// <summary>
		/// Gets the amount of times a dislodged pixel of this Material will attempt to relocate to an open position.
		/// </summary>
		/// <returns>The amount of attempts at relocating.</returns>
		int GetPiling() const { return m_Piling; }

		/// <summary>
		/// The impulse force that a particle needs to knock loose a terrain pixel of this material. In kg * m/s.
		/// </summary>
		/// <returns>The impulse force that a particle needs to knock loose a terrain pixel of this material.</returns>
		float GetIntegrity() const { return m_Integrity; }

		/// <summary>
		/// Gets the scalar value that defines the restitution of this Material. 1.0 = no kinetic energy is lost in a collision, 0.0 = all energy is lost (plastic).
		/// </summary>
		/// <returns>A float scalar value that defines the restitution of this Material.</returns>
		float GetRestitution() const { return m_Restitution; }

		/// <summary>
		/// Gets the scalar value that defines the friction of this Material. 1.0 = will snag onto everything, 0.0 = will glide with no friction.
		/// </summary>
		/// <returns>A float scalar value that defines the friction of this Material.</returns>
		float GetFriction() const { return m_Friction; }

		/// <summary>
		/// Gets the scalar value that defines the stickiness of this Material. 1.0 = will stick to everything, 0.0 = will never stick to anything.
		/// </summary>
		/// <returns>A float scalar value that defines the stickiness of this Material.</returns>
		float GetStickiness() const { return m_Stickiness; }

		/// <summary>
		/// Gets the density of this Material in Kg/L.
		/// </summary>
		/// <returns>The density of this Material.</returns>
		float GetVolumeDensity() const { return m_VolumeDensity; }

		/// <summary>
		/// Gets the density of this Material in kg/pixel, usually calculated from the KG per Volume L property.
		/// </summary>
		/// <returns>The pixel density of this Material.</returns>
		float GetPixelDensity() const { return m_PixelDensity; }

		/// <summary>
		/// If this material transforms into something else when settling into the terrain, this will return that different material index. If not, it will just return the regular index of this material.
		/// </summary>
		/// <returns>The settling material index of this or the regular index.</returns>
		unsigned char GetSettleMaterial() const { return (m_SettleMaterialIndex != 0) ? m_SettleMaterialIndex : m_Index; }

		/// <summary>
		/// Gets the material index to spawn instead of this one for special effects.
		/// </summary>
		/// <returns>The material index to spawn instead of this one for special effects. 0 means to spawn the same material as this.</returns>
		unsigned char GetSpawnMaterial() const { return m_SpawnMaterialIndex; }

		/// <summary>
		/// Whether this material is scrap material made from gibs of things that have already been blown apart.
		/// </summary>
		/// <returns>Whether this material is scrap material.</returns>
		bool IsScrap() const { return m_IsScrap; }

		/// <summary>
		/// Gets the Color of this Material.
		/// </summary>
		/// <returns>The color of this material.</returns>
		Color GetColor() const { return m_Color; }

		/// <summary>
		/// Indicates whether or not to use the Material's own color when a pixel of this Material is knocked loose from the terrain.
		/// </summary>
		/// <returns>Whether the Material's color, or the terrain pixel's color should be applied.</returns>
		bool UsesOwnColor() const { return m_UseOwnColor; }
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// An assignment operator for setting one Material equal to another.
		/// </summary>
		/// <param name="rhs">A Material reference.</param>
		/// <returns>A reference to the changed Material.</returns>
		Material & operator=(const Material &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		unsigned char m_Index; //!< Index of this in the material palette. 0 - 255.
		int m_Priority; //!< The priority that a pixel of this material has to be displayed. The higher the number, the higher chances that a pixel of this material will be drawn on top of others.
		int m_Piling; //! The amount of times a dislodged pixel of this Material will attempt to relocate upwards, when intersecting a terrain pixel of the same Material. TODO: Better property name?

		float m_Integrity; //!< The impulse force that a particle needs to knock loose a terrain pixel of this material. In kg * m/s.
		float m_Restitution; //!< A scalar value that defines the restitution (elasticity). 1.0 = no kinetic energy is lost in a collision, 0.0 = all energy is lost (plastic).
		float m_Friction; //!< A scalar value that defines the friction coefficient. 1.0 = will snag onto everything, 0.0 = will glide with no friction.
		float m_Stickiness; //!< A scalar value that defines the stickiness coefficient (no sticky 0.0 - 1.0 max). Determines the likelihood of something of this material sticking when a collision occurs.

		float m_VolumeDensity; //!< Density in Kg/L.
		float m_PixelDensity; //!< Density in kg/pixel, usually calculated from the KG per Volume L property.

		// TODO: Implement these properties maybe? They aren't referenced anywhere or do anything as of now.
		float m_GibImpulseLimitPerLiter; //!< How much impulse gib limit of an object increases per liter of this material.
		float m_GibWoundLimitPerLiter; //!< How much wound gib limit of an object increases per liter of this material.

		unsigned char m_SettleMaterialIndex; //!< The material to turn particles of this into when they settle on the terrain. 0 here means to spawn this material.
		unsigned char m_SpawnMaterialIndex; //!< The material to spawn instead of this one for special effects, etc. 0 here means to spawn this material.
		bool m_IsScrap; //!< Whether this material is scrap material made from gibs of things that have already been blown apart.

		Color m_Color; //!< The natural color of this material.
		bool m_UseOwnColor; //!< Whether or not to use the own color when a pixel of this material is knocked loose from the terrain. If 0, then the terrain pixel's color will be applied instead.

		ContentFile m_TextureFile; //!< The file pointing to the terrain texture of this Material 
		BITMAP *m_TerrainTexture; //!< The texture of this material, used when building an SLTerrain.

	private:

		/// <summary>
		/// Clears all the member variables of this Material, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif