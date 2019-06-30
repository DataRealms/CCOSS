#pragma once

namespace RTE
{

#define MAX_PIXEL_LINE_BUFFER_SIZE 8192
#define MAX_BACKGROUND_LAYERS_TRANSMITTED 10
#define FRAMES_TO_REMEMBER 3

#define MAX_CLIENTS 4


	enum CustomMessageIDTypes
	{
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

#pragma pack(push, 1)
	struct MsgRegisterServer
	{
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
		char ServerGuid[64];
	};

	struct MsgRegisterServerAccepted
	{
		unsigned char Id;
	};

	struct MsgGetServerRequest
	{
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
	};

	struct MsgGetServerAnswer
	{
		unsigned char Id;

		char ServerGuid[64];
	};

	struct MsgGetServerNoAnswer
	{
		unsigned char Id;
	};

	struct MsgRegister
	{
		unsigned char Id;

		int ResolutionX;
		int ResolutionY;

		char Name[64];
	};

	struct MsgFrameSetup
	{
		unsigned char Id;
		unsigned char FrameNumber;

		short int TargetPosX;
		short int TargetPosY;

		float OffsetX[MAX_BACKGROUND_LAYERS_TRANSMITTED];
		float OffsetY[MAX_BACKGROUND_LAYERS_TRANSMITTED];
	};

	struct MsgFrameLine
	{
		unsigned char Id;
		unsigned char FrameNumber;

		unsigned char Layer;
		unsigned short int LineNumber;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	struct MsgFrameBox
	{
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

	struct MsgDisconnect
	{
		unsigned char Id;
	};

	struct MsgAccepted
	{
		unsigned char Id;
	};

	struct LightweightSceneLayer
	{
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

	struct MsgSceneSetup
	{
		unsigned char Id;
		unsigned char SceneId;
		short int Width;
		short int Height;
		bool SceneWrapsX;

		short int BackgroundLayerCount;
		LightweightSceneLayer BackgroundLayers[MAX_BACKGROUND_LAYERS_TRANSMITTED];
	};

	struct MsgSceneLine
	{
		unsigned char Id;
		unsigned char SceneId;
		unsigned short int X;
		unsigned short int Y;
		unsigned short int Width;
		unsigned char Layer;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	struct MsgSceneEnd
	{
		unsigned char Id;
	};

	struct MsgSceneAccepted
	{
		unsigned char Id;
	};

	struct MsgTerrainChange
	{
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

	struct PostEffectNetworkData
	{
		short int X;
		short int Y;
		size_t BitmapHash;
		short int Strength;
	};

	struct MsgPostEffects
	{
		unsigned char Id;
		unsigned char FrameNumber;
		int PostEffectsCount;
	};

	struct MsgSoundEvents
	{
		unsigned char Id;
		unsigned char FrameNumber;
		int SoundEventsCount;
	};

	struct MsgMusicEvents
	{
		unsigned char Id;
		unsigned char FrameNumber;
		int MusicEventsCount;
	};

#pragma pack(pop)


}

