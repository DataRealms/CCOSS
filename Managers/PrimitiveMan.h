#ifndef _RTEPRIMITIVEMAN_
#define _RTEPRIMITIVEMAN_

#include "Singleton.h"
#include "Primitive.h"

#define g_PrimitiveMan PrimitiveMan::Instance()

namespace RTE {

	class Entity;

	/// <summary>
	/// Singleton manager responsible for all primitive drawing.
	/// </summary>
	class PrimitiveMan : public Singleton<PrimitiveMan> {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PrimitiveMan object in system memory.
		/// </summary>
		PrimitiveMan() { ClearPrimitivesList(); }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PrimitiveMan object before deletion from system memory.
		/// </summary>
		~PrimitiveMan() { ClearPrimitivesList(); }

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
		/// <param name="targetBitmap">Bitmap to draw on.</param>
		/// <param name="targetPos">Position to draw.</param>
		void DrawPrimitives(short player, BITMAP *targetBitmap, const Vector &targetPos) const;

		/// <summary>
		/// Schedule to draw a line primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(Vector startPos, Vector endPos, unsigned char color) { m_Primitives.push_back(new LinePrimitive(-1, startPos, endPos, color)); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(short player, Vector startPos, Vector endPos, unsigned char color) { m_Primitives.push_back(new LinePrimitive(player, startPos, endPos, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(Vector centerPos, float startAngle, float endAngle, short radius, unsigned char color) { m_Primitives.push_back(new ArcPrimitive(-1, centerPos, startAngle, endAngle, radius, 1, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive with the option to change thickness.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(Vector centerPos, float startAngle, float endAngle, short radius, unsigned char color, short thickness) { m_Primitives.push_back(new ArcPrimitive(-1, centerPos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(short player, Vector centerPos, float startAngle, float endAngle, short radius, unsigned char color) { m_Primitives.push_back(new ArcPrimitive(player, centerPos, startAngle, endAngle, radius, 1, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player with the option to change thickness.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(short player, Vector centerPos, float startAngle, float endAngle, short radius, unsigned char color, short thickness) { m_Primitives.push_back(new ArcPrimitive(player, centerPos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(Vector startPos, Vector guideA, Vector guideB, Vector endPos, unsigned char color) { m_Primitives.push_back(new SplinePrimitive(-1, startPos, guideA, guideB, endPos, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(short player, Vector startPos, Vector guideA, Vector guideB, Vector endPos, unsigned char color) { m_Primitives.push_back(new SplinePrimitive(player, startPos, guideA, guideB, endPos, color)); }

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(Vector topLeftPos, Vector bottomRightPos, unsigned char color) { m_Primitives.push_back(new BoxPrimitive(-1, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(short player, Vector topLeftPos, Vector bottomRightPos, unsigned char color) { m_Primitives.push_back(new BoxPrimitive(player, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(Vector topLeftPos, Vector bottomRightPos, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(-1, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(short player, Vector topLeftPos, Vector bottomRightPos, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(player, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(Vector topLeftPos, Vector bottomRightPos, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundedBoxPrimitive(-1, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(short player, Vector topLeftPos, Vector bottomRightPos, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundedBoxPrimitive(player, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(Vector topLeftPos, Vector bottomRightPos, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundedBoxFillPrimitive(-1, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(short player, Vector topLeftPos, Vector bottomRightPos, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundedBoxFillPrimitive(player, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(Vector centerPos, short radius, unsigned char color) { m_Primitives.push_back(new CirclePrimitive(-1, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(short player, Vector centerPos, short radius, unsigned char color) { m_Primitives.push_back(new CirclePrimitive(player, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(Vector centerPos, short radius, unsigned char color) { m_Primitives.push_back(new CircleFillPrimitive(-1, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(short player, Vector centerPos, short radius, unsigned char color) { m_Primitives.push_back(new CircleFillPrimitive(player, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(Vector centerPos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipsePrimitive(-1, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(short player, Vector centerPos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipsePrimitive(player, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(Vector centerPos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipseFillPrimitive(-1, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(short player, Vector centerPos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipseFillPrimitive(player, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(Vector pointA, Vector pointB, Vector pointC, unsigned char color) { m_Primitives.push_back(new TrianglePrimitive(-1, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(short player, Vector pointA, Vector pointB, Vector pointC, unsigned char color) { m_Primitives.push_back(new TrianglePrimitive(player, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a filled triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(Vector pointA, Vector pointB, Vector pointC, unsigned char color) { m_Primitives.push_back(new TriangleFillPrimitive(-1, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a filled triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(short player, Vector pointA, Vector pointB, Vector pointC, unsigned char color) { m_Primitives.push_back(new TriangleFillPrimitive(player, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a text primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(Vector start, std::string text, bool isSmall, short alignment) { m_Primitives.push_back(new TextPrimitive(-1, start, text, isSmall, alignment)); }

		/// <summary>
		/// Schedule to draw a text primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(short player, Vector start, std::string text, bool isSmall, short alignment) { m_Primitives.push_back(new TextPrimitive(player, start, text, isSmall, alignment)); }

		/// <summary>
		/// Schedule to draw a bitmap primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		void DrawBitmapPrimitive(Vector centerPos, Entity *entity, float rotAngle, unsigned short frame) { DrawBitmapPrimitive(-1, centerPos, entity, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(Vector centerPos, Entity *entity, float rotAngle, unsigned short frame, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, entity, rotAngle, frame, hFlipped, vFlipped); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(short player, Vector centerPos, Entity *entity, float rotAngle, unsigned short frame) { DrawBitmapPrimitive(player, centerPos, entity, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(short player, Vector centerPos, Entity *entity, float rotAngle, unsigned short frame, bool hFlipped, bool vFlipped);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Manager.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		const std::string & GetClassName() const { return c_ClassName; }
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