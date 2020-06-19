//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActivityMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ActivityMan.h"
#include "Activity.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "FrameMan.h"
#include "AudioMan.h"
#include "ConsoleMan.h"
#include "GAScripted.h"

extern bool g_ResumeActivity;
extern bool g_InActivity;

namespace RTE {














		}





	}



		}
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ActivityMan, effectively
//                  resetting the members of this abstraction level only.

void ActivityMan::Clear()
{
    m_DefaultActivityType = "GATutorial";
    m_DefaultActivityName = "Tutorial Mission";
    m_pStartActivity = 0;
    m_pActivity = 0;
    m_LastMusicPath = "";
    m_LastMusicPos = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActivityMan object ready for use.

int ActivityMan::Create()
{
//    if (Serializable::Create() < 0)
//        return -1;

// Don't do this here, let RestartActivity be called separately
//    RestartActivity();

    return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActivityMan object.

void ActivityMan::Destroy(bool notInherited)
{
//    delete m_pStartActivity;
//    delete m_pActivity;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new activity for next restart, which can be customized and not
//                  recognized by PresetMan. You have to use RestartActivity to get it going.

void ActivityMan::SetStartActivity(Activity *pNewActivity)
{ 
    RTEAssert(pNewActivity, "Trying to replace an activity with a null one!");

    delete m_pStartActivity;
    m_pStartActivity = pNewActivity;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the activity passed in. Ownership IS transferred!

int ActivityMan::StartActivity(Activity *pActivity)
{
    RTEAssert(pActivity, "Trying to start a null activity!");

    int error = 0;

    // Stop all music, will be started by the Activity right below
    g_AudioMan.StopMusic();

    // Replace the start activity
    delete m_pStartActivity;
    m_pStartActivity = pActivity;

    // End and delete the old activity
// Don't end it, unnecessary causes endgame music playback problems
//    if (m_pActivity)
//        m_pActivity->End();
    delete m_pActivity;
    // Replace it with a clone of the start activity
    m_pActivity = dynamic_cast<Activity *>(m_pStartActivity->Clone());
    // Setup the players
    m_pActivity->SetupPlayers();
    // and START THAT BITCH
    error = m_pActivity->Start();

    if (error >= 0)
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was successfully started");
    else
    {
        g_ConsoleMan.PrintString("ERROR: Activity \"" + m_pActivity->GetPresetName() + "\" was NOT started due to errors!");
        m_pActivity->SetActivityState(Activity::INERROR);
        return error;
    }

    // Make sure the main menu and console exits and we're in the game when the activity starts
    g_ConsoleMan.SetEnabled(false);
    g_ResumeActivity = true;
    g_InActivity = true;

// Why was this being done?
//    g_MovableMan.Update();

    // Clear the post effects
	g_PostProcessMan.ClearScenePostEffects();

    // Clear the screen messages
    g_FrameMan.ClearScreenText();

    // Reset the mouse input to the center
    g_UInputMan.SetMouseValueMagnitude(0.05);
    
    // Reset the last music pos
    m_LastMusicPath = "";
    m_LastMusicPos = 0;

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially gets and starts the activity described.

int ActivityMan::StartActivity(string className, string instanceName)
{
    // Get the object instance first, make sure we can
    const Entity *pEntity = g_PresetMan.GetEntityPreset(className, instanceName);

    if (pEntity)
    {
        // Ok, then make a copy of it and get it going
        Activity *pNewActivity = dynamic_cast<Activity *>(pEntity->Clone());
        if (pNewActivity)
            return StartActivity(pNewActivity);
    }
    else
    {
        g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + instanceName + " to start! Has it been defined?");
        return -1;
    }
/*
    // Just load the default
    else
        StartActivity(m_DefaultActivityType, m_DefaultActivityName);
*/

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Completely restarts whatever activity was last started.

int ActivityMan::RestartActivity()
{
    // If we have a start activity set, then clone it and pass in. (have to clone, or will delete self in StartActivity)
    if (m_pStartActivity)
        return StartActivity(dynamic_cast<Activity *>(m_pStartActivity->Clone()));
    // Use the default activity instead
    else
        return StartActivity(m_DefaultActivityType, m_DefaultActivityName);

    // Report that we had to start the default because there wasn't a specified start activity
    return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void ActivityMan::PauseActivity(bool pause)
{
    if (!m_pActivity || (pause && m_pActivity->Paused()) || (!pause && !m_pActivity->Paused()))
        return;

    if (m_pActivity)
    {
        // Save the current in-game music position on pause
        if (pause)
        {
            m_LastMusicPath = g_AudioMan.GetMusicPath();
            m_LastMusicPos = g_AudioMan.GetMusicPosition();
        }
        // Re-start it again where it was on unpause
        else
        {
            // Only if we have a position to actually resume
            if (!m_LastMusicPath.empty() && m_LastMusicPos > 0)
            {
                g_AudioMan.ClearMusicQueue();
                g_AudioMan.PlayMusic(m_LastMusicPath.c_str());
                g_AudioMan.SetMusicPosition(m_LastMusicPos);
                g_AudioMan.QueueSilence(30);
                g_AudioMan.QueueMusicStream("Base.rte/Music/Watts/Last Man.ogg");
                g_AudioMan.QueueSilence(30);
                g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");
            }
        }

        m_pActivity->Pause(pause);
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was " + (pause ? "paused" : "unpaused"));
    }
    else
        g_ConsoleMan.PrintString("ERROR: No Activity to pause!");

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void ActivityMan::EndActivity()
{
    if (m_pActivity)
    {
        m_pActivity->End();
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was ended");
    }
    else
        g_ConsoleMan.PrintString("ERROR: No Activity to end!");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this and the current Activity. Supposed to be
//                  done every frame before drawing.

void ActivityMan::Update()
{
    if (m_pActivity)
        m_pActivity->Update();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LateUpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only updates Global Scripts of the current activity with LateUpdate flag enabled

void ActivityMan::LateUpdateGlobalScripts()
{
	GAScripted * sa = dynamic_cast<GAScripted *>(m_pActivity);
	if (sa)
		sa->UpdateGlobalScripts(true);
}

} // namespace RTE
