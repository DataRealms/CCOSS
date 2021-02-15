//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUISkin.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUISkin class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "Reader.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUISkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUISkin object in system
//                  memory.

GUISkin::GUISkin(GUIScreen *Screen)
{
    m_Screen = Screen;
    m_FontCache.clear();

    m_MousePointers[0] = 0;
    m_MousePointers[1] = 0;
    m_MousePointers[2] = 0;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:        GUISkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to free a GUISkin object in system
//                  memory.

GUISkin::~GUISkin()
{
    Destroy();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the data

void GUISkin::Clear(void)
{
    m_PropList.clear();
    m_ImageCache.clear();
    m_FontCache.clear();
    m_Directory = "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Load
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a skin for a directory

bool GUISkin::Load(const std::string &directory, const std::string &fileName) {
	// Destroy any previous instances
	Destroy();

	m_Directory = directory;

	RTE::Reader skinFile;
	if (skinFile.Create((m_Directory + "/" + fileName).c_str()) == -1) {
		return false;
	}

	// Go through the skin file adding the sections and properties
	GUIProperties *CurProp = nullptr;

	while (!skinFile.GetStream()->eof()) {
		std::string line = skinFile.ReadLine();

		if (line.empty()) {
			continue;
		}

		// Is the line a section?
		if (line.front() == '[' && line.back() == ']') {
			GUIProperties *p = new GUIProperties(line.substr(1, line.size() - 2));
			CurProp = p;
			m_PropList.push_back(p);
			continue;
		}

		// Is the line a valid property?
		size_t Position = line.find('=');
		if (Position != std::string::npos) {
			// Break the line into variable & value, but only add a property if it belongs to a section
			if (CurProp) {
				// Grab the variable & value strings and trim them
				std::string Name = skinFile.TrimString(line.substr(0, Position));
				std::string Value = skinFile.TrimString(line.substr(Position + 1, std::string::npos));

				// Add it to the current property
				CurProp->AddVariable(Name, Value);
			}
			continue;
		}
	}

	// Load the mouse pointers
	m_MousePointers[0] = LoadMousePointer("Mouse_Pointer");
	m_MousePointers[1] = LoadMousePointer("Mouse_Text");
	m_MousePointers[2] = LoadMousePointer("Mouse_HSize");

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a string value

bool GUISkin::GetValue(const std::string Section, const std::string Variable, std::string *Value)
{
    vector <GUIProperties *>::iterator it;

    // Find the property
    for(it = m_PropList.begin(); it != m_PropList.end(); it++) {
        GUIProperties *p = *it;        

        if (stricmp(p->GetName().c_str(), Section.c_str()) == 0) {
            
            if (p->GetValue(Variable, Value))
                return true;
        }
    }

    // Not found
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets an integer array of values

int GUISkin::GetValue(const std::string Section, const std::string Variable, int *Array, int MaxArraySize)
{
    vector <GUIProperties *>::iterator it;

    // Find the property
    for(it = m_PropList.begin(); it != m_PropList.end(); it++) {
        GUIProperties *p = *it;        

        if (stricmp(p->GetName().c_str(), Section.c_str()) == 0) {
            
            if (p->GetValue(Variable, Array, MaxArraySize))
                return true;
        }
    }

    // Not found
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a single integer value.

bool GUISkin::GetValue(const std::string Section, const std::string Variable, int *Value)
{
    vector <GUIProperties *>::iterator it;

    // Find the property
    for(it = m_PropList.begin(); it != m_PropList.end(); it++) {
        GUIProperties *p = *it;        

        if (stricmp(p->GetName().c_str(), Section.c_str()) == 0) {
            
            if (p->GetValue(Variable, Value))
                return true;
        }
    }

    // Not found
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a single unsigned integer value.

bool GUISkin::GetValue(const std::string Section, const std::string Variable, unsigned long *Value)
{
    vector <GUIProperties *>::iterator it;

    // Find the property
    for(it = m_PropList.begin(); it != m_PropList.end(); it++) {
        GUIProperties *p = *it;        

        if (stricmp(p->GetName().c_str(), Section.c_str()) == 0) {
            
            if (p->GetValue(Variable, Value))
                return true;
        }
    }

    // Not found
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Frees the allocated data.

void GUISkin::Destroy(void)
{
    vector <GUIProperties *>::iterator it;

    // Free the properties
    for(it = m_PropList.begin(); it != m_PropList.end(); it++) {
        GUIProperties *p = *it;        

        if (p)
            delete p;
    }

    m_PropList.clear();

    // Destroy the fonts in the list
    vector<GUIFont *>::iterator itf;
    
    for(itf = m_FontCache.begin(); itf != m_FontCache.end(); itf++) {
        GUIFont *F = *itf;
        if (F) {
            F->Destroy();
            delete F;
        }
    }

    m_FontCache.clear();


    // Destroy the images in the image cache
    vector<GUIBitmap *>::iterator iti;

    for(iti = m_ImageCache.begin(); iti != m_ImageCache.end(); iti++) {
        GUIBitmap *Surf = *iti;
        if (Surf) {
            Surf->Destroy();
            delete Surf;
        }
    }

    m_ImageCache.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a blank bitmap

GUIBitmap *GUISkin::CreateBitmap(int Width, int Height)
{
    return m_Screen->CreateBitmap(Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a bitmap from a filename.

GUIBitmap *GUISkin::CreateBitmap(const string Filename)
{
    // Add the filename onto the current directory
    string File = m_Directory + "/" + Filename;

    // Check if the image is in our cache
    vector<GUIBitmap *>::iterator it;
    for(it = m_ImageCache.begin(); it != m_ImageCache.end(); it++) {
        GUIBitmap *Surf = *it;

        if (stricmp(File.c_str(), Surf->GetDataPath().c_str()) == 0)
            return Surf;
    }

    // Not found in cache, so we create a new bitmap from the file
    GUIBitmap *Bitmap = m_Screen->CreateBitmap(File);
    if (!Bitmap)
        return 0;

    // Add the new bitmap to the cache
    m_ImageCache.push_back(Bitmap);

    return Bitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates or gets a cached copy of a font.

GUIFont *GUISkin::GetFont(const string Name)
{
    // Check if the font is already in the list
    vector<GUIFont *>::iterator it;

    for(it = m_FontCache.begin(); it != m_FontCache.end(); it++) {
        GUIFont *F = *it;
        if (stricmp(F->GetName().c_str(), Name.c_str()) == 0)
            return F;
    }

    // Not found, so we create the font
    GUIFont *Font = new GUIFont(Name);
    if (!Font->Load(m_Screen, m_Directory + "/" + Name))
    {
        delete Font;
        return 0;
    }

    m_FontCache.push_back(Font);

    return Font;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadMousePointer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a mouse pointer image & details

GUIBitmap *GUISkin::LoadMousePointer(const string Section)
{
    string File;
    int ColorKey;

    if (!GetValue(Section, "Filename", &File))
        return 0;
    if (!GetValue(Section, "ColorKeyIndex", &ColorKey))
        return 0;

    GUIBitmap *Bitmap = CreateBitmap(File);
    if (!Bitmap)
        return 0;

    Bitmap->SetColorKey(/*ConvertColor(ColorKey)*/);
    return Bitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the mouse onto the screen.

void GUISkin::DrawMouse(int Image, int X, int Y)
{
    assert(Image >= 0 && Image <= 2);

    if (m_MousePointers[Image])
        m_Screen->DrawBitmapTrans(m_MousePointers[Image], X - 1, Y - 1, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildStandardRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Builds a bitmap from a standard skin property section.

void GUISkin::BuildStandardRect(GUIBitmap *Dest, const std::string Section, int X, int Y,
                                int Width, int Height, bool buildBG, bool buildFrame)
{
    /*
     * Note:
     * For a control to use a 'Standard Rect' it must use the 8 side names, a filler name
     * and a filename property.
     */

    int VTopLeft[4]; int VTop[4]; int VTopRight[4];
    int VLeft[4]; int VFiller[4]; int VRight[4];
    int VBottomLeft[4]; int VBottom[4]; int VBottomRight[4];
    int i, j;
    GUIRect Rect;

    // Load the filename
    string Filename;
    GetValue(Section, "Filename", &Filename);
    GUIBitmap *SrcBitmap = CreateBitmap(Filename);

    // Set the color key to be the same color as the Top-Right hand corner pixel
    SrcBitmap->SetColorKey(SrcBitmap->GetPixel(SrcBitmap->GetWidth()-1, 0));
    Dest->DrawRectangle(X, Y, Width, Height,
                        SrcBitmap->GetPixel(SrcBitmap->GetWidth()-1, 0), true);
    Dest->SetColorKey(SrcBitmap->GetPixel(SrcBitmap->GetWidth()-1, 0));

    GetValue(Section, "Filler", VFiller, 4);
    GetValue(Section, "Top", VTop, 4);
    GetValue(Section, "Right", VRight, 4);
    GetValue(Section, "Bottom", VBottom, 4);
    GetValue(Section, "Left", VLeft, 4);
    GetValue(Section, "TopLeft", VTopLeft, 4);
    GetValue(Section, "TopRight", VTopRight, 4);
    GetValue(Section, "BottomRight", VBottomRight, 4);
    GetValue(Section, "BottomLeft", VBottomLeft, 4);

    if (buildBG)
    {
        // Use the filler first
        SetRect(&Rect, VFiller[0], VFiller[1], VFiller[0]+VFiller[2], VFiller[1]+VFiller[3]);
        // Tile the filler across
        for(j=Y+VTop[3]; j<Y+Height-VBottom[3]; j+=VFiller[3])
        {
            for(i=X+VLeft[2]; i<X+Width-VRight[2]; i+=VFiller[2])
            {
                SrcBitmap->DrawTrans(Dest, i, j, &Rect);
            }
        }
    }

    if (buildFrame)
    {
        // Tile the four sides first, then place the four corners last

        // Tile the Top side
        SetRect(&Rect, VTop[0], VTop[1], VTop[0]+VTop[2], VTop[1]+VTop[3]);
        for(i=X+VTopLeft[2]; i<=X+Width-VTopRight[2]; i+=VTop[2])
            SrcBitmap->DrawTrans(Dest, i, Y, &Rect);

        // Tile the Right side
        SetRect(&Rect, VRight[0], VRight[1], VRight[0]+VRight[2], VRight[1]+VRight[3]);
        for(j=Y+VTopRight[3]; j<Y+Height-VBottomRight[3]; j+=VRight[3])
            SrcBitmap->DrawTrans(Dest, X+Width-VRight[2], j, &Rect);
        
        // Tile the Bottom side
        SetRect(&Rect, VBottom[0], VBottom[1], VBottom[0]+VBottom[2], VBottom[1]+VBottom[3]);
        for(i=X+VBottomLeft[2]; i<X+Width-VBottomRight[2]; i+=VBottom[2])
            SrcBitmap->DrawTrans(Dest, i, Y+Height-VBottom[3], &Rect);

        // Tile the Left side
        SetRect(&Rect, VLeft[0], VLeft[1], VLeft[0]+VLeft[2], VLeft[1]+VLeft[3]);
        for(j=Y+VTopLeft[3]; j<Y+Height-VBottomLeft[3]; j+=VLeft[3])
            SrcBitmap->DrawTrans(Dest, X, j, &Rect);


        // Top-Left Corner
        SetRect(&Rect, VTopLeft[0], VTopLeft[1], VTopLeft[0]+VTopLeft[2], VTopLeft[1]+VTopLeft[3]);
        SrcBitmap->DrawTrans(Dest, X, Y, &Rect);
        
        // Top-Right Corner
        SetRect(&Rect, VTopRight[0], VTopRight[1], VTopRight[0]+VTopRight[2], VTopRight[1]+VTopRight[3]);
        SrcBitmap->DrawTrans(Dest, X+Width-VTopRight[2], Y, &Rect);
        
        // Bottom-Right Corner
        SetRect(&Rect, VBottomRight[0], VBottomRight[1], VBottomRight[0]+VBottomRight[2], VBottomRight[1]+VBottomRight[3]);
        SrcBitmap->DrawTrans(Dest, X+Width-VBottomRight[2], Y+Height-VBottomRight[3], &Rect);

        // Bottom-Left Corner
        SetRect(&Rect, VBottomLeft[0], VBottomLeft[1], VBottomLeft[0]+VBottomLeft[2], VBottomLeft[1]+VBottomLeft[3]);
        SrcBitmap->DrawTrans(Dest, X, Y+Height-VBottomLeft[3], &Rect);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConvertColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts an 8bit palette index to a valid pixel format.
//                  Primarily used for development in windowed mode.
    
unsigned long GUISkin::ConvertColor(unsigned long color, int targetDepth)
{
    return m_Screen->ConvertColor(color, targetDepth);
}
