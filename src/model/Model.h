// This file is part of flobby (GPL v2 or later), see the LICENSE file

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
#include "ServerInfo.h"
#include "AI.h"

#include <boost/signals2/signal.hpp>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <map>
#include <set>
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
    Model(IController & controller, bool zerok);
    virtual ~Model();

    bool isZeroK() { return zerok_; }
    void setSpringPath(std::string const & path) { springPath_ = path; }
    void setSpringOptions(std::string const & options) { springOptions_ = options; }
    void setUnitSyncPath(std::string const & path);
    void useExternalPrDownloader(bool useExternal);
    void setPrDownloaderCmd(std::string const & cmd);
    std::string const & getSpringPath() const { return springPath_; }
    std::string const & getUnitSyncPath() const { return unitSyncPath_; }
    std::string const & getPrDownloaderCmd() const { return prDownloaderCmd_; }

    void connect(std::string const & host, std::string const & port);
    void login(std::string const & username, std::string const & passwordHash);
    std::string calcPasswordHash(std::string const& str);
    void sendMessage(std::string const& msg);

    void registerAccount(std::string const & username, std::string const & passwordHash, std::string const & email);
    void confirmAgreement();
    void renameAccount(std::string const & username);

    std::vector<Battle const *> getBattles();
    Battle const & getBattle(int battleId);

    std::vector<User const *> getUsers();
    User const & getUser(std::string const & str);
    Bot & getBot(std::string const & str);

    typedef std::map<std::string,Bot*> Bots;
    Bots const & getBots();
    void addBot(Bot const & bot);
    void botAllyTeam(std::string const& name, int allyTeam);
    void botSide(std::string const& name, int side);
    void removeBot(std::string const & name);

    void joinBattle(int battleId, std::string const & password = "_");
    void leaveBattle();
    void sayBattle(std::string const & msg);
    void sayPrivate(std::string const & userName, std::string const & msg);
    void ring(std::string const & userName);

    std::string serverCommand(std::string const& str);

    void startSpring(); // throws on failure
    void startDemo(std::string const& springPath, std::string const& demoPath);
    void disconnect();

    void getChannels();
    void joinChannel(std::string const & channelName, std::string const & password = "");
    void sayChannel(std::string const & channelName, std::string const & message);
    void leaveChannel(std::string const & channelName);

    User & me();
    void meSpec(bool spec);
    void meReady(bool ready);
    void meAllyTeam(int allyTeam);
    void meSide(int side);
    void meAway(bool away);

    std::string const & getWriteableDataDir() const;

    void checkPing(); // call regularly to make sure we send a PING at least every 30s, it will also check if we got the PONG back

    // map
    unsigned int getMapChecksum(std::string const & mapName); // returns 0 if map not found
    std::vector<std::string> getMaps();
    MapInfo getMapInfo(std::string const & mapName);
    void getMapSize(std::string const & mapName, int & w, int & h); // TODO remove
    std::unique_ptr<uint8_t[]> getMapImage(std::string const & mapName, int mipLevel); // returns RGB data, mipLevel: 0->1024x1024, 1->512x512 ...
    std::unique_ptr<uint8_t[]> getMetalMap(std::string const & mapName, int & w, int & h); // returns single component data
    std::unique_ptr<uint8_t[]> getHeightMap(std::string const & mapName, int & w, int & h); // returns single component data

    enum DownloadType { DT_MAP, DT_GAME, DT_ENGINE, DT_DEMO };
    unsigned int download(std::string const & name, DownloadType type); // returns >0 (job id) if download attempt is done

    void testThread(); // TODO remove some day

    // mod
    bool gameExist(std::string const & gameName);

    void refresh(); // to find new mods and maps

    std::vector<AI> getModAIs(std::string const & modName);
    std::vector<std::string> getModSideNames(std::string const & modName);

    // ServerCommands specific methods
    void subscribeChannel(std::string const & channelName);
    void unsubscribeChannel(std::string const & channelName);
    void listChannelSubscriptions();

    // signals
    //
    typedef boost::signals2::signal<void (bool connected)> ConnectedSignal;
    boost::signals2::connection connectConnected(ConnectedSignal::slot_type subscriber)
    { return connectedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (ServerInfo const & serverInfo)> ServerInfoSignal;
    boost::signals2::connection connectServerInfo(ServerInfoSignal::slot_type subscriber)
    { return serverInfoSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (bool success, std::string const & msg)> LoginResultSignal;
    boost::signals2::connection connectLoginResult(LoginResultSignal::slot_type subscriber)
    { return loginResultSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (bool success, std::string const & msg)> RegisterResultSignal;
    boost::signals2::connection connectRegisterResult(RegisterResultSignal::slot_type subscriber)
    { return registerResultSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & text)> AgreementSignal;
    boost::signals2::connection connectAgreement(AgreementSignal::slot_type subscriber)
    { return agreementSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (User const & user)> UserJoinedSignal;
    boost::signals2::connection connectUserJoined(UserJoinedSignal::slot_type subscriber)
    { return userJoinedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (User const & user)> UserChangedSignal;
    boost::signals2::connection connectUserChanged(UserChangedSignal::slot_type subscriber)
    { return userChangedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (User const & user)> UserLeftSignal;
    boost::signals2::connection connectUserLeft(UserLeftSignal::slot_type subscriber)
    { return userLeftSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Battle const & battle)> BattleOpenedSignal;
    boost::signals2::connection connectBattleOpened(BattleOpenedSignal::slot_type subscriber)
    { return battleOpenedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Battle const & battle)> BattleClosedSignal;
    boost::signals2::connection connectBattleClosed(BattleClosedSignal::slot_type subscriber)
    { return battleClosedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Battle const & battle)> BattleChangedSignal;
    boost::signals2::connection connectBattleChanged(BattleChangedSignal::slot_type subscriber)
    { return battleChangedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Battle const & battle)> BattleJoinedSignal;
    boost::signals2::connection connectBattleJoined(BattleJoinedSignal::slot_type subscriber)
    { return battleJoinedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & reason)> JoinBattleFailedSignal;
    boost::signals2::connection connectJoinBattleFailed(JoinBattleFailedSignal::slot_type subscriber)
    { return joinBattleFailedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (User const & user, Battle const & battle)> UserJoinedBattleSignal;
    boost::signals2::connection connectUserJoinedBattle(UserJoinedBattleSignal::slot_type subscriber)
    { return userJoinedBattleSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (User const & user, Battle const & battle)> UserLeftBattleSignal;
    boost::signals2::connection connectUserLeftBattle(UserLeftBattleSignal::slot_type subscriber)
    { return userLeftBattleSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Bot const & bot)> BotAddedSignal;
    boost::signals2::connection connectBotAdded(BotAddedSignal::slot_type subscriber)
    { return botAddedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Bot const & bot)> BotChangedSignal;
    boost::signals2::connection connectBotChanged(BotChangedSignal::slot_type subscriber)
    { return botChangedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Bot const & bot)> BotRemovedSignal;
    boost::signals2::connection connectBotRemoved(BotRemovedSignal::slot_type subscriber)
    { return botRemovedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & userName, std::string const & msg)> BattleChatMsgSignal;
    boost::signals2::connection connectBattleChatMsg(BattleChatMsgSignal::slot_type subscriber)
    { return battleChatMsgSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void ()> SpringExitSignal;
    boost::signals2::connection connectSpringExit(SpringExitSignal::slot_type subscriber)
    { return springExitSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (DownloadType downloadType, std::string const & name, bool success)> DownloadDoneSignal;
    boost::signals2::connection connectDownloadDone(DownloadDoneSignal::slot_type subscriber)
    { return downloadDoneSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & msg, int interest)> ServerMsgSignal;
    boost::signals2::connection connectServerMsg(ServerMsgSignal::slot_type subscriber)
    { return serverMsgSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & userName, std::string const & msg)> SayPrivateSignal;
    boost::signals2::connection connectSayPrivate(SayPrivateSignal::slot_type subscriber)
    { return sayPrivateSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & userName, std::string const & msg)> SaidPrivateSignal;
    boost::signals2::connection connectSaidPrivate(SaidPrivateSignal::slot_type subscriber)
    { return saidPrivateSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (Channels const &)> ChannelsSignal;
    boost::signals2::connection connectChannels(ChannelsSignal::slot_type subscriber)
    { return channelsSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName)> ChannelJoinedSignal;
    boost::signals2::connection connectChannelJoined(ChannelJoinedSignal::slot_type subscriber)
    { return channelJoinedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic)> ChannelTopicSignal;
    boost::signals2::connection connectChannelTopicSignal(ChannelTopicSignal::slot_type subscriber)
    { return channelTopicSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::string const & message)> ChannelMessageSignal;
    boost::signals2::connection connectChannelMessageSignal(ChannelMessageSignal::slot_type subscriber)
    { return channelMessageSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::vector<std::string> const & clients)> ChannelClientsSignal;
    boost::signals2::connection connectChannelClients(ChannelClientsSignal::slot_type subscriber)
    { return channelClientsSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::string const & userName)> UserJoinedChannelSignal;
    boost::signals2::connection connectUserJoinedChannel(UserJoinedChannelSignal::slot_type subscriber)
    { return userJoinedChannelSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::string const & userName, std::string const & reason)> UserLeftChannelSignal;
    boost::signals2::connection connectUserLeftChannel(UserLeftChannelSignal::slot_type subscriber)
    { return userLeftChannelSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & channelName, std::string const & userName, std::string const & message)> SaidChannelSignal;
    boost::signals2::connection connectSaidChannel(SaidChannelSignal::slot_type subscriber)
    { return saidChannelSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & userName)> RingSignal;
    boost::signals2::connection connectRing(RingSignal::slot_type subscriber)
    { return ringSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (StartRect const & startRect)> AddStartRectSignal;
    boost::signals2::connection connectAddStartRect(AddStartRectSignal::slot_type subscriber)
    { return addStartRectSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (int ally)> RemoveStartRectSignal;
    boost::signals2::connection connectRemoveStartRect(RemoveStartRectSignal::slot_type subscriber)
    { return removeStartRectSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & key, std::string const & value)> SetScriptTagSignal;
    boost::signals2::connection connectSetScriptTag(SetScriptTagSignal::slot_type subscriber)
    { return setScriptTagSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (std::string const & key)> RemoveScriptTagSignal;
    boost::signals2::connection connectRemoveScriptTag(RemoveScriptTagSignal::slot_type subscriber)
    { return removeScriptTagSignal_.connect(subscriber); }

private:
    IController & controller_;
    bool zerok_;
    bool connected_;
    bool checkFirstMsg_;
    bool loginInProgress_;
    bool loggedIn_; // set to true when we get LOGININFOEND
    uint64_t timePingSent_;
    int waitingForPong_;
    std::unique_ptr<UnitSync> unitSync_;

    std::string writeableDataDir_;
    std::string userName_;
    std::string password_;
    std::string myScriptPassword_;
    int joinedBattleId_;
    User * me_;

    // what is being downloaded
    DownloadType downloadType_;
    std::string downloadName_;

    // thread ids (0 if not running)
    unsigned int springId_;
    unsigned int prDownloaderId_;

    std::string springPath_;
    std::string springOptions_;
    std::string unitSyncPath_;
    bool useExternalPrDownloader_;
    std::string prDownloaderCmd_;
    Script script_;

    int runProcess(std::string const& cmd, bool logToFile);

    unsigned int downloadExternal(std::string const& name, DownloadType type); // returns >0 if download attempt is done

    // TODO disabled for now
    int downloadInternal(std::string const& name, DownloadType type); // returns true if download attempt is done

    // IControllerEvent
    //
    void connected(bool connected);
    void message(std::string const & msg);
    void processDone(std::pair<unsigned int, int> idRetPair);

    ConnectedSignal connectedSignal_;
    ServerInfoSignal serverInfoSignal_;
    LoginResultSignal loginResultSignal_;
    RegisterResultSignal registerResultSignal_;
    AgreementSignal agreementSignal_;
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
    DownloadDoneSignal downloadDoneSignal_;
    ServerMsgSignal serverMsgSignal_;
    SayPrivateSignal sayPrivateSignal_;
    SaidPrivateSignal saidPrivateSignal_;
    ChannelsSignal channelsSignal_;
    ChannelJoinedSignal channelJoinedSignal_;
    ChannelTopicSignal channelTopicSignal_;
    ChannelMessageSignal channelMessageSignal_;
    ChannelClientsSignal channelClientsSignal_;
    UserJoinedChannelSignal userJoinedChannelSignal_;
    UserLeftChannelSignal userLeftChannelSignal_;
    SaidChannelSignal saidChannelSignal_;
    RingSignal ringSignal_;
    AddStartRectSignal addStartRectSignal_;
    RemoveStartRectSignal removeStartRectSignal_;
    SetScriptTagSignal setScriptTagSignal_;
    RemoveScriptTagSignal removeScriptTagSignal_;

    void attemptLogin();
    void processServerMsg(const std::string & msg);

    typedef std::map<std::string, std::shared_ptr<User>> Users;
    Users users_;

    std::map<int, std::shared_ptr<Battle>> battles_;

    std::ostringstream agreementStream_;

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

    void sendUpdateBot(std::string const& name, UserBattleStatus const& ubs, int color);

    typedef std::unordered_map<std::string, std::function<void (std::istream &)>> MessageHandlers;
    MessageHandlers messageHandlers_;
    MessageHandlers messageHandlersZerok_;

    // spring message handlers
    void handle_TASServer(std::istream & is);
    void handle_ACCEPTED(std::istream & is);
    void handle_DENIED(std::istream & is);
    void handle_ADDUSER(std::istream & is);
    void handle_REMOVEUSER(std::istream & is);
    void handle_BATTLEOPENED(std::istream & is);
    void handle_BATTLEOPENEDEX(std::istream & is);
    void handle_BATTLECLOSED(std::istream & is);
    void handle_UPDATEBATTLEINFO(std::istream & is);
    void handle_JOINEDBATTLE(std::istream & is);
    void handle_LEFTBATTLE(std::istream & is);
    void handle_CLIENTSTATUS(std::istream & is);
    void handle_LOGININFOEND(std::istream & is);
    void handle_JOINBATTLE(std::istream & is);
    void handle_JOINBATTLEFAILED(std::istream & is);
    void handle_SETSCRIPTTAGS(std::istream & is);
    void handle_REMOVESCRIPTTAGS(std::istream & is);
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
    void handle_CHANNELMESSAGE(std::istream & is);
    void handle_SAID_SAIDEX(std::istream & is);
    void handle_RING(std::istream & is);
    void handle_ADDSTARTRECT(std::istream & is);
    void handle_REMOVESTARTRECT(std::istream & is);
    void handle_REGISTRATIONACCEPTED(std::istream & is);
    void handle_REGISTRATIONDENIED(std::istream & is);
    void handle_AGREEMENT(std::istream & is);
    void handle_AGREEMENTEND(std::istream & is);
    void handle_PONG(std::istream & is);
    void handle_HOSTPORT(std::istream & is);
    void handle_FORCEJOINBATTLE(std::istream & is);
    void handle_STARTLISTSUBSCRIPTION(std::istream & is);
    void handle_LISTSUBSCRIPTION(std::istream & is);
    void handle_ENDLISTSUBSCRIPTION(std::istream & is);
    void handle_OK(std::istream & is);
    void handle_FAILED(std::istream & is);

    // zerok message handlers
    void handle_Welcome(std::istream & is);
    void handle_RegisterResponse(std::istream & is);
    void handle_LoginResponse(std::istream & is);
    void handle_Ping(std::istream & is);
    void handle_User(std::istream & is);
    void handle_UserDisconnected(std::istream & is);
    void handle_BattleAdded(std::istream & is);
    void handle_BattleRemoved(std::istream & is);
    void handle_BattleUpdate(std::istream & is);
    void handle_JoinedBattle(std::istream & is);
    void handle_LeftBattle(std::istream & is);
    void handle_JoinChannelResponse(std::istream & is);
    void handle_ChannelUserAdded(std::istream & is);
    void handle_ChannelUserRemoved(std::istream & is);
    void handle_Say(std::istream & is);
    bool handle_Nightwatch(Json::Value & jv);
    void handle_UpdateUserBattleStatus(std::istream & is);
    void handle_SetRectangle(std::istream & is);
    void handle_UpdateBotStatus(std::istream & is);
    void handle_RemoveBot(std::istream & is);
    void handle_SetModOptions(std::istream & is);
    void handle_SiteToLobbyCommand(std::istream & is);

    // ZeroK specific methods and attributes
    void handleZerokAction(std::string const& action, std::string const& arg);
    std::vector<std::string> start_replay_Args_;
    std::string const flobbyDemo_;
    std::set<unsigned int> demoDownloadJobs_;
};
