#ifndef _RTELUAREGISTERDEFINITIONS_
#define _RTELUAREGISTERDEFINITIONS_

#include "LuabindDefinitions.h"
#include "LuaAdapterDefinitions.h"

namespace RTE {

// Should be ordered with most-derived classes first
#define LIST_OF_LUABOUND_OBJECTS 					\
	/* SystemLuaBindings */							\
		PER_LUA_BINDING(Box)						\
		PER_LUA_BINDING(Controller)					\
		PER_LUA_BINDING(DataModule)					\
		PER_LUA_BINDING(Timer)						\
		PER_LUA_BINDING(Vector)						\
		PER_LUA_BINDING(PathRequest)				\
	/* ManagerLuaBindings */						\
		PER_LUA_BINDING(ActivityMan)				\
		PER_LUA_BINDING(AudioMan)					\
		PER_LUA_BINDING(CameraMan)					\
		PER_LUA_BINDING(ConsoleMan)					\
		PER_LUA_BINDING(FrameMan)					\
		PER_LUA_BINDING(MetaMan)					\
		PER_LUA_BINDING(MovableMan)					\
		PER_LUA_BINDING(PostProcessMan)				\
		PER_LUA_BINDING(PresetMan)					\
		PER_LUA_BINDING(PrimitiveMan)				\
		PER_LUA_BINDING(SceneMan)					\
		PER_LUA_BINDING(SettingsMan)				\
		PER_LUA_BINDING(TimerMan)					\
		PER_LUA_BINDING(UInputMan)					\
	/* EntityLuaBindings */							\
		/* ThrownDevice-Derived */					\
		PER_LUA_BINDING(TDExplosive)				\
		/* HeldDevice-Derived */					\
		PER_LUA_BINDING(HDFirearm)					\
		PER_LUA_BINDING(ThrownDevice)				\
		/* AEmitter-Derived */						\
		PER_LUA_BINDING(AEJetpack)					\
		/* ACraft-Derived */						\
		PER_LUA_BINDING(ACDropShip)					\
		PER_LUA_BINDING(ACRocket)					\
		/* Attachable-Derived */					\
		PER_LUA_BINDING(Arm)						\
		PER_LUA_BINDING(Leg)						\
		PER_LUA_BINDING(AEmitter)					\
		PER_LUA_BINDING(HeldDevice)					\
		PER_LUA_BINDING(Magazine)					\
		PER_LUA_BINDING(Turret)						\
		/* Actor-Derived */							\
		PER_LUA_BINDING(ACrab)						\
		PER_LUA_BINDING(ACraft)						\
		PER_LUA_BINDING(AHuman)						\
		PER_LUA_BINDING(ADoor)						\
		/* MOSRotating-Derived */					\
		PER_LUA_BINDING(Actor)						\
		PER_LUA_BINDING(Attachable)					\
		/* MOSParticle-Derived */					\
		PER_LUA_BINDING(PEmitter)					\
		/* MOSprite-Derived */						\
		PER_LUA_BINDING(MOSRotating)				\
		PER_LUA_BINDING(MOSParticle)				\
		/* MovableObject-Derived */					\
		PER_LUA_BINDING(MOPixel)					\
		PER_LUA_BINDING(MOSprite)					\
		/* SceneObject-Derived */					\
		PER_LUA_BINDING(TerrainObject)				\
		PER_LUA_BINDING(Deployment)					\
		PER_LUA_BINDING(MovableObject)				\
		/* Entity-Derived */						\
		PER_LUA_BINDING(SoundContainer)				\
		PER_LUA_BINDING(PieSlice)					\
		PER_LUA_BINDING(GlobalScript)				\
		PER_LUA_BINDING(Emission)					\
		PER_LUA_BINDING(LimbPath)					\
		PER_LUA_BINDING(PieMenu)					\
		PER_LUA_BINDING(Round)						\
		PER_LUA_BINDING(Scene)						\
		PER_LUA_BINDING(Scene::Area)				\
		PER_LUA_BINDING(Material)					\
		PER_LUA_BINDING(MetaPlayer)					\
		PER_LUA_BINDING(SceneObject)				\
		/* SceneLayer-Derived */					\
		PER_LUA_BINDING(SLBackground)				\
		/* Base Classes */							\
		PER_LUA_BINDING(SoundSet)					\
		PER_LUA_BINDING(Gib)						\
		PER_LUA_BINDING(SceneLayer)					\
		PER_LUA_BINDING(Entity)						\
	/* ActivityLuaBindings */						\
		PER_LUA_BINDING(GameActivity)				\
		PER_LUA_BINDING(Activity)					\
	/* GUILuaBindings */							\
		PER_LUA_BINDING(BuyMenuGUI)					\
		PER_LUA_BINDING(SceneEditorGUI)				\
		PER_LUA_BINDING(GUIBanner)					\
	/* PrimitiveLuaBindings */						\
		/* GraphicalPrimitive-Derived */			\
		PER_LUA_BINDING(LinePrimitive)				\
		PER_LUA_BINDING(ArcPrimitive)				\
		PER_LUA_BINDING(SplinePrimitive)			\
		PER_LUA_BINDING(BoxPrimitive)				\
		PER_LUA_BINDING(BoxFillPrimitive)			\
		PER_LUA_BINDING(RoundedBoxPrimitive)		\
		PER_LUA_BINDING(RoundedBoxFillPrimitive)	\
		PER_LUA_BINDING(CirclePrimitive)			\
		PER_LUA_BINDING(CircleFillPrimitive)		\
		PER_LUA_BINDING(EllipsePrimitive)			\
		PER_LUA_BINDING(EllipseFillPrimitive)		\
		PER_LUA_BINDING(TrianglePrimitive)			\
		PER_LUA_BINDING(TriangleFillPrimitive)		\
		PER_LUA_BINDING(TextPrimitive)				\
		PER_LUA_BINDING(BitmapPrimitive)			\
		/* Base Classes */							\
		PER_LUA_BINDING(GraphicalPrimitive)			\
	/* InputLuaBindings */							\
		PER_LUA_BINDING(InputDevice)				\
		PER_LUA_BINDING(InputElements)				\
		PER_LUA_BINDING(MouseButtons)				\
		PER_LUA_BINDING(JoyButtons)					\
		PER_LUA_BINDING(JoyDirections)				\
		PER_LUA_BINDING(SDL_Scancode)				\
		PER_LUA_BINDING(SDL_Keycode)				\
		PER_LUA_BINDING(SDL_GameControllerButton)	\
		PER_LUA_BINDING(SDL_GameControllerAxis)		\
	/* MiscLuaBindings */							\
		PER_LUA_BINDING(AlarmEvent)					\
		PER_LUA_BINDING(Directions)					\
		PER_LUA_BINDING(DrawBlendMode)
	
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
	#define AbstractTypeLuaClassDefinition(TYPE, PARENTTYPE) \
		luabind::class_<TYPE, PARENTTYPE>(#TYPE) \
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// Convenience macro for a LuaBind scope definition of a concrete type.
	/// </summary>
	#define ConcreteTypeLuaClassDefinition(TYPE, PARENTTYPE) \
		luabind::class_<TYPE, PARENTTYPE>(#TYPE)													\
			.def("Clone", &LuaAdaptersEntityClone::Clone##TYPE, luabind::adopt(luabind::result))	\
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// Convenience macro for calling a register function of a type.
	/// </summary>
	#define RegisterLuaBindingsOfType(OWNINGSCOPE, TYPE) \
		OWNINGSCOPE::Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for calling a register function of an abstract type, along with registering global bindings for adapters relevant to the type.
	/// </summary>
	#define RegisterLuaBindingsOfAbstractType(OWNINGSCOPE, TYPE) \
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&LuaAdaptersEntityCast::To##TYPE),						\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&LuaAdaptersEntityCast::ToConst##TYPE),		\
		luabind::def((std::string("Is") + std::string(#TYPE)).c_str(), (bool(*)(const Entity *))&LuaAdaptersEntityCast::Is##TYPE),																\
		OWNINGSCOPE::Register##TYPE##LuaBindings()

	/// <summary>
	/// Convenience macro for calling a register function of a concrete type, along with registering global bindings for adapters relevant to the type.
	/// </summary>
	#define RegisterLuaBindingsOfConcreteType(OWNINGSCOPE, TYPE) \
		luabind::def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&LuaAdaptersEntityCreate::Create##TYPE, luabind::adopt(luabind::result)),		\
		luabind::def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&LuaAdaptersEntityCreate::Create##TYPE, luabind::adopt(luabind::result)),					\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, int))&LuaAdaptersEntityCreate::Random##TYPE, luabind::adopt(luabind::result)),				\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&LuaAdaptersEntityCreate::Random##TYPE, luabind::adopt(luabind::result)),		\
		luabind::def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&LuaAdaptersEntityCreate::Random##TYPE, luabind::adopt(luabind::result)),					\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&LuaAdaptersEntityCast::To##TYPE),																	\
		luabind::def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&LuaAdaptersEntityCast::ToConst##TYPE),													\
		luabind::def((std::string("Is") + std::string(#TYPE)).c_str(), (bool(*)(const Entity *))&LuaAdaptersEntityCast::Is##TYPE),																\
		OWNINGSCOPE::Register##TYPE##LuaBindings()
#pragma endregion

	/// <summary>
	/// Struct that contains Lua binding registration functions for System classes.
	/// </summary>
	struct SystemLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Box);
		LuaBindingRegisterFunctionDeclarationForType(Controller);
		LuaBindingRegisterFunctionDeclarationForType(DataModule);
		LuaBindingRegisterFunctionDeclarationForType(Timer);
		LuaBindingRegisterFunctionDeclarationForType(Vector);
		LuaBindingRegisterFunctionDeclarationForType(PathRequest);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for Manager classes.
	/// </summary>
	struct ManagerLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(ActivityMan);
		LuaBindingRegisterFunctionDeclarationForType(AudioMan);
		LuaBindingRegisterFunctionDeclarationForType(CameraMan);
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
		LuaBindingRegisterFunctionDeclarationForType(AEJetpack);
		LuaBindingRegisterFunctionDeclarationForType(AHuman);
		LuaBindingRegisterFunctionDeclarationForType(Arm);
		LuaBindingRegisterFunctionDeclarationForType(Attachable);
		LuaBindingRegisterFunctionDeclarationForType(Deployment);
		LuaBindingRegisterFunctionDeclarationForType(Emission);
		LuaBindingRegisterFunctionDeclarationForType(Gib);
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
		LuaBindingRegisterFunctionDeclarationForType(PieSlice);
		LuaBindingRegisterFunctionDeclarationForType(PieMenu);
		LuaBindingRegisterFunctionDeclarationForType(Round);
		LuaBindingRegisterFunctionDeclarationForType(Scene);
		LuaBindingRegisterFunctionDeclarationForType(SceneArea);
		LuaBindingRegisterFunctionDeclarationForType(SceneLayer);
		LuaBindingRegisterFunctionDeclarationForType(SceneObject);
		LuaBindingRegisterFunctionDeclarationForType(SLBackground);
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
	/// Struct that contains Lua binding registration functions for GraphicalPrimitive classes.
	/// </summary>
	struct PrimitiveLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(GraphicalPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(LinePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(ArcPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(SplinePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BoxPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BoxFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(RoundedBoxPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(RoundedBoxFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(CirclePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(CircleFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(EllipsePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(EllipseFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TrianglePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TriangleFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TextPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BitmapPrimitive);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for various input enumerations.
	/// </summary>
	struct InputLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(InputDevice);
		LuaBindingRegisterFunctionDeclarationForType(InputElements);
		LuaBindingRegisterFunctionDeclarationForType(MouseButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyDirections);
		LuaBindingRegisterFunctionDeclarationForType(SDL_Scancode);
		LuaBindingRegisterFunctionDeclarationForType(SDL_Keycode);
		LuaBindingRegisterFunctionDeclarationForType(SDL_GameControllerButton);
		LuaBindingRegisterFunctionDeclarationForType(SDL_GameControllerAxis);
	};

	/// <summary>
	/// Struct that contains Lua binding registration functions for types that don't really belong in any of the other binding structs.
	/// </summary>
	struct MiscLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(AlarmEvent);
		LuaBindingRegisterFunctionDeclarationForType(Directions);
		LuaBindingRegisterFunctionDeclarationForType(DrawBlendMode);
	};
}
#endif
