find_package(Jsoncpp REQUIRED)

include_directories(${JSONCPP_INCLUDE_DIR})

add_library (model STATIC
    Battle.cpp
    Bot.cpp
    LobbyProtocol.cpp
    Model.cpp
    Script.cpp
    UnitSync.cpp
    User.cpp
    UserBattleStatus.cpp
    UserStatus.cpp
    Channel.cpp
    MapInfo.cpp
    StartRect.cpp
    AI.cpp
    UserId.cpp
    ServerCommands.cpp
)

add_dependencies(model FlobbyConfig)

target_link_libraries (model
    md5
    dl
    ${JSONCPP_LIBRARY}
)
