#include "CameraMan.h"

#include "Activity.h"
#include "FrameMan.h"
#include "Scene.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "SLTerrain.h"
#include "NetworkClient.h"

namespace RTE {

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::Clear() {
		m_ScreenShakeStrength = 1.0F;
		m_ScreenShakeDecay = 50.0F;
		m_MaxScreenShakeTime = 1.0F;
		m_DefaultShakePerUnitOfGibEnergy = 0.001F;
		m_DefaultShakePerUnitOfRecoilEnergy = 0.5F;
		m_DefaultShakeFromRecoilMaximum = 0.0F;

        for (Screen& screen : m_Screens) {
            screen.m_Offset.Reset();
            screen.m_DeltaOffset.Reset();
            screen.m_ScrollTarget.Reset();
            screen.m_ScreenTeam = Activity::NoTeam;
            screen.m_ScrollSpeed = 0.1F;
            screen.m_ScrollTimer.Reset();
            screen.m_ScreenOcclusion.Reset();
            screen.m_TargetWrapped = false;
            screen.m_SeamCrossCount[X] = 0;
            screen.m_SeamCrossCount[Y] = 0;
            screen.m_ScreenShakeMagnitude = 0.0F;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int CameraMan::Initialize() {
        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::Destroy() {
        Clear();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::Update(int screenId) {
        Screen& screen = m_Screens[screenId];
        const SLTerrain* terrain = g_SceneMan.GetScene()->GetTerrain();

        // Don't let our screen shake beyond our max
        screen.m_ScreenShakeMagnitude = std::min(screen.m_ScreenShakeMagnitude, m_ScreenShakeDecay * m_MaxScreenShakeTime);

        // Reduce screen shake over time
        screen.m_ScreenShakeMagnitude -= m_ScreenShakeDecay * screen.m_ScrollTimer.GetElapsedRealTimeS();
        screen.m_ScreenShakeMagnitude = std::max(screen.m_ScreenShakeMagnitude, 0.0F);

        // Feedback was that the best screenshake strength was between 25% and 40% of default
        // As such, we want the default setting to reflect that, instead the default setting being 30%
        // So just hard-coded multiply to make 100% in settings correspond to 30% here (much easier than rebalancing everything)
        const float screenShakeScale = 0.3f;

        Vector screenShakeOffset(1.0f, 0.0f);
        screenShakeOffset.RadRotate(RandomNormalNum() * c_PI);
        screenShakeOffset *= screen.m_ScreenShakeMagnitude * m_ScreenShakeStrength * screenShakeScale;

        if (g_TimerMan.DrawnSimUpdate()) {
            // Adjust for wrapping if the scroll target jumped a seam this frame, as reported by whatever screen set it (the scroll target) this frame. This is to avoid big, scene-wide jumps in scrolling when traversing the seam.
            if (screen.m_TargetWrapped) {
                if (terrain->WrapsX()) {
                    int wrappingScrollDirection = (screen.m_ScrollTarget.GetFloorIntX() < (terrain->GetBitmap()->w / 2)) ? 1 : -1;
                    screen.m_Offset.SetX(screen.m_Offset.GetX() - (static_cast<float>(terrain->GetBitmap()->w * wrappingScrollDirection)));
                    screen.m_SeamCrossCount[X] += wrappingScrollDirection;
                }
                if (terrain->WrapsY()) {
                    int wrappingScrollDirection = (screen.m_ScrollTarget.GetFloorIntY() < (terrain->GetBitmap()->h / 2)) ? 1 : -1;
                    screen.m_Offset.SetY(screen.m_Offset.GetY() - (static_cast<float>(terrain->GetBitmap()->h * wrappingScrollDirection)));
                    screen.m_SeamCrossCount[Y] += wrappingScrollDirection;
                }
            }
            screen.m_TargetWrapped = false;
        }
        
        Vector oldOffset(screen.m_Offset);

        Vector offsetTarget;
        if (g_FrameMan.IsInMultiplayerMode()) {
            offsetTarget.SetX(screen.m_ScrollTarget.GetX() - static_cast<float>(g_FrameMan.GetPlayerFrameBufferWidth(screenId) / 2));
            offsetTarget.SetY(screen.m_ScrollTarget.GetY() - static_cast<float>(g_FrameMan.GetPlayerFrameBufferHeight(screenId) / 2));
        }
        else {
            offsetTarget.SetX(screen.m_ScrollTarget.GetX() - static_cast<float>(g_FrameMan.GetResX() / (g_FrameMan.GetVSplit() ? 4 : 2)));
            offsetTarget.SetY(screen.m_ScrollTarget.GetY() - static_cast<float>(g_FrameMan.GetResY() / (g_FrameMan.GetHSplit() ? 4 : 2)));
        }
        // Take the occlusion of the screens into account so that the scroll target is still centered on the terrain-visible portion of the screen.
        offsetTarget -= (screen.m_ScreenOcclusion / 2);

        Vector newOffset = screen.m_Offset;
        if (offsetTarget.GetFloored() != screen.m_Offset.GetFloored()) {
            Vector scrollVec(offsetTarget - screen.m_Offset);
            float scrollProgress = std::min(1.0F, static_cast<float>(screen.m_ScrollSpeed * screen.m_ScrollTimer.GetElapsedRealTimeMS() * 0.05F));
            newOffset += scrollVec * scrollProgress;
        }

        newOffset += screenShakeOffset;
        SetOffset(newOffset, screenId);

        screen.m_DeltaOffset = screen.m_Offset - oldOffset;
        screen.m_ScrollTimer.Reset();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::SetOffset(const Vector& offset, int screenId) {
        m_Screens[screenId].m_Offset = offset.GetFloored();
        CheckOffset(screenId);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector CameraMan::GetUnwrappedOffset(int screenId) const {
        const Screen& screen = m_Screens[screenId];
        const SLTerrain* pTerrain = g_SceneMan.GetScene()->GetTerrain();
        return Vector(screen.m_Offset.GetX() + static_cast<float>(pTerrain->GetBitmap()->w * screen.m_SeamCrossCount[X]),
            screen.m_Offset.GetY() + static_cast<float>(pTerrain->GetBitmap()->h * screen.m_SeamCrossCount[Y]));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::SetScroll(const Vector &center, int screenId) {
        Screen& screen = m_Screens[screenId];
        if (g_FrameMan.IsInMultiplayerMode()) {
            screen.m_Offset.m_X = center.GetFloorIntX() - (g_FrameMan.GetPlayerFrameBufferWidth(screenId) / 2);
            screen.m_Offset.m_Y = center.GetFloorIntY() - (g_FrameMan.GetPlayerFrameBufferHeight(screenId) / 2);
        }
        else {
            screen.m_Offset.m_X = center.GetFloorIntX() - (g_FrameMan.GetResX() / 2);
            screen.m_Offset.m_Y = center.GetFloorIntY() - (g_FrameMan.GetResY() / 2);
        }

        CheckOffset(screenId);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::SetScrollTarget(const Vector &targetCenter, float speed, bool targetWrapped, int screenId) {
        Screen& screen = m_Screens[screenId];
        // See if it would make sense to automatically wrap
        if (!targetWrapped) {
            const SLTerrain* pTerrain = g_SceneMan.GetScene()->GetTerrain();
            // If the difference is more than half the scene width, then wrap
            if ((pTerrain->WrapsX() && fabs(targetCenter.m_X - screen.m_ScrollTarget.m_X) > pTerrain->GetBitmap()->w / 2) ||
                (pTerrain->WrapsY() && fabs(targetCenter.m_Y - screen.m_ScrollTarget.m_Y) > pTerrain->GetBitmap()->h / 2)) {
                targetWrapped = true;
            }
        }

        screen.m_ScrollTarget.m_X = targetCenter.m_X;
        screen.m_ScrollTarget.m_Y = targetCenter.m_Y;
        screen.m_ScrollSpeed = speed;

        // Don't override a set wrapping, it will be reset to false upon a drawn frame
        screen.m_TargetWrapped = screen.m_TargetWrapped || targetWrapped;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector CameraMan::GetScrollTarget(int screenId) const {
        const Vector& offsetTarget = (g_NetworkClient.IsConnectedAndRegistered()) ? g_NetworkClient.GetFrameTarget() : m_Screens[screenId].m_ScrollTarget;
        return offsetTarget;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float CameraMan::TargetDistanceScalar(const Vector &point) const {
        if (!g_SceneMan.GetScene()) {
            return 0.0F;
        }

        int screenCount = g_FrameMan.GetScreenCount();
        float screenRadius = static_cast<float>(std::max(g_FrameMan.GetPlayerScreenWidth(), g_FrameMan.GetPlayerScreenHeight())) / 2.0F;
        float sceneRadius = static_cast<float>(std::max(g_SceneMan.GetScene()->GetWidth(), g_SceneMan.GetScene()->GetHeight())) / 2.0F;

        // Avoid divide by zero problems if scene and screen radius are the same
        if (screenRadius == sceneRadius) {
            sceneRadius += 100.0F;
        }

        float closestScalar = 1.0F;

        for (const Screen& screen : m_Screens) {
            float distance = g_SceneMan.ShortestDistance(point, screen.m_ScrollTarget).GetMagnitude();

            float scalar = 0.0F;

            // Check if we're off the screen and then fall off
            if (distance > screenRadius) {
                // Get ratio of how close to the very opposite of the scene the point is
                scalar = 0.5F + 0.5F * (distance - screenRadius) / (sceneRadius - screenRadius);
            }
            // Full audio if within the screen
            else {
                scalar = 0.0F;
            }

            // See if this screen's distance scalar is the closest one yet
            if (scalar < closestScalar) {
                closestScalar = scalar;
            }
        }

        // Return the scalar that was shows the closest scroll target of any current screen to the point
        return closestScalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::CheckOffset(int screenId) {
        RTEAssert(g_SceneMan.GetScene(), "Trying to check offset before there is a scene or terrain!");

        Screen& screen = m_Screens[screenId];

        // Handy
        const SLTerrain* pTerrain = g_SceneMan.GetScene()->GetTerrain();
        RTEAssert(pTerrain, "Trying to get terrain matter before there is a scene or terrain!");

        if (!pTerrain->WrapsX() && screen.m_Offset.m_X < 0) {
            screen.m_Offset.m_X = 0.0F;
        }

        if (!pTerrain->WrapsY() && screen.m_Offset.m_Y < 0) {
            screen.m_Offset.m_Y = 0.0F;
        }

        Vector frameSize = GetFrameSize(screenId);
        int frameWidth = frameSize.GetX();
        int frameHeight = frameSize.GetY();

        if (!pTerrain->WrapsX() && screen.m_Offset.m_X >= pTerrain->GetBitmap()->w - frameWidth) {
            screen.m_Offset.m_X = pTerrain->GetBitmap()->w - frameWidth;
        }

        if (!pTerrain->WrapsY() && screen.m_Offset.m_Y >= pTerrain->GetBitmap()->h - frameHeight) {
            screen.m_Offset.m_Y = pTerrain->GetBitmap()->h - frameHeight;
        }

        if (!pTerrain->WrapsX() && screen.m_Offset.m_X >= pTerrain->GetBitmap()->w - frameWidth) {
            screen.m_Offset.m_X = pTerrain->GetBitmap()->w - frameWidth;
        }

        if (!pTerrain->WrapsY() && screen.m_Offset.m_Y >= pTerrain->GetBitmap()->h - frameHeight) {
            screen.m_Offset.m_Y = pTerrain->GetBitmap()->h - frameHeight;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector CameraMan::GetFrameSize(int screenId) {
        int frameWidth = g_FrameMan.GetResX();
        int frameHeight = g_FrameMan.GetResY();
        frameWidth = frameWidth / (g_FrameMan.GetVSplit() ? 2 : 1);
        frameHeight = frameHeight / (g_FrameMan.GetHSplit() ? 2 : 1);

        if (g_FrameMan.IsInMultiplayerMode())
        {
            frameWidth = g_FrameMan.GetPlayerFrameBufferWidth(screenId);
            frameHeight = g_FrameMan.GetPlayerFrameBufferHeight(screenId);
        }

        return Vector(frameWidth, frameHeight);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CameraMan::AddScreenShake(float magnitude, const Vector &position) {
        for (int screenId = 0; screenId < g_FrameMan.GetScreenCount(); ++screenId) {
            Screen& screen = m_Screens[screenId];

            Vector frameSize = GetFrameSize(screenId);

            Box screenBox(screen.m_Offset, frameSize.GetX(), frameSize.GetY());
            std::list<Box> wrappedBoxes;
            g_SceneMan.WrapBox(screenBox, wrappedBoxes);

            float closestDistanceFromScreen = std::numeric_limits<float>::max();
            for (const Box &box : wrappedBoxes) {
                // Determine how far the position is from the box
                Vector closestPointOnBox = box.GetWithinBox(position);
                Vector diff = closestPointOnBox - position;
                closestDistanceFromScreen = std::min(closestDistanceFromScreen, diff.GetMagnitude());
            }

            // Beyond this many screen's distance, no shake will be applied
            const float screenShakeFalloff = 0.3F;

            float screenDistance = std::max(frameSize.GetX(), frameSize.GetY()) * screenShakeFalloff;
            float screenShakeMultipler = std::max(1.0F - (closestDistanceFromScreen / screenDistance), 0.0F);
            screen.m_ScreenShakeMagnitude += magnitude * screenShakeMultipler;
        }
    }

}