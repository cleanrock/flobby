
find_package(PkgConfig REQUIRED)

pkg_check_modules(JsonCpp REQUIRED jsoncpp)

include_directories(${JsonCpp_INCLUDE_DIRS})

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
    Nightwatch.cpp
)

add_dependencies(model FlobbyConfig)

target_link_libraries (model
    md5
    dl
    ${JsonCpp_LIBRARIES}
)
