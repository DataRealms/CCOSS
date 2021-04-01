#ifndef _RTENETWORKMSG_
#define _RTENETWORKMSG_

#include "MessageIdentifiers.h"

namespace RTE {

	/// <summary>
	/// Enumeration for the different types of network message IDs.
	/// </summary>
	enum CustomMessageIDTypes {
		ID_CUSTOM_START = ID_USER_PACKET_ENUM,
		ID_NAT_SERVER_REGISTER_SERVER,
		ID_NAT_SERVER_REGISTER_ACCEPTED,
		ID_NAT_SERVER_GET_SERVER_GUID,
		ID_NAT_SERVER_GUID,
		ID_NAT_SERVER_NO_GUID,
		ID_CLT_REGISTER,
		ID_CLT_INPUT,
		ID_SRV_ACCEPTED,
		ID_SRV_FRAME_SETUP,
		ID_SRV_FRAME_LINE,
		ID_SRV_FRAME_BOX,
		ID_SRV_SCENE_SETUP,
		ID_CLT_SCENE_SETUP_ACCEPTED,
		ID_SRV_SCENE,
		ID_SRV_SCENE_END,
		ID_CLT_SCENE_ACCEPTED,
		ID_CLT_DISCONNECT,
		ID_SRV_TERRAIN,
		ID_SRV_POST_EFFECTS,
		ID_SRV_SOUND_EVENTS,
		ID_SRV_MUSIC_EVENTS
	};

// Pack the structs so 1 byte members are exactly 1 byte in memory instead of being aligned by 4 bytes (padding) so the correct representation is sent over the network without empty bytes consumed by alignment.
#pragma pack(push, 1)

	// TODO: Figure out all these and add comments.

	/// <summary>
	/// 
	/// </summary>
	struct MsgRegisterServer {
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
		char ServerGuid[64];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgRegisterServerAccepted {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgGetServerRequest {
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgGetServerAnswer {
		unsigned char Id;

		char ServerGuid[64];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgGetServerNoAnswer {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgRegister {
		unsigned char Id;

		int ResolutionX;
		int ResolutionY;

		char Name[64];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgFrameSetup {
		unsigned char Id;
		unsigned char FrameNumber;

		short int TargetPosX;
		short int TargetPosY;

		float OffsetX[c_MaxLayersStoredForNetwork];
		float OffsetY[c_MaxLayersStoredForNetwork];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgFrameLine {
		unsigned char Id;
		unsigned char FrameNumber;

		unsigned char Layer;
		unsigned short int LineNumber;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgFrameBox {
		unsigned char Id;
		unsigned char FrameNumber;

		unsigned char Layer;
		unsigned short int BoxX;
		unsigned short int BoxY;
		unsigned char BoxWidth;
		unsigned char BoxHeight;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgDisconnect {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgAccepted {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct LightweightSceneLayer {
		size_t BitmapHash;
		bool DrawTrans;
		float OffsetX;
		float OffsetY;
		float ScrollInfoX;
		float ScrollInfoY;
		float ScrollRatioX;
		float ScrollRatioY;
		float ScaleFactorX;
		float ScaleFactorY;
		float ScaleInverseX;
		float ScaleInverseY;
		float ScaledDimensionsX;
		float ScaledDimensionsY;

		bool WrapX;
		bool WrapY;

		unsigned char FillLeftColor;
		unsigned char FillRightColor;
		unsigned char FillUpColor;
		unsigned char FillDownColor;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgSceneSetup {
		unsigned char Id;
		unsigned char SceneId;
		short int Width;
		short int Height;
		bool SceneWrapsX;

		short int BackgroundLayerCount;
		LightweightSceneLayer BackgroundLayers[c_MaxLayersStoredForNetwork];
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgSceneLine {
		unsigned char Id;
		unsigned char SceneId;
		unsigned short int X;
		unsigned short int Y;
		unsigned short int Width;
		unsigned char Layer;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgSceneEnd {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgSceneAccepted {
		unsigned char Id;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgTerrainChange {
		unsigned char Id;

		unsigned short int X;
		unsigned short int Y;
		unsigned short int W;
		unsigned short int H;
		bool Back;
		unsigned char Color;
		unsigned char SceneId;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	/// <summary>
	/// 
	/// </summary>
	struct PostEffectNetworkData {
		short int X;
		short int Y;
		size_t BitmapHash;
		short int Strength;
		float Angle;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgPostEffects {
		unsigned char Id;
		unsigned char FrameNumber;
		int PostEffectsCount;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgSoundEvents {
		unsigned char Id;
		unsigned char FrameNumber;
		int SoundEventsCount;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgMusicEvents {
		unsigned char Id;
		unsigned char FrameNumber;
		int MusicEventsCount;
	};

	/// <summary>
	/// 
	/// </summary>
	struct MsgInput {
		unsigned char Id;

		int MouseX;
		int MouseY;
		bool MouseButtonPressed[MAX_MOUSE_BUTTONS];
		bool MouseButtonReleased[MAX_MOUSE_BUTTONS];
		bool MouseButtonHeld[MAX_MOUSE_BUTTONS];
		bool ResetActivityVote;

		int MouseWheelMoved;

		unsigned int InputElementPressed;
		unsigned int InputElementReleased;
		unsigned int InputElementHeld;
	};

// Disables the previously set pack pragma.
#pragma pack(pop)
}
#endif