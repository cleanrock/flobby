#pragma once

// include everything needed for users if Model
#include "IControllerEvent.h"
#include "User.h"
#include "Battle.h"
#include "Bot.h"
#include "Script.h"
#include "Channel.h"
#include "MapInfo.h"
#include "StartRect.h"
#include "AI.h"

#include <boost/signal.hpp>
#include <unordered_map>
#include <functional>
#include <map>
#include <string>
#include <memory>


// forwards
//
class IController;
class IViewEvent;
class UnitSync;

class Model: public IControllerEvent
{
public:
    Model(IController & controller);
    virtual ~Model();

    void setSpringPath(std::string const & path) { springPath_ = path; }
    void setUnitSyncPath(std::string const & path);
    std::string const & getSpringPath() const { return springPath_; }
    std::string const & getUnitSyncPath() const { return unitSyncPath_; }

    void login(const std::string & host, const std::string & port,
            const std::string & username, const std::string & password);

    std::vector<Battle const *> getBattles();
    Battle const & getBattle(int battleId);

    std::vector<User const *> getUsers();
    User const & getUser(std::string const & str);
    Bot & getBot(std::string const & str);

    typedef std::map<std::string,Bot*> Bots;
    Bots const & getBots();
    void addBot(Bot const & bot);
    void removeBot(std::string const & name);

    void joinBattle(int battleId, std::string const & password = "_");
    void leaveBattle();
    void sayBattle(std::string const & msg);
    void sayPrivate(std::string const & userName, std::string const & msg);
    void ring(std::string const & userName);

    void startSpring(); // throws on failure
    void disconnect();

    void getChannels();
    void joinChannel(std::string const & channelName);
    void sayChannel(std::string const & channelName, std::string const & message);
    void leaveChannel(std::string const & channelName);

    User & me();
    void meSpec(bool spec);
    void meReady(bool ready);
    void meAllyTeam(int allyTeam);

    std::string const & getWriteableDataDir() const;

    // map
    //
    unsigned int getMapChecksum(std::string const & mapName); // returns 0 if map not found
    std::vector<std::string> getMaps();
    MapInfo getMapInfo(std::string const & mapName);
    void getMapSize(std::string const & mapName, int & w, int & h); // TODO remove
    std::unique_ptr<uint8_t[]> getMapImage(std::string const & mapName, int mipLevel); // returns RGB data, mipLevel: 0->1024x1024, 1->512x512 ...
    std::unique_ptr<uint8_t[]> getMetalMap(std::string const & mapName, int & w, int & h); // returns single component data
    std::unique_ptr<uint8_t[]> getHeightMap(std::string const & mapName, int & w, int & h); // returns single component data

    void refresh(); // to find new mods and maps

    std::vector<AI> getModAIs(std::string const & modName);

    // signals
    //
    typedef boost::signal<void (bool connected)> ConnectedSignal;
    boost::signals::connection connectConnected(ConnectedSignal::slot_type subscriber)
    { return connectedSignal_.connect(subscriber); }

    typedef boost::signal<void (bool success, std::string const & msg)> LoginResultSignal;
    boost::signals::connection connectLoginResult(LoginResultSignal::slot_type subscriber)
    { return loginResultSignal_.connect(subscriber); }

    typedef boost::signal<void (User const & user)> UserJoinedSignal;
    boost::signals::connection connectUserJoined(UserJoinedSignal::slot_type subscriber)
    { return userJoinedSignal_.connect(subscriber); }

    typedef boost::signal<void (User const & user)> UserChangedSignal;
    boost::signals::connection connectUserChanged(UserChangedSignal::slot_type subscriber)
    { return userChangedSignal_.connect(subscriber); }

    typedef boost::signal<void (User const & user)> UserLeftSignal;
    boost::signals::connection connectUserLeft(UserLeftSignal::slot_type subscriber)
    { return userLeftSignal_.connect(subscriber); }

    typedef boost::signal<void (Battle const & battle)> BattleOpenedSignal;
    boost::signals::connection connectBattleOpened(BattleOpenedSignal::slot_type subscriber)
    { return battleOpenedSignal_.connect(subscriber); }

    typedef boost::signal<void (Battle const & battle)> BattleClosedSignal;
    boost::signals::connection connectBattleClosed(BattleClosedSignal::slot_type subscriber)
    { return battleClosedSignal_.connect(subscriber); }

    typedef boost::signal<void (Battle const & battle)> BattleChangedSignal;
    boost::signals::connection connectBattleChanged(BattleChangedSignal::slot_type subscriber)
    { return battleChangedSignal_.connect(subscriber); }

    typedef boost::signal<void (Battle const & battle)> BattleJoinedSignal;
    boost::signals::connection connectBattleJoined(BattleJoinedSignal::slot_type subscriber)
    { return battleJoinedSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & reason)> JoinBattleFailedSignal;
    boost::signals::connection connectJoinBattleFailed(JoinBattleFailedSignal::slot_type subscriber)
    { return joinBattleFailedSignal_.connect(subscriber); }

    typedef boost::signal<void (User const & user, Battle const & battle)> UserJoinedBattleSignal;
    boost::signals::connection connectUserJoinedBattle(UserJoinedBattleSignal::slot_type subscriber)
    { return userJoinedBattleSignal_.connect(subscriber); }

    typedef boost::signal<void (User const & user, Battle const & battle)> UserLeftBattleSignal;
    boost::signals::connection connectUserLeftBattle(UserLeftBattleSignal::slot_type subscriber)
    { return userLeftBattleSignal_.connect(subscriber); }

    typedef boost::signal<void (Bot const & bot)> BotAddedSignal;
    boost::signals::connection connectBotAdded(BotAddedSignal::slot_type subscriber)
    { return botAddedSignal_.connect(subscriber); }

    typedef boost::signal<void (Bot const & bot)> BotChangedSignal;
    boost::signals::connection connectBotChanged(BotChangedSignal::slot_type subscriber)
    { return botChangedSignal_.connect(subscriber); }

    typedef boost::signal<void (Bot const & bot)> BotRemovedSignal;
    boost::signals::connection connectBotRemoved(BotRemovedSignal::slot_type subscriber)
    { return botRemovedSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & userName, std::string const & msg)> BattleChatMsgSignal;
    boost::signals::connection connectBattleChatMsg(BattleChatMsgSignal::slot_type subscriber)
    { return battleChatMsgSignal_.connect(subscriber); }

    typedef boost::signal<void ()> SpringExitSignal;
    boost::signals::connection connectSpringExit(SpringExitSignal::slot_type subscriber)
    { return springExitSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & msg)> ServerMsgSignal;
    boost::signals::connection connectServerMsg(ServerMsgSignal::slot_type subscriber)
    { return serverMsgSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & userName, std::string const & msg)> SayPrivateSignal;
    boost::signals::connection connectSayPrivate(SayPrivateSignal::slot_type subscriber)
    { return sayPrivateSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & userName, std::string const & msg)> SaidPrivateSignal;
    boost::signals::connection connectSaidPrivate(SaidPrivateSignal::slot_type subscriber)
    { return saidPrivateSignal_.connect(subscriber); }

    typedef boost::signal<void (Channels const &)> ChannelsSignal;
    boost::signals::connection connectChannels(ChannelsSignal::slot_type subscriber)
    { return channelsSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName)> ChannelJoinedSignal;
    boost::signals::connection connectChannelJoined(ChannelJoinedSignal::slot_type subscriber)
    { return channelJoinedSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic)> ChannelTopicSignal;
    boost::signals::connection connectChannelTopicSignal(ChannelTopicSignal::slot_type subscriber)
    { return channelTopicSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName, std::vector<std::string> const & clients)> ChannelClientsSignal;
    boost::signals::connection connectChannelClients(ChannelClientsSignal::slot_type subscriber)
    { return channelClientsSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName, std::string const & userName)> UserJoinedChannelSignal;
    boost::signals::connection connectUserJoinedChannel(UserJoinedChannelSignal::slot_type subscriber)
    { return userJoinedChannelSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName, std::string const & userName, std::string const & reason)> UserLeftChannelSignal;
    boost::signals::connection connectUserLeftChannel(UserLeftChannelSignal::slot_type subscriber)
    { return userLeftChannelSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & channelName, std::string const & userName, std::string const & message)> SaidChannelSignal;
    boost::signals::connection connectSaidChannel(SaidChannelSignal::slot_type subscriber)
    { return saidChannelSignal_.connect(subscriber); }

    typedef boost::signal<void (std::string const & userName)> RingSignal;
    boost::signals::connection connectRing(RingSignal::slot_type subscriber)
    { return ringSignal_.connect(subscriber); }

    typedef boost::signal<void (StartRect const & startRect)> AddStartRectSignal;
    boost::signals::connection connectAddStartRect(AddStartRectSignal::slot_type subscriber)
    { return addStartRectSignal_.connect(subscriber); }

    typedef boost::signal<void (int ally)> RemoveStartRectSignal;
    boost::signals::connection connectRemoveStartRect(RemoveStartRectSignal::slot_type subscriber)
    { return removeStartRectSignal_.connect(subscriber); }

private:
    IController & controller_;
    bool connected_;
    bool loggedIn_; // set to true when we get LOGININFOEND
    std::unique_ptr<UnitSync> unitSync_;

    std::string writeableDataDir_;
    std::string userName_;
    std::string password_;
    std::string myScriptPassword_;
    int joinedBattleId_;
    unsigned int springId_;
    User * me_;

    std::string springPath_;
    std::string unitSyncPath_;
    Script script_;

    // IControllerEvent
    //
    void connected(bool connected);
    void message(std::string const & msg);
    void processDone(unsigned int id);

    ConnectedSignal connectedSignal_;
    LoginResultSignal loginResultSignal_;
    UserJoinedSignal userJoinedSignal_;
    UserChangedSignal userChangedSignal_;
    UserLeftSignal userLeftSignal_;
    BattleOpenedSignal battleOpenedSignal_;
    BattleClosedSignal battleClosedSignal_;
    BattleChangedSignal battleChangedSignal_;
    BattleJoinedSignal battleJoinedSignal_;
    JoinBattleFailedSignal joinBattleFailedSignal_;
    UserJoinedBattleSignal userJoinedBattleSignal_;
    UserLeftBattleSignal userLeftBattleSignal_;
    BotAddedSignal botAddedSignal_;
    BotChangedSignal botChangedSignal_;
    BotRemovedSignal botRemovedSignal_;
    BattleChatMsgSignal battleChatMsgSignal_;
    SpringExitSignal springExitSignal_;
    ServerMsgSignal serverMsgSignal_;
    SayPrivateSignal sayPrivateSignal_;
    SaidPrivateSignal saidPrivateSignal_;
    ChannelsSignal channelsSignal_;
    ChannelJoinedSignal channelJoinedSignal_;
    ChannelTopicSignal channelTopicSignal_;
    ChannelClientsSignal channelClientsSignal_;
    UserJoinedChannelSignal userJoinedChannelSignal_;
    UserLeftChannelSignal userLeftChannelSignal_;
    SaidChannelSignal saidChannelSignal_;
    RingSignal ringSignal_;
    AddStartRectSignal addStartRectSignal_;
    RemoveStartRectSignal removeStartRectSignal_;

    void attemptLogin();
    void processServerMsg(const std::string & msg);

    std::map<std::string, std::shared_ptr<User> > users_;

    std::map<int, std::shared_ptr<Battle> > battles_;


    Bots bots_;
    Channels channels_; // last retrieved channel list

    std::map<std::string, int> mapIndex_;
    void initMapIndex();
    std::unique_ptr<uint8_t[]> getInfoMap(std::string const & mapName, std::string const & type, int & w, int & h);

    User & user(std::string const & str);
    Battle & getBattle(std::string const & str);
    Battle & battle(int battleId);
    void updateBattleRunningStatus(User const & user); // signal battle changed if user is founder of battle

    void sendMyInitialBattleStatus(Battle const & battle);
    void sendMyBattleStatus();
    int calcSync(Battle const & battle);

    void meInGame(bool inGame);

    typedef std::unordered_map<std::string, std::function<void (std::istream &)>> MessageHandlers;
    MessageHandlers messageHandlers_;
    void handle_ACCEPTED(std::istream & is);
    void handle_DENIED(std::istream & is);
    void handle_ADDUSER(std::istream & is);
    void handle_REMOVEUSER(std::istream & is);
    void handle_BATTLEOPENED(std::istream & is);
    void handle_BATTLECLOSED(std::istream & is);
    void handle_UPDATEBATTLEINFO(std::istream & is);
    void handle_JOINEDBATTLE(std::istream & is);
    void handle_LEFTBATTLE(std::istream & is);
    void handle_CLIENTSTATUS(std::istream & is);
    void handle_LOGININFOEND(std::istream & is);
    void handle_JOINBATTLE(std::istream & is);
    void handle_JOINBATTLEFAILED(std::istream & is);
    void handle_SETSCRIPTTAGS(std::istream & is);
    void handle_CLIENTBATTLESTATUS(std::istream & is);
    void handle_REQUESTBATTLESTATUS(std::istream & is);
    void handle_SAIDBATTLE_SAIDBATTLEEX(std::istream & is);
    void handle_ADDBOT(std::istream & is);
    void handle_REMOVEBOT(std::istream & is);
    void handle_UPDATEBOT(std::istream & is);
    void handle_MOTD(std::istream & is);
    void handle_SERVERMSG(std::istream & is);
    void handle_SERVERMSGBOX(std::istream & is);
    void handle_SAYPRIVATE(std::istream & is);
    void handle_SAIDPRIVATE(std::istream & is);
    void handle_CHANNEL(std::istream & is);
    void handle_ENDOFCHANNELS(std::istream & is);
    void handle_JOIN(std::istream & is);
    void handle_CLIENTS(std::istream & is);
    void handle_JOINED(std::istream & is);
    void handle_LEFT(std::istream & is);
    void handle_CHANNELTOPIC(std::istream & is);
    void handle_SAID_SAIDEX(std::istream & is);
    void handle_RING(std::istream & is);
    void handle_ADDSTARTRECT(std::istream & is);
    void handle_REMOVESTARTRECT(std::istream & is);

};
