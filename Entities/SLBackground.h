#ifndef _RTESLBACKGROUND_
#define _RTESLBACKGROUND_

#include "SceneLayer.h"
#include "Timer.h"

namespace RTE {

	/// <summary>
	/// A scrolling background layer of the Scene, placed behind the terrain.
	/// </summary>
	class SLBackground : public SceneLayer {
		friend class NetworkServer;

	public:

		EntityAllocation(SLBackground);
		SerializableOverrideMethods;
		ClassInfoGetters;

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
		/// Gets whether this SLBackground's animation is handled manually/externally.
		/// </summary>
		/// <returns>Whether this SLBackground's animation is handled manually/externally. If true, animation will not be handled during Update().</returns>
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// <summary>
		/// Sets whether this SLBackground is animated manually/externally.
		/// </summary>
		/// <param name="isAnimatedManually">Whether this SLBackground is animated manually/externally and should skip animation handling during Update().</param>
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }

		/// <summary>
		/// Gets the frame number of this SLBackground that is currently set to be drawn.
		/// </summary>
		/// <returns>The frame number that is currently set to be drawn.</returns>
		int GetFrame() const { return m_Frame; }

		/// <summary>
		/// Sets frame number that this SLBackground will draw.
		/// </summary>
		/// <param name="newFrame">The frame number that is supposed to be drawn.</param>
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
		/// Gets the time it takes to complete a full animation cycle of this SLBackground.
		/// </summary>
		/// <returns>The animation cycle duration, in milliseconds.</returns>
		int GetSpriteAnimDuration() const { return m_SpriteAnimDuration; }

		/// <summary>
		/// Sets the time it takes to complete a full animation cycle of this SLBackground.
		/// </summary>
		/// <param name="newDuration">The new animation cycle duration, in milliseconds.</param>
		void SetSpriteAnimDuration(int newDuration) { m_SpriteAnimDuration = newDuration; }

		/// <summary>
		/// Gets whether this SLBackground has auto-scrolling enabled and meets the requirements to actually auto-scroll.
		/// </summary>
		/// <returns>Whether this has auto-scrolling enabled and meets the requirements to actually auto-scroll.</returns>
		bool IsAutoScrolling() const { return (m_WrapX && m_AutoScrollX) || (m_WrapY && m_AutoScrollY); }

		/// <summary>
		/// Gets whether auto-scrolling is enabled on the X axis.
		/// </summary>
		/// <returns>Whether auto-scrolling is enabled on the X axis. This may be true even if auto-scrolling isn't actually happening due to not meeting requirements.</returns>
		bool GetAutoScrollX() const { return m_AutoScrollX; }

		/// <summary>
		/// Sets whether auto-scrolling is enabled on the X axis.
		/// </summary>
		/// <param name="autoScroll">Whether auto-scrolling is enabled on the X axis or not. If requirements aren't met, this will not auto-scroll even if set to true.</param>
		void SetAutoScrollX(bool autoScroll) { m_AutoScrollX = autoScroll; }

		/// <summary>
		/// Gets whether auto-scrolling is enabled on the Y axis.
		/// </summary>
		/// <returns>Whether auto-scrolling is enabled on the Y axis. This may be true even if auto-scrolling isn't actually happening due to not meeting requirements.</returns>
		bool GetAutoScrollY() const { return m_AutoScrollY; }

		/// <summary>
		/// Sets whether auto-scrolling is enabled on the Y axis.
		/// </summary>
		/// <param name="autoScroll">Whether auto-scrolling is enabled on the Y axis or not. If requirements aren't met, this will not auto-scroll even if set to true.</param>
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
		float GetAutoScrollStepX() const { return m_AutoScrollStep.GetX(); }

		/// <summary>
		/// Sets the auto-scroll step (pixels to advance per interval) value on the X axis.
		/// </summary>
		/// <param name="newStepX">The new auto-scroll step value on the X axis.</param>
		void SetAutoScrollStepX(float newStepX) { m_AutoScrollStep.SetX(newStepX); }

		/// <summary>
		/// Gets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// </summary>
		/// <returns>The auto-scroll step value on the Y axis.</returns>
		float GetAutoScrollStepY() const { return m_AutoScrollStep.GetY(); }

		/// <summary>
		/// Sets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// </summary>
		/// <param name="newStepY">The new auto-scroll step value on the Y axis.</param>
		void SetAutoScrollStepY(float newStepY) { m_AutoScrollStep.SetY(newStepY); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Initializes the scale factors for all auto-scaling modes for this SLBackground, then sets the appropriate factor according to the auto-scaling setting.
		/// Has to be done during Scene loading to correctly adjust the factors in cases the Scene does not vertically cover the player's whole screen.
		/// </summary>
		void InitScaleFactors();
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the state of this SLBackground.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this SLBackground's current scrolled position to a bitmap.
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
		bool m_SpriteAnimIsReversingFrames; //!< Keeps track of animation direction (mainly for ALWAYSPINGPONG). True is decreasing frame, false is increasing frame.
		Timer m_SpriteAnimTimer; //!< Timer to keep track of animation.

		bool m_IsAnimatedManually; //!< Whether this SLBackground's animation is handled manually/externally and should not be handled during Update().

		bool m_AutoScrollX; //!< Whether auto-scrolling is enabled on the X axis, but not whether auto-scrolling actually happens on this axis.
		bool m_AutoScrollY; //!< Whether auto-scrolling is enabled on the Y axis, but not whether auto-scrolling actually happens on this axis.
		Vector m_AutoScrollStep; //!< Vector with the number of pixels on each axis to advance per interval when auto-scrolling. Can be fractions of a pixel, but will not be visible until AutoScrollOffset adds up to a full pixel step.
		int m_AutoScrollStepInterval; //!< The duration between auto-scroll steps, in milliseconds.
		Timer m_AutoScrollStepTimer; //!< Timer to keep track of auto-scrolling steps.
		Vector m_AutoScrollOffset; //!< The offset to adjust the main offset with when auto-scrolling to actually get the auto-scrolling effect, adjusted for wrapping.

		int m_FillColorLeft; //!< Palette index to use for filling the gap between the left edge of the bitmap and the left edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorRight; //!< Palette index to use for filling the gap between the right edge of the bitmap and the right edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorUp; //!< Palette index to use for filling the gap between the upper edge of the bitmap and the upper edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorDown; //!< Palette index to use for filling the gap between the lower edge of the bitmap and the lower edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.

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