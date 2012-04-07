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
)

target_link_libraries (model
    md5
    dl
)
