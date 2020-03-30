#ifndef _RTEPOSTPROCESSMAN_
#define _RTEPOSTPROCESSMAN_

#include "Serializable.h"
#include "Singleton.h"
#include "Box.h"
#include "SceneMan.h"

#define g_PostProcessMan PostProcessMan::Instance()

namespace RTE {

	/// <summary>
	/// Structure for storing a post-process screen effect to be applied at the last stage of 32bpp rendering.
	/// </summary>
	struct PostEffect {
		BITMAP *m_pBitmap; //!< The bitmap to blend, not owned.
		size_t m_BitmapHash; //!< Hash used to transmit glow events over the network.
		Vector m_Pos; //!< Where, can be relative to the scene, or to the screen, depending on context.
		float m_Angle; // Post effect angle.
		int m_Strength; //!< Scalar float for how hard to blend it in, 0 - 255.

		/// <summary>
		/// Constructor method used to instantiate a PostEffect object in system memory.
		/// </summary>
		PostEffect() { m_Pos.Reset(); m_pBitmap = 0; m_BitmapHash = 0;  m_Strength = 128; m_Angle = 0; }

		/// <summary>
		/// Constructor method used to instantiate a PostEffect object in system memory.
		/// </summary>
		PostEffect(const Vector &pos, BITMAP *pBitmap, size_t bitmapHash, int strength, float angle) {
			m_Pos = pos;
			m_pBitmap = pBitmap;
			m_BitmapHash = bitmapHash;
			m_Strength = strength;
			m_Angle = angle;
		}
	};

	/// <summary>
	/// Singleton manager responsible for all 32bpp post-process effect drawing.
	/// </summary>
	class PostProcessMan : public Singleton<PostProcessMan>, public Serializable {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PostProcessMan object in system memory. Create() should be called before using the object.
		/// </summary>
		PostProcessMan() { Clear(); }

		/// <summary>
		/// Makes the PostProcessMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Clears the list of registered post-processing screen effects and glow boxes.
		/// </summary>
		void Reset() { m_PostScreenEffects.clear(); m_PostScreenGlowBoxes.clear(); }

		/// <summary>
		/// Clears the list of registered post-processing scene effects and glow areas.
		/// </summary>
		void ClearPostEffects() { m_PostSceneEffects.clear(); m_GlowAreas.clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Takes the current state of the 8bpp back buffer, copies it, and adds post-processing effects on top like glows etc. Only works in 32bpp mode.
		/// </summary>
		void PostProcess();
#pragma endregion

#pragma region Post Effect Handling
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::list<PostEffect> *GetPostScreenEffectsList() { return &m_PostScreenEffects; }

		/// <summary>
		/// Registers a post effect to be added at the very last stage of 32bpp rendering by the FrameMan.
		/// </summary>
		/// <param name="effectPos">The absolute scene coordinates of the center of the effect.</param>
		/// <param name="pEffect">A 32bpp BITMAP screen should be drawn centered on the above scene location in the final frame buffer. Ownership is NOT transferred!</param>
		/// <param name="hash"></param>
		/// <param name="strength">The intensity level this effect should have when blended in post. 0 - 255.</param>
		/// <param name="angle"></param>
		void RegisterPostEffect(const Vector &effectPos, BITMAP *pEffect, size_t hash, int strength = 255, float angle = 0);

		/// <summary>
		/// Gets all screen effects that are located within a box in the scene.
		/// Their coordinates will be returned relative to the upper left corner of the box passed in here. Wrapping of the box will be taken care of.
		/// </summary>
		/// <param name="boxPos">The top left coordinates of the box to get post effects for.</param>
		/// <param name="boxWidth">The width of the box.</param>
		/// <param name="boxHeight">The height of the box.</param>
		/// <param name="effectsList">The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.</param>
		/// <param name="team">The team whose unseen layer should obscure the screen effects here.</param>
		/// <returns>Whether any active post effects were found in that box.</returns>
		bool GetPostScreenEffectsWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team = -1);

		BITMAP *GetTempEffectBitmap(unsigned short bitmapSize) const;
#pragma endregion

#pragma region Post Pixel Glow Handling
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::list<Box> *GetPostScreenGlowBoxesList() { return &m_PostScreenGlowBoxes; }

		/// <summary>
		/// Registers a specific IntRect to be post-processed and have special pixel colors lit up by glow effects in it.
		/// </summary>
		/// <param name="glowArea">The IntRect to have special color pixels glow in, in scene coordinates.</param>
		void RegisterGlowArea(const IntRect &glowArea);

		/// <summary>
		/// Registers a specific IntRect to be post-processed and have special pixel colors lit up by glow effects in it.
		/// </summary>
		/// <param name="center">The center of the IntRect.</param>
		/// <param name="radius">The radius around it to add as an area.</param>
		void RegisterGlowArea(const Vector &center, float radius);

		/// <summary>
		/// Registers a specific post yellow glow effect to be added at the very last stage of 32bpp rendering by the FrameMan.
		/// </summary>
		/// <param name="effectPos">The absolute scene coordinates of the center of the effect.</param>
		/// <param name="color">Which glow dot color to register, see the DotGlowColor enumerator.</param>
		/// <param name="strength">The intensity level this effect should have when blended in post. 0 - 255.</param>
		void RegisterGlowDotEffect(const Vector &effectPos, DotGlowColor color, int strength = 255);

		/// <summary>
		/// Gets all glow areas that affect anything within a box in the scene. 
		/// Their coordinates will be returned relative to the upper left corner of the box passed in here. Wrapping of the box will be taken care of.
		/// </summary>
		/// <param name="boxPos">The top left coordinates of the box to get post effects for.</param>
		/// <param name="boxWidth">The width of the box.</param>
		/// <param name="boxHeight">The height of the box.</param>
		/// <param name="areaList">The list to add the glow Boxes that intersect to. The coordinates of the Boxes returned here will be relative to the boxPos passed in above.</param>
		/// <returns>Whether any active post effects were found in that box.</returns>
		bool GetGlowAreasWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<Box> &areaList);
#pragma endregion

#pragma region Network Post Effect Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="whichScreen"></param>
		/// <param name="outputList"></param>
		void GetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> & outputList);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="whichScreen"></param>
		/// <param name="inputList"></param>
		void SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> & inputList);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Manager.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		std::list<PostEffect> m_PostScreenEffects; //!< List of effects to apply at the end of each frame. This list gets cleared out and re-filled each frame.
		std::list<PostEffect> m_PostSceneEffects; //!< All post-processing effects registered for this draw frame in the scene.

		std::list<Box> m_PostScreenGlowBoxes; //!< List of screen-relative areas that will be processed with glow.
		std::list<IntRect> m_GlowAreas; //!< All the areas to do post glow pixel effects on, in scene coordinates.

		std::list<PostEffect> m_ScreenRelativeEffects[c_MaxScreenCount]; //!<
		std::mutex ScreenRelativeEffectsMutex[c_MaxScreenCount]; //!<

		BITMAP *m_pYellowGlow; //!<
		size_t m_YellowGlowHash; //!<

		BITMAP *m_pRedGlow; //!<
		size_t m_RedGlowHash; //!<

		BITMAP *m_pBlueGlow; //!<
		size_t m_BlueGlowHash; //!<

		//!< Temp bitmaps to rotate post effects in.
		BITMAP *m_pTempEffectBitmap_16;
		BITMAP *m_pTempEffectBitmap_32;
		BITMAP *m_pTempEffectBitmap_64;
		BITMAP *m_pTempEffectBitmap_128;
		BITMAP *m_pTempEffectBitmap_256;
		BITMAP *m_pTempEffectBitmap_512;

#pragma region Post Effect Handling
		/// <summary>
		/// Gets all screen effects that are located within a box in the scene. Their coordinates will be returned relative to the upper left corner of the box passed in here.
		/// </summary>
		/// <param name="boxPos">The top left coordinates of the box to get post effects for.</param>
		/// <param name="boxWidth">The width of the box.</param>
		/// <param name="boxHeight">The height of the box.</param>
		/// <param name="effectsList">The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.</param>
		/// <param name="team">The team whose unseen area should block the glows.</param>
		/// <returns>Whether any active post effects were found in that box.</returns>
		bool GetPostScreenEffects(Vector boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team = -1);

		/// <summary>
		/// Gets all screen effects that are located within a box in the scene. Their coordinates will be returned relative to the upper left corner of the box passed in here.
		/// </summary>
		/// <param name="left">Dimensions of the box.</param>
		/// <param name="top"></param>
		/// <param name="right"></param>
		/// <param name="bottom"></param>
		/// <param name="effectsList">The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.</param>
		/// <param name="team">The team whose unseen area should block the glows.</param>
		/// <returns>Whether any active post effects were found in that box.</returns>
		bool GetPostScreenEffects(int left, int top, int right, int bottom, std::list<PostEffect> &effectsList, int team = -1);
#pragma endregion

#pragma region Post Pixel Glow Handling
		/// <summary>
		/// Gets a specific standard dot glow effect for making pixels glow.
		/// </summary>
		/// <param name="which">Which of the dot glow colors to get, see the DotGlowColor enumerator.</param>
		/// <returns>The requested glow dot BITMAP.</returns>
		BITMAP * GetDotGlowEffect(DotGlowColor which) const;

		/// <summary>
		/// Gets the hash value of a specific standard dot glow effect for making pixels glow.
		/// </summary>
		/// <param name="which">Which of the dot glow colors to get, see the DotGlowColor enumerator.</param>
		/// <returns>The hash value of the requested glow dot BITMAP.</returns>
		size_t GetDotGlowEffectHash(DotGlowColor which) const;
#pragma endregion

	private:

		/// <summary>
		/// Clears all the member variables of this PostProcessMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PostProcessMan(const PostProcessMan &reference) {}
		PostProcessMan & operator=(const PostProcessMan &rhs) {}
	};
}
#endif