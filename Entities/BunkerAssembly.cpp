//////////////////////////////////////////////////////////////////////////////////////////
// File:            BunkerAssembly.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the BunkerAssembly class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "BunkerAssembly.h"
#include "PresetMan.h"
#include "ContentFile.h"
#include "MOSRotating.h"
#include "Actor.h"
#include "ADoor.h"
#include "TerrainObject.h"
#include "BunkerAssemblyScheme.h"
#include "Deployment.h"

namespace RTE {

ConcreteClassInfo(BunkerAssembly, SceneObject, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this BunkerAssembly, effectively
//                  resetting the members of this abstraction level only.

void BunkerAssembly::Clear()
{
    m_FGColorFile.Reset();
    m_MaterialFile.Reset();
    m_BGColorFile.Reset();
    m_pFGColor = 0;
    m_pMaterial = 0;
    m_pBGColor = 0;
    m_BitmapOffset.Reset();
    m_OffsetDefined = false;
    m_ChildObjects.clear();
	m_PlacedObjects.clear();
	m_ParentAssemblyScheme.clear();
	m_pPresentationBitmap = 0;
	m_SymmetricAssembly.clear();
	m_ParentSchemeGroup.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BunkerAssembly object ready for use.

int BunkerAssembly::Create()
{
    if (TerrainObject::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds placed object to the internallist of placed objects for this assembly,
//					applies it's image to presentation bitmap and sets assembly price accordingly.
//					Added scene object MUST have coordinates relative to this assembly.

void BunkerAssembly::AddPlacedObject(SceneObject * pSO)
{
    m_PlacedObjects.push_back(pSO);

	//Increase gold value for every bunker module if it's not a deployment
	Deployment * pDeployment  =  dynamic_cast<Deployment *>(pSO);
	//Set fixed price
	//if (!pDeployment && !pSO->IsInGroup("Bunker Backgrounds"))
	//	m_OzValue += pSO->GetGoldValue();

	// Draw this terrain object to presentaion bitmap
	TerrainObject * pTObject = dynamic_cast<TerrainObject *>(pSO);
	if (pTObject)
	{
		Vector loc = pTObject->GetPos() + pTObject->GetBitmapOffset();

		// Regular drawing
		draw_sprite(m_pMaterial, pTObject->GetMaterialBitmap(), loc.m_X, loc.m_Y);
		if (pTObject->HasBGColor())
		{
			draw_sprite(m_pBGColor, pTObject->GetBGColorBitmap(), loc.m_X, loc.m_Y);
			draw_sprite(m_pPresentationBitmap, pTObject->GetBGColorBitmap(), loc.m_X, loc.m_Y);
		}
		draw_sprite(m_pFGColor, pTObject->GetFGColorBitmap(), loc.m_X, loc.m_Y);
		draw_sprite(m_pPresentationBitmap, pTObject->GetFGColorBitmap(), loc.m_X, loc.m_Y);

		// Read and add all child objects
		pTObject->SetTeam(GetTeam());
		for (list<SceneObject::SOPlacer>::const_iterator itr = pTObject->GetChildObjects().begin(); itr != pTObject->GetChildObjects().end() ; ++itr)
		{
			SOPlacer newPlacer = (*itr);
			newPlacer.SetTeam(pTObject->GetTeam());
			// Explicitly set child object's offset, because it will be a part of a bigger 'terrain object'
			newPlacer.SetOffset(newPlacer.GetOffset() + pTObject->GetPos() + m_BitmapOffset);
			m_ChildObjects.push_back(newPlacer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BunkerAssembly object ready for use.

int BunkerAssembly::Create(BunkerAssemblyScheme * pScheme)
{
    if (TerrainObject::Create() < 0)
        return -1;

	m_pPresentationBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
	clear_to_color(m_pPresentationBitmap, g_MaskColor);

	m_pFGColor = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
	clear_to_color(m_pFGColor, g_MaskColor);

	m_pMaterial = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
	clear_to_color(m_pMaterial, g_MaskColor);

	m_pBGColor = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
	clear_to_color(m_pBGColor, g_MaskColor);

	m_BitmapOffset = pScheme->GetBitmapOffset();
	m_ParentAssemblyScheme = pScheme->GetPresetName();
	m_Pos = pScheme->GetPos();

	AddToGroup("Assemblies");
	AddToGroup(m_ParentAssemblyScheme);
	if (pScheme->GetAssemblyGroup().length() > 0)
	{
		AddToGroup(pScheme->GetAssemblyGroup());
		m_ParentSchemeGroup = pScheme->GetAssemblyGroup();
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.

int BunkerAssembly::Create(const BunkerAssembly &reference)
{
    TerrainObject::Create(reference);

    for (list<SceneObject *>::const_iterator oItr = reference.m_PlacedObjects.begin(); oItr != reference.m_PlacedObjects.end(); ++oItr)
        m_PlacedObjects.push_back(dynamic_cast<SceneObject *>((*oItr)->Clone()));

	m_ParentAssemblyScheme = reference.m_ParentAssemblyScheme;
	m_pPresentationBitmap = reference.m_pPresentationBitmap;
	m_SymmetricAssembly = reference.m_SymmetricAssembly;
	m_ParentSchemeGroup = reference.m_ParentSchemeGroup;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int BunkerAssembly::ReadProperty(const std::string_view &propName, Reader &reader)
{
    // Ignore TerrainObject's specific properties, but don't let parent class process them
	if (propName == "FGColorFile")
    {
    }
    else if (propName == "MaterialFile")
    {
    }
    else if (propName == "BGColorFile")
    {
    }
    else if (propName == "BitmapOffset")
    {
    }
    else if (propName == "Location")
    {
    }
    else if (propName == "AddChildObject")
    {
    }
    else if (propName == "SymmetricAssembly")
    {
		reader >> m_SymmetricAssembly;
    }
    else if (propName == "PlaceObject")
    {
        SceneObject *pSO = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
        if (pSO)
			AddPlacedObject(pSO);
    }
    else if (propName == "ParentScheme")
    {
		//Add to group like Entity::ReadProperty does
        string parentScheme;
        reader >> parentScheme;
        AddToGroup(parentScheme);
        g_PresetMan.RegisterGroup(parentScheme, reader.GetReadModuleID());

		// Find the scheme's group, read it's dimensions and create presentation bitmap
		const BunkerAssemblyScheme * pScheme = dynamic_cast<const BunkerAssemblyScheme *>(g_PresetMan.GetEntityPreset("BunkerAssemblyScheme", parentScheme, -1));
		if (pScheme)
		{
			//Calculate fixed scheme price based on the scheme size
			if (pScheme->GetGoldValue() == 0)
				m_OzValue = pScheme->GetArea() * 3;
			else 
				pScheme->GetGoldValue();

			//Delete existing bitmaps to avoid leaks if someone adds assembly to multiple groups by mistake
			delete m_pPresentationBitmap;
			m_pPresentationBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
			clear_to_color(m_pPresentationBitmap, g_MaskColor);

			delete m_pFGColor;
			m_pFGColor = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
			clear_to_color(m_pFGColor, g_MaskColor);

			delete m_pMaterial;
			m_pMaterial = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
			clear_to_color(m_pMaterial, g_MaskColor);

			delete m_pBGColor;
			m_pBGColor = create_bitmap_ex(8, pScheme->GetBitmapWidth() , pScheme->GetBitmapHeight());
			clear_to_color(m_pBGColor, g_MaskColor);

			m_ParentAssemblyScheme = parentScheme;
			m_BitmapOffset = pScheme->GetBitmapOffset();
			if (pScheme->GetAssemblyGroup().length() > 0)
			{
				AddToGroup(pScheme->GetAssemblyGroup());
				m_ParentSchemeGroup = pScheme->GetAssemblyGroup();
		        g_PresetMan.RegisterGroup(pScheme->GetAssemblyGroup(), reader.GetReadModuleID());
			}

			// Also add to Assemblies group
			AddToGroup("Assemblies");
	        g_PresetMan.RegisterGroup("Assemblies", reader.GetReadModuleID());
		} else {
			// Do not allow to define assemblies prior to corresponding assembly scheme
			char s[256];
			std::snprintf(s, sizeof(s), "Required BunkerAssemblyScheme '%s%' not found when trying to load BunkerAssembly '%s'! BunkerAssemblySchemes MUST be defined before dependent BunkerAssmeblies.", parentScheme.c_str(), m_PresetName.c_str());
			RTEAbort(s);
		}
	} else
        return SceneObject::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this BunkerAssembly with a Writer for
//                  later recreation with Create(Reader &reader);

int BunkerAssembly::Save(Writer &writer) const
{
	SceneObject::Save(writer);

    // Groups are essential for BunkerAssemblies so save them, because entity seem to ignore them
	for (list<string>::const_iterator itr = m_Groups.begin(); itr != m_Groups.end(); ++itr)
    {
		if ((*itr) != m_ParentAssemblyScheme && (*itr) != m_ParentSchemeGroup)
		{
	        writer.NewProperty("AddToGroup");
		    writer << *itr;
		}
    }
	if (m_SymmetricAssembly.size() > 0)
	{
		writer.NewProperty("SymmetricAssembly");
		writer << m_SymmetricAssembly;
	}
	if (m_ParentAssemblyScheme.size() > 0)
	{
		writer.NewProperty("ParentScheme");
		writer << m_ParentAssemblyScheme;
	}

    for (list<SceneObject *>::const_iterator oItr = m_PlacedObjects.begin(); oItr != m_PlacedObjects.end(); ++oItr)
    {
		writer.NewProperty("PlaceObject");
        writer.ObjectStart((*oItr)->GetClassName());
        writer.NewProperty("CopyOf");
        writer << (*oItr)->GetModuleAndPresetName();

        writer.NewProperty("Position");
        writer << (*oItr)->GetPos();

        // Only write certain properties if they are applicable to the type of SceneObject being written
        MOSRotating *pSpriteObj = dynamic_cast<MOSRotating *>(*oItr);
        if (pSpriteObj)
        {
            writer.NewProperty("HFlipped");
            writer << pSpriteObj->IsHFlipped();
            Actor *pActor = dynamic_cast<Actor *>(pSpriteObj);
            if (pActor)
            {
                writer.NewProperty("Team");
                writer << pActor->GetTeam();
                // Rotation of doors is important
                ADoor *pDoor = dynamic_cast<ADoor *>(pActor);
                if (pDoor)
                {
                    writer.NewProperty("Rotation");
                    writer << pDoor->GetRotMatrix();
                }
            }
        }
        TerrainObject *pTObject = dynamic_cast<TerrainObject *>(*oItr);
        if (pTObject && !pTObject->GetChildObjects().empty())
        {
            writer.NewProperty("Team");
            writer << pTObject->GetTeam();
        }
		Deployment *pDeployment = dynamic_cast<Deployment *>(*oItr);
		if (pDeployment)
		{
			writer.NewProperty("HFlipped");
			writer << pDeployment->IsHFlipped();
		}


        writer.ObjectEnd();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the BunkerAssembly object.

void BunkerAssembly::Destroy(bool notInherited)
{
    for (list<SceneObject *>::iterator oItr = m_PlacedObjects.begin(); oItr != m_PlacedObjects.end(); ++oItr)
    {
        delete (*oItr);
        *oItr = 0;
    }

	// Probably no need to delete those, as bitmaps are only created when preset is read from file
	// and then they just copy pointers in via Clone()	
	//delete m_pPresentationBitmap;
	//m_pPresentationBitmap = 0;
	
	//delete m_pFGColor;
    //m_pFGColor = 0;
	
	//delete m_pMaterial;
    //m_pMaterial = 0;
    
	//delete m_pBGColor;
	//m_pBGColor = 0;

    if (!notInherited)
        SceneObject::Destroy();
    Clear();
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDeployments
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Retrieves the list of random deployemtns selected to be deployed by this assembly
//					based on it's parent scheme MaxDeployments value. This list will always include all
//					brain deployments so it can be longer that MaxDeployments. OWNERSHIP NOT TRANSFERRED.

std::vector<Deployment *> BunkerAssembly::GetDeployments()
{
	std::vector<Deployment *> deploymentsList;
	std::vector<Deployment *> candidatesList;

	// Sort objects, brains are added by default, everything else are candidates
	for (std::list<SceneObject *>::const_iterator itr = m_PlacedObjects.begin(); itr != m_PlacedObjects.end() ; ++itr)
	{
		Deployment * pDeployment = dynamic_cast<Deployment *>(*itr);

		if (pDeployment)
		{
			if (pDeployment->IsInGroup("Brains"))
				deploymentsList.push_back(pDeployment);
			else
				candidatesList.push_back(pDeployment);
		}
	}

	int maxDeployments = 0;

	const BunkerAssemblyScheme * pScheme = dynamic_cast<const BunkerAssemblyScheme *>(g_PresetMan.GetEntityPreset("BunkerAssemblyScheme", m_ParentAssemblyScheme, -1));
	if (pScheme)
		maxDeployments = pScheme->GetMaxDeployments();

	int selected = 0;
	for (int i = 0; i<maxDeployments; i++)
	{
		if (candidatesList.size() == 0 )
			break;

		int selection = RandomNum<int>(0, candidatesList.size() - 1);
		deploymentsList.push_back(candidatesList.at(selection));
		candidatesList.erase(candidatesList.begin() + selection);
	}

	return deploymentsList;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool BunkerAssembly::IsOnScenePoint(Vector &scenePoint) const
{
    if (!m_pPresentationBitmap)
        return false;

    Vector bitmapPos = m_Pos + m_BitmapOffset;
    if (WithinBox(scenePoint, bitmapPos, m_pPresentationBitmap->w, m_pPresentationBitmap->h))
    {
        // Scene point on the bitmap
        Vector bitmapPoint = scenePoint - bitmapPos;
        if (getpixel(m_pPresentationBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor)
           return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this Actor belongs to.

void BunkerAssembly::SetTeam(int team)
{
    TerrainObject::SetTeam(team);

    // Make sure all the objects to be placed will be of the same team
    for (list<SceneObject *>::iterator itr = m_PlacedObjects.begin(); itr != m_PlacedObjects.end(); ++itr)
        (*itr)->SetTeam(team);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this TerrainObject's current graphical representation to a
//                  BITMAP of choice.

void BunkerAssembly::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const
{
    if (!m_pFGColor)
        RTEAbort("TerrainObject's bitmaps are null when drawing!");

    // Take care of wrapping situations
    Vector aDrawPos[4];
    aDrawPos[0] = m_Pos + m_BitmapOffset - targetPos;
    int passes = 1;

    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
	if (targetPos.IsZero() && g_SceneMan.GetSceneWidth() <= pTargetBitmap->w)
    {
        if (aDrawPos[0].m_X < m_pFGColor->w)
        {
            aDrawPos[passes] = aDrawPos[0];
            aDrawPos[passes].m_X += pTargetBitmap->w;
            passes++;
        }
        else if (aDrawPos[0].m_X > pTargetBitmap->w - m_pFGColor->w)
        {
            aDrawPos[passes] = aDrawPos[0];
            aDrawPos[passes].m_X -= pTargetBitmap->w;
            passes++;
        }
    }
    // Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
    else
    {
        if (g_SceneMan.SceneWrapsX())
        {
            int sceneWidth = g_SceneMan.GetSceneWidth();
            if (targetPos.m_X < 0)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X -= sceneWidth;
                passes++;
            }
            if (targetPos.m_X + pTargetBitmap->w > sceneWidth)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X += sceneWidth;
                passes++;
            }
        }
    }

    // Draw all the passes needed
    for (int i = 0; i < passes; ++i)
    {
        if (mode == g_DrawColor)
        {
            masked_blit(m_pPresentationBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pBGColor->w, m_pBGColor->h);
        }
        else if (mode == g_DrawMaterial)
        {
            masked_blit(m_pMaterial, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pMaterial->w, m_pMaterial->h);
        }
        else if (mode == g_DrawLess)
        {
            masked_blit(m_pPresentationBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pFGColor->w, m_pFGColor->h);
        }
        else if (mode == g_DrawTrans)
        {
            draw_trans_sprite(pTargetBitmap, m_pPresentationBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
        }
    }
}


} // namespace RTE
