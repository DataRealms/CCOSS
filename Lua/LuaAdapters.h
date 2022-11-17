#ifndef _RTELUAADAPTERS_
#define _RTELUAADAPTERS_

#include "MovableMan.h"
#include "ConsoleMan.h"

#include "HeldDevice.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

namespace RTE {

#pragma region Manager Lua Adapters
	/// <summary>
	/// Adds the given MovableObject to MovableMan if it doesn't already exist in there, or prints an error if it does.
	/// </summary>
	/// <param name="movableMan">A reference to MovableMan, provided by Lua.</param>
	/// <param name="movableObject">A pointer to the MovableObject to be added.</param>
	static void AddMO(MovableMan &movableMan, MovableObject *movableObject) {
		if (movableMan.ValidMO(movableObject)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a MovableObject that already exists in the simulation! " + movableObject->GetPresetName());
		} else {
			movableMan.AddMO(movableObject);
		}
	}

	/// <summary>
	/// Adds the given Actor to MovableMan if it doesn't already exist in there, or prints an error if it does.
	/// </summary>
	/// <param name="movableMan">A reference to MovableMan, provided by Lua.</param>
	/// <param name="actor">A pointer to the Actor to be added.</param>
	static void AddActor(MovableMan &movableMan, Actor *actor) {
		if (movableMan.IsActor(actor)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Actor that already exists in the simulation!" + actor->GetPresetName());
		} else {
			movableMan.AddActor(actor);
		}
	}

	/// <summary>
	/// Adds the given item MovableObject (generally a HeldDevice) to MovableMan if it doesn't already exist in there, or prints an error if it does.
	/// </summary>
	/// <param name="movableMan">A reference to MovableMan, provided by Lua.</param>
	/// <param name="item">A pointer to the item to be added.</param>
	static void AddItem(MovableMan &movableMan, HeldDevice *item) {
		if (movableMan.ValidMO(dynamic_cast<MovableObject *>(item))) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Item that already exists in the simulation!" + item->GetPresetName());
		} else {
			movableMan.AddItem(item);
		}
	}

	/// <summary>
	/// Adds the given particle MovableObject to MovableMan if it doesn't already exist in there, or prints an error if it does.
	/// </summary>
	/// <param name="movableMan">A reference to MovableMan, provided by Lua.</param>
	/// <param name="particle">A pointer to the pctor to be added.</param>
	static void AddParticle(MovableMan &movableMan, MovableObject *particle) {
		if (movableMan.ValidMO(particle)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a Particle that already exists in the simulation!" + particle->GetPresetName());
		} else {
			movableMan.AddParticle(particle);
		}
	}

	/// <summary>
	/// Gets the number of ticks per second. Lua can't handle int64 (or long long apparently) so we'll expose this specialized function.
	/// </summary>
	/// <returns>The number of ticks per second.</returns>
	static double GetTicksPerSecond(const TimerMan &timerMan) {
		return static_cast<double>(timerMan.GetTicksPerSecond());
	}

	/// <summary>
	/// Gets whether a mouse button is being held down right now.
	/// </summary>
	/// <param name="whichButton">Which button to check for.</param>
	/// <returns>Whether the mouse button is held or not.</returns>
	static bool MouseButtonHeld(const UInputMan &uinputMan, int whichButton) { return uinputMan.MouseButtonHeld(Players::PlayerOne, whichButton); }

	/// <summary>
	/// Gets whether a mouse button was pressed between the last update and the one previous to it.
	/// </summary>
	/// <param name="whichButton">Which button to check for.</param>
	/// <returns>Whether the mouse button is pressed or not.</returns>
	static bool MouseButtonPressed(const UInputMan &uinputMan, int whichButton) { return uinputMan.MouseButtonPressed(Players::PlayerOne, whichButton); }

	/// <summary>
	/// Gets whether a mouse button was released between the last update and the one previous to it.
	/// </summary>
	/// <param name="whichButton">Which button to check for.</param>
	/// <returns>Whether the mouse button is released or not.</returns>
	static bool MouseButtonReleased(const UInputMan &uinputMan, int whichButton) { return uinputMan.MouseButtonReleased(Players::PlayerOne, whichButton); }

	/// <summary>
	/// Reloads the specified Entity preset in PresetMan.
	/// </summary>
	/// <param name="presetName">The preset name of the Entity to reload.</param>
	/// <param name="className">The class name of the Entity to reload.</param>
	/// <param name="moduleName">The module name of the Entity to reload.</param>
	/// <returns>Whether or not the Entity was reloaded.</returns>
	static bool ReloadEntityPreset1(PresetMan &presetMan, const std::string &presetName, const std::string &className, const std::string &moduleName) {
		return presetMan.ReloadEntityPreset(presetName, className, moduleName);
	}

	/// <summary>
	/// Reloads the specified Entity preset in PresetMan.
	/// </summary>
	/// <param name="presetName">The preset name of the Entity to reload.</param>
	/// <param name="className">The class name of the Entity to reload.</param>
	/// <returns>Whether or not the Entity was reloaded.</returns>
	static bool ReloadEntityPreset2(PresetMan &presetMan, const std::string &presetName, const std::string &className) {
		return ReloadEntityPreset1(presetMan, presetName, className, "");
	}
#pragma endregion

#pragma region PrimitiveMan Lua Adapters
	/// <summary>
	/// Schedule to draw a polygon primitive.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
	/// <param name="color">Color to draw primitive with.</param>
	/// <param name="vertices">A vector that contains the positions of the primitive's vertices, relative to the center position.</param>
	static void DrawPolygonPrimitive(PrimitiveMan &primitiveMan, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonPrimitive(-1, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable));
	}

	/// <summary>
	/// Schedule to draw a polygon primitive visible only to a specified player.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="player">Player screen to draw primitive on.</param>
	/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
	/// <param name="color">Color to draw primitive with.</param>
	/// <param name="vertices">A vector that contains the positions of the primitive's vertices, relative to the center position.</param>
	static void DrawPolygonPrimitiveForPlayer(PrimitiveMan &primitiveMan, int player, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonPrimitive(player, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable));
	}

	/// <summary>
	/// Schedule to draw a filled polygon primitive.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
	/// <param name="color">Color to draw primitive with.</param>
	/// <param name="vertices">A vector that contains the positions of the primitive's vertices, relative to the center position.</param>
	static void DrawPolygonFillPrimitive(PrimitiveMan &primitiveMan, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonFillPrimitive(-1, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable));
	}

	/// <summary>
	/// Schedule to draw a filled polygon primitive visible only to a specified player.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="player">Player screen to draw primitive on.</param>
	/// <param name="centerPos">Position of primitive's center in scene coordinates.</param>
	/// <param name="color">Color to draw primitive with.</param>
	/// <param name="vertices">A vector that contains the positions of the primitive's vertices, relative to the center position.</param>
	static void DrawPolygonFillPrimitiveForPlayer(PrimitiveMan &primitiveMan, int player, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonFillPrimitive(player, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable));
	}

	/// <summary>
	/// Schedules to draw multiple primitives of varying type with transparency enabled.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="transValue">The transparency value the primitives should be drawn at. From 0 (opaque) to 100 (transparent).</param>
	/// <param name="primitivesTable">A Lua table of primitives to schedule drawing for.</param>
	static void DrawPrimitivesWithTransparency(PrimitiveMan &primitiveMan, int transValue, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(DrawBlendMode::BlendTransparency, transValue, transValue, transValue, BlendAmountLimits::MinBlend, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}

	/// <summary>
	/// Schedule to draw multiple primitives of varying type with blending enabled.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="blendMode">The blending mode the primitives should be drawn with. See DrawBlendMode enumeration.</param>
	/// <param name="blendAmount">The blending amount for all the channels. 0-100.</param>
	/// <param name="primitivesTable">A Lua table of primitives to schedule drawing for.</param>
	static void DrawPrimitivesWithBlending(PrimitiveMan &primitiveMan, int blendMode, int blendAmount, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(static_cast<DrawBlendMode>(blendMode), blendAmount, blendAmount, blendAmount, blendAmount, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}

	/// <summary>
	/// Schedule to draw multiple primitives of varying type with blending enabled.
	/// </summary>
	/// <param name="primitiveMan">A reference to PrimitiveMan, provided by Lua.</param>
	/// <param name="blendMode">The blending mode the primitives should be drawn with. See DrawBlendMode enumeration.</param>
	/// <param name="blendAmountR">The blending amount for the Red channel. 0-100.</param>
	/// <param name="blendAmountG">The blending amount for the Green channel. 0-100.</param>
	/// <param name="blendAmountB">The blending amount for the Blue channel. 0-100.</param>
	/// <param name="blendAmountA">The blending amount for the Alpha channel. 0-100.</param>
	/// <param name="primitivesTable">A Lua table of primitives to schedule drawing for.</param>
	static void DrawPrimitivesWithBlendingPerChannel(PrimitiveMan &primitiveMan, int blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(static_cast<DrawBlendMode>(blendMode), blendAmountR, blendAmountG, blendAmountB, blendAmountA, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}
#pragma endregion

#pragma region Misc Lua Adapters
	/// <summary>
	/// Gets a random number between -1 and 1.
	/// </summary>
	/// <returns>A random number between -1 and 1.</returns>
	static double NormalRand() {
		return RandomNormalNum<double>();
	}

	/// <summary>
	/// Gets a random number between 0 and 1.
	/// </summary>
	/// <returns>A random number between 0 and 1.</returns>
	static double PosRand() {
		return RandomNum<double>();
	}

	/// <summary>
	/// Gets a random number between 1 and the passed in upper limit number. The limits are included in the available random range.
	/// </summary>
	/// <param name="upperLimitInclusive">The upper limit for the random number.</param>
	/// <returns>A random number between 1 and the passed in number.</returns>
	static double LuaRand(double upperLimitInclusive) {
		return RandomNum<double>(1, upperLimitInclusive);
	}

	/// <summary>
	/// Explicit deletion of any Entity instance that Lua owns. It will probably be handled by the GC, but this makes it instantaneous.
	/// </summary>
	/// <param name="entityToDelete">The Entity to delete.</param>
	static void DeleteEntity(Entity *entityToDelete) {
		delete entityToDelete;
		entityToDelete = nullptr;
	}
#pragma endregion
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
#endif