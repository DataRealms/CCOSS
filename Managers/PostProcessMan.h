#ifndef _RTEPOSTPROCESSMAN_
#define _RTEPOSTPROCESSMAN_

#include "Singleton.h"
#include "Box.h"
#include "SceneMan.h"

#define g_PostProcessMan PostProcessMan::Instance()

namespace RTE {

	/// <summary>
	/// Structure for storing a post-process screen effect to be applied at the last stage of 32bpp rendering.
	/// </summary>
	struct PostEffect {
		BITMAP *m_Bitmap = nullptr; //!< The bitmap to blend, not owned.
		size_t m_BitmapHash = 0; //!< Hash used to transmit glow events over the network.
		float m_Angle = 0; // Post effect angle.
		int m_Strength = 128; //!< Scalar float for how hard to blend it in, 0 - 255.
		Vector m_Pos; //!< Post effect position. Can be relative to the scene, or to the screen, depending on context.

		/// <summary>
		/// Constructor method used to instantiate a PostEffect object in system memory.
		/// </summary>
		PostEffect(const Vector &pos, BITMAP *bitmap, size_t bitmapHash, int strength, float angle) : m_Bitmap(bitmap), m_BitmapHash(bitmapHash), m_Angle(angle), m_Strength(strength), m_Pos(pos) {}
	};

	/// <summary>
	/// Singleton manager responsible for all 32bpp post-process effect drawing.
	/// </summary>
	class PostProcessMan : public Singleton<PostProcessMan> {

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
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PostProcessMan object before deletion from system memory.
		/// </summary>
		~PostProcessMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PostProcessMan object.
		/// </summary>
		void Destroy();

		/// <summary>
		/// Clears the list of registered post-processing screen effects and glow boxes.
		/// </summary>
		void ClearScreenPostEffects() { m_PostScreenEffects.clear(); m_PostScreenGlowBoxes.clear(); }

		/// <summary>
		/// Clears the list of registered post-processing scene effects and glow areas.
		/// </summary>
		void ClearScenePostEffects() { m_PostSceneEffects.clear(); m_GlowAreas.clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Takes the current state of the 8bpp back-buffer, copies it, and adds post-processing effects on top like glows etc.
		/// </summary>
		void PostProcess();

		/// <summary>
		/// Adjusts the offsets of all effects relative to the specified player screen and adds them to the total screen effects list so they can be drawn in PostProcess().
		/// </summary>
		/// <param name="playerScreen">Player screen to adjust effect offsets for.</param>
		/// <param name="targetBitmap">Bitmap representing the player screen.</param>
		/// <param name="targetBitmapOffset">The position of the specified player's draw screen on the backbuffer.</param>
		/// <param name="screenRelativeEffectsList">List of the specified player's accumulated post effects for this frame.</param>
		/// <param name="screenRelativeGlowBoxesList">List of the specified player's accumulated glow boxes for this frame.</param>
		void AdjustEffectsPosToPlayerScreen(int playerScreen, BITMAP *targetBitmap, const Vector &targetBitmapOffset, std::list<PostEffect> &screenRelativeEffectsList, std::list<Box> &screenRelativeGlowBoxesList) const;
#pragma endregion

#pragma region Post Effect Handling
		/// <summary>
		/// Gets the list of effects to apply at the end of each frame.
		/// </summary>
		/// <returns>The list of effects to apply at the end of each frame.</returns>
		std::list<PostEffect> * GetPostScreenEffectsList() { return &m_PostScreenEffects; }

		/// <summary>
		/// Registers a post effect to be added at the very last stage of 32bpp rendering by the FrameMan.
		/// </summary>
		/// <param name="effectPos">The absolute scene coordinates of the center of the effect.</param>
		/// <param name="effect">A 32bpp BITMAP screen should be drawn centered on the above scene location in the final frame buffer. Ownership is NOT transferred!</param>
		/// <param name="hash">Hash value of the effect for transmitting over the network.</param>
		/// <param name="strength">The intensity level this effect should have when blended in post. 0 - 255.</param>
		/// <param name="angle">The angle this effect should be rotated at.</param>
		void RegisterPostEffect(const Vector &effectPos, BITMAP *effect, size_t hash, int strength = 255, float angle = 0);

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

		/// <summary>
		/// Gets a temporary bitmap of specified size to rotate post effects in.
		/// </summary>
		/// <param name="bitmapSize">Size of bitmap to get.</param>
		/// <returns>Pointer to the temporary bitmap.</returns>
		BITMAP * GetTempEffectBitmap(BITMAP *bitmap) const;
#pragma endregion

#pragma region Post Pixel Glow Handling
		/// <summary>
		/// Gets the list of areas that will be processed with glow.
		/// </summary>
		/// <returns>The list of areas that will be processed with glow.</returns>
		std::list<Box> * GetPostScreenGlowBoxesList() { return &m_PostScreenGlowBoxes; }

		/// <summary>
		/// Registers a specific IntRect to be post-processed and have special pixel colors lit up by glow effects in it.
		/// </summary>
		/// <param name="glowArea">The IntRect to have special color pixels glow in, in scene coordinates.</param>
		void RegisterGlowArea(const IntRect &glowArea) { if (g_TimerMan.DrawnSimUpdate() && g_TimerMan.SimUpdatesSinceDrawn() >= 0) { m_GlowAreas.push_back(glowArea); } }

		/// <summary>
		/// Creates an IntRect and registers it to be post-processed and have special pixel colors lit up by glow effects in it.
		/// </summary>
		/// <param name="center">The center of the IntRect.</param>
		/// <param name="radius">The radius around it to add as an area.</param>
		void RegisterGlowArea(const Vector &center, float radius) {
			RegisterGlowArea(IntRect(static_cast<int>(center.m_X - radius), static_cast<int>(center.m_Y - radius), static_cast<int>(center.m_X + radius), static_cast<int>(center.m_Y + radius)));
		}

		/// <summary>
		/// Registers a specific glow dot effect to be added at the very last stage of 32bpp rendering by the FrameMan.
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
		bool GetGlowAreasWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<Box> &areaList) const;
#pragma endregion

#pragma region Network Post Effect Handling
		/// <summary>
		/// Copies the specified player's screen relative post effects list of this PostProcessMan to the referenced list. Used for sending post effect data over the network.
		/// </summary>
		/// <param name="whichScreen">Which player screen to get list for.</param>
		/// <param name="outputList">Reference to the list of post effects to copy into.</param>
		void GetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> &outputList);

		/// <summary>
		/// Copies the player's screen relative post effects from the referenced list to the list of this PostProcessMan. Used for receiving post effect data over the network.
		/// </summary>
		/// <param name="whichScreen">Which player screen to set list for.</param>
		/// <param name="inputList">Reference to the list of post effects to copy from.</param>
		void SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> &inputList);
#pragma endregion

	protected:

		std::list<PostEffect> m_PostScreenEffects; //!< List of effects to apply at the end of each frame. This list gets cleared out and re-filled each frame.
		std::list<PostEffect> m_PostSceneEffects; //!< All post-processing effects registered for this draw frame in the scene.

		std::list<Box> m_PostScreenGlowBoxes; //!< List of areas that will be processed with glow.
		std::list<IntRect> m_GlowAreas; //!< All the areas to do post glow pixel effects on, in scene coordinates.

		std::array<std::list<PostEffect>, c_MaxScreenCount> m_ScreenRelativeEffects; //!< List of screen relative effects for each player in online multiplayer.
		std::array<std::mutex, c_MaxScreenCount> ScreenRelativeEffectsMutex; //!< Mutex for the ScreenRelativeEffects list when accessed by multiple threads in online multiplayer.

		BITMAP *m_YellowGlow; //!< Bitmap for the yellow dot glow effect.
		BITMAP *m_RedGlow; //!< Bitmap for the red dot glow effect.
		BITMAP *m_BlueGlow; //!< Bitmap for the blue dot glow effect.

		size_t m_YellowGlowHash; //!< Hash value for the yellow dot glow effect bitmap.
		size_t m_RedGlowHash; //!< Hash value for the red dot glow effect bitmap.
		size_t m_BlueGlowHash; //!< Hash value for the blue dot glow effect bitmap.

		std::unordered_map<int, BITMAP *> m_TempEffectBitmaps; //!< Stores temporary bitmaps to rotate post effects in for quick access.

	private:

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
		/// <param name="left">Position of box left plane (X start).</param>
		/// <param name="top">Position of box top plane (Y start).</param>
		/// <param name="right">Position of box right plane (X end).</param>
		/// <param name="bottom">Position of box bottom plane (Y end).</param>
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
		BITMAP * GetDotGlowEffect(DotGlowColor whichColor) const;

		/// <summary>
		/// Gets the hash value of a specific standard dot glow effect for making pixels glow.
		/// </summary>
		/// <param name="which">Which of the dot glow colors to get, see the DotGlowColor enumerator.</param>
		/// <returns>The hash value of the requested glow dot BITMAP.</returns>
		size_t GetDotGlowEffectHash(DotGlowColor whichColor) const;
#pragma endregion

#pragma region PostProcess Breakdown
		/// <summary>
		/// Draws all the glow dot effects on pixels registered inside glow boxes for this frame. This is called from PostProcess().
		/// </summary>
		void DrawDotGlowEffects();

		/// <summary>
		/// Draws all the glow effects registered for this frame. This is called from PostProcess().
		/// </summary>
		void DrawPostScreenEffects() const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this PostProcessMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PostProcessMan(const PostProcessMan &reference) = delete;
		PostProcessMan & operator=(const PostProcessMan &rhs) = delete;
	};
}
#endif