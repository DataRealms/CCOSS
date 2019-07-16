#!/bin/bash

ENTITIES_SRCS="
    Entities/MultiplayerGame.cpp
    Entities/MultiplayerServerLobby.cpp
    Entities/Controller.cpp
    Entities/Emission.cpp
    Entities/PEmitter.cpp
    Entities/ACDropShip.cpp
    Entities/ACrab.cpp
    Entities/ACraft.cpp
    Entities/ACRocket.cpp
    Entities/Actor.cpp
    Entities/ADoor.cpp
    Entities/AEmitter.cpp
    Entities/AHuman.cpp
    Entities/Arm.cpp
    Entities/Atom.cpp
    Entities/AtomGroup.cpp
    Entities/Attachable.cpp
    Entities/BunkerAssembly.cpp
    Entities/BunkerAssemblyScheme.cpp
    Entities/Deployment.cpp
    Entities/Entity.cpp
    Entities/GlobalScript.cpp
    Entities/HDFirearm.cpp
    Entities/HeldDevice.cpp
    Entities/Icon.cpp
    Entities/Leg.cpp
    Entities/LimbPath.cpp
    Entities/Loadout.cpp
    Entities/Magazine.cpp
    Entities/Material.cpp
    Entities/MetaPlayer.cpp
    Entities/MOPixel.cpp
    Entities/MOSParticle.cpp
    Entities/MOSprite.cpp
    Entities/MOSRotating.cpp
    Entities/MovableObject.cpp
    Entities/Scene.cpp
    Entities/SceneLayer.cpp
    Entities/SceneObject.cpp
    Entities/SLTerrain.cpp
    Entities/Sound.cpp
    Entities/TDExplosive.cpp
    Entities/TerrainDebris.cpp
    Entities/TerrainObject.cpp
    Entities/ThrownDevice.cpp
    Entities/Turret.cpp
    Entities/ActorEditor.cpp
    Entities/AreaEditor.cpp
    Entities/AssemblyEditor.cpp
    Entities/BaseEditor.cpp
    Entities/EditorActivity.cpp
    Entities/GABaseDefense.cpp
    Entities/GABrainMatch.cpp
    Entities/GameActivity.cpp
    Entities/GAScripted.cpp
    Entities/GATutorial.cpp
    Entities/GibEditor.cpp
    Entities/SceneEditor.cpp
"
RAKNET_LOCATION="external/sources/RakNet"
RAKNET_SRCS="
    $RAKNET_LOCATION/Base64Encoder.cpp
    $RAKNET_LOCATION/BitStream.cpp
    $RAKNET_LOCATION/CCRakNetSlidingWindow.cpp
    $RAKNET_LOCATION/CCRakNetUDT.cpp
    $RAKNET_LOCATION/CheckSum.cpp
    $RAKNET_LOCATION/CloudClient.cpp
    $RAKNET_LOCATION/CloudCommon.cpp
    $RAKNET_LOCATION/CloudServer.cpp
    $RAKNET_LOCATION/CommandParserInterface.cpp
    $RAKNET_LOCATION/ConnectionGraph2.cpp
    $RAKNET_LOCATION/ConsoleServer.cpp
    $RAKNET_LOCATION/DataCompressor.cpp
    $RAKNET_LOCATION/DirectoryDeltaTransfer.cpp
    $RAKNET_LOCATION/DR_SHA1.cpp
    $RAKNET_LOCATION/DS_BytePool.cpp
    $RAKNET_LOCATION/DS_ByteQueue.cpp
    $RAKNET_LOCATION/DS_HuffmanEncodingTree.cpp
    $RAKNET_LOCATION/DS_Table.cpp
    $RAKNET_LOCATION/DynDNS.cpp
    $RAKNET_LOCATION/EmailSender.cpp
    $RAKNET_LOCATION/EpochTimeToString.cpp
    $RAKNET_LOCATION/FileList.cpp
    $RAKNET_LOCATION/FileListTransfer.cpp
    $RAKNET_LOCATION/FileOperations.cpp
    $RAKNET_LOCATION/FormatString.cpp
    $RAKNET_LOCATION/FullyConnectedMesh2.cpp
    $RAKNET_LOCATION/Getche.cpp
    $RAKNET_LOCATION/Gets.cpp
    $RAKNET_LOCATION/GetTime.cpp
    $RAKNET_LOCATION/gettimeofday.cpp
    $RAKNET_LOCATION/GridSectorizer.cpp
    $RAKNET_LOCATION/HTTPConnection.cpp
    $RAKNET_LOCATION/HTTPConnection2.cpp
    $RAKNET_LOCATION/IncrementalReadInterface.cpp
    $RAKNET_LOCATION/Itoa.cpp
    $RAKNET_LOCATION/LinuxStrings.cpp
    $RAKNET_LOCATION/LocklessTypes.cpp
    $RAKNET_LOCATION/LogCommandParser.cpp
    $RAKNET_LOCATION/MessageFilter.cpp
    $RAKNET_LOCATION/NatPunchthroughClient.cpp
    $RAKNET_LOCATION/NatPunchthroughServer.cpp
    $RAKNET_LOCATION/NatTypeDetectionClient.cpp
    $RAKNET_LOCATION/NatTypeDetectionCommon.cpp
    $RAKNET_LOCATION/NatTypeDetectionServer.cpp
    $RAKNET_LOCATION/NetworkIDManager.cpp
    $RAKNET_LOCATION/NetworkIDObject.cpp
    $RAKNET_LOCATION/PacketConsoleLogger.cpp
    $RAKNET_LOCATION/PacketFileLogger.cpp
    $RAKNET_LOCATION/PacketizedTCP.cpp
    $RAKNET_LOCATION/PacketLogger.cpp
    $RAKNET_LOCATION/PacketOutputWindowLogger.cpp
    $RAKNET_LOCATION/PluginInterface2.cpp
    $RAKNET_LOCATION/PS4Includes.cpp
    $RAKNET_LOCATION/Rackspace.cpp
    $RAKNET_LOCATION/RakMemoryOverride.cpp
    $RAKNET_LOCATION/RakNetCommandParser.cpp
    $RAKNET_LOCATION/RakNetSocket.cpp
    $RAKNET_LOCATION/RakNetSocket2.cpp
    $RAKNET_LOCATION/RakNetSocket2_360_720.cpp
    $RAKNET_LOCATION/RakNetSocket2_Berkley.cpp
    $RAKNET_LOCATION/RakNetSocket2_Berkley_NativeClient.cpp
    $RAKNET_LOCATION/RakNetSocket2_NativeClient.cpp
    $RAKNET_LOCATION/RakNetSocket2_PS3_PS4.cpp
    $RAKNET_LOCATION/RakNetSocket2_PS4.cpp
    $RAKNET_LOCATION/RakNetSocket2_Vita.cpp
    $RAKNET_LOCATION/RakNetSocket2_WindowsStore8.cpp
    $RAKNET_LOCATION/RakNetSocket2_Windows_Linux.cpp
    $RAKNET_LOCATION/RakNetSocket2_Windows_Linux_360.cpp
    $RAKNET_LOCATION/RakNetStatistics.cpp
    $RAKNET_LOCATION/RakNetTransport2.cpp
    $RAKNET_LOCATION/RakNetTypes.cpp
    $RAKNET_LOCATION/RakPeer.cpp
    $RAKNET_LOCATION/RakSleep.cpp
    $RAKNET_LOCATION/RakString.cpp
    $RAKNET_LOCATION/RakThread.cpp
    $RAKNET_LOCATION/RakWString.cpp
    $RAKNET_LOCATION/Rand.cpp
    $RAKNET_LOCATION/RandSync.cpp
    $RAKNET_LOCATION/ReadyEvent.cpp
    $RAKNET_LOCATION/RelayPlugin.cpp
    $RAKNET_LOCATION/ReliabilityLayer.cpp
    $RAKNET_LOCATION/ReplicaManager3.cpp
    $RAKNET_LOCATION/Router2.cpp
    $RAKNET_LOCATION/RPC4Plugin.cpp
    $RAKNET_LOCATION/SecureHandshake.cpp
    $RAKNET_LOCATION/SendToThread.cpp
    $RAKNET_LOCATION/SignaledEvent.cpp
    $RAKNET_LOCATION/SimpleMutex.cpp
    $RAKNET_LOCATION/SocketLayer.cpp
    $RAKNET_LOCATION/StatisticsHistory.cpp
    $RAKNET_LOCATION/StringCompressor.cpp
    $RAKNET_LOCATION/StringTable.cpp
    $RAKNET_LOCATION/SuperFastHash.cpp
    $RAKNET_LOCATION/TableSerializer.cpp
    $RAKNET_LOCATION/TCPInterface.cpp
    $RAKNET_LOCATION/TeamBalancer.cpp
    $RAKNET_LOCATION/TeamManager.cpp
    $RAKNET_LOCATION/TelnetTransport.cpp
    $RAKNET_LOCATION/ThreadsafePacketLogger.cpp
    $RAKNET_LOCATION/TwoWayAuthentication.cpp
    $RAKNET_LOCATION/UDPForwarder.cpp
    $RAKNET_LOCATION/UDPProxyClient.cpp
    $RAKNET_LOCATION/UDPProxyCoordinator.cpp
    $RAKNET_LOCATION/UDPProxyServer.cpp
    $RAKNET_LOCATION/VariableDeltaSerializer.cpp
    $RAKNET_LOCATION/VariableListDeltaTracker.cpp
    $RAKNET_LOCATION/VariadicSQLParser.cpp
    $RAKNET_LOCATION/VitaIncludes.cpp
    $RAKNET_LOCATION/_FindFirst.cpp
    $RAKNET_LOCATION/WSAStartupSingleton.cpp
"

MANAGERS_SRCS="
    Managers/NetworkServer.cpp
    Managers/NetworkClient.cpp
    Managers/AchievementMan.cpp
    Managers/ActivityMan.cpp
    Managers/AudioMan.cpp
    Managers/ConsoleMan.cpp
    Managers/EntityMan.cpp
    Managers/FrameMan.cpp
    Managers/LicenseMan.cpp
    Managers/LuaMan.cpp
    Managers/MetaMan.cpp
    Managers/MovableMan.cpp
    Managers/PresetMan.cpp
    Managers/SceneMan.cpp
    Managers/SettingsMan.cpp
    Managers/TimerMan.cpp
    Managers/UInputMan.cpp
"
SYSTEM_SRCS="
    System/Box.cpp
    System/Color.cpp
    System/ContentFile.cpp
    System/DataModule.cpp
    System/DDTError.cpp
    System/DDTTools.cpp
    System/Matrix.cpp
    System/PathFinder.cpp
    System/Reader.cpp
    System/System.cpp
    System/Timer.cpp
    System/Vector.cpp
    System/Writer.cpp
    System/MicroPather/micropather.cpp
    System/LZ4/lz4.c
    System/LZ4/lz4hc.c
"
#GUI/WinUtil.cpp
GUI_SRCS="
    GUI/AllegroBitmap.cpp
    GUI/AllegroInput.cpp
    GUI/AllegroScreen.cpp
    GUI/GUI.cpp
    GUI/GUIBanner.cpp
    GUI/GUIButton.cpp
    GUI/GUICheckbox.cpp
    GUI/GUICollectionBox.cpp
    GUI/GUIComboBox.cpp
    GUI/GUIControl.cpp
    GUI/GUIControlFactory.cpp
    GUI/GUIControlManager.cpp
    GUI/GUIEvent.cpp
    GUI/GUIFont.cpp
    GUI/GUIInput.cpp
    GUI/GUILabel.cpp
    GUI/GUIListBox.cpp
    GUI/GUIListPanel.cpp
    GUI/GUIManager.cpp
    GUI/GUIPanel.cpp
    GUI/GUIProgressBar.cpp
    GUI/GUIProperties.cpp
    GUI/GUIPropertyPage.cpp
    GUI/GUIRadioButton.cpp
    GUI/GUIScrollbar.cpp
    GUI/GUIScrollPanel.cpp
    GUI/GUISkin.cpp
    GUI/GUISlider.cpp
    GUI/GUITab.cpp
    GUI/GUITextBox.cpp
    GUI/GUITextPanel.cpp
    GUI/GUIUtil.cpp
"
MENUS_SRCS="
    Menus/AreaEditorGUI.cpp
    Menus/AreaPickerGUI.cpp
    Menus/AssemblyEditorGUI.cpp
    Menus/BuyMenuGUI.cpp
    Menus/GibEditorGUI.cpp
    Menus/MainMenuGUI.cpp
    Menus/MetagameGUI.cpp
    Menus/ObjectPickerGUI.cpp
    Menus/PieMenuGUI.cpp
    Menus/ScenarioGUI.cpp
    Menus/SceneEditorGUI.cpp
"
LUABIND_LOCATION="external/sources/luabind-0.9.1/src"
LUABIND_SRCS="
    $LUABIND_LOCATION/class.cpp
    $LUABIND_LOCATION/class_info.cpp
    $LUABIND_LOCATION/class_registry.cpp
    $LUABIND_LOCATION/class_rep.cpp
    $LUABIND_LOCATION/create_class.cpp
    $LUABIND_LOCATION/error.cpp
    $LUABIND_LOCATION/exception_handler.cpp
    $LUABIND_LOCATION/function.cpp
    $LUABIND_LOCATION/inheritance.cpp
    $LUABIND_LOCATION/link_compatibility.cpp
    $LUABIND_LOCATION/object_rep.cpp
    $LUABIND_LOCATION/open.cpp
    $LUABIND_LOCATION/pcall.cpp
    $LUABIND_LOCATION/scope.cpp
    $LUABIND_LOCATION/stack_content_by_name.cpp
    $LUABIND_LOCATION/weak_ref.cpp
    $LUABIND_LOCATION/wrapper_base.cpp
"

ALLEGRO_INSTALL="external/linux/allegra-5.2.5.0/install/"
PREPARED_DIR="external/linux/prepared"
ENTITIES_INC="-IEntities/"
RAKNET_INC="-I$RAKNET_LOCATION"
MANAGERS_INC="-IManagers/"
SYSTEM_INC="-ISystem -ISystem/MicroPather -ISystem/InterGif -ISystem/MD5 -ISystem/Steam -ISystem/Slick_Profiler"
GUI_INC="-IGUI"
MENUS_INC="-IMenus"
#ALLEGRO_INC="`pkg-config --cflags allegro`"
ALLEGRO_INC="-I$ALLEGRO_INSTALL/include"
LUABIND_INC="-Iexternal/include/luabind/2017/"
SDL_INC="-I$PREPARED_DIR/include/SDL2"
OPENAL_INC="`pkg-config --cflags openal`"
CURL_INC="`pkg-config --cflags libcurl`"

ALLEGRO_LIBS="-lalleg"
SDL_LIBS="$PREPARED_DIR/libSDL2.a"
SDL_MIXER_LIBS="$PREPARED_DIR/libSDL2_mixer.a"
LUA_LIBS="$PREPARED_DIR/liblua.a"
CURL_LIBS="`pkg-config --libs libcurl`"
OGG_LIBS="$PREPARED_DIR/libogg.a"
VORBIS_LIBS="$PREPARED_DIR/libvorbis.a"
VORBISFILE_LIBS="$PREPARED_DIR/libvorbisfile.a"
MINIZIP_LIBS="$PREPARED_DIR/libminizip.a $PREPARED_DIR/libaes.a -lbsd"

INCLUDES="-I. -Iexternal/include -I$PREPARED_DIR/include $LUA_INC $ENTITIES_INC $RAKNET_INC $MANAGERS_INC $SYSTEM_INC $GUI_INC $MENUS_INC $ALLEGRO_INC $BOOST_INC $LUABIND_INC $SDL_INC $CURL_INC"
INCLUDES+=" $OPENAL_INC $OGG_INC $VORBIS_INC $VORBISFILE_INC $MINIZIP_INC $OPENSSL_INC"
WARN="-Wno-write-strings -Wno-endif-labels -Wno-deprecated-declarations"
CPPFLAGS="-std=c++11 $INCLUDES $WARN -DALLEGRO_NO_FIX_ALIASES -fpermissive -g"
LDFLAGS="-Llibs/ $ALLEGRO_LIBS $OPENSSL_LIBS $BOOST_LIBS $SDL_LIBS $SDL_MIXER_LIBS $LUABIND_LIBS $LUA_LIBS $ZIPIO_LIBS $CURL_LIBS $OPENAL_LIBS $OGG_LIBS $VORBIS_LIBS $VORBISFILE_LIBS -ldl -lz"
LDFLAGS+=" $MINIZIP_LIBS -lpthread"

SRCS="$SYSTEM_SRCS $ENTITIES_SRCS $MANAGERS_SRCS $GUI_SRCS $MENUS_SRCS $RAKNET_SRCS $LUABIND_SRCS"
CC="c++"
if `hash ccache 2> /dev/null`; then
    CC="ccache $CC"
    echo "Building with ccache ENABLED"
else
    echo "Building with ccache DISABLED. Consider installing it."
fi

LIBS_DIR="$PWD/libs"
if [ ! -d "$LIBS_DIR" ]; then
    mkdir $LIBS_DIR
fi

if [ ! -f libs/liballeg.so ]; then
    pushd $PWD/external/linux
        ./prepare.sh
    popd
fi


OBJ_DIR="objs"
if [ ! -d "$OBJ_DIR" ]; then
    mkdir $OBJ_DIR
fi

OBJ_FILES=""
TOTAL_FILES=0
for src in $SRCS; do
    TOTAL_FILES=$((TOTAL_FILES + 1))
done
NUM_FILES=0
for src in $SRCS; do
    echo "($NUM_FILES/$TOTAL_FILES)"
    path=$OBJ_DIR/$(basename $src).o
    $CC $CPPFLAGS -c $src -o $path
    OBJ_FILES+="$path "
    NUM_FILES=$((NUM_FILES + 1))
done

c++ $CPPFLAGS -o cortex Main.cpp $OBJ_FILES $LDFLAGS
