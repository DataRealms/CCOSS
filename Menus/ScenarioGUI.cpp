#include "ScenarioGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "GameActivity.h"
#include "Entity.h"
#include "Scene.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUIComboBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::Clear() {
		m_UpdateResult = ScenarioMenuUpdateResult::NoEvent;

		m_ScenarioActivities.clear();
		m_SelectedActivity = nullptr;

		m_ActivityScenes = nullptr;
		m_SelectedScene = nullptr;
		m_HoveredScene = nullptr;

		m_PlanetCenter.Reset();
		m_PlanetRadius = 0;
		m_LineToSitePoints.clear();

		m_DraggedBox = nullptr;
		m_PrevMousePos.Reset();

		m_BlinkTimer.Reset();
		m_ScenePreviewAnimTimer.Reset();

		m_DefaultScenePreview.Reset();
		m_DrawDefaultScenePreview = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::Create(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/ScenarioGUI.ini");

		m_RootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		m_RootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		m_ActivityConfigBoxRootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfigRoot"));
		m_ActivityConfigBoxRootBox->Resize(m_RootBox->GetWidth(), m_RootBox->GetHeight());
		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("BackToMainButton"));
		m_BackToMainButton->SetPositionRel(m_RootBox->GetWidth() - m_BackToMainButton->GetWidth() - 16, m_RootBox->GetHeight() - m_BackToMainButton->GetHeight() - 22);
		m_ResumeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonResume"));
		m_ResumeButton->SetPositionRel(m_RootBox->GetWidth() - m_ResumeButton->GetWidth() - 16, m_RootBox->GetHeight() - m_ResumeButton->GetHeight() - 47);

		m_SitePointNameLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSceneNameOnPlanet"));

		CreateActivityInfoBox();
		CreateSceneInfoBox();

		m_ActivityConfigBox = std::make_unique<ScenarioActivityConfigGUI>(m_GUIControlManager.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::CreateActivityInfoBox() {
		m_ActivityInfoBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxActivityInfo"));
		m_ActivityInfoBox->SetPositionRel(16, 16);
		m_ActivitySelectComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboBoxActivitySelect"));
		m_ActivitySelectComboBox->Move(m_ActivityInfoBox->GetXPos() + 8, m_ActivityInfoBox->GetYPos() + 25);
		m_ActivityDescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelActivityDescription"));
		m_ActivityDescriptionLabel->SetFont(m_GUIControlManager->GetSkin()->GetFont("FontSmall.png"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::CreateSceneInfoBox() {
		m_SceneInfoBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSceneInfo"));
		m_SceneInfoBox->SetPositionRel(m_RootBox->GetWidth() - m_SceneInfoBox->GetWidth() - 16, 16);
		m_SceneBoxCloseButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCloseSceneBox"));
		m_SceneNameLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSceneName"));
		m_SceneDescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSceneDescription"));
		m_SceneDescriptionLabel->SetFont(m_GUIControlManager->GetSkin()->GetFont("FontSmall.png"));
		m_ScenePreviewImageBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxScenePreview"));
		m_StartActivityConfigButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonStartActivityConfig"));

		m_DefaultScenePreview.Create(ContentFile("Base.rte/GUIs/DefaultPreview.png"), 5);
		m_DefaultScenePreview.SetSpriteAnimMode(MOSprite::SpriteAnimMode::ALWAYSLOOP);
		m_DefaultScenePreview.SetSpriteAnimDuration(200);
		m_DefaultScenePreview.SetPos(Vector(static_cast<float>(m_ScenePreviewImageBox->GetXPos() + (m_ScenePreviewImageBox->GetWidth() / 2)), static_cast<float>(m_ScenePreviewImageBox->GetYPos() + (m_ScenePreviewImageBox->GetHeight() / 2))));

		m_ScenePreviewBitmap = std::make_unique<AllegroBitmap>();
		m_ScenePreviewBitmap->Create(c_ScenePreviewWidth, c_ScenePreviewHeight, 32);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetEnabled(const Vector &center, float radius) {
		bool centerChanged = (center != m_PlanetCenter);
		m_PlanetCenter = center;
		m_PlanetRadius = radius;
		if (centerChanged) { CalculateLinesToSitePoint(); }

		FetchActivitiesAndScenesLists();

		// Only show the resume button if the current Activity is a GameActivity. Editor or Multiplayer Activities are resumed from the main menu, so the resume button shouldn't show for them.
		const GameActivity *currentActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
		m_ResumeButton->SetVisible(currentActivity && (currentActivity->GetActivityState() == Activity::Running || currentActivity->GetActivityState() == Activity::Editing));

		m_ActivityInfoBox->SetVisible(true);

		m_ScenePreviewAnimTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetDraggedBox(int mouseX, int mouseY) {
		GUICollectionBox *hoveredBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControlUnderPoint(mouseX, mouseY, m_RootBox, 1));
		const GUIControl *hoveredControl = m_GUIControlManager->GetControlUnderPoint(mouseX, mouseY, hoveredBox, 1);
		bool nonDragControl = (dynamic_cast<const GUIButton *>(hoveredControl) || dynamic_cast<const GUIComboBox *>(hoveredControl));
		if (hoveredBox && !nonDragControl && !m_DraggedBox && !m_ActivitySelectComboBox->IsDropped()) { m_DraggedBox = hoveredBox; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetSelectedActivity(const Activity *newSelectedActivity) {
		m_SelectedActivity = newSelectedActivity;
		m_ActivityScenes = nullptr;

		if (m_SelectedActivity) {
			for (auto &[activity, sceneList] : m_ScenarioActivities) {
				if (activity == m_SelectedActivity) {
					m_ActivityScenes = &sceneList;
					break;
				}
			}
			if (m_ActivityScenes && m_ActivityScenes->size() == 1) {
				m_ActivityDescriptionLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
			} else if (m_ActivityScenes && m_ActivityScenes->size() > 1) {
				m_ActivityDescriptionLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
			} else {
				m_ActivityDescriptionLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nNo sites appear to be compatible with this selected activity! Please try another.");
			}
			SetSelectedScene((m_ActivityScenes && m_ActivityScenes->size() == 1) ? m_ActivityScenes->front() : nullptr);
		} else {
			m_ActivityDescriptionLabel->SetText("No Activity selected.");
		}
		m_ActivityInfoBox->Resize(m_ActivityInfoBox->GetWidth(), m_ActivityDescriptionLabel->ResizeHeightToFit() + 60);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetSelectedScene(Scene *newSelectedScene) {
		m_SelectedScene = newSelectedScene;
		if (m_SelectedScene) {
			m_SceneInfoBox->SetVisible(true);

			if (BITMAP *preview = m_SelectedScene->GetPreviewBitmap()) {
				clear_to_color(m_ScenePreviewBitmap->GetBitmap(), ColorKeys::g_MaskColor);
				draw_sprite(m_ScenePreviewBitmap->GetBitmap(), preview, 0, 0);
				m_ScenePreviewImageBox->SetDrawImage(new AllegroBitmap(m_ScenePreviewBitmap->GetBitmap()));
				m_DrawDefaultScenePreview = false;
			} else {
				m_DrawDefaultScenePreview = true;
			}

			m_SceneNameLabel->SetText(m_SelectedScene->GetPresetName());
			m_SceneDescriptionLabel->SetText(m_SelectedScene->GetDescription());

			// TODO: Some stupidity going on so have to do this twice, otherwise everything resizes correctly but some description text gets cut off as if it didn't.
			for (int i = 0; i < 2; ++i) {
				m_SceneInfoBox->Resize(m_SceneInfoBox->GetWidth(), m_SceneDescriptionLabel->ResizeHeightToFit() + 152);
			}
			CalculateLinesToSitePoint();
		} else {
			m_SceneInfoBox->SetVisible(false);
			m_LineToSitePoints.clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::DragBox(int mouseX, int mouseY) {
		if (m_DraggedBox) {
			m_DraggedBox->MoveRelative(mouseX - m_PrevMousePos.GetFloorIntX(), mouseY - m_PrevMousePos.GetFloorIntY());
			m_DraggedBox->SetPositionAbs(std::clamp(m_DraggedBox->GetXPos(), 0, m_RootBox->GetWidth() - m_DraggedBox->GetWidth()), std::clamp(m_DraggedBox->GetYPos(), 0, m_RootBox->GetHeight() - m_DraggedBox->GetHeight()));
			if (m_DraggedBox == m_ActivityInfoBox) {
				// The Activity ComboBox isn't a child of the Activity info box (dirty hack to allow the drop-down list to extend beyond the parent box bounds without clipping) so we need to move as well.
				m_ActivitySelectComboBox->Move(m_ActivityInfoBox->GetXPos() + 8, m_ActivityInfoBox->GetYPos() + 25);
			} else if (m_DraggedBox == m_SceneInfoBox) {
				// The default preview "bitmap" isn't actually a bitmap and isn't a child of the Scene info box so we need to move it as well.
				m_DefaultScenePreview.SetPos(Vector(static_cast<float>(m_ScenePreviewImageBox->GetXPos() + (m_ScenePreviewImageBox->GetWidth() / 2)), static_cast<float>(m_ScenePreviewImageBox->GetYPos() + (m_ScenePreviewImageBox->GetHeight() / 2))));
				CalculateLinesToSitePoint();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::FetchActivitiesAndScenesLists() {
		int prevSelectedActivityIndex = m_ActivitySelectComboBox->GetSelectedIndex();
		Scene *prevSelectedScene = m_SelectedScene;

		m_ActivitySelectComboBox->ClearList();
		m_ScenarioActivities.clear();
		m_ActivityScenes = nullptr;

		std::list<Entity *> presetList;
		g_PresetMan.GetAllOfType(presetList, "Scene");
		std::vector<Scene *> filteredScenes;
		for (Entity *presetEntity : presetList) {
			Scene *presetScene = dynamic_cast<Scene *>(presetEntity);
			if (presetScene && !presetScene->GetLocation().IsZero() && !presetScene->IsMetagameInternal() && (presetScene->GetMetasceneParent().empty() || g_SettingsMan.ShowMetascenes())) { filteredScenes.emplace_back(presetScene); }
		}
		AdjustSitePointOffsetsOnPlanet(filteredScenes);

		presetList.clear();
		g_PresetMan.GetAllOfType(presetList, "Activity");
		int index = 0;
		for (Entity *presetEntity : presetList) {
			if (GameActivity *presetActivity = dynamic_cast<GameActivity *>(presetEntity)) {
				std::pair<Activity *, std::vector<Scene *>> activityAndCompatibleScenes(presetActivity, std::vector<Scene *>());
				for (Scene *filteredScene : filteredScenes) {
					if (presetActivity->SceneIsCompatible(filteredScene)) { activityAndCompatibleScenes.second.emplace_back(filteredScene); }
				}
				m_ScenarioActivities.insert(activityAndCompatibleScenes);
				// Add to the activity selection ComboBox and attach the activity pointer, not passing in ownership.
				m_ActivitySelectComboBox->AddItem(presetActivity->GetPresetName(), "", nullptr, presetActivity);

				if (prevSelectedActivityIndex < 0 && presetActivity->GetClassName() == "GATutorial") {
					prevSelectedActivityIndex = index;
					prevSelectedScene = nullptr;
				}
				index++;
			}
		}
		if (prevSelectedActivityIndex >= 0) {
			m_ActivitySelectComboBox->SetSelectedIndex(prevSelectedActivityIndex);
			SetSelectedActivity(dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity));
			if (prevSelectedScene) { SetSelectedScene(prevSelectedScene); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::AdjustSitePointOffsetsOnPlanet(const std::vector<Scene *> &sceneList) const {
		for (Scene *sceneListEntry : sceneList) {
			int sceneYPos = (m_PlanetCenter + sceneListEntry->GetLocation() + sceneListEntry->GetLocationOffset()).GetFloorIntY();
			if (std::abs(sceneListEntry->GetLocation().GetY()) < m_PlanetRadius + 100 && std::abs(sceneListEntry->GetLocation().GetX()) < m_PlanetRadius + 100) {
				if (sceneYPos < 10) {
					sceneListEntry->SetLocationOffset(sceneListEntry->GetLocationOffset() + Vector(0, static_cast<float>(10 - sceneYPos)));
				} else if (sceneYPos > g_FrameMan.GetResY() - 10) {
					sceneListEntry->SetLocationOffset(sceneListEntry->GetLocationOffset() + Vector(0, static_cast<float>(g_FrameMan.GetResY() - 10 - sceneYPos)));
				} else {
					sceneListEntry->SetLocationOffset(Vector(0, 0));
				}
			}
		}
		// If site points are overlapping then move one of them towards the planet center.
		float requiredDistance = 8.0F;
		bool foundOverlap = true;
		while (foundOverlap) {
			foundOverlap = false;
			for (Scene *sceneListEntry1 : sceneList) {
				for (const Scene *sceneListEntry2 : sceneList) {
					if (sceneListEntry1 != sceneListEntry2) {
						Vector pos1 = sceneListEntry1->GetLocation() + sceneListEntry1->GetLocationOffset();
						Vector pos2 = sceneListEntry2->GetLocation() + sceneListEntry2->GetLocationOffset();
						Vector overlap = pos1 - pos2;
						float overlapMagnitude = overlap.GetMagnitude();
						if (overlapMagnitude < requiredDistance) {
							foundOverlap = true;
							float overlapX = overlap.GetX();
							float xDirMult = 0;
							if (overlapX > 0 && pos1.GetX() > 0) {
								xDirMult = -1.0F;
							} else if (overlapX < 0 && pos1.GetX() < 0) {
								xDirMult = 1.0F;
							}
							float overlapY = overlap.GetY();
							float yDirMult = 0;
							if (overlapY > 0 && pos1.GetY() > 0) {
								yDirMult = -1.0F;
							} else if (overlapY < 0 && pos1.GetY() < 0) {
								yDirMult = 1.0F;
							}
							if (yDirMult != 0) {
								sceneListEntry1->SetLocationOffset(sceneListEntry1->GetLocationOffset() + Vector(-overlapX + (requiredDistance * xDirMult), -overlapY + (requiredDistance * yDirMult)));
							} else if (overlapMagnitude == 0.0F) {
								sceneListEntry1->SetLocationOffset(sceneListEntry1->GetLocationOffset() + Vector((pos1.GetX() > 0) ? -requiredDistance : requiredDistance, (pos1.GetY() > 0) ? -requiredDistance : requiredDistance));
							} else {
								sceneListEntry1->SetLocationOffset(sceneListEntry1->GetLocationOffset() + Vector(overlapX, overlapY));
							}
						}
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::CalculateLinesToSitePoint() {
		m_LineToSitePoints.clear();
		if (!m_SelectedScene) {
			return;
		}

		Vector sitePos = m_PlanetCenter + m_SelectedScene->GetLocation() + m_SelectedScene->GetLocationOffset();
		if (m_SceneInfoBox->PointInside(sitePos.GetFloorIntX(), sitePos.GetFloorIntY())) {
			return;
		}

		int halfBoxHeight = m_SceneInfoBox->GetHeight() / 2;
		Vector sceneBoxCenter(static_cast<float>(m_SceneInfoBox->GetXPos() + (m_SceneInfoBox->GetWidth() / 2)), static_cast<float>(m_SceneInfoBox->GetYPos() + halfBoxHeight));
		float yDirMult = sitePos.GetY() < sceneBoxCenter.GetY() ? -1.0F : 1.0F;
		Vector sceneBoxEdge = sceneBoxCenter + Vector(0, static_cast<float>(halfBoxHeight)) * yDirMult;

		int circleRadius = 8;
		int minStraightLength = 15;
		// Minimum distance from a chamfer point to the site point.
		int minSiteDistance = circleRadius + minStraightLength;

		// No bends, meaning the line goes straight up/down to the site circle.
		if (std::abs(sceneBoxCenter.GetFloorIntX() - sitePos.GetFloorIntX()) < minSiteDistance) {
			m_LineToSitePoints.emplace_back(sceneBoxEdge + Vector(sitePos.GetX() - sceneBoxEdge.GetX(), 0));
			m_LineToSitePoints.emplace_back(sitePos + Vector(0, (static_cast<float>(circleRadius + 1)) * -yDirMult));
			return;
		}
		m_LineToSitePoints.emplace_back(sceneBoxEdge);

		int chamferSize = 0;
		int minChamferSize = 15;
		int maxChamferSize = 40;
		float xDirMult = sitePos.GetX() < sceneBoxEdge.GetX() ? -1.0F : 1.0F;

		// One bend. At this point the line bends. If the bend is chamfered then the two chamfer points will be equally distanced from this point.
		if (std::abs(sitePos.GetFloorIntY() - sceneBoxCenter.GetFloorIntY()) > halfBoxHeight + minStraightLength) {
			Vector bendPoint(sceneBoxEdge.GetX(), sitePos.GetY());

			chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(sceneBoxEdge.GetFloorIntY() - sitePos.GetFloorIntY()) - minStraightLength);
			chamferSize = std::min(chamferSize, maxChamferSize);
			if (chamferSize < minChamferSize) { chamferSize = 0; }

			m_LineToSitePoints.emplace_back(Vector(bendPoint.GetX(), bendPoint.GetY() + static_cast<float>(chamferSize) * -yDirMult));
			if (chamferSize > 0) { m_LineToSitePoints.emplace_back(Vector(bendPoint.GetX() + static_cast<float>(chamferSize) * xDirMult, bendPoint.GetY())); }
			m_LineToSitePoints.emplace_back(sitePos + Vector((static_cast<float>(circleRadius + 1)) * -xDirMult, 0));
		} else {
			// Two bends. extraLength ensures that there will be straight lines coming out of the site and the box, and that they are nearly as short as possible.
			int extraLength = std::clamp(minSiteDistance + (sitePos.GetFloorIntY() - sceneBoxEdge.GetFloorIntY()) * static_cast<int>(yDirMult), 0, minSiteDistance);

			Vector firstBend(sceneBoxEdge.GetX(), sceneBoxEdge.GetY() + (static_cast<float>(extraLength + minStraightLength)) * yDirMult);
			Vector secondBend(sitePos.GetX(), firstBend.GetY());

			chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(secondBend.GetFloorIntY() - sitePos.GetFloorIntY()) - minSiteDistance);
			chamferSize = std::min(chamferSize, maxChamferSize);
			if (chamferSize < minChamferSize) { chamferSize = 0; }

			m_LineToSitePoints.emplace_back(firstBend);
			m_LineToSitePoints.emplace_back(Vector(secondBend.GetX() + static_cast<float>(chamferSize) * -xDirMult, secondBend.GetY()));
			if (chamferSize > 0) { m_LineToSitePoints.emplace_back(Vector(secondBend.GetX(), secondBend.GetY() + static_cast<float>(chamferSize) * -yDirMult)); }
			m_LineToSitePoints.emplace_back(sitePos + Vector(0, (static_cast<float>(circleRadius + 1)) * yDirMult));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioGUI::ScenarioMenuUpdateResult ScenarioGUI::Update() {
		m_UpdateResult = ScenarioMenuUpdateResult::NoEvent;

		if (g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) {
			return m_UpdateResult;
		}
		if (g_UInputMan.KeyPressed(KEY_ESC)) {
			g_GUISound.BackButtonPressSound()->Play();
			if (m_ActivityConfigBox->IsEnabled()) {
				m_ActivityConfigBox->SetEnabled(false);
			} else {
				return ScenarioMenuUpdateResult::BackToMain;
			}
		}

		int mousePosX;
		int mousePosY;
		m_GUIControlManager->GetManager()->GetInputController()->GetMousePosition(&mousePosX, &mousePosY);
		m_GUIControlManager->Update();

		if (!m_ActivityConfigBox->IsEnabled()) {
			m_RootBox->SetVisible(true);
			m_ActivityConfigBoxRootBox->SetVisible(false);

			UpdateHoveredSitePointLabel(mousePosX, mousePosY);
			HandleInputEvents(mousePosX, mousePosY);

			if (m_SceneInfoBox->GetVisible()) {
				m_StartActivityConfigButton->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <");

				if (m_DrawDefaultScenePreview && m_ScenePreviewAnimTimer.GetElapsedRealTimeMS() > m_DefaultScenePreview.GetSpriteAnimDuration() / m_DefaultScenePreview.GetFrameCount()) {
					m_DefaultScenePreview.SetNextFrame();
					m_ScenePreviewAnimTimer.Reset();
				}
			}
			if (m_ResumeButton->GetVisible()) { m_GUIControlManager->GetManager()->SetFocus((m_BlinkTimer.AlternateReal(500)) ? m_ResumeButton : nullptr); }
		} else {
			m_RootBox->SetVisible(false);
			m_ActivityConfigBoxRootBox->SetVisible(true);

			if (m_ActivityConfigBox->Update(mousePosX, mousePosY)) { m_UpdateResult = ScenarioMenuUpdateResult::ActivityStarted; }
		}
		return m_UpdateResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::UpdateHoveredSitePointLabel(int mouseX, int mouseY) {
		bool foundAnyHover = false;
		if (m_ActivityScenes && !m_DraggedBox && !m_ActivityInfoBox->PointInside(mouseX, mouseY) && !m_SceneInfoBox->PointInside(mouseX, mouseY)) {
			Scene *candidateScene = nullptr;
			float shortestDist = 10.0F;
			for (Scene *activityScene : *m_ActivityScenes) {
				float distance = (m_PlanetCenter + activityScene->GetLocation() + activityScene->GetLocationOffset() - Vector(static_cast<float>(mouseX), static_cast<float>(mouseY))).GetMagnitude();
				if (distance < shortestDist) {
					shortestDist = distance;
					candidateScene = activityScene;
					foundAnyHover = true;
				}
			}
			if (candidateScene && candidateScene != m_HoveredScene) {
				g_GUISound.SelectionChangeSound()->Play();
				m_HoveredScene = candidateScene;

				m_SitePointNameLabel->SetText(m_HoveredScene->GetPresetName());
				Vector sceneLabelPos = m_PlanetCenter + Vector(m_HoveredScene->GetLocation() + m_HoveredScene->GetLocationOffset()) - Vector(static_cast<float>(m_SitePointNameLabel->GetWidth() / 2), 0) - Vector(0, static_cast<float>(m_SitePointNameLabel->GetHeight()) * 1.5F);
				int padding = 5;
				sceneLabelPos.SetX(static_cast<float>(std::clamp(sceneLabelPos.GetFloorIntX(), padding, g_FrameMan.GetResX() - m_SitePointNameLabel->GetWidth() - padding)));
				sceneLabelPos.SetY(static_cast<float>(std::clamp(sceneLabelPos.GetFloorIntY(), padding, g_FrameMan.GetResY() - m_SitePointNameLabel->GetHeight() - padding)));
				m_SitePointNameLabel->SetPositionAbs(sceneLabelPos.GetFloorIntX(), sceneLabelPos.GetFloorIntY());
				m_SitePointNameLabel->SetVisible(true);
			}
		}
		if (!foundAnyHover) {
			m_HoveredScene = nullptr;
			m_SitePointNameLabel->SetVisible(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::HandleInputEvents(int mouseX, int mouseY) {
		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					g_GUISound.BackButtonPressSound()->Play();
					m_UpdateResult = ScenarioMenuUpdateResult::BackToMain;
				} else if (guiEvent.GetControl() == m_ResumeButton) {
					g_GUISound.BackButtonPressSound()->Play();
					m_UpdateResult = ScenarioMenuUpdateResult::ActivityResumed;
				} else if (guiEvent.GetControl() == m_SceneBoxCloseButton) {
					g_GUISound.ButtonPressSound()->Play();
					SetSelectedScene(nullptr);
				} else if (guiEvent.GetControl() == m_StartActivityConfigButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_ActivityConfigBox->SetEnabled(true, m_SelectedActivity, m_SelectedScene);
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<const GUIButton *>(guiEvent.GetControl())) { g_GUISound.SelectionChangeSound()->Play(); }

				if (guiEvent.GetMsg() == GUIComboBox::Closed && guiEvent.GetControl() == m_ActivitySelectComboBox) {
					g_GUISound.ItemChangeSound()->Play();
					SetSelectedActivity((m_ActivitySelectComboBox->GetSelectedItem()) ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr);
				}
			}
		}
		if (g_UInputMan.MouseButtonPressed(UInputMan::MenuCursorButtons::MENU_PRIMARY)) {
			if (m_HoveredScene) {
				g_GUISound.ItemChangeSound()->Play();
				SetSelectedScene(m_HoveredScene);
			} else {
				SetDraggedBox(mouseX, mouseY);
			}
		} else if (g_UInputMan.MouseButtonReleased(UInputMan::MenuCursorButtons::MENU_PRIMARY)) {
			m_DraggedBox = nullptr;
		}
		if (g_UInputMan.MenuButtonHeld(UInputMan::MenuCursorButtons::MENU_PRIMARY)) { DragBox(mouseX, mouseY); }
		m_PrevMousePos.SetXY(static_cast<float>(mouseX), static_cast<float>(mouseY));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::Draw() const {
		// Early return to avoid single frame flicker before title screen goes into fadeout.
		if (m_UpdateResult == ScenarioMenuUpdateResult::ActivityStarted) {
			return;
		}
		if (!m_ActivityConfigBox->IsEnabled()) {
			if (m_ActivityScenes) {
				drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
				DrawSitePoints(g_FrameMan.GetBackBuffer32());
				if (m_SelectedScene && m_SceneInfoBox->GetVisible()) { DrawLinesToSitePoint(g_FrameMan.GetBackBuffer32()); }
				drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
			}
			m_GUIControlManager->Draw();
			if (m_DrawDefaultScenePreview && m_SceneInfoBox->GetVisible()) { m_DefaultScenePreview.Draw(g_FrameMan.GetBackBuffer32()); }
		} else {
			m_ActivityConfigBox->Draw();
		}
		m_GUIControlManager->DrawMouse();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::DrawSitePoints(BITMAP *drawBitmap) const {
		int blendAmount = 0;
		for (const Scene *scenePointer : *m_ActivityScenes) {
			int drawColor = 0;
			if (scenePointer->GetModuleID() == g_PresetMan.GetModuleID("Base.rte")) {
				drawColor = c_GUIColorYellow;
			} else if (scenePointer->GetModuleID() == g_PresetMan.GetModuleID("Missions.rte")) {
				drawColor = c_GUIColorGreen;
			} else {
				drawColor = c_GUIColorCyan;
			}
			Vector sitePos(m_PlanetCenter + scenePointer->GetLocation() + scenePointer->GetLocationOffset());
			int sitePosX = sitePos.GetFloorIntX();
			int sitePosY = sitePos.GetFloorIntY();

			blendAmount = 70 + RandomNum(0, 40);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, sitePosX, sitePosY, 4, drawColor);
			circlefill(drawBitmap, sitePosX, sitePosY, 2, drawColor);

			blendAmount = 145 + RandomNum(0, 110);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, sitePosX, sitePosY, 1, drawColor);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::DrawLinesToSitePoint(BITMAP *drawBitmap) const {
		int blendAmount = 0;
		int drawColor = c_GUIColorWhite;

		if (!m_LineToSitePoints.empty()) {
			for (int i = 0; i < m_LineToSitePoints.size() - 1; i++) {
				int lineStartX = m_LineToSitePoints.at(i).GetFloorIntX();
				int lineStartY = m_LineToSitePoints.at(i).GetFloorIntY();
				int lineEndX = m_LineToSitePoints.at(i + 1).GetFloorIntX();
				int lineEndY = m_LineToSitePoints.at(i + 1).GetFloorIntY();

				blendAmount = 195 + RandomNum(0, 30);
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				line(drawBitmap, lineStartX, lineStartY, lineEndX, lineEndY, drawColor);

				blendAmount = 30 + RandomNum(0, 50);
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				line(drawBitmap, lineStartX + 1, lineStartY, lineEndX + 1, lineEndY, drawColor);
				line(drawBitmap, lineStartX - 1, lineStartY, lineEndX - 1, lineEndY, drawColor);
				line(drawBitmap, lineStartX, lineStartY + 1, lineEndX, lineEndY + 1, drawColor);
				line(drawBitmap, lineStartX, lineStartY - 1, lineEndX, lineEndY - 1, drawColor);
			}
		}
		Vector sitePos = m_PlanetCenter + m_SelectedScene->GetLocation() + m_SelectedScene->GetLocationOffset();
		int sitePosX = sitePos.GetFloorIntX();
		int sitePosY = sitePos.GetFloorIntY();
		int circleRadius = 8;

		blendAmount = 195 + RandomNum(0, 30);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
		circle(drawBitmap, sitePosX, sitePosY, circleRadius, drawColor);

		blendAmount = 120 + RandomNum(0, 50);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
		circle(drawBitmap, sitePosX, sitePosY, circleRadius - 1, drawColor);
	}
}