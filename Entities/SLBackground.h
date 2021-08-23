#ifndef _RTESLBACKGROUND_
#define _RTESLBACKGROUND_

#include "SceneLayer.h"
#include "Timer.h"

namespace RTE {

	/// <summary>
	/// A scrolling background layer of the Scene.
	/// </summary>
	class SLBackground : public SceneLayer {
		friend class NetworkServer;

	public:

		EntityAllocation(SLBackground)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SLBackground object in system memory. Create() should be called before using the object.
		/// </summary>
		SLBackground() { Clear(); }

		/// <summary>
		/// Makes the SLBackground object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
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
		/// Destructor method used to clean up a SLBackground object before deletion from system memory.
		/// </summary>
		~SLBackground() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SLBackground object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { SceneLayer::Destroy(); } Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// If true, then the frame will not be changed automatically during update
		/// </summary>
		/// <returns>Whether or not the SLBackground's Frame will change automatically during update.</returns>
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// <summary>
		/// Sets whether this SLBackground is animated manually.
		/// </summary>
		/// <param name="isAnimatedManually">Whether or not to animate manually.</param>
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }

		/// <summary>
		/// Gets the current frame this SLBackground is showing.
		/// </summary>
		/// <returns>The frame that is currently showing.</returns>
		int GetFrame() const { return m_Frame; }

		/// <summary>
		/// Sets frame this SLBackground is supposed to show.
		/// </summary>
		/// <param name="newFrame">The frame that is supposed to be shown.</param>
		void SetFrame(int newFrame) { m_Frame = std::clamp(newFrame, 0, m_FrameCount - 1); }

		/// <summary>
		/// Gets the animation mode of this SLBackground.
		/// </summary>
		/// <returns>The currently set animation mode. See SpriteAnimMode enumeration.</returns>
		int GetSpriteAnimMode() const { return m_SpriteAnimMode; }

		/// <summary>
		/// Sets the animation mode of this SLBackground.
		/// </summary>
		/// <param name="newAnimMode">The new animation mode. See SpirteAnimMode enumeration.</param>
		void SetSpriteAnimMode(SpriteAnimMode newAnimMode = SpriteAnimMode::NOANIM) { m_SpriteAnimMode = std::clamp(newAnimMode, SpriteAnimMode::NOANIM, SpriteAnimMode::ALWAYSPINGPONG); }

		/// <summary>
		/// Gets animation duration of this SLBackground.
		/// </summary>
		/// <returns>The current animation duration, in milliseconds.</returns>
		int GetSpriteAnimDuration() const { return m_SpriteAnimDuration; }

		/// <summary>
		/// Sets animation duration of this SLBackground.
		/// </summary>
		/// <param name="newDuration">The new animation duration, in milliseconds.</param>
		void SetSpriteAnimDuration(int newDuration) { m_SpriteAnimDuration = newDuration; }

		/// <summary>
		/// Gets whether this has auto-scrolling enabled and meets the requirements to actually auto-scroll.
		/// </summary>
		/// <returns>Whether this has auto-scrolling enabled and meets the requirements to actually auto-scroll.</returns>
		bool IsAutoScrolling() const { return (m_WrapX && m_AutoScrollX) || (m_WrapY && m_AutoScrollY); }

		/// <summary>
		/// Gets whether auto-scrolling is enabled on the X axis.
		/// </summary>
		/// <returns>Whether auto-scrolling is enabled on the X axis.</returns>
		bool GetAutoScrollX() const { return m_AutoScrollX; }

		/// <summary>
		/// Sets whether auto-scrolling is enabled on the X axis.
		/// </summary>
		/// <param name="autoScroll">Whether auto-scrolling is enabled on the X axis or not.</param>
		void SetAutoScrollX(bool autoScroll) { m_AutoScrollX = autoScroll; }

		/// <summary>
		/// Gets whether auto-scrolling is enabled on the Y axis.
		/// </summary>
		/// <returns>Whether auto-scrolling is enabled on the Y axis.</returns>
		bool GetAutoScrollY() const { return m_AutoScrollY; }

		/// <summary>
		/// Sets whether auto-scrolling is enabled on the Y axis.
		/// </summary>
		/// <param name="autoScroll">Whether auto-scrolling is enabled on the Y axis or not.</param>
		void SetAutoScrollY(bool autoScroll) { m_AutoScrollY = autoScroll; }

		/// <summary>
		/// Gets the duration between auto-scroll steps.
		/// </summary>
		/// <returns>The duration between auto-scroll steps, in milliseconds.</returns>
		int GetAutoScrollStepInterval() const { return m_AutoScrollStepInterval; }

		/// <summary>
		/// Sets the duration between auto-scroll steps.
		/// </summary>
		/// <param name="newStepInterval">The new duration between auto-scroll steps, in milliseconds.</param>
		void SetAutoScrollStepInterval(int newStepInterval) { m_AutoScrollStepInterval = newStepInterval; }

		/// <summary>
		/// Gets the auto-scroll step (pixels to advance per interval) values.
		/// </summary>
		/// <returns>A Vector with the auto-scroll step values.</returns>
		Vector GetAutoScrollStep() const { return m_AutoScrollStep; }

		/// <summary>
		/// Sets the auto-scroll step (pixels to advance per interval) values.
		/// </summary>
		/// <param name="newStep">A Vector with the new auto-scroll step values.</param>
		void SetAutoScrollStep(const Vector &newStep) { m_AutoScrollStep = newStep; }

		/// <summary>
		/// Gets the auto-scroll step (pixels to advance per interval) value on the X axis.
		/// </summary>
		/// <returns>The auto-scroll step value on the X axis.</returns>
		int GetAutoScrollStepX() const { return m_AutoScrollStep.GetFloorIntX(); }

		/// <summary>
		/// Sets the auto-scroll step (pixels to advance per interval) value on the X axis.
		/// </summary>
		/// <param name="newStepX">The new auto-scroll step value on the X axis.</param>
		void SetAutoScrollStepX(int newStepX) { m_AutoScrollStep.SetX(static_cast<float>(newStepX)); }

		/// <summary>
		/// Gets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// </summary>
		/// <returns>The auto-scroll step value on the Y axis.</returns>
		int GetAutoScrollStepY() const { return m_AutoScrollStep.GetFloorIntY(); }

		/// <summary>
		/// Sets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// </summary>
		/// <param name="newStepY">The new auto-scroll step value on the Y axis.</param>
		void SetAutoScrollStepY(int newStepY) { m_AutoScrollStep.SetY(static_cast<float>(newStepY)); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		void InitScaleFactors();
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
		/// <param name="offsetNeedsScrollRatioAdjustment">Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.</param>
		void Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment = false) override;
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different modes of SLBackground auto-scaling.
		/// </summary>
		enum LayerAutoScaleMode { AutoScaleOff, FitScreen, AlwaysUpscaled, LayerAutoScaleModeCount };

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::vector<BITMAP *> m_Bitmaps; //!< Vector containing all the BITMAPs of this SLBackground. Not owned.
		int m_FrameCount; //!< The total number of frames in this SLBackground's animation.
		int m_Frame; //!< The frame that is currently being shown/drawn.

		SpriteAnimMode m_SpriteAnimMode; //!< The mode in which this SLBackground is animating. See SpriteAnimMode enumeration.
		int m_SpriteAnimDuration; //!< The duration it takes to complete a full animation cycle, in milliseconds.
		bool m_SpriteAnimIsReversingFrames; //!< Keeps track of animation direction (mainly for ALWAYSPINGPONG), true is decreasing frame, false is increasing frame.
		Timer m_SpriteAnimTimer; //!< Timer to keep track of animation.

		bool m_IsAnimatedManually; //!< Whether this SLBackground's animation is handled manually/externally and should not be handled during Update().

		bool m_AutoScrollX; //!< Whether auto-scrolling is enabled on the X axis.
		bool m_AutoScrollY; //!< Whether auto-scrolling is enabled on the Y axis.
		Vector m_AutoScrollStep; //!< Vector with the number of pixels to advance per interval when auto-scrolling.
		int m_AutoScrollStepInterval; //!< The duration between auto-scroll steps, in milliseconds.
		Timer m_AutoScrollStepTimer; //!< Timer to keep track of auto-scrolling steps.
		Vector m_AutoScrollOffset; //!< The offset to adjust the this' main offset with when auto-scrolling to actually get the auto-scrolling effect, adjusted for wrapping.

		int m_FillColorLeft; //!< Palette index to use for filling the gap between the left edge of the bitmap and the left edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorRight; //!< Palette index to use for filling the gap between the right edge of the bitmap and the right edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorUp; //!< Palette index to use for filling the gap between the upper edge of the bitmap and the upper edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorDown; //!< Palette index to use for filling the gap between the lower edge of the bitmap and the lower edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.

		std::array<Vector, LayerAutoScaleMode::LayerAutoScaleModeCount> m_LayerScaleFactors; //!< Array of Vectors containing scale factors for each auto-scale mode.
		bool m_IgnoreAutoScale; //!< Whether auto-scaling settings are ignored and the read-in scale factor is used instead.

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