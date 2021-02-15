#ifndef _RTEPRIMITIVEMAN_
#define _RTEPRIMITIVEMAN_

#include "Singleton.h"
#include "GraphicalPrimitive.h"

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
		PrimitiveMan() { ClearPrimitivesQueue(); }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Delete all scheduled primitives, called on every FrameMan sim update.
		/// </summary>
		void ClearPrimitivesQueue() { m_ScheduledPrimitives.clear(); }
#pragma endregion

#pragma region Primitive Drawing
		/// <summary>
		/// Draws all stored primitives on the screen for specified player.
		/// </summary>
		/// <param name="player">Player to draw for.</param>
		/// <param name="targetBitmap">Bitmap to draw on.</param>
		/// <param name="targetPos">Position to draw.</param>
		void DrawPrimitives(int player, BITMAP *targetBitmap, const Vector &targetPos) const;

		/// <summary>
		/// Schedule to draw a line primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(const Vector &startPos, const Vector &endPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<LinePrimitive>(-1, startPos, endPos, color)); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(int player, const Vector &startPos, const Vector &endPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<LinePrimitive>(player, startPos, endPos, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<ArcPrimitive>(-1, centerPos, startAngle, endAngle, radius, 1, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive with the option to change thickness.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness) { m_ScheduledPrimitives.push_back(std::make_unique<ArcPrimitive>(-1, centerPos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<ArcPrimitive>(player, centerPos, startAngle, endAngle, radius, 1, color)); }

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
		void DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness) { m_ScheduledPrimitives.push_back(std::make_unique<ArcPrimitive>(player, centerPos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<SplinePrimitive>(-1, startPos, guideA, guideB, endPos, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(int player, const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<SplinePrimitive>(player, startPos, guideA, guideB, endPos, color)); }

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<BoxPrimitive>(-1, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<BoxPrimitive>(player, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<BoxFillPrimitive>(-1, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<BoxFillPrimitive>(player, topLeftPos, bottomRightPos, color)); }

		/// <summary>
		/// Schedule to draw a rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<RoundedBoxPrimitive>(-1, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<RoundedBoxPrimitive>(player, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<RoundedBoxFillPrimitive>(-1, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<RoundedBoxFillPrimitive>(player, topLeftPos, bottomRightPos, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(const Vector &centerPos, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<CirclePrimitive>(-1, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(int player, const Vector &centerPos, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<CirclePrimitive>(player, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(const Vector &centerPos, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<CircleFillPrimitive>(-1, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(int player, const Vector &centerPos, int radius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<CircleFillPrimitive>(player, centerPos, radius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<EllipsePrimitive>(-1, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<EllipsePrimitive>(player, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<EllipseFillPrimitive>(-1, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<EllipseFillPrimitive>(player, centerPos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<TrianglePrimitive>(-1, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<TrianglePrimitive>(player, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a filled triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<TriangleFillPrimitive>(-1, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a filled triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) { m_ScheduledPrimitives.push_back(std::make_unique<TriangleFillPrimitive>(player, pointA, pointB, pointC, color)); }

		/// <summary>
		/// Schedule to draw a text primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(const Vector &start, const std::string &text, bool isSmall, int alignment) { m_ScheduledPrimitives.push_back(std::make_unique<TextPrimitive>(-1, start, text, isSmall, alignment)); }

		/// <summary>
		/// Schedule to draw a text primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(int player, const Vector &start, const std::string &text, bool isSmall, int alignment) { m_ScheduledPrimitives.push_back(std::make_unique<TextPrimitive>(player, start, text, isSmall, alignment)); }

		/// <summary>
		/// Schedule to draw a bitmap primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, Entity *entity, float rotAngle, int frame) { DrawBitmapPrimitive(-1, centerPos, entity, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, entity, rotAngle, frame, hFlipped, vFlipped); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame) { DrawBitmapPrimitive(player, centerPos, entity, rotAngle, frame, false, false); }

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
		void DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped);
#pragma endregion

	protected:

		std::deque<std::unique_ptr<GraphicalPrimitive>> m_ScheduledPrimitives; //!< List of graphical primitives scheduled to draw this frame, cleared every frame during FrameMan::Draw().

	private:

		// Disallow the use of some implicit methods.
		PrimitiveMan(const PrimitiveMan &reference) = delete;
		PrimitiveMan & operator=(const PrimitiveMan &rhs) = delete;
	};
}
#endif