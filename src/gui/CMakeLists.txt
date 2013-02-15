set(FLTK_SKIP_OPENGL true)
set(FLTK_SKIP_FORMS true)
# set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)

add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=8 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

find_package(X11 COMPONENTS X11_Xpm_LIB X11_Xscreensaver_LIB REQUIRED)

include_directories (
    ${FLTK_INCLUDE_DIR}
    ${ImageMagick_Magick++_INCLUDE_DIR}
)

add_library (gui STATIC
    BattleChat.cpp
    BattleInfo.cpp
    BattleList.cpp
    BattleRoom.cpp
    Tabs.cpp
    LogUsersTab.cpp
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
)

target_link_libraries (gui
    ${FLTK_LIBRARIES}
    ${ImageMagick_Magick++_LIBRARY}
    ${X11_Xpm_LIB}
    ${X11_Xscreensaver_LIB}
)
