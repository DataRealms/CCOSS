#ifndef _RTEFRAMEMAN_
#define _RTEFRAMEMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            FrameMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the FrameMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

typedef int MOID;
typedef int MID;

namespace RTE
{
enum
{
    g_KeyColor = 0,
	//g_KeyColorS = 16711935,// Key color to clear 32-bit bitmaps used for MOID silhouettes layers
	g_KeyColorS = 0xF81F,// Key color to clear 16-bit bitmaps used for MOID silhouettes layers
	//g_KeyColorS = 0, //Key color to cleat 8-bit bitmaps used for MOID silhouettes layers
    g_BlackColor = 245,
    g_WhiteColor = 254,
    g_RedColor = 13,
    g_YellowGlowColor = 117,
    g_NoMOID = 255
};

enum DotGlowColor
{
    NoDot = 0,
    YellowDot,
    RedDot,
    BlueDot
};
}

#include <allegro.h>
#include "DDTTools.h"
#include "Singleton.h"
#define g_FrameMan FrameMan::Instance()
#include "Serializable.h"
#include "Vector.h"
#include "Box.h"
#include "Material.h"
#include <map>
#include <list>
#include "SceneMan.h"

#include "MovableMan.h"

namespace RTE
{

class AllegroScreen;
class GUIFont;

#define MAXSCREENCOUNT 4
#define MAX_LAYERS_STORED_FOR_NETWORK 10

enum TransperencyPreset
{
    LessTrans = 0,
    HalfTrans,
    MoreTrans
};



//////////////////////////////////////////////////////////////////////////////////////////
// Struct:          PostEffect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     For storing a Post screen effect to be applied at the last stage of
//                  32bpp rendering
// Parent(s):       None.
// Class history:   8/18/2007 PostEffect created.

struct PostEffect
{
    // Where, can be relative to the scene, or to the screen, depending on context
    Vector m_Pos;
    // The bitmap to blend, not owned
    BITMAP *m_pBitmap;
    // Scalar float for how hard to blend it in, 0 - 255
    int m_Strength;
	// Hash used to transimt glow events over the network
	size_t m_BitmapHash;
	// Post effect angle
	float m_Angle;

    
	PostEffect() { m_Pos.Reset(); m_pBitmap = 0; m_BitmapHash = 0;  m_Strength = 128; m_Angle = 0; }
	PostEffect(const Vector &pos, BITMAP *pBitmap, size_t bitmapHash, int strength, float angle) { m_Pos = pos; m_pBitmap = pBitmap; m_BitmapHash = bitmapHash; m_Strength = strength; m_Angle = angle; }
};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           FrameMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager over the composition and display of frames.
// Parent(s):       Singleton, Serializable.
// Class history:   12/26/2001 FrameMan created.

class FrameMan:
    public Singleton<FrameMan>,
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


	// Enumeration of all available perofrmance counters
	enum PerformanceCounters
	{
		PERF_SIM_TOTAL = 0,
		PERF_ACTORS_AI,
		PERF_ACTORS_PASS2,
		PERF_ACTORS_PASS1,
		PERF_PARTICLES_PASS2,
		PERF_PARTICLES_PASS1,
		PERF_ACTIVITY,
#if __USE_SOUND_GORILLA
		PERF_SOUND,
#endif
		PERF_COUNT
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           GraphicalPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of graphical primitives created from Lua
	//					all coordintaes passed to GraphicalPrimitive objects are Scene coordinates
	class GraphicalPrimitive
	{
	public:
		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:			Constructor
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates GraphicalPrimitive objects
		//                  
		GraphicalPrimitive()
		{
			m_Color = 0;
			m_Player = -1;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos) = 0;


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:			TranslateCoordinates
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Translates coordinats from scene to this bitmap offset
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset, scene coordinates
		// Return value:    Vector with translated coordinates
		Vector TranslateCoordinates(Vector targetPos, Vector scenePos);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:			TranslateCoordinates
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Translates coordinats from scene to this bitmap offset producing two coordinates 
		//					for 'left' scene bitmap with negative values as if scene seam is 0,0 and
		//					for 'right' dcini bitmap with positive values.
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset, scene coordinates, translated coordinates
		// Return value:    Vector with translated coordinates
		void TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos);


		// Start of the primitive
		Vector m_Start;
		// Color to draw this primitive with
		int m_Color;
		// Player screen to draw primitive on
		int m_Player;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           LinePrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of line primitives created from Lua
	class LinePrimitive: public GraphicalPrimitive
	{
	public:
		LinePrimitive()
		{
		}

		LinePrimitive(Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = -1;
		}

		LinePrimitive(int player, Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = player;
		}


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		Vector m_End;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           BoxPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of box primitives created from Lua
	class BoxPrimitive: public GraphicalPrimitive
	{
	public:
		BoxPrimitive()
		{
		}

		BoxPrimitive(Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = -1;
		}

		BoxPrimitive(int player, Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = player;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		Vector m_End;
	};

	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           BoxFillPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of filled box primitives created from Lua
	class BoxFillPrimitive: public GraphicalPrimitive
	{
	public:
		BoxFillPrimitive()
		{
		}

		BoxFillPrimitive(Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = -1;
		}

		BoxFillPrimitive(int player, Vector start, Vector end, int color)
		{
			m_Start = start;
			m_End = end;
			m_Color = color;
			m_Player = player;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		Vector m_End;
	};



	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           CirclePrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of circle primitives created from Lua
	class CirclePrimitive: public GraphicalPrimitive
	{
	public:
		CirclePrimitive()
		{
			m_Radius = 0;
		}

		CirclePrimitive(Vector pos, int radius, int color)
		{
			m_Start = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = -1;
		}

		CirclePrimitive(int player, Vector pos, int radius, int color)
		{
			m_Start = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = player;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		float m_Radius;
	};

	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:           CircleFillPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of filled circle primitives created from Lua
	class CircleFillPrimitive: public GraphicalPrimitive
	{
	public:
		CircleFillPrimitive()
		{
			m_Radius = 0;
		}

		CircleFillPrimitive(Vector pos, int radius, int color)
		{
			m_Start = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = -1;
		}

		CircleFillPrimitive(int player, Vector pos, int radius, int color)
		{
			m_Start = pos;
			m_Color = color;
			m_Radius = radius;
			m_Player = player;
		}

		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		float m_Radius;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:    TextPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of text primitives created from Lua
	class TextPrimitive: public GraphicalPrimitive
	{
	public:
		TextPrimitive()
		{
			m_IsSmall = true;
			m_Text = "";
			m_Alignment = 0;
			m_Player = -1;
		}

		TextPrimitive(Vector pos, std::string text, bool isSmall, int alignment)
		{
			m_Start = pos;
			m_Text = text;
			m_IsSmall = isSmall;
			m_Alignment = alignment;
			m_Player = -1;
		}

		TextPrimitive(int player, Vector pos, std::string text, bool isSmall, int alignment)
		{
			m_Start = pos;
			m_Text = text;
			m_IsSmall = isSmall;
			m_Alignment = alignment;
			m_Player = player;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		bool m_IsSmall;
		std::string m_Text;
		int m_Alignment;
	};



	//////////////////////////////////////////////////////////////////////////////////////////
	// Nested class:    BitmapPrimitive
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Class used to schedule drawing of bitmap primitives created from Lua
	class BitmapPrimitive: public GraphicalPrimitive
	{
	public:
		BitmapPrimitive()
		{
			m_pBitmap = 0;
			m_RotAngle = 0.0;
			m_Player = -1;
		}

		BitmapPrimitive(Vector pos, BITMAP * bitmap, float rotAngle)
		{
			m_Start = pos;
			m_pBitmap = bitmap;
			m_RotAngle = rotAngle;
			m_Player = -1;
		}

		BitmapPrimitive(int player, Vector pos, BITMAP * bitmap, float rotAngle)
		{
			m_Start = pos;
			m_pBitmap = bitmap;
			m_RotAngle = rotAngle;
			m_Player = player;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this primitive on provided bitmap
		//                  
		// Arguments:       Bitmap to draw on, bitmap offset
		// Return value:    None
		virtual void Draw(BITMAP *pDrawScreen, Vector targetPos);

		BITMAP * m_pBitmap;
		float m_RotAngle;

	};


	// List of graphical primitives scheduled to draw this frame, cleared every frame during FrameMan::Draw()
	std::list<GraphicalPrimitive *> m_Primitives;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawCirclePrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a circle primitive.
// Arguments:       Position of primitive in scene coordintaes, radius, color.
// Return value:    None.

	void DrawCirclePrimitive(Vector pos, int radius, int color)
	{
	    m_Primitives.push_back(new CirclePrimitive(pos, radius,color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawCirclePrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a circle primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, radius, color.
// Return value:    None.

	void DrawCirclePrimitive(int player, Vector pos, int radius, int color)
	{
		m_Primitives.push_back(new CirclePrimitive(player, pos, radius, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawCircleFillPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a filled circle primitive.
// Arguments:       Position of primitive in scene coordintaes, radius, color.
// Return value:    None.

	void DrawCircleFillPrimitive(Vector pos, int radius, int color)
	{
	    m_Primitives.push_back(new CircleFillPrimitive(pos, radius,color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawCircleFillPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a filled circle primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, radius, color.
// Return value:    None.

	void DrawCircleFillPrimitive(int player, Vector pos, int radius, int color)
	{
		m_Primitives.push_back(new CircleFillPrimitive(player, pos, radius, color));
	}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLinePrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a line primitive.
// Arguments:       Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawLinePrimitive(Vector start, Vector end, int color)
	{
	    m_Primitives.push_back(new LinePrimitive(start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLinePrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a line primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawLinePrimitive(int player, Vector start, Vector end, int color)
	{
		m_Primitives.push_back(new LinePrimitive(player, start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBoxPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a box primitive.
// Arguments:       Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawBoxPrimitive(Vector start, Vector end, int color)
	{
	    m_Primitives.push_back(new BoxPrimitive(start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBoxPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a box primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawBoxPrimitive(int player, Vector start, Vector end, int color)
	{
		m_Primitives.push_back(new BoxPrimitive(player, start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBoxPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a filled box primitive.
// Arguments:       Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawBoxFillPrimitive(Vector start, Vector end, int color)
	{
	    m_Primitives.push_back(new BoxFillPrimitive(start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBoxPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a filled box primitive.
// Arguments:       Player screen to draw primitive on. Player screen to draw primitive on. Position of primitive in scene coordintaes, color.
// Return value:    None.

	void DrawBoxFillPrimitive(int player, Vector start, Vector end, int color)
	{
		m_Primitives.push_back(new BoxFillPrimitive(player, start, end, color));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawTextPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a text primitive.
// Arguments:       Position of primitive in scene coordintaes, text, small or big font
//					alignment 0 = left, 1 = center, 2 = right
// Return value:    None.

	void DrawTextPrimitive(Vector start, std::string text, bool isSmall, int alignment)
	{
	    m_Primitives.push_back(new TextPrimitive(start, text, isSmall, alignment));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawTextPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a text primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, text, small or big font
//					alignment 0 = left, 1 = center, 2 = right
// Return value:    None.

	void DrawTextPrimitive(int player, Vector start, std::string text, bool isSmall, int alignment)
	{
		m_Primitives.push_back(new TextPrimitive(player, start, text, isSmall, alignment));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmapPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a bitmap primitive.
// Arguments:       Position of primitive in scene coordintaes, an entity to sraw sprite from, 
//					rotation angle in radians, frame to draw
// Return value:    None.

	void DrawBitmapPrimitive(Vector start, Entity * pEntity, float rotAngle, int frame);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmapPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a bitmap primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, an entity to sraw sprite from, 
//					rotation angle in radians, frame to draw
// Return value:    None.

	void DrawBitmapPrimitive(int player, Vector start, Entity * pEntity, float rotAngle, int frame);

	
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text height
// Arguments:       Text, max width, whether to use small or large font
// Return value:    None.

	int CalculateTextHeight(std::string text, int maxWidth, bool isSmall);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text width
// Arguments:       Text, whether to use small or large font
// Return value:    None.

	int CalculateTextWidth(std::string text, bool isSmall);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearPrimitivesList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Delete all scheduled primitives, called on every FrameMan sim update.
// Arguments:       None.
// Return value:    None.

	void ClearPrimitivesList()
	{
		for(std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it)
		{
			delete (*it);
		}
		m_Primitives.clear();
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     FrameMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a FrameMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    FrameMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~FrameMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a FrameMan object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~FrameMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the FrameMan object ready for use, which is to be used with
//                  SettingsMan first.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire FrameMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetSplitScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets new values for the split screen configuration.
// Arguments:       Whether the new setting should be horizontally split (over and under).
//                  Whether the new setting should be vertically split (side by side)
// Return value:    None.

    void ResetSplitScreens(bool hSplit = false, bool vSplit = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this FrameMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the FrameMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the FrameMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPM
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between on-screen pixels and the physics engine's
//                  meters.
// Arguments:       None.
// Return value:    A float describing the current PPM ratio.

    float GetPPM() const { return m_PPM; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the physics engine's meters and on-screen
//                  pixels.
// Arguments:       None.
// Return value:    A float describing the current MPP ratio.

    float GetMPP() const { return m_MPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPL
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the on-screen pixels and the physics engine's
//                  Litres.
// Arguments:       None.
// Return value:    A float describing the current PPL ratio.

    float GetPPL() const { return m_PPL; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the physics engine's Litres and on-screen
//                  pixels.
// Arguments:       None.
// Return value:    A float describing the current LPP ratio.

    float GetLPP() const { return m_LPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the horizontal resolution of the screen.
// Arguments:       None.
// Return value:    An int describing the horizontal resolution of the screen in pixels.

    int GetResX() const { return m_ResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the vertical resolution of the screen.
// Arguments:       None.
// Return value:    An int describing the vertical resolution of the screen in pixels.

    int GetResY() const { return m_ResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the horizontal resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       None.
// Return value:    An int describing the horizontal resolution of the new screen in pixels.

    int GetNewResX() const { return m_NewResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the vertical resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       None.
// Return value:    An int describing the vertical resolution of the new screen in pixels.

    int GetNewResY() const { return m_NewResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the horizontal resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       An int describing the horizontal resolution of the new screen in pixels.
// Return value:    None.

    void SetNewResX(int newResX) { m_NewResX = newResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the vertical resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       An int describing the vertical resolution of the new screen in pixels.
// Return value:    None.

    void SetNewResY(int newResY) { m_NewResY = newResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsNewResSet
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whetehr a new resolution has been set for the next time
//                  this FrameMan is created.
// Arguments:       None.
// Return value:    Whether the new resolution set differs from the current one.

    bool IsNewResSet() const { return m_NewResX != m_ResX || m_NewResY != m_ResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bits per pixel color depth
// Arguments:       None.
// Return value:    An int describing the number of bits per pixel of the current color depth.

    int GetBPP() const { return m_BPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPaletteFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ContentFile describing the locaiton of the color palette.
// Arguments:       None.
// Return value:    An reference to a ContentFile which described the palette location.

    const ContentFile & GetPaletteFile() const { return m_PaletteFile; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 8bpp back buffer bitmap
// Arguments:       None.
// Return value:    A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

    BITMAP * GetBackBuffer8() const { return m_pBackBuffer8; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNetworkClinteBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 8bpp back buffer bitmap used to draw network transmitted image on top of everything
// Arguments:       None.
// Return value:    A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

	BITMAP * GetNetworkBackBufferIntermediateGUI8Ready(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBufferGUI8Ready(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBufferIntermediate8Ready(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBuffer8Ready(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameReady][player]; }


	BITMAP * GetNetworkBackBufferIntermediateGUI8Current(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBufferGUI8Current(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBufferIntermediate8Current(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBuffer8Current(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][player]; }




	bool IsNetworkBitmapLocked(int player) const { return m_NetworkBitmapIsLocked[player]; }

	bool GetDrawNetworkBackBuffer() { return m_DrawNetworkBackBuffer; }

	void SetDrawNetworkBackBuffer(bool value) { m_DrawNetworkBackBuffer = value; }

	bool GetStoreNetworkBackBuffer() { return m_StoreNetworkBackBuffer; }

	void SetStoreNetworkBackBuffer(bool value) { m_StoreNetworkBackBuffer = value; }

	void CreateNewPlayerBackBuffer(int player, int w, int h);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBackBuffer32
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 32bpp back buffer bitmap, if available. If not, the 8bpp is
//                  returned. Make sure you don't do any blending stuff to the 8bpp one!
// Arguments:       None.
// Return value:    A pointer to the BITMAP 32bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

    BITMAP * GetBackBuffer32() const { return m_pBackBuffer32 ? m_pBackBuffer32 : m_pBackBuffer8; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of currently active screens, counting all splits.
// Arguments:       None.
// Return value:    The number of currently active screens.

    int GetScreenCount() const { return m_HSplit || m_VSplit ? (m_HSplit && m_VSplit ? 4 : 2) : 1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether the screen is split horizontally across the screen, ie
//                  as two splitscreens one above the other.
// Arguments:       None.
// Return value:    Whether or not screen has a horizontal split.

    bool GetHSplit() const { return m_HSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetVSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether the screen is split vertically across the screen, ie
//                  as two splitscreens side by side.
// Arguments:       None.
// Return value:    Whether screen has a vertical split.

    bool GetVSplit() const { return m_VSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The width of the player screens.

	int GetPlayerScreenWidth() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The width of the player screens.

	int GetPlayerFrameBufferWidth(int whichPlayer) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The height of the player screens.

	int GetPlayerScreenHeight() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The height of the player screens.

	int GetPlayerFrameBufferHeight(int whichPlayer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBlackIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a font from the GUI engine's current skin. Ownership is NOT
//                  transferred!
// Arguments:       The font filename in the currently used skin, NOT including the skin
//                  path.
// Return value:    A pointer to the requested font, or 0 if no font with that filename
//                  is in the current skin was found. Ownership is NOT transferred!

    unsigned char GetBlackIndex() const { return m_BlackColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLargeFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the requested font, or 0 if no large font was found.
//                  Ownership is NOT transferred!

    GUIFont * GetLargeFont();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSmallFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the requested font, or 0 if no large font was found.
//                  Ownership is NOT transferred!

    GUIFont * GetSmallFont();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether the screen is split horizontally across the screen, ie
//                  as two splitscreens one above the other.
// Arguments:       Whether or not to have a horizontal split.
// Return value:    None.

    void SetHSplit(bool hSplit) { m_HSplit = hSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetVSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether the screen is split vertically across the screen, ie
//                  as two splitscreens side by side.
// Arguments:       Whether or not to have a vertical split.
// Return value:    None.

    void SetVSplit(bool vSplit) { m_VSplit = vSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPPM
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ratio between on-screen pixels and the physics engine's
//                  meters.
// Arguments:       A float specifying the new PPM ratio.
// Return value:    None.

    void SetPPM(const float newPPM) { m_PPM = newPPM; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the message to be displayed on top of each player's screen
// Arguments:       An std::string that specifies what should be displayed.
//                  Which screen you want to set text to.
//                  The interval with which the screen will be blinking, in ms. 0 means no
//                  blinking.
//                  The duration, in MS to force this message to display. No other message
//                  can be displayed before this expires. ClearScreenText overrides it though.
//                  Vertically centered on the screen
// Return value:    None.

    void SetScreenText(const std::string &msg, int which = 0, int blinkInterval = 0, int displayDuration = -1, bool centered = false);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the message to be displayed on top of each player's screen
// Arguments:       None.
// Return value:    Current message shown to player.

	std::string GetScreenText(int which = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the message to be displayed on top of each player's screen
// Arguments:       Which screen you want to set text to.
// Return value:    None.

    void ClearScreenText(int which = 0) { if (which >= 0 && which < MAXSCREENCOUNT) { m_ScreenText[which].clear(); m_TextDuration[which] = -1; m_TextDurationTimer[which].Reset(); m_TextBlinking[which] = 0; } }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether we're in fullscreen mode or not.
// Arguments:       None.
// Return value:    Whether we're in fullscreen mode.

    bool IsFullscreen() const { return m_Fullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NxWindowed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how many times the windowed screen resolution is being multiplied
//                  and the back buffer stretched across for better readibility.
// Arguments:       None.
// Return value:    What multiple the windowed mode screen resolution is run in. (1 normal)

    int NxWindowed() const { return m_NxWindowed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how many times the fullscreen resolution is being multiplied
//                  and the back buffer stretched across for better readibility.
// Arguments:       None.
// Return value:    What multiple the fullscreen mode screen resolution is run in. (1 normal)

    int NxFullscreen() const { return m_NxFullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewNxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how many times the fullscreen resolution will be multiplied ON
//                  NEXT RESTART and the back buffer stretched.
// Arguments:       What multiple the fullscreen mode screen resolution will be run in.
//                  on next restart of game.
// Return value:    None.

    void SetNewNxFullscreen(int newMultiple) { m_NewNxFullscreen = newMultiple; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewNxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many times the fullscreen resolution will be multiplied ON
//                  NEXT RESTART and the back buffer stretched.
// Arguments:       None.
// Return value:    What multiple the fullscreen mode screen resolution will be run in.
//                  on next restart of game.

    int GetNewNxFullscreen() const { return m_NewNxFullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnablePostProcessing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether we're postprocessing at all.
// Arguments:       Whether post processing should be enabled or not.
// Return value:    None.

    void EnablePostProcessing(bool enable = true) { m_PostProcessing = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPostProcessing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether we're postprocessing or not
// Arguments:       None.
// Return value:    Whether post processing is enabled or not.

    bool IsPostProcessing() const { return m_PostProcessing && m_BPP == 32; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnablePixelGlow
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether we're postprocessing the pixel glow effect or not
// Arguments:       Whether post processing should be enabled or not.
// Return value:    None.

    void EnablePixelGlow(bool enable = true) { if (enable) { EnablePostProcessing(enable); } m_PostPixelGlow = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPixelGlowEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether we're postprocessing the pixel glow effect or not
// Arguments:       None.
// Return value:    Whether post processing is enabled or not.

    bool IsPixelGlowEnabled() const { return IsPostProcessing() && m_PostPixelGlow; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDotGlowEffect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific standard dot glow effect for making pixels glow
// Arguments:       Which of the dot glow colors to get, see the DotGlowColor enum.
// Return value:    The requested glow dot BITMAP.

    BITMAP * GetDotGlowEffect(DotGlowColor which) const
    {
        if (which == NoDot)
            return 0;
        else if (which == YellowDot)
            return m_pYellowGlow;
        else if (which == RedDot)
            return m_pRedGlow;
        else
            return m_pBlueGlow;
    }

	size_t GetDotGlowEffectHash(DotGlowColor which) const
	{
		if (which == NoDot)
			return 0;
		else if (which == YellowDot)
			return m_YellowGlowHash;
		else if (which == RedDot)
			return m_RedGlowHash;
		else
			return m_BlueGlowHash;
	}

	void GetPostEffectsList(int whichScreen, std::list<PostEffect> & outputList);

	void SetPostEffectsList(int whichScreen, std::list<PostEffect> & inputList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a palette from a .dat file and sets it as the currently used
//                  screen palette.
// Arguments:       String with the data path to the palette data object within a .dat.
// Return value:    Whether palette laoded successfully or not.

    bool LoadPalette(std::string palettePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeInPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette in from black at a specified speed.
// Arguments:       Speed specififed from (slowest) 1 - 64 (fastest)
// Return value:    None.

    void FadeInPalette(int fadeSpeed = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOutPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette out to black at a specified speed.
// Arguments:       Speed specififed from (slowest) 1 - 64 (fastest)
// Return value:    None.

    void FadeOutPalette(int fadeSpeed = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTransTable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific precalculated transperency table which is used for
//                  any subsequent transparency drawing.
// Arguments:       The transperency setting, see the enum.
// Return value:    None.

    void SetTransTable(TransperencyPreset transSetting);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScreenToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap of the screen back buffer to a 8bpp BMP file.
// Arguments:       The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveScreenToBMP(const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveWorldToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap of everything on the scene to the BMP file.
// Arguments:       The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveWorldToBMP(const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveBitmapToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap to a 8bpp BMP file.
// Arguments:       A pointer to a BITMAP to save.
//                  The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveBitmapToBMP(BITMAP *pBitmap, const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetFrameTimer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the frame timer to restart counting.
// Arguments:       None.
// Return value:    None.

    void ResetFrameTimer() { m_pFrameTimer->Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetRTE
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Orders to reset the entire Retro Terrain Engine system next iteration.
// Arguments:       None.
// Return value:    None.

    void ResetRTE() { m_ResetRTE = true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsResettingRTE
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether the system is about to be reset before the next loop
//                  starts.
// Arguments:       None.
// Return value:    Whether the RTE is about to reset next iteration of the loop or not.

    bool IsResettingRTE() { return m_ResetRTE; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Toggles to and from fullscreen and windowed mode.
// Arguments:       None.
// Return value:    Error code, anything other than 0 is error..

    int ToggleFullscreen();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchWindowMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets and switches to a new windowed mode multiplier.
// Arguments:       The multiplier to switch to.
// Return value:    Error code, anything other than 0 is error..

    int SwitchWindowMultiplier(int multiplier = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PostProcess
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current state of the 8bpp back buffer, copies it, and adds
//                  post processing effects on top like glows etc. Only works in 32bpp mode.
// Arguments:       None.
// Return value:    None.

    void PostProcess();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the 8bpp backbuffer with black.
// Arguments:       None.
// Return value:    None.

    void ClearBackBuffer8() { clear_to_color(m_pBackBuffer8, m_BlackColor); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearBackBuffer32
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the 32bpp backbuffer with black.
// Arguments:       None.
// Return value:    None.

    void ClearBackBuffer32() { if (m_pBackBuffer32) clear_to_color(m_pBackBuffer32, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlipFrameBuffers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flips the framebuffer.
// Arguments:       None.
// Return value:    None.

    void FlipFrameBuffers();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlippingWith32BPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the 32bpp framebuffer is currnetly being used or not
//                  when flipping the frame buffers.
// Arguments:       None.
// Return value:    Whether the 32bpp is used or not (8bpp one is).

    bool FlippingWith32BPP() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowPerformanceStats
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whetehr to display the performance stats on-screen or not.
// Arguments:       Whether to show the performance stats or not.
// Return value:    None.

    void ShowPerformanceStats(bool showStats = true) { m_ShowPerfStats = showStats; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsShowingPerformanceStats
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whetehr to display the performance stats on-screen or not.
// Arguments:       None.
// Return value:    Whether to show the performance stats or not.

    bool IsShowingPerformanceStats() const { return m_ShowPerfStats; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowMessageBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pops up a message box dialog in the OS. For debug purposes mostly.
// Arguments:       The string that the message box should display.
// Return value:    None.

    void ShowMessageBox(std::string message) const { allegro_message("%s", message.c_str()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlashScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flashes any of the players' screen red for this frame.
// Arguments:       Which screen to flash.
//                  What color to flash it. -1 means no color or flash.
//                  How long a period to fill the frame with color. If 0, a single-frame
//                  flash will happen.
// Return value:    None.

    void FlashScreen(int screen, int color, float periodMS = 0) { m_FlashScreenColor[screen] = color; m_FlashTimer[screen].SetRealTimeLimitMS(periodMS); m_FlashTimer[screen].Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a text string to the bitmap of choice, using the internal
//                  fontsets.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The string containing the text to draw.
//                  the position the upper right corner of the first char of the text
//                  string will have on the bitmap.
//                  Whether to draw text with black chars or not (white).
// Return value:    None.

    void DrawText(BITMAP *pTargetBitmap,
                  const std::string &str,
                  const Vector &pos,
                  bool black = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted or with other effects.
// Arguments:       The Bitmap to draw to. Ownership is NOT transferred.
//                  The absolute Start point.
//                  The absolute end point.
//                  The color value of the line.
//                  A color to alternate with every other pixel drawn will have this if !0.
//                  How many pixels to skip drawing between drawn ones. 0 means solid line
//                  2 means there's a gap of two pixels between each drawn one.
//                  The start of the skipping phase. If skip is 10 and this is 5, the first
//                  dot will will be drawn after 5 pixels.
//                  Whether the line should take the shortest possible route across scene
//                  wraps.
// Return value:    The end state of the skipping phase. Eg if 4 is returned here the last
//                  dot was placed 4 pixels ago.

    int DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawDotLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted with bitmaps.
// Arguments:       The Bitmap to draw to. Ownership is NOT transferred.
//                  The absolute Start point.
//                  The absolute end point.
//                  The bitmap to be used for dots; will be centered.
//                  How many pixels to gap between drawing dots. Should be more than 0
//                  The start of the skipping phase. If skip is 10 and this is 5, the first
//                  dot will will be drawn after 5 pixels.
//                  Whether the line should take the shortest possible route across scene
//                  wraps.
// Return value:    The end state of the skipping phase. Eg if 4 is returned here the last
//                  dot was placed 4 pixels ago.

    int DrawDotLine(BITMAP *pBitmap, const Vector &start, const Vector &end, BITMAP *pDot, int skip = 0, int skipStart = 0, bool shortestWrap = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this FrameMan. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the current frame to the screen.
// Arguments:       None.
// Return value:    None.

    void Draw();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawPrimitives
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws all stored primitives on the screen
// Arguments:       Bitmap to draw on, poistion to draw.
// Return value:    None.

    void DrawPrimitives(int player, BITMAP *pTargetBitmap, const Vector &targetPos);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NewPerformanceSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Moves sample counter to next sample and clears it's values.
// Arguments:       None.
// Return value:    None.
	void NewPerformanceSample()
	{
		m_Sample++;
		if (m_Sample >= MAXSAMPLES)
			m_Sample = 0;

		for (int pc = 0; pc < PERF_COUNT; ++pc)
		{
			m_PerfData[pc][m_Sample] = 0;
			m_PerfPercentages[pc][m_Sample] = 0;
		}
	}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartPerformanceMeasurement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves current absolute time in microseconds as a start of performance measurerement.
// Arguments:       Counter to start measurement for.
// Return value:    None.
	void StartPerformanceMeasurement(PerformanceCounters counter) { m_PerfMeasureStart[counter] = g_TimerMan.GetAbsoulteTime(); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopPerformanceMeasurement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves current absolute time in microseconds as an end of performance measurerement. 
//					The difference is added to the value of current perofrmance sample.
// Arguments:       Counter to stop and updated measurement for.
// Return value:    None.
	void StopPerformanceMeasurement(PerformanceCounters counter) 
	{ 
		m_PerfMeasureStop[counter] = g_TimerMan.GetAbsoulteTime(); 
		AddPerformanceSample(counter, m_PerfMeasureStop[counter] - m_PerfMeasureStart[counter]);
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsValidResolution	
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if this resolution is supported
// Arguments:       Resolution width and height
// Return value:    True if resolution is supported
	bool IsValidResolution(int width, int height) const;

	Vector GetTargetPos(int screen) { return m_TargetPos[m_NetworkFrameReady][screen]; }

	Vector SLOffset[MAXSCREENCOUNT][MAX_LAYERS_STORED_FOR_NETWORK];

	bool IsInMultiplayerMode() { return m_StoreNetworkBackBuffer; }

	void SetCurrentPing(unsigned int ping) { m_CurrentPing = ping; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

    // Resolution
    int m_ResX;
    int m_ResY;
    // Color depth (bits per pixel)
    int m_BPP;

    // These are the new resolution settings that will take effect next time the FrameMan is started
    int m_NewResX;
    int m_NewResY;

    // Screen back buffer, always 8bpp, gets copied to the 32bpp buffer if post processing is used
    BITMAP *m_pBackBuffer8;
    // 32Bits per pixel back buffer, only used if player elects, and only if in 32bpp video mode
    BITMAP *m_pBackBuffer32;

	// Per-player allocated frame buffer to draw upon during frameman draw
	BITMAP *m_pNetworkBackBufferIntermediate8[2][MAXSCREENCOUNT];

	// Per-player allocated frame buffer to draw upon during frameman draw used to draw UI only
	BITMAP *m_pNetworkBackBufferIntermediateGUI8[2][MAXSCREENCOUNT];


	// Per-player allocated frame buffer to copy Intermediate before sending
	BITMAP *m_pNetworkBackBufferFinal8[2][MAXSCREENCOUNT];

	// Per-player allocated frame buffer to copy Intermediate before sending used to draw UI only
	BITMAP *m_pNetworkBackBufferFinalGUI8[2][MAXSCREENCOUNT];


	// If true, draws the contents of the m_pNetworkBackBuffer8 on top of m_pBackBuffer8 every frame in FrameMan.Draw
	bool m_DrawNetworkBackBuffer;

	// If true, dumps the contents of the m_pBackBuffer8 to the m_pNetworkBackBuffer8 every frame
	bool m_StoreNetworkBackBuffer;

    // Temporary buffer for making quick screencaps
    BITMAP *m_pScreendumpBuffer;

    // Data file of the screen palette
    ContentFile m_PaletteFile;
    DATAFILE *m_pPaletteDataFile;

    // Transparency color tables
    COLOR_MAP m_LessTransTable;
    COLOR_MAP m_HalfTransTable;
    COLOR_MAP m_MoreTransTable;

    // Color black index
    unsigned char m_BlackColor;
    unsigned char m_AlmostBlackColor;

    // Whether in fullscreen mode or not
    bool m_Fullscreen;
    // The number of times the windowed mode resoltion should be multiplied and streched across for better visibility
    int m_NxWindowed;
    // The number of times the fullscreen mode resolution should be multiplied and streched across for better visibility
    // The internal virtual resolution (m_ResX, m_ResY) is 1/n of the acutal fullscreen res that the graphics card outputs
    int m_NxFullscreen;
    // This is the new fullscreen multiple that will take effect next time the FrameMan is started
    int m_NewNxFullscreen;

    // Whether postprocessing is enabled or not
    bool m_PostProcessing;
    // Whether postprocessing pixel glow is enabled or not
    bool m_PostPixelGlow;
    BITMAP *m_pYellowGlow;
	size_t m_YellowGlowHash;
    BITMAP *m_pRedGlow;
	size_t m_RedGlowHash;
	BITMAP *m_pBlueGlow;
	size_t m_BlueGlowHash;

    // List of effects to apply at the end of each frame, Vector is in total absolute screen coordinates, and the BITMAP is not owned.
    // This list gets cleared out and re-filled each frame.
    std::list<PostEffect> m_PostScreenEffects;
    // List of screen-relative areas that will be processed with glow
    std::list<Box> m_PostScreenGlowBoxes;
	// Temp bitmap to rotate post effects in it
	BITMAP * m_pTempEffectBitmap_16;
	BITMAP * m_pTempEffectBitmap_32;
	BITMAP * m_pTempEffectBitmap_64;
	BITMAP * m_pTempEffectBitmap_128;
	BITMAP * m_pTempEffectBitmap_256;

    // Whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.
    bool m_HSplit;
    // Whether the screen is split vertically across the screen, ie as two splitscreens side by side.
    bool m_VSplit;
    // Whether the screen is set to split horizontally in settings
    bool m_HSplitOverride;
    // Whether the screen is set to split vertically in settings
    bool m_VSplitOverride;
    // Intermediary split screen bitmap.
    BITMAP *m_pPlayerScreen;
    // Dimensions of each of the screens of each player. Will be smaller than resolution only if the screen is split
    int m_PlayerScreenWidth;
    int m_PlayerScreenHeight;

    // Pixels Per Meter constant
    float m_PPM;
    // Meters Per Pixel constant.
    float m_MPP;
    // Pixels per Litre contstant
    float m_PPL;
    // Litres Per Pixel contstant
    float m_LPP;

    // Timer for measuring millisecs per frame for performance stats readings
    Timer *m_pFrameTimer;
    // History log of readings, for averaging the results
    std::deque<int> m_MSPFs;
    // The average of the MSPF reading buffer above, computer each frame
    int m_MSPFAverage;
    // The sim speed over real time
    float m_SimSpeed;

    // Signals to reset the entire RTE next iteration.
    bool m_ResetRTE;

    // The GUI control managers for all teams
//    std::vector<GUIControlManager *> m_BuyGUIs;

    // GUI screen object kept and owned just for the fonts
    AllegroScreen *m_pGUIScreen;
    // Standard fonts for quick access.
    GUIFont *m_pLargeFont;
    GUIFont *m_pSmallFont;
    // Whether to show performance stats on screen or not
    bool m_ShowPerfStats;
    // The text to be displayed on each player's screen
    std::string m_ScreenText[MAXSCREENCOUNT];
    // The minimum duration the current message is supposed to show vefore it can be overwritten
    int m_TextDuration[MAXSCREENCOUNT];
    // Screen text display duration time
    Timer m_TextDurationTimer[MAXSCREENCOUNT];
    // Screen text messages blinking interval in ms. 0 is no blink at all, just show message.
    int m_TextBlinking[MAXSCREENCOUNT];
    // Whether screen text is centered vertically
    bool m_TextCentered[MAXSCREENCOUNT];
    // Screen text blink timer
    Timer m_TextBlinkTimer;
    // Whether to flash a player's screen a specific color this frame. -1 means no flash
    int m_FlashScreenColor[MAXSCREENCOUNT];
    // Whether we flashed last frame or not
    bool m_FlashedLastFrame[MAXSCREENCOUNT];
    // Flash screen timer
    Timer m_FlashTimer[MAXSCREENCOUNT];
	// Frame target pos for network players
	Vector m_TargetPos[2][MAXSCREENCOUNT];
	// Which frame index is being rendered, 0 or 1
	int m_NetworkFrameCurrent;
	// Which frame is rendered and ready for transmission, 0 or 1
	int m_NetworkFrameReady;

	std::list<PostEffect> m_ScreenRelativeEffects[MAXSCREENCOUNT];


	//Performance data
	//How many performance samples to store, directly affects graph size
	const static int MAXSAMPLES = 120;
	// How many samples to use to calculate average value displayed on screen
	const static int AVERAGE = 10;

	// Array to store perormance measurements in microseconds
	int64_t m_PerfData[PERF_COUNT][MAXSAMPLES];
	// Current measurement start time in microseconds
	int64_t m_PerfMeasureStart[PERF_COUNT];
	// Current measurement stop time in microseconds
	int64_t m_PerfMeasureStop[PERF_COUNT];
	// Array to store percentages from PERF_SIM_TOTAL
	int m_PerfPercentages[PERF_COUNT][MAXSAMPLES];
	// Perormance counter's names displayed on screen
	string m_PerfCounterNames[PERF_COUNT];
	// Sample counter
	int m_Sample;
	// Current ping value to display on screen
	int m_CurrentPing;

	// If true then the network bitmap is being updated
	bool m_NetworkBitmapIsLocked[MAXSCREENCOUNT];
	//std::mutex m_NetworkBitmapIsLocked[MAXSCREENCOUNT];

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPerformanceSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds provided value to current sample of specified performance counter
// Arguments:       Counter to update, value to add to this counter
// Return value:    None.
	void AddPerformanceSample(PerformanceCounters counter, int64_t value) { m_PerfData[counter][m_Sample] += value; }
	
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateSamplePercentages
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates current sample's percentages from SIM_TOTAL for all performance counters 
//					and stores them to m_PerfPercenrages.
// Arguments:       None.
// Return value:    None.
	void CalculateSamplePercentages()
	{
		for(int pc = 0 ; pc < FrameMan::PERF_COUNT; ++pc)
		{
			int perc = (int)((float)m_PerfData[pc][m_Sample] / (float)m_PerfData[pc][FrameMan::PERF_SIM_TOTAL] * 100);
			m_PerfPercentages[pc][m_Sample] = perc;
		}
	}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPerormanceCounterAverage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an average value of AVERAGE last samples for specified performance counter
// Arguments:       None.
// Return value:    An average value for specified counter.
	int64_t GetPerormanceCounterAverage(PerformanceCounters counter)
	{
		int64_t accum = 0;
		int smpl = m_Sample;

		for (int i = 0 ; i < AVERAGE; ++i)
		{
			accum += m_PerfData[counter][smpl];

			smpl--;
			if (smpl < 0)
				smpl = MAXSAMPLES - 1;
		}

		return accum / AVERAGE;
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this FrameMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    FrameMan(const FrameMan &reference);
    FrameMan & operator=(const FrameMan &rhs);

};

} // namespace RTE

#endif // File
