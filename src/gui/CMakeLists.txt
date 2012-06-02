set(FLTK_SKIP_OPENGL true)
set(FLTK_SKIP_FORMS true)
# set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)


include_directories (
    ${FLTK_INCLUDE_DIR}
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
    MyImage.cpp
    Prefs.cpp
    StringTable.cpp
    TextDisplay.cpp
    UserInterface.cpp
    ChannelsWindow.cpp
    MapImage.cpp
    ProgressDialog.cpp
    VoteLine.cpp
    BattleFilterDialog.cpp
    AddBotDialog.cpp
    PopupMenu.cpp
)

target_link_libraries (gui
    ${FLTK_LIBRARIES}
    boost_regex
    fontconfig
    Xft
    Xinerama
    Xpm
    png
)
