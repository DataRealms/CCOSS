#ifndef _RTELUAADAPTERS_
#define _RTELUAADAPTERS_

#include "MovableMan.h"
#include "ConsoleMan.h"

namespace RTE {

#pragma region Manager Lua Adapters
	static void AddMO(MovableMan &movableMan, MovableObject *movableObject) {
		if (movableMan.ValidMO(movableObject)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a MovableObject that already exists in the simulation! " + movableObject->GetPresetName());
		} else {
			movableMan.AddMO(movableObject);
		}
	}

	static void AddActor(MovableMan &movableMan, Actor *actor) {
		if (movableMan.IsActor(actor)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Actor that already exists in the simulation!" + actor->GetPresetName());
		} else {
			movableMan.AddActor(actor);
		}
	}

	static void AddItem(MovableMan &movableMan, MovableObject *item) {
		if (movableMan.ValidMO(item)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Item that already exists in the simulation!" + item->GetPresetName());
		} else {
			movableMan.AddItem(item);
		}
	}

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
#pragma endregion

#pragma region Misc Lua Adapters
	static double NormalRand() {
		return RandomNormalNum<double>();
	}

	static double PosRand() {
		return RandomNum<double>();
	}

	static double LuaRand(double num) {
		return RandomNum<double>(1, num);
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
#endif