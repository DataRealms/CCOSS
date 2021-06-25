#ifndef _RTELUABINDGUIS_
#define _RTELUABINDGUIS_

#include "LuaMacros.h"

#include "GameActivity.h"
#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	struct GUILuaBindings {

		LuaBindingRegisterFunctionForType(GUIBanner) {
			return luabind::class_<GUIBanner>("GUIBanner")
				.enum_("AnimMode")[
					value("BLINKING", GUIBanner::AnimMode::BLINKING),
					value("FLYBYLEFTWARD", GUIBanner::AnimMode::FLYBYLEFTWARD),
					value("FLYBYRIGHTWARD", GUIBanner::AnimMode::FLYBYRIGHTWARD),
					value("ANIMMODECOUNT", GUIBanner::AnimMode::ANIMMODECOUNT)
				]
				.enum_("AnimState")[
					value("NOTSTARTED", GUIBanner::AnimState::NOTSTARTED),
					value("SHOWING", GUIBanner::AnimState::SHOWING),
					value("SHOW", GUIBanner::AnimState::SHOW),
					value("HIDING", GUIBanner::AnimState::HIDING),
					value("OVER", GUIBanner::AnimState::OVER),
					value("ANIMSTATECOUNT", GUIBanner::AnimState::ANIMSTATECOUNT)
				]
				.enum_("BannerColor")[
					value("RED", GameActivity::BannerColor::RED),
					value("YELLOW", GameActivity::BannerColor::YELLOW)
				]
				.property("BannerText", &GUIBanner::GetBannerText)
				.property("AnimState", &GUIBanner::GetAnimState)
				.def("IsVisible", &GUIBanner::IsVisible)
				.property("Kerning", &GUIBanner::GetKerning, &GUIBanner::SetKerning)
				.def("ShowText", &GUIBanner::ShowText)
				.def("HideText", &GUIBanner::HideText)
				.def("ClearText", &GUIBanner::ClearText);
		}

		LuaBindingRegisterFunctionForType(BuyMenuGUI) {
			return luabind::class_<BuyMenuGUI>("BuyMenuGUI")
				.def("SetMetaPlayer", &BuyMenuGUI::SetMetaPlayer)
				.def("SetNativeTechModule", &BuyMenuGUI::SetNativeTechModule)
				.def("SetForeignCostMultiplier", &BuyMenuGUI::SetForeignCostMultiplier)
				.def("SetModuleExpanded", &BuyMenuGUI::SetModuleExpanded)
				.def("LoadAllLoadoutsFromFile", &BuyMenuGUI::LoadAllLoadoutsFromFile)
				.def("AddAllowedItem", &BuyMenuGUI::AddAllowedItem)
				.def("RemoveAllowedItem", &BuyMenuGUI::RemoveAllowedItem)
				.def("ClearAllowedItems", &BuyMenuGUI::ClearAllowedItems)
				.def("AddAlwaysAllowedItem", &BuyMenuGUI::AddAlwaysAllowedItem)
				.def("RemoveAlwaysAllowedItem", &BuyMenuGUI::RemoveAlwaysAllowedItem)
				.def("ClearAlwaysAllowedItems", &BuyMenuGUI::ClearAlwaysAllowedItems)
				.def("AddProhibitedItem", &BuyMenuGUI::AddProhibitedItem)
				.def("RemoveProhibitedItem", &BuyMenuGUI::RemoveProhibitedItem)
				.def("ClearProhibitedItems", &BuyMenuGUI::ClearProhibitedItems)
				.def("ForceRefresh", &BuyMenuGUI::ForceRefresh)
				.def("SetOwnedItemsAmount", &BuyMenuGUI::SetOwnedItemsAmount)
				.def("GetOwnedItemsAmount", &BuyMenuGUI::GetOwnedItemsAmount)
				.def("SetHeaderImage", &BuyMenuGUI::SetHeaderImage)
				.def("SetLogoImage", &BuyMenuGUI::SetLogoImage)
				.def("ClearCartList", &BuyMenuGUI::ClearCartList)
				.def("LoadDefaultLoadoutToCart", &BuyMenuGUI::LoadDefaultLoadoutToCart)
				.property("ShowOnlyOwnedItems", &BuyMenuGUI::GetOnlyShowOwnedItems, &BuyMenuGUI::SetOnlyShowOwnedItems)
				.property("EnforceMaxPassengersConstraint", &BuyMenuGUI::EnforceMaxPassengersConstraint, &BuyMenuGUI::SetEnforceMaxPassengersConstraint)
				.property("EnforceMaxMassConstraint", &BuyMenuGUI::EnforceMaxMassConstraint, &BuyMenuGUI::SetEnforceMaxMassConstraint);
		}

		LuaBindingRegisterFunctionForType(SceneEditorGUI) {
			return luabind::class_<SceneEditorGUI>("SceneEditorGUI")
				.enum_("EditorGUIMode")[
					value("INACTIVE", SceneEditorGUI::EditorGUIMode::INACTIVE),
					value("PICKINGOBJECT", SceneEditorGUI::EditorGUIMode::PICKINGOBJECT),
					value("ADDINGOBJECT", SceneEditorGUI::EditorGUIMode::ADDINGOBJECT),
					value("INSTALLINGBRAIN", SceneEditorGUI::EditorGUIMode::INSTALLINGBRAIN),
					value("PLACINGOBJECT", SceneEditorGUI::EditorGUIMode::PLACINGOBJECT),
					value("MOVINGOBJECT", SceneEditorGUI::EditorGUIMode::MOVINGOBJECT),
					value("DELETINGOBJECT", SceneEditorGUI::EditorGUIMode::DELETINGOBJECT),
					value("PLACEINFRONT", SceneEditorGUI::EditorGUIMode::PLACEINFRONT),
					value("PLACEBEHIND", SceneEditorGUI::EditorGUIMode::PLACEBEHIND),
					value("DONEEDITING", SceneEditorGUI::EditorGUIMode::DONEEDITING),
					value("EDITORGUIMODECOUNT", SceneEditorGUI::EditorGUIMode::EDITORGUIMODECOUNT)
				]
				.def("SetCursorPos", &SceneEditorGUI::SetCursorPos)
				.property("EditorMode", &SceneEditorGUI::GetEditorGUIMode, &SceneEditorGUI::SetEditorGUIMode)
				.def("GetCurrentObject", &SceneEditorGUI::GetCurrentObject)
				.def("SetCurrentObject", &SceneEditorGUI::SetCurrentObject)
				.def("SetModuleSpace", &SceneEditorGUI::SetModuleSpace)
				.def("SetNativeTechModule", &SceneEditorGUI::SetNativeTechModule)
				.def("SetForeignCostMultiplier", &SceneEditorGUI::SetForeignCostMultiplier)
				.def("TestBrainResidence", &SceneEditorGUI::TestBrainResidence);
		}
	};
}
#endif