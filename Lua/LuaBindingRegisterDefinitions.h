#ifndef _RTELUAREGISTERDEFINITIONS_
#define _RTELUAREGISTERDEFINITIONS_

#include "LuaBindDefinitions.h"

#include "GameActivity.h"
#include "MetaPlayer.h"
#include "SLTerrain.h"

#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

#include "ActivityMan.h"
#include "AudioMan.h"
#include "ConsoleMan.h"
#include "FrameMan.h"
#include "MetaMan.h"
#include "MovableMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "TimerMan.h"
#include "UInputMan.h"

#include "Box.h"
#include "Controller.h"
#include "DataModule.h"
#include "PieSlice.h"

namespace RTE {

#pragma region Lua Binding Registration Macros
	/// <summary>
	/// Convenience macro for declaring a binding register function.
	/// </summary>
	#define LuaBindingRegisterFunctionDeclarationForType(TYPE) \
		static luabind::scope Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for defining a binding register function.
	/// </summary>
	#define LuaBindingRegisterFunctionDefinitionForType(OWNINGSCOPE, TYPE) \
		luabind::scope OWNINGSCOPE::Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for a LuaBind scope definition of an abstract type.
	/// </summary>
	#define AbstractTypeLuaClassDefinition(TYPE, PARENTTYPE)	\
		luabind::class_<TYPE, PARENTTYPE>(#TYPE)				\
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// Convenience macro for a LuaBind scope definition of a concrete type.
	/// </summary>
	#define ConcreteTypeLuaClassDefinition(TYPE, PARENTTYPE)				\
		luabind::class_<TYPE, PARENTTYPE>(#TYPE)							\
			.def("Clone", &Clone##TYPE, luabind::adopt(luabind::result))	\
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// Convenience macro for calling a register function of a type.
	/// </summary>
	#define RegisterLuaBindingsOfType(OWNINGSCOPE, TYPE) \
		OWNINGSCOPE::Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for calling a register function of an abstract type, along with registering global bindings for adapters relevant to the type.
	/// </summary>
	#define RegisterLuaBindingsOfAbstractType(OWNINGSCOPE, TYPE)															\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&To##TYPE),						\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&ToConst##TYPE),	\
		OWNINGSCOPE::Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for calling a register function of a concrete type, along with registering global bindings for adapters relevant to the type.
	/// </summary>
	#define RegisterLuaBindingsOfConcreteType(OWNINGSCOPE, TYPE)																									\
		luabind::def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&Create##TYPE, luabind::adopt(luabind::result)),	\
		luabind::def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&Create##TYPE, luabind::adopt(luabind::result)),					\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, int))&Random##TYPE, luabind::adopt(luabind::result)),			\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&Random##TYPE, luabind::adopt(luabind::result)),	\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&Random##TYPE, luabind::adopt(luabind::result)),					\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&To##TYPE),																\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&ToConst##TYPE),											\
		luabind::def((std::string("Is") + std::string(#TYPE)).c_str(), (bool(*)(const Entity *))&Is##TYPE),															\
		OWNINGSCOPE::Register##TYPE##LuaBindings()
#pragma endregion

	/// <summary>
	/// Struct that contains Lua binding registration functions for System classes.
	/// </summary>
	struct SystemLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Box);
		LuaBindingRegisterFunctionDeclarationForType(Controller);
		LuaBindingRegisterFunctionDeclarationForType(DataModule);
		LuaBindingRegisterFunctionDeclarationForType(PieSlice);
		LuaBindingRegisterFunctionDeclarationForType(Timer);
		LuaBindingRegisterFunctionDeclarationForType(Vector);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for Manager classes.
	/// </summary>
	struct ManagerLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(ActivityMan);
		LuaBindingRegisterFunctionDeclarationForType(AudioMan);
		LuaBindingRegisterFunctionDeclarationForType(ConsoleMan);
		LuaBindingRegisterFunctionDeclarationForType(FrameMan);
		LuaBindingRegisterFunctionDeclarationForType(MetaMan);
		LuaBindingRegisterFunctionDeclarationForType(MovableMan);
		LuaBindingRegisterFunctionDeclarationForType(PostProcessMan);
		LuaBindingRegisterFunctionDeclarationForType(PresetMan);
		LuaBindingRegisterFunctionDeclarationForType(PrimitiveMan);
		LuaBindingRegisterFunctionDeclarationForType(SceneMan);
		LuaBindingRegisterFunctionDeclarationForType(SettingsMan);
		LuaBindingRegisterFunctionDeclarationForType(TimerMan);
		LuaBindingRegisterFunctionDeclarationForType(UInputMan);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for Entity classes.
	/// </summary>
	struct EntityLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Entity);
		LuaBindingRegisterFunctionDeclarationForType(ACDropShip);
		LuaBindingRegisterFunctionDeclarationForType(ACrab);
		LuaBindingRegisterFunctionDeclarationForType(ACraft);
		LuaBindingRegisterFunctionDeclarationForType(ACRocket);
		LuaBindingRegisterFunctionDeclarationForType(Actor);
		LuaBindingRegisterFunctionDeclarationForType(ADoor);
		LuaBindingRegisterFunctionDeclarationForType(AEmitter);
		LuaBindingRegisterFunctionDeclarationForType(AHuman);
		LuaBindingRegisterFunctionDeclarationForType(Arm);
		LuaBindingRegisterFunctionDeclarationForType(Attachable);
		LuaBindingRegisterFunctionDeclarationForType(Deployment);
		LuaBindingRegisterFunctionDeclarationForType(Emission);
		LuaBindingRegisterFunctionDeclarationForType(GlobalScript);
		LuaBindingRegisterFunctionDeclarationForType(HDFirearm);
		LuaBindingRegisterFunctionDeclarationForType(HeldDevice);
		LuaBindingRegisterFunctionDeclarationForType(Leg);
		LuaBindingRegisterFunctionDeclarationForType(LimbPath);
		LuaBindingRegisterFunctionDeclarationForType(Magazine);
		LuaBindingRegisterFunctionDeclarationForType(Material);
		LuaBindingRegisterFunctionDeclarationForType(MetaPlayer);
		LuaBindingRegisterFunctionDeclarationForType(MOPixel);
		LuaBindingRegisterFunctionDeclarationForType(MOSParticle);
		LuaBindingRegisterFunctionDeclarationForType(MOSprite);
		LuaBindingRegisterFunctionDeclarationForType(MOSRotating);
		LuaBindingRegisterFunctionDeclarationForType(MovableObject);
		LuaBindingRegisterFunctionDeclarationForType(PEmitter);
		LuaBindingRegisterFunctionDeclarationForType(Round);
		LuaBindingRegisterFunctionDeclarationForType(Scene);
		LuaBindingRegisterFunctionDeclarationForType(SceneArea);
		LuaBindingRegisterFunctionDeclarationForType(SceneObject);
		LuaBindingRegisterFunctionDeclarationForType(SoundContainer);
		LuaBindingRegisterFunctionDeclarationForType(SoundSet);
		LuaBindingRegisterFunctionDeclarationForType(TDExplosive);
		LuaBindingRegisterFunctionDeclarationForType(TerrainObject);
		LuaBindingRegisterFunctionDeclarationForType(ThrownDevice);
		LuaBindingRegisterFunctionDeclarationForType(Turret);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for Activity classes.
	/// </summary>
	struct ActivityLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Activity);
		LuaBindingRegisterFunctionDeclarationForType(GameActivity);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for GUI classes.
	/// </summary>
	struct GUILuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(GUIBanner);
		LuaBindingRegisterFunctionDeclarationForType(BuyMenuGUI);
		LuaBindingRegisterFunctionDeclarationForType(SceneEditorGUI);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for types that don't really belong in any of the other binding structs.
	/// </summary>
	struct MiscLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(AlarmEvent);
		LuaBindingRegisterFunctionDeclarationForType(InputDevice);
		LuaBindingRegisterFunctionDeclarationForType(InputElements);
		LuaBindingRegisterFunctionDeclarationForType(MouseButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyDirections);
	};
}
#endif