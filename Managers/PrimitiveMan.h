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
#pragma endregion

#pragma region Primitive Draw Scheduling
		/// <summary>
		/// Schedule to draw multiple primitives of varying type with blending enabled.
		/// </summary>
		/// <param name="blendMode">The blending mode to use when drawing each primitive.</param>
		/// <param name="blendAmountR">The blending amount for the Red channel. 0-100.</param>
		/// <param name="blendAmountG">The blending amount for the Green channel. 0-100.</param>
		/// <param name="blendAmountB">The blending amount for the Blue channel. 0-100.</param>
		/// <param name="blendAmountA">The blending amount for the Alpha channel. 0-100.</param>
		/// <param name="primitives">A vector of primitives to schedule drawing for.</param>
		void SchedulePrimitivesForBlendedDrawing(DrawBlendMode blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const std::vector<GraphicalPrimitive *> &primitives);

		/// <summary>
		/// Schedule to draw a line primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(const Vector &startPos, const Vector &endPos, unsigned char color) { DrawLinePrimitive(-1, startPos, endPos, color, 1); }

		/// <summary>
		/// Schedule to draw a line primitive with the option to change thickness.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the line in pixels.</param>
		void DrawLinePrimitive(const Vector &startPos, const Vector &endPos, unsigned char color, int thickness) { DrawLinePrimitive(-1, startPos, endPos, color, thickness); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(int player, const Vector &startPos, const Vector &endPos, unsigned char color) { DrawLinePrimitive(player, startPos, endPos, color, 1); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player with the option to change thickness.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the line in pixels.</param>
		void DrawLinePrimitive(int player, const Vector &startPos, const Vector &endPos, unsigned char color, int thickness);

		/// <summary>
		/// Schedule to draw an arc primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color);

		/// <summary>
		/// Schedule to draw an arc primitive with the option to change thickness.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness);

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color);

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
		void DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness);

		/// <summary>
		/// Schedule to draw a Bezier spline primitive.
		/// </summary>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a Bezier spline primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="startPos">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="endPos">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(int player, const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color);

		/// <summary>
		/// Schedule to draw a rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled rounded box primitive.
		/// </summary>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled rounded box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="topLeftPos">Start position of primitive in scene coordinates. Top left corner.</param>
		/// <param name="bottomRightPos">End position of primitive in scene coordinates. Bottom right corner.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundedBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(const Vector &centerPos, int radius, unsigned char color);

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(int player, const Vector &centerPos, int radius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(const Vector &centerPos, int radius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(int player, const Vector &centerPos, int radius, unsigned char color);

		/// <summary>
		/// Schedule to draw an ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw an ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled ellipse primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// <summary>
		/// Schedule to draw a triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color);

		/// <summary>
		/// Schedule to draw a triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTrianglePrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled triangle primitive.
		/// </summary>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color);

		/// <summary>
		/// Schedule to draw a filled triangle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pointA">Position of the first point of the triangle in scene coordinates.</param>
		/// <param name="pointB">Position of the second point of the triangle in scene coordinates.</param>
		/// <param name="pointC">Position of the third point of the triangle in scene coordinates.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawTriangleFillPrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color);

		/// <summary>
		/// Schedule to draw a polygon primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on, or -1 for all players.</param>
		/// <param name="startPos">Start position of the primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="vertices">A vector containing the positions of the vertices of the polygon, relative to the center position.</param>
		/// <param name="filled">Whether a PolygonFillPrimitive should be scheduled instead of PolygonPrimitive.</param>
		void DrawPolygonOrPolygonFillPrimitive(int player, const Vector &startPos, unsigned char color, const std::vector<Vector *> &vertices, bool filled);

		/// <summary>
		/// Schedule to draw a text primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(const Vector &start, const std::string &text, bool isSmall, int alignment);

		/// <summary>
		/// Schedule to draw a text primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		/// <param name="rotAngle">Angle to rotate text in radians.</param>
		void DrawTextPrimitive(const Vector &start, const std::string &text, bool isSmall, int alignment, float rotAngle);

		/// <summary>
		/// Schedule to draw a text primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		void DrawTextPrimitive(int player, const Vector &start, const std::string &text, bool isSmall, int alignment);

		/// <summary>
		/// Schedule to draw a text primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="text">Text string to draw.</param>
		/// <param name="isSmall">Use small or large font. True for small font.</param>
		/// <param name="alignment">Alignment of text.</param>
		/// <param name="rotAngle">Angle to rotate text in radians.</param>
		void DrawTextPrimitive(int player, const Vector &start, const std::string &text, bool isSmall, int alignment, float rotAngle);

		/// <summary>
		/// Schedule to draw a bitmap primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="moSprite">A MOSprite to draw BITMAP from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, const MOSprite *moSprite, float rotAngle, int frame) { DrawBitmapPrimitive(-1, centerPos, moSprite, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="moSprite">A MOSprite to draw BITMAP from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, const MOSprite *moSprite, float rotAngle, int frame, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, moSprite, rotAngle, frame, hFlipped, vFlipped); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="moSprite">A MOSprite to draw BITMAP from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		void DrawBitmapPrimitive(int player, const Vector &centerPos, const MOSprite *moSprite, float rotAngle, int frame) { DrawBitmapPrimitive(player, centerPos, moSprite, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="moSprite">A MOSprite to draw BITMAP from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(int player, const Vector &centerPos, const MOSprite *moSprite, float rotAngle, int frame, bool hFlipped, bool vFlipped);

		/// <summary>
		/// Schedule to draw a bitmap primitive.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="filePath">Path to the bitmap to draw.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, const std::string &filePath, float rotAngle) { DrawBitmapPrimitive(-1, centerPos, filePath, rotAngle, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="filePath">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(const Vector &centerPos, const std::string &filePath, float rotAngle, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, filePath, rotAngle, hFlipped, vFlipped); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="filePath">Path to the bitmap to draw.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		void DrawBitmapPrimitive(int player, const Vector &centerPos, const std::string &filePath, float rotAngle) { DrawBitmapPrimitive(player, centerPos, filePath, rotAngle, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="filePath">Path to the bitmap to draw.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(int player, const Vector &centerPos, const std::string &filePath, float rotAngle, bool hFlipped, bool vFlipped);

		/// <summary>
		/// Schedule to draw the GUI icon of an object.
		/// </summary>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		void DrawIconPrimitive(const Vector &centerPos, Entity *entity) { DrawIconPrimitive(-1, centerPos, entity); }

		/// <summary>
		/// Schedule to draw the GUI icon of an object, visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		void DrawIconPrimitive(int player, const Vector &centerPos, Entity *entity);
#pragma endregion

	protected:

		std::mutex m_Mutex; //!< Mutex so that mutiple threads (i.e multithreaded scripts) can safely queue up draws
		std::deque<std::unique_ptr<GraphicalPrimitive>> m_ScheduledPrimitives; //!< List of graphical primitives scheduled to draw this frame, cleared every frame during FrameMan::Draw().

	private:
		/// <summary>
		/// Constructs a unique_ptr of the appropriate derived type from the passed in GraphicalPrimitive raw pointer.
		/// This is used for preparing primitives constructed in Lua for scheduling.
		/// </summary>
		/// <param name="primitive">Raw pointer to the GraphicalPrimitive object to make unique.</param>
		/// <returns>A unique_ptr of the appropriate derived GraphicalPrimitive type. Ownership is transferred!</returns>
		std::unique_ptr<GraphicalPrimitive> MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(GraphicalPrimitive *primitive);

		/// <summary>
		/// Safely schedules a primite to draw in a thread-safe manner.
		/// </summary>
		/// <param name="primitive">A unique ptr of the primitive to schedule for drawing.</param>
		void SchedulePrimitive(std::unique_ptr<GraphicalPrimitive>&& primitive);

		// Disallow the use of some implicit methods.
		PrimitiveMan(const PrimitiveMan &reference) = delete;
		PrimitiveMan & operator=(const PrimitiveMan &rhs) = delete;
	};
}
#endif