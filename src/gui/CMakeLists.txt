add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=8 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )

find_package(PkgConfig REQUIRED)

pkg_check_modules(GraphicsMagick REQUIRED GraphicsMagick++>=1.3.21)

find_package(X11 REQUIRED COMPONENTS Xpm Xscreensaver)

include_directories (
    ${GraphicsMagick_INCLUDE_DIRS}
)

add_library (gui STATIC
    BattleChat.cpp
    BattleInfo.cpp
    BattleList.cpp
    BattleRoom.cpp
    Tabs.cpp
    ServerTab.cpp
    PrivateChatTab.cpp
    ChannelChatTab.cpp
    Cache.cpp
    LoginDialog.cpp
    Prefs.cpp
    StringTable.cpp
    TextDisplay2.cpp
    UserInterface.cpp
    ChannelsWindow.cpp
    MapImage.cpp
    ProgressDialog.cpp
    VoteLine.cpp
    BattleFilterDialog.cpp
    AddBotDialog.cpp
    PopupMenu.cpp
    Sound.cpp
    LogFile.cpp
    LoggingDialog.cpp
    TextDialog.cpp
    SpringDialog.cpp
    RegisterDialog.cpp
    AgreementDialog.cpp
    UserList.cpp
    ChatSettingsDialog.cpp
    GameSettings.cpp
    ChatInput.cpp
    SoundSettingsDialog.cpp
    MyImage.cpp
    TextFunctions.cpp
    FontSettingsDialog.cpp
    MapsWindow.cpp
    DownloadSettingsDialog.cpp
    OpenBattleZkDialog.cpp
)

add_dependencies(gui FlobbyConfig)

target_link_libraries (gui
    fltk fltk_images
    ${GraphicsMagick_LIBRARIES}
    ${X11_Xpm_LIB}
    ${X11_Xscreensaver_LIB}
)
