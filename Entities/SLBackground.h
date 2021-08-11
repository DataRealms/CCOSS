#ifndef _RTESLBACKGROUND_
#define _RTESLBACKGROUND_

#include "SceneLayer.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	class SLBackground : public SceneLayer {

	public:

		EntityAllocation(SLBackground)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		SLBackground() { Clear(); }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		int Create() override;

		/// <summary>
		/// Creates a SLBackground to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SLBackground to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SLBackground &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// 
		/// </summary>
		~SLBackground() override { Destroy(true); }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="notInherited"></param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { SceneLayer::Destroy(); } Clear(); }
#pragma endregion

#pragma region Getters and Setters

#pragma endregion

#pragma region Concrete Methods

#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the state of this SLTerrain. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this SLTerrain's foreground's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="scrollOverride">If a non-{-1,-1} vector is passed, the internal scroll offset of this is overridden with it. It becomes the new source coordinates.</param>
		void Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1)) override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass;

		std::vector<BITMAP *> m_Bitmaps;
		int m_FrameCount;
	private:

		/// <summary>
		/// Clears all the member variables of this SLBackground, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SLBackground(const SLBackground &reference) = delete;
		SLBackground & operator=(const SLBackground &rhs) = delete;
	};
}
#endif