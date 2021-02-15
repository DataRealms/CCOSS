#ifndef _RTEPRIMITIVE_
#define _RTEPRIMITIVE_

#include "Vector.h"

namespace RTE {

#pragma region Graphical Primitive
	/// <summary>
	/// Class used to schedule drawing of graphical primitives created from Lua.
	/// All coordinates passed to GraphicalPrimitive objects are Scene coordinates.
	/// </summary>
	class GraphicalPrimitive {

	public:

		Vector m_StartPos; //!< Start position of the primitive.
		Vector m_EndPos; //!< End position of the primitive.
		unsigned char m_Color; //!< Color to draw this primitive with.
		int m_Player; //!< Player screen to draw this primitive on.

		/// <summary>
		/// Destructor method used to clean up a GraphicalPrimitive object before deletion from system memory.
		/// </summary>
		virtual ~GraphicalPrimitive() = default;

		/// <summary>
		/// Translates coordinates from scene to this bitmap offset producing two coordinates.
		/// </summary>
		/// <param name="targetPos">Target position.</param>
		/// <param name="scenePos">Position on scene.</param>
		/// <param name="drawLeftPos">'Left' position of bitmap on scene with negative values as if scene seam is 0,0.</param>
		/// <param name="drawRightPos">'Right' position of bitmap on scene with positive values.</param>
		/// <remarks>
		/// Unfortunately it's hard to explain how this works. It tries to represent scene bitmap as two parts with center in 0,0.
		/// Right part is just plain visible part with coordinates from [0, scenewidth] and left part is imaginary bitmap as if we traversed it across the seam right-to-left with coordinates [0, -scenewidth].
		/// So in order to be drawn each screen coordinates calculated twice for left and right 'bitmaps' and then one of them either flies away off-screen or gets drawn on the screen.
		/// When we cross the seam either left or right part is actually drawn in the bitmap, and negative coordinates of right part are compensated by view point offset coordinates when we cross the seam right to left.
		/// I really don't know how to make it simpler, because it has so many special cases and simply wrapping all out-of-the scene coordinates don't work because this way nothing will be ever draw across the seam.
		/// You're welcome to rewrite this nightmare if you can, I wasted a whole week on this (I can admit that I'm just too dumb for this) )))
		/// </remarks>
		void TranslateCoordinates(Vector targetPos, const Vector &scenePos, Vector &drawLeftPos, Vector &drawRightPos) const;

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, const Vector &targetPos) = 0;
	};
#pragma endregion

#pragma region Line Primitive
	/// <summary>
	/// Class used to schedule drawing of line primitives created from Lua.
	/// </summary>
	class LinePrimitive : public GraphicalPrimitive {

	public:

		/// <summary>
		/// Constructor method for LinePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="startPos">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		LinePrimitive(int player, const Vector &startPos, const Vector &endPos, unsigned char color) {
			m_StartPos = startPos;
			m_EndPos = endPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Arc Primitive
	/// <summary>
	/// Class used to schedule drawing of arc line primitives created from Lua.
	/// </summary>
	class ArcPrimitive : public GraphicalPrimitive {

	public:

		float m_StartAngle; //!< The angle from which the arc begins.
		float m_EndAngle; //!< The angle at which the arc ends.
		int m_Radius; //!< Radius of the arc primitive.
		int m_Thickness; //!< Thickness of the arc primitive in pixels.

		/// <summary>
		/// Constructor method for ArcPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="centerPos">Position of this primitive's center.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		ArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, int thickness, unsigned char color) :
			m_StartAngle(startAngle), m_EndAngle(endAngle), m_Radius(radius), m_Thickness(thickness) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Spline Primitive
	/// <summary>
	/// Class used to schedule drawing of spline primitives created from Lua.
	/// </summary>
	class SplinePrimitive : public GraphicalPrimitive {

	public:

		Vector m_GuidePointAPos; //!< A guide point that controls the curve of the spline.
		Vector m_GuidePointBPos; //!< A guide point that controls the curve of the spline.

		/// <summary>
		/// Constructor method for SplinePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="startPos">Start position of the primitive.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		SplinePrimitive(int player, const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color) :
			m_GuidePointAPos(guideA), m_GuidePointBPos(guideB) {

			m_StartPos = startPos;
			m_EndPos = endPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Box Primitive
	/// <summary>
	/// Class used to schedule drawing of box primitives created from Lua.
	/// </summary>
	class BoxPrimitive : public GraphicalPrimitive {

	public:

		/// <summary>
		/// Constructor method for BoxPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="topLeftPos">Start position of the primitive. Top left corner.</param>
		/// <param name="bottomRightPos">End position of the primitive. Bottom right corner.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Filled Box Primitive
	/// <summary>
	/// Class used to schedule drawing of filled box primitives created from Lua.
	/// </summary>
	class BoxFillPrimitive : public GraphicalPrimitive {

	public:

		/// <summary>
		/// Constructor method for BoxFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="topLeftPos">Start position of the primitive. Top left corner.</param>
		/// <param name="bottomRightPos">End position of the primitive. Bottom right corner.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Rounded Box Primitive
	/// <summary>
	/// Class used to schedule drawing of box with round corners primitives created from Lua.
	/// </summary>
	class RoundedBoxPrimitive : public GraphicalPrimitive {

	public:

		int m_CornerRadius; //!< The radius of the corners of the box.

		/// <summary>
		/// Constructor method for RoundedBoxPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="topLeftPos">Start position of the primitive. Top left corner.</param>
		/// <param name="bottomRightPos">End position of the primitive. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		RoundedBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) :
			m_CornerRadius(cornerRadius) {

			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Filled Rounded Box Primitive
	/// <summary>
	/// Class used to schedule drawing of filled box with round corners primitives created from Lua.
	/// </summary>
	class RoundedBoxFillPrimitive : public GraphicalPrimitive {

	public:

		int m_CornerRadius; //!< The radius of the corners of the box.

		/// <summary>
		/// Constructor method for RoundedBoxFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="topLeftPos">Start position of the primitive. Top left corner.</param>
		/// <param name="bottomRightPos">End position of the primitive. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		RoundedBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) :
			m_CornerRadius(cornerRadius) {

			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Cicrcle Primitive
	/// <summary>
	/// Class used to schedule drawing of circle primitives created from Lua.
	/// </summary>
	class CirclePrimitive : public GraphicalPrimitive {

	public:

		int m_Radius; //!< Radius of the circle primitive.

		/// <summary>
		/// Constructor method for CirclePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="centerPos">Position of this primitive's center.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CirclePrimitive(int player, const Vector &centerPos, int radius, unsigned char color) :
			m_Radius(radius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Filled Circle Primitive
	/// <summary>
	/// Class used to schedule drawing of filled circle primitives created from Lua
	/// </summary>
	class CircleFillPrimitive : public GraphicalPrimitive {

	public:

		int m_Radius; //!< Radius of the circle primitive.

		/// <summary>
		/// Constructor method for CircleFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="centerPos">Position of this primitive's center.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CircleFillPrimitive(int player, const Vector &centerPos, int radius, unsigned char color) :
			m_Radius(radius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Ellipse Primitive
	/// <summary>
	/// Class used to schedule drawing of ellipse primitives created from Lua.
	/// </summary>
	class EllipsePrimitive : public GraphicalPrimitive {

	public:

		int m_HorizRadius; //!< The horizontal radius of the ellipse primitive.
		int m_VertRadius; //!< The vertical radius of the ellipse primitive.

		/// <summary>
		/// Constructor method for EllipsePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="centerPos">Position of this primitive's center.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		EllipsePrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) :
			m_HorizRadius(horizRadius), m_VertRadius(vertRadius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Filled Ellipse Primitive
	/// <summary>
	/// Class used to schedule drawing of filled ellipse primitives created from Lua
	/// </summary>
	class EllipseFillPrimitive : public GraphicalPrimitive {

	public:

		int m_HorizRadius; //!< The horizontal radius of the ellipse primitive.
		int m_VertRadius; //!< The vertical radius of the ellipse primitive.

		/// <summary>
		/// Constructor method for EllipseFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="centerPos">Position of this primitive's center.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		EllipseFillPrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) :
			m_HorizRadius(horizRadius), m_VertRadius(vertRadius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Triangle Primitive
	/// <summary>
	/// Class used to schedule drawing of triangle primitives created from Lua.
	/// </summary>
	class TrianglePrimitive : public GraphicalPrimitive {

	public:

		Vector m_PointAPos; //!< First point of the triangle.
		Vector m_PointBPos; //!< Second point of the triangle.
		Vector m_PointCPos; //!< Third point of the triangle.

		/// <summary>
		/// Constructor method for TrianglePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle.</param>
		/// <param name="pointB">Position of the second point of the triangle</param>
		/// <param name="pointC">Position of the third point of the triangle</param>
		/// <param name="color">Color to draw this primitive with.</param>
		TrianglePrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) :
			m_PointAPos(pointA), m_PointBPos(pointB), m_PointCPos(pointC) {

			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Filled Triangle Primitive
	/// <summary>
	/// Class used to schedule drawing of filled triangle primitives created from Lua.
	/// </summary>
	class TriangleFillPrimitive : public GraphicalPrimitive {

	public:

		Vector m_PointAPos; //!< First point of the triangle.
		Vector m_PointBPos; //!< Second point of the triangle.
		Vector m_PointCPos; //!< Third point of the triangle.

		/// <summary>
		/// Constructor method for TriangleFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle.</param>
		/// <param name="pointB">Position of the second point of the triangle</param>
		/// <param name="pointC">Position of the third point of the triangle</param>
		/// <param name="color">Color to draw this primitive with.</param>
		TriangleFillPrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) :
			m_PointAPos(pointA), m_PointBPos(pointB), m_PointCPos(pointC) {

			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Text Primitive
	/// <summary>
	/// Class used to schedule drawing of text primitives created from Lua.
	/// </summary>
	class TextPrimitive : public GraphicalPrimitive {

	public:

		std::string m_Text; //!< String containing text to draw.
		bool m_IsSmall; //!< Use small or large font. True for small font.
		int m_Alignment; //!< Alignment of text.

		/// <summary>
		/// Constructor method for TextPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="text">String containing text to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		TextPrimitive(int player, const Vector &pos, const std::string &text, bool isSmall, int alignment) :
			m_Text(text), m_IsSmall(isSmall), m_Alignment(alignment) {

			m_StartPos = pos;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion

#pragma region Bitmap Primitive
	/// <summary>
	/// Class used to schedule drawing of bitmap primitives created from Lua.
	/// </summary>
	class BitmapPrimitive : public GraphicalPrimitive {

	public:

		BITMAP *m_Bitmap; //!< Bitmap to draw.
		float m_RotAngle; //!< Angle to rotate bitmap in radians.
		bool m_HFlipped; //!< Whether the Bitmap to draw should be horizontally flipped.
		bool m_VFlipped; //!< Whether the Bitmap to draw should be vertically flipped.

		/// <summary>
		/// Constructor method for BitmapPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive's center.</param>
		/// <param name="bitmap">Bitmap to draw.</param>
		/// <param name="rotAngle">Angle to rotate bitmap in radians.</param>
		/// <param name="hFlipped">Whether the bitmap to draw should be horizontally flipped.</param>
		/// <param name="vFlipped">Whether the bitmap to draw should be vertically flipped.</param>
		BitmapPrimitive(int player, const Vector &centerPos, BITMAP *bitmap, float rotAngle, bool hFlipped, bool vFlipped) :
			m_Bitmap(bitmap), m_RotAngle(rotAngle), m_HFlipped(hFlipped), m_VFlipped(vFlipped) {

			m_StartPos = centerPos;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		void Draw(BITMAP *drawScreen, const Vector &targetPos) override;
	};
#pragma endregion
}
#endif