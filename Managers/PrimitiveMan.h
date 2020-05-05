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
		/// <param name="pTargetBitmap">Bitmap to draw on.</param>
		/// <param name="targetPos">Position to draw.</param>
		void DrawPrimitives(short player, BITMAP *pTargetBitmap, const Vector &targetPos);

		/// <summary>
		/// Schedule to draw a line primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new LinePrimitive(start, end, color)); }

		/// <summary>
		/// Schedule to draw a line primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawLinePrimitive(short player, Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new LinePrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a box primitive.
		/// </summary>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxPrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxPrimitive(start, end, color)); }

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
		void DrawBoxFillPrimitive(Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(start, end, color)); }

		/// <summary>
		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="end">End position of primitive in scene coordinates.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawBoxFillPrimitive(short player, Vector start, Vector end, unsigned char color) { m_Primitives.push_back(new BoxFillPrimitive(player, start, end, color)); }

		/// <summary>
		/// Schedule to draw a circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(Vector pos, short radius, unsigned char color) {m_Primitives.push_back(new CirclePrimitive(pos, radius, color));}

		/// <summary>
		/// Schedule to draw a circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to draw primitive with.</param>
		void DrawCirclePrimitive(short player, Vector pos, short radius, unsigned char color) {m_Primitives.push_back(new CirclePrimitive(player, pos, radius, color));}

		/// <summary>
		/// Schedule to draw a filled circle primitive.
		/// </summary>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(Vector pos, short radius, unsigned char color) {m_Primitives.push_back(new CircleFillPrimitive(pos, radius, color));}

		/// <summary>
		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="pos">Position of primitive in scene coordinates.</param>
		/// <param name="radius">Radius of circle primitive.</param>
		/// <param name="color">Color to fill primitive with.</param>
		void DrawCircleFillPrimitive(short player, Vector pos, short radius, unsigned char color) {m_Primitives.push_back(new CircleFillPrimitive(player, pos, radius, color));}

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
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(Vector start, Entity *entity, float rotAngle, unsigned short frame);

		/// <summary>
		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// </summary>
		/// <param name="player">Player screen to draw primitive on.</param>
		/// <param name="start">Start position of primitive in scene coordinates.</param>
		/// <param name="entity">An entity to draw sprite from.</param>
		/// <param name="rotAngle">Rotation angle in radians.</param>
		/// <param name="">Frame to draw.</param>
		void DrawBitmapPrimitive(short player, Vector start, Entity *entity, float rotAngle, unsigned short frame);
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