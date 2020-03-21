#ifndef _RTEPRIMITIVEMAN_
#define _RTEPRIMITIVEMAN_

#include "Singleton.h"
#include "Vector.h"

#define g_PrimitiveMan PrimitiveMan::Instance()

struct BITMAP;

namespace RTE {

	class Entity;

	/// <summary>
	/// Singleton manager responsible for all primitive drawing.
	/// </summary>
	class PrimitiveMan : public Singleton<PrimitiveMan> {

	public:

#pragma region Primitive Objects
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
			short m_Player; //!< Player screen to draw primitive on.

			/// <summary>
			/// Constructor method for GraphicalPrimitive objects.
			/// </summary>
			GraphicalPrimitive() { m_Color = 0; m_Player = -1; }

			/// <summary>
			/// Translates coordinates from scene to this bitmap offset.
			/// </summary>
			/// <param name="targetPos">Bitmap to draw on.</param>
			/// <param name="scenePos">Bitmap offset (scene coordinates).</param>
			/// <returns>Vector with translated coordinates.</returns>
			Vector TranslateCoordinates(Vector targetPos, Vector scenePos) { return scenePos - targetPos; }

			/// <summary>
			/// Translates coordinates from scene to this bitmap offset producing two coordinates.
			/// 'Left' scene bitmap with negative values as if scene seam is 0,0 and 'Right' scene bitmap with positive values.
			/// </summary>
			/// <param name="targetPos">Target position.</param>
			/// <param name="scenePos">Position in scene.</param>
			/// <param name="drawLeftPos"></param>
			/// <param name="drawRightPos"></param>
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
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos) = 0;
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
			LinePrimitive(Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for LinePrimitive object.
			/// </summary>
			LinePrimitive(short player, Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
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
			BoxPrimitive(Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for BoxPrimitive object.
			/// </summary>
			BoxPrimitive(short player, Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);	
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
			BoxFillPrimitive(Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for BoxFillPrimitive object.
			/// </summary>
			BoxFillPrimitive(short player, Vector start, Vector end, unsigned short color) {
				m_StartPos = start;
				m_EndPos = end;
				m_Color = color;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
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
			CirclePrimitive(Vector pos, short radius, unsigned short color) {
				m_StartPos = pos;
				m_Color = color;
				m_Radius = radius;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for CirclePrimitive object.
			/// </summary>
			CirclePrimitive(short player, Vector pos, short radius, unsigned short color) {
				m_StartPos = pos;
				m_Color = color;
				m_Radius = radius;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
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
			CircleFillPrimitive(Vector pos, short radius, unsigned short color) {
				m_StartPos = pos;
				m_Color = color;
				m_Radius = radius;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for CircleFillPrimitive object.
			/// </summary>
			CircleFillPrimitive(short player, Vector pos, short radius, unsigned short color) {
				m_StartPos = pos;
				m_Color = color;
				m_Radius = radius;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
		};
#pragma endregion

#pragma region Text Primitive
		/// <summary>
		/// Class used to schedule drawing of text primitives created from Lua.
		/// </summary>
		class TextPrimitive : public GraphicalPrimitive {

		public:

			bool m_IsSmall; //! Use small or large font. True for small font.
			short m_Alignment; //! Alignment of text.
			std::string m_Text; //! String containing text to draw.

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
			TextPrimitive(Vector pos, std::string text, bool isSmall, short alignment) {
				m_StartPos = pos;
				m_Text = text;
				m_IsSmall = isSmall;
				m_Alignment = alignment;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for TextPrimitive object.
			/// </summary>
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
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
		};
#pragma endregion

#pragma region Bitmap Primitive
		/// <summary>
		/// Class used to schedule drawing of bitmap primitives created from Lua.
		/// </summary>
		class BitmapPrimitive : public GraphicalPrimitive {

		public:

			float m_RotAngle; //! Angle to rotate bitmap in radians.
			BITMAP *m_pBitmap; //! Bitmap to draw.

			/// <summary>
			/// Constructor method for BitmapPrimitive object.
			/// </summary>
			BitmapPrimitive() {
				m_pBitmap = 0;
				m_RotAngle = 0.0;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for BitmapPrimitive object.
			/// </summary>
			BitmapPrimitive(Vector pos, BITMAP * bitmap, float rotAngle) {
				m_StartPos = pos;
				m_pBitmap = bitmap;
				m_RotAngle = rotAngle;
				m_Player = -1;
			}

			/// <summary>
			/// Constructor method for BitmapPrimitive object.
			/// </summary>
			BitmapPrimitive(short player, Vector pos, BITMAP * bitmap, float rotAngle) {
				m_StartPos = pos;
				m_pBitmap = bitmap;
				m_RotAngle = rotAngle;
				m_Player = player;
			}

			/// <summary>
			/// Draws this primitive on provided bitmap.
			/// </summary>
			/// <param name="pDrawScreen">Bitmap to draw on.</param>
			/// <param name="targetPos">Position of graphical primitive.</param>
			virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);
		};
#pragma endregion
#pragma endregion

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PrimitiveMan object in system memory.
		/// </summary>
		PrimitiveMan() { ClearPrimitivesList(); }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Delete all scheduled primitives, called on every FrameMan sim update.
		/// </summary>
		void ClearPrimitivesList();
#pragma endregion

#pragma region Primitive Drawing
		/// <summary>
		/// Draws all stored primitives on the screen for specified player.
		/// </summary>
		/// <param name="player">Player to draw for.</param>
		/// <param name="pTargetBitmap">Bitmap to draw on.</param>
		/// <param name="targetPos">Position to draw.</param>
		void DrawPrimitives(short player, BITMAP *pTargetBitmap, const Vector &targetPos);

		/// <summary>
		/// Schedule to draw a line primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new LinePrimitive(start, end, color)); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(short player, Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new LinePrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new BoxPrimitive(start, end, color)); }

		/// <summary>
		/// Schedule to draw a box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(short player, Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new BoxPrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new BoxFillPrimitive(start, end, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(short player, Vector start, Vector end, unsigned short color) { m_Primitives.push_back(new BoxFillPrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(Vector pos, short radius, unsigned short color) {m_Primitives.push_back(new CirclePrimitive(pos, radius, color));}

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(short player, Vector pos, short radius, unsigned short color) {m_Primitives.push_back(new CirclePrimitive(player, pos, radius, color));}

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(Vector pos, short radius, unsigned short color) {m_Primitives.push_back(new CircleFillPrimitive(pos, radius, color));}

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(short player, Vector pos, short radius, unsigned short color) {m_Primitives.push_back(new CircleFillPrimitive(player, pos, radius, color));}

		/// <summary>
		/// Schedule to draw a text primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(Vector start, std::string text, bool isSmall, short alignment) {m_Primitives.push_back(new TextPrimitive(start, text, isSmall, alignment));}

		/// <summary>
		/// Schedule to draw a text primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(short player, Vector start, std::string text, bool isSmall, short alignment) {m_Primitives.push_back(new TextPrimitive(player, start, text, isSmall, alignment));}

		/// <summary>
		/// Schedule to draw a bitmap primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="pEntity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(Vector start, Entity * pEntity, float rotAngle, unsigned short frame);

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="pEntity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(short player, Vector start, Entity * pEntity, float rotAngle, unsigned short frame);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Manager.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //! A string with the friendly-formatted type name of this object.
		std::list<GraphicalPrimitive *> m_Primitives; //!< List of graphical primitives scheduled to draw this frame, cleared every frame during FrameMan::Draw().

	private:

		// Disallow the use of some implicit methods.
		PrimitiveMan(const PrimitiveMan &reference) {}
		PrimitiveMan & operator=(const PrimitiveMan &rhs) {}
	};
}
#endif