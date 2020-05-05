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
		unsigned short m_Color; //!< Color to draw this primitive with.		
		short m_Player; //!< Player screen to draw this primitive on.

		/// <summary>
		/// Constructor method for GraphicalPrimitive objects.
		/// </summary>
		GraphicalPrimitive() { m_Color = 0; m_Player = -1; }

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
		void TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos);

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos) = 0;
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
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		LinePrimitive(Vector start, Vector end, unsigned char color) { LinePrimitive(-1, start, end, color); }

		/// <summary>
		/// Constructor method for LinePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		LinePrimitive(short player, Vector start, Vector end, unsigned char color) {
			m_StartPos = start;
			m_EndPos = end;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
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
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxPrimitive(Vector start, Vector end, unsigned char color) { BoxPrimitive(-1, start, end, color); }

		/// <summary>
		/// Constructor method for BoxPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxPrimitive(short player, Vector start, Vector end, unsigned char color) {
			m_StartPos = start;
			m_EndPos = end;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
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
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxFillPrimitive(Vector start, Vector end, unsigned char color) { BoxFillPrimitive(-1, start, end, color); }

		/// <summary>
		/// Constructor method for BoxFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="start">Start position of the primitive.</param>
		/// <param name="end">End position of the primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		BoxFillPrimitive(short player, Vector start, Vector end, unsigned char color) {
			m_StartPos = start;
			m_EndPos = end;
			m_Color = color;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
	};
#pragma endregion

#pragma region Cicrcle Primitive
	/// <summary>
	/// Class used to schedule drawing of circle primitives created from Lua.
	/// </summary>
	class CirclePrimitive : public GraphicalPrimitive {

	public:

		float m_Radius; //!< Radius of the circle primitive.

		/// <summary>
		/// Constructor method for CirclePrimitive object.
		/// </summary>
		CirclePrimitive() { m_Radius = 0; }

		/// <summary>
		/// Constructor method for CirclePrimitive object.
		/// </summary>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CirclePrimitive(Vector pos, short radius, unsigned char color) { CirclePrimitive(-1, pos, radius, color); }

		/// <summary>
		/// Constructor method for CirclePrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CirclePrimitive(short player, Vector pos, short radius, unsigned char color) {
			m_StartPos = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
	};
#pragma endregion

#pragma region Filled Circle Primitive
	/// <summary>
	/// Class used to schedule drawing of filled circle primitives created from Lua
	/// </summary>
	class CircleFillPrimitive : public GraphicalPrimitive {

	public:

		float m_Radius; //!< Radius of the circle primitive.

		/// <summary>
		/// Constructor method for CircleFillPrimitive object.
		/// </summary>
		CircleFillPrimitive() { m_Radius = 0; }

		/// <summary>
		/// Constructor method for CircleFillPrimitive object.
		/// </summary>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CircleFillPrimitive(Vector pos, short radius, unsigned char color) { CircleFillPrimitive(-1, pos, radius, color); }

		/// <summary>
		/// Constructor method for CircleFillPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="radius">Radius of the circle primitive.</param>
		/// <param name="color">Color to draw this primitive with.</param>
		CircleFillPrimitive(short player, Vector pos, short radius, unsigned char color) {
			m_StartPos = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
	};
#pragma endregion

#pragma region Text Primitive
	/// <summary>
	/// Class used to schedule drawing of text primitives created from Lua.
	/// </summary>
	class TextPrimitive : public GraphicalPrimitive {

	public:

		bool m_IsSmall; //!< Use small or large font. True for small font.
		short m_Alignment; //!< Alignment of text.
		std::string m_Text; //!< String containing text to draw.

		/// <summary>
		/// Constructor method for TextPrimitive object.
		/// </summary>
		TextPrimitive() {
			m_IsSmall = true;
			m_Text = "";
			m_Alignment = 0;
			m_Player = -1;
		}

		/// <summary>
		/// Constructor method for TextPrimitive object.
		/// </summary>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="text">String containing text to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		TextPrimitive(Vector pos, std::string text, bool isSmall, short alignment) { TextPrimitive(-1, pos, text, isSmall, alignment); }

		/// <summary>
		/// Constructor method for TextPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="text">String containing text to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		TextPrimitive(short player, Vector pos, std::string text, bool isSmall, short alignment) {
			m_StartPos = pos;
			m_Text = text;
			m_IsSmall = isSmall;
			m_Alignment = alignment;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
	};
#pragma endregion

#pragma region Bitmap Primitive
	/// <summary>
	/// Class used to schedule drawing of bitmap primitives created from Lua.
	/// </summary>
	class BitmapPrimitive : public GraphicalPrimitive {

	public:

		float m_RotAngle; //!< Angle to rotate bitmap in radians.
		BITMAP *m_Bitmap; //!< Bitmap to draw.

		/// <summary>
		/// Constructor method for BitmapPrimitive object.
		/// </summary>
		BitmapPrimitive() {
			m_Bitmap = 0;
			m_RotAngle = 0.0;
			m_Player = -1;
		}

		/// <summary>
		/// Constructor method for BitmapPrimitive object.
		/// </summary>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="bitmap">Bitmap to draw.</param>
		/// <param name="rotAngle">Angle to rotate bitmap in radians.</param>
		BitmapPrimitive(Vector pos, BITMAP * bitmap, float rotAngle) { BitmapPrimitive(-1, pos, bitmap, rotAngle); }

		/// <summary>
		/// Constructor method for BitmapPrimitive object.
		/// </summary>
		/// <param name="player">Player screen to draw this primitive on.</param>
		/// <param name="pos">Position of this primitive.</param>
		/// <param name="bitmap">Bitmap to draw.</param>
		/// <param name="rotAngle">Angle to rotate bitmap in radians.</param>
		BitmapPrimitive(short player, Vector pos, BITMAP * bitmap, float rotAngle) {
			m_StartPos = pos;
			m_Bitmap = bitmap;
			m_RotAngle = rotAngle;
			m_Player = player;
		}

		/// <summary>
		/// Draws this primitive on provided bitmap.
		/// </summary>
		/// <param name="drawScreen">Bitmap to draw on.</param>
		/// <param name="targetPos">Position of graphical primitive.</param>
		virtual void Draw(BITMAP *drawScreen, Vector targetPos);
	};
#pragma endregion
}
#endif