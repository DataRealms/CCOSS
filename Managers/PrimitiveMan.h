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
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new LinePrimitive(-1, start, end, color)); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(short player, Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new LinePrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(Vector pos, float startAngle, float endAngle, short radius, unsigned char color) { m_Primitives.push_back(new ArcPrimitive(-1, pos, startAngle, endAngle, radius, 1, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive with the option to change thickness.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(Vector pos, float startAngle, float endAngle, short radius, unsigned char color, short thickness) { m_Primitives.push_back(new ArcPrimitive(-1, pos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawArcPrimitive(short player, Vector pos, float startAngle, float endAngle, short radius, unsigned char color) { m_Primitives.push_back(new ArcPrimitive(player, pos, startAngle, endAngle, radius, 1, color)); }

		/// <summary>
		/// Schedule to draw an arc primitive visible only to a specified player with the option to change thickness.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="startAngle">The angle from which the arc drawing begins.</param>
		/// <param name="endAngle">The angle at which the arc drawing ends.</param>
		/// <param name="radius">Radius of the arc primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		/// <param name="thickness">Thickness of the arc in pixels.</param>
		void DrawArcPrimitive(short player, Vector pos, float startAngle, float endAngle, short radius, unsigned char color, short thickness) { m_Primitives.push_back(new ArcPrimitive(player, pos, startAngle, endAngle, radius, thickness, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(Vector start, Vector guideA, Vector guideB, Vector end, unsigned char color) { m_Primitives.push_back(new SplinePrimitive(-1, start, guideA, guideB, end, color)); }

		/// <summary>
		/// Schedule to draw a Bezier spline primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="guideA">The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="guideB">The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawSplinePrimitive(short player, Vector start, Vector guideA, Vector guideB, Vector end, unsigned char color) { m_Primitives.push_back(new SplinePrimitive(player, start, guideA, guideB, end, color)); }

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxPrimitive(-1, start, end, color)); }

		/// <summary>
		/// Schedule to draw a box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(short player, Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxPrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(-1, start, end, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(short player, Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a round box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundBoxPrimitive(Vector start, Vector end, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundBoxPrimitive(-1, start, end, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a round box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundBoxPrimitive(short player, Vector start, Vector end, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundBoxPrimitive(player, start, end, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled round box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundBoxFillPrimitive(Vector start, Vector end, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundBoxFillPrimitive(-1, start, end, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled round box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="cornerRadius">The radius of the corners of the box. Smaller radius equals sharper corners.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawRoundBoxFillPrimitive(short player, Vector start, Vector end, float cornerRadius, unsigned char color) { m_Primitives.push_back(new RoundBoxFillPrimitive(player, start, end, cornerRadius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(Vector pos, short radius, unsigned char color) { m_Primitives.push_back(new CirclePrimitive(-1, pos, radius, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(short player, Vector pos, short radius, unsigned char color) { m_Primitives.push_back(new CirclePrimitive(player, pos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(Vector pos, short radius, unsigned char color) { m_Primitives.push_back(new CircleFillPrimitive(-1, pos, radius, color)); }

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(short player, Vector pos, short radius, unsigned char color) { m_Primitives.push_back(new CircleFillPrimitive(player, pos, radius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(Vector pos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipsePrimitive(-1, pos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw an ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawEllipsePrimitive(short player, Vector pos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipsePrimitive(player, pos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(Vector pos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipseFillPrimitive(-1, pos, horizRadius, vertRadius, color)); }

		/// <summary>
		/// Schedule to draw a filled ellipse primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="horizRadius">Horizontal radius of the ellipse primitive.</param>
		/// <param name="vertRadius">Vertical radius of the ellipse primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawEllipseFillPrimitive(short player, Vector pos, short horizRadius, short vertRadius, unsigned char color) { m_Primitives.push_back(new EllipseFillPrimitive(player, pos, horizRadius, vertRadius, color)); }

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
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		void DrawBitmapPrimitive(Vector start, Entity *entity, float rotAngle, unsigned short frame) { DrawBitmapPrimitive(-1, start, entity, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="frame">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(Vector start, Entity *entity, float rotAngle, unsigned short frame, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, start, entity, rotAngle, frame, hFlipped, vFlipped); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(short player, Vector start, Entity *entity, float rotAngle, unsigned short frame) { DrawBitmapPrimitive(player, start, entity, rotAngle, frame, false, false); }

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		/// <param name="hFlipped">Whether to flip the sprite horizontally.</param>
		/// <param name="vFlipped">Whether to flip the sprite vertically.</param>
		void DrawBitmapPrimitive(short player, Vector start, Entity *entity, float rotAngle, unsigned short frame, bool hFlipped, bool vFlipped);
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