// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Model.h"
#include "LobbyProtocol.h"
#include "User.h"
#include "UserBattleStatus.h"
#include "Battle.h"
#include "IController.h"
#include "Bot.h"
#include "UnitSync.h"
#include "UserId.h"
#include "ServerCommands.h"
#include "Nightwatch.h"

#include "md5/md5.h"
#include "md5/base64.h"

#include "log/Log.h"
#include "FlobbyDirs.h"
#include "FlobbyConfig.h"

// TODO #include <pr-downloader.h>
#include <json/json.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <sstream>
#include <cassert>

#define ADD_MSG_HANDLER(MSG) \
    messageHandlers_[#MSG] = std::bind(&Model::handle_##MSG, this, std::placeholders::_1);
#define ADD_MSG_HANDLER2(MSG, METHOD) \
    messageHandlers_[#MSG] = std::bind(&Model::handle_##METHOD, this, std::placeholders::_1);

#define ADD_ZK_MSG_HANDLER(MSG) \
    messageHandlersZerok_[#MSG] = std::bind(&Model::handle_##MSG, this, std::placeholders::_1);

Model::Model(IController & controller, bool zerok):
    controller_(controller),
    zerok_(zerok),
    connected_(false),
    checkFirstMsg_(false),
    loggedIn_(false),
    timePingSent_(0),
    waitingForPong_(0),
    joinedBattleId_(-1),
    me_(0),
    springId_(0),
    prDownloaderId_(0),
    curlId_(0),
    flobbyDemo_("flobby_demo"),
    requestedConnectSpring_(false)
{
    controller_.setIControllerEvent(*this);
    ServerCommand::init(*this);

    // setup spring message handlers
    ADD_MSG_HANDLER(TASServer)
    ADD_MSG_HANDLER(ACCEPTED)
    ADD_MSG_HANDLER(DENIED)
    ADD_MSG_HANDLER(ADDUSER)
    ADD_MSG_HANDLER(REMOVEUSER)
    ADD_MSG_HANDLER(BATTLEOPENED)
    ADD_MSG_HANDLER(BATTLECLOSED)
    ADD_MSG_HANDLER(UPDATEBATTLEINFO)
    ADD_MSG_HANDLER(JOINEDBATTLE)
    ADD_MSG_HANDLER(LEFTBATTLE)
    ADD_MSG_HANDLER(CLIENTSTATUS)
    ADD_MSG_HANDLER(LOGININFOEND)
    ADD_MSG_HANDLER(JOINBATTLE)
    ADD_MSG_HANDLER(JOINBATTLEFAILED)
    ADD_MSG_HANDLER(SETSCRIPTTAGS)
    ADD_MSG_HANDLER(CLIENTBATTLESTATUS)
    ADD_MSG_HANDLER(REQUESTBATTLESTATUS)
    ADD_MSG_HANDLER(ADDBOT)
    ADD_MSG_HANDLER(REMOVEBOT)
    ADD_MSG_HANDLER(UPDATEBOT)
    ADD_MSG_HANDLER(MOTD)
    ADD_MSG_HANDLER(SERVERMSG)
    ADD_MSG_HANDLER(SERVERMSGBOX)
    ADD_MSG_HANDLER2(SAIDBATTLE, SAIDBATTLE_SAIDBATTLEEX)
    ADD_MSG_HANDLER2(SAIDBATTLEEX, SAIDBATTLE_SAIDBATTLEEX)
    ADD_MSG_HANDLER(SAYPRIVATE)
    ADD_MSG_HANDLER(SAIDPRIVATE)
    ADD_MSG_HANDLER(CHANNEL)
    ADD_MSG_HANDLER(ENDOFCHANNELS)
    ADD_MSG_HANDLER(JOIN)
    ADD_MSG_HANDLER(CHANNELTOPIC)
    ADD_MSG_HANDLER(CHANNELMESSAGE)
    ADD_MSG_HANDLER(CLIENTS)
    ADD_MSG_HANDLER(JOINED)
    ADD_MSG_HANDLER(LEFT)
    ADD_MSG_HANDLER2(SAID, SAID_SAIDEX)
    ADD_MSG_HANDLER2(SAIDEX, SAID_SAIDEX)
    ADD_MSG_HANDLER(RING)
    ADD_MSG_HANDLER(ADDSTARTRECT)
    ADD_MSG_HANDLER(REMOVESTARTRECT)
    ADD_MSG_HANDLER(REGISTRATIONACCEPTED)
    ADD_MSG_HANDLER(REGISTRATIONDENIED)
    ADD_MSG_HANDLER(AGREEMENT)
    ADD_MSG_HANDLER(AGREEMENTEND)
    ADD_MSG_HANDLER(SETSCRIPTTAGS)
    ADD_MSG_HANDLER(REMOVESCRIPTTAGS)
    ADD_MSG_HANDLER(PONG)
    ADD_MSG_HANDLER(HOSTPORT)
    ADD_MSG_HANDLER(FORCEJOINBATTLE)
    ADD_MSG_HANDLER(STARTLISTSUBSCRIPTION)
    ADD_MSG_HANDLER(LISTSUBSCRIPTION)
    ADD_MSG_HANDLER(ENDLISTSUBSCRIPTION)
    ADD_MSG_HANDLER(OK)
    ADD_MSG_HANDLER(FAILED)

    // setup zerok message handlers
    ADD_ZK_MSG_HANDLER(Welcome)
    ADD_ZK_MSG_HANDLER(RegisterResponse)
    ADD_ZK_MSG_HANDLER(LoginResponse)
    ADD_ZK_MSG_HANDLER(User)
    ADD_ZK_MSG_HANDLER(UserDisconnected)
    ADD_ZK_MSG_HANDLER(BattleAdded)
    ADD_ZK_MSG_HANDLER(BattleRemoved)
    ADD_ZK_MSG_HANDLER(BattleUpdate)
    ADD_ZK_MSG_HANDLER(BattlePoll)
    ADD_ZK_MSG_HANDLER(BattlePollOutcome)
    ADD_ZK_MSG_HANDLER(JoinedBattle)
    ADD_ZK_MSG_HANDLER(JoinBattleSuccess)
    ADD_ZK_MSG_HANDLER(LeftBattle)
    ADD_ZK_MSG_HANDLER(JoinChannelResponse)
    ADD_ZK_MSG_HANDLER(ChannelUserAdded)
    ADD_ZK_MSG_HANDLER(ChannelUserRemoved)
    ADD_ZK_MSG_HANDLER(Say)
    ADD_ZK_MSG_HANDLER(UpdateUserBattleStatus)
    ADD_ZK_MSG_HANDLER(SetRectangle)
    ADD_ZK_MSG_HANDLER(UpdateBotStatus)
    ADD_ZK_MSG_HANDLER(RemoveBot)
    ADD_ZK_MSG_HANDLER(SetModOptions)
    ADD_ZK_MSG_HANDLER(SiteToLobbyCommand)
    ADD_ZK_MSG_HANDLER(ConnectSpring)
    ADD_ZK_MSG_HANDLER(FriendList)
    ADD_ZK_MSG_HANDLER(IgnoreList)
    ADD_ZK_MSG_HANDLER(MatchMakerSetup)
    ADD_ZK_MSG_HANDLER(MatchMakerStatus)
    ADD_ZK_MSG_HANDLER(BattleDebriefing)
    ADD_ZK_MSG_HANDLER(NewsList)
    ADD_ZK_MSG_HANDLER(ForumList)
    ADD_ZK_MSG_HANDLER(LadderList)
    ADD_ZK_MSG_HANDLER(UserProfile)
    ADD_ZK_MSG_HANDLER(DefaultGameChanged)
}

Model::~Model()
{
}

void Model::setUnitSyncPath(std::string const & path)
{
    unitSyncPath_ = path;
    unitSync_.reset( new UnitSync(unitSyncPath_) );

    refresh();

    writeableDataDir_ = unitSync_->GetWritableDataDirectory();
    assert(!writeableDataDir_.empty());

    LOG(DEBUG) << "writeableDataDir_:" << writeableDataDir_;
}

void Model::useExternalPrDownloader(bool useExternal)
{
    useExternalPrDownloader_ = useExternal;
    LOG(DEBUG) << "useExternalPrDownloader_:" << useExternalPrDownloader_;
}

void Model::setPrDownloaderCmd(std::string const & cmd)
{
    prDownloaderCmd_ = cmd;
    LOG(DEBUG) << "prDownloaderCmd_:" << prDownloaderCmd_;
}

Battle & Model::getBattle(std::string const & str)
{
    int battleId = boost::lexical_cast<int>(str);
    auto it = battles_.find(battleId);
    if (it == battles_.end())
    {
        throw std::invalid_argument("battle not found:" + str);
    }
    return *it->second;
}

Battle & Model::battle(int battleId)
{
    return const_cast<Battle&>(getBattle(battleId));
}

void Model::connected(bool connected)
{
    LOG(DEBUG) << "Model::connected:" << connected;

    connected_ = connected;
    if (connected_)
    {
        checkFirstMsg_ = true; // check first message again
        timePingSent_ = controller_.timeNow();
    }
    else
    {
        // reset model on disconnect
        loggedIn_ = false;
        waitingForPong_ = 0;
        userName_.clear();
        password_.clear();
        myScriptPassword_.clear();
        joinedBattleId_ = -1;
        springId_ = 0;
        me_ = 0;
        battles_.clear();
        users_.clear();
        bots_.clear();

        if (loginInProgress_)
        {
            loginResultSignal_(false, "no connection to server");
            loginInProgress_ = false;
        }
    }
    connectedSignal_(connected_);
}

void Model::attemptLogin()
{
    uint32_t const userId = UserId::get();

    std::ostringstream oss;
    if (zerok_)
    {
        Json::Value login;
        login["Name"] = userName_;
        login["PasswordHash"] = password_;
        login["UserID"] = userId;
        login["LobbyVersion"] = "flobby " FLOBBY_VERSION;
        login["ClientType"] = 3; // ZKL(1)|Linux(2), see enum ClientTypes in ZKS code

        Json::FastWriter writer;
        oss << "Login " << writer.write(login);
    }
    else
    {
        oss << "LOGIN " << userName_ << " " << password_ << " " << 0x464C4C /*FLL*/ << " * flobby "<< FLOBBY_VERSION <<"\t" << userId << "\tcl sp p m";
    }
    controller_.send(oss.str());
}

void Model::message(std::string const & msg)
{
    LOG(DEBUG) << "message: " << msg;

    processServerMsg(msg);
}

int Model::runProcess(std::string const& cmd, bool logToFile)
{
    LOG(DEBUG) << "runProcess: '" << cmd << "'";

    int ret;
    if (logToFile)
    {
        // create log filename
        std::string const first = cmd.substr(0, cmd.find(' '));
        boost::filesystem::path const path(first);
        std::string const log = cacheDir() + "flobby_process_" + path.stem().string() + ".log";
        LOG(DEBUG) << "runProcess logFile: '" << log << "'";

        // redirect stdout and stderr to log file
        std::string cmdLine = cmd + " >> " + log + " 2>&1";

        LOG(DEBUG) << "runProcess system(): '" << cmdLine << "'";
        ret = std::system(cmdLine.c_str());
    }
    else
    {
        std::string cmdLine = cmd + " >> /dev/null" + " 2>&1";
        LOG(DEBUG) << "runProcess system(): '" << cmdLine << "'";
        ret = std::system(cmdLine.c_str());
    }

    return ret;
}

void Model::processDone(std::pair<unsigned int, int> idRetPair)
{
    LOG(DEBUG)<< "processDone, id:"<< idRetPair.first << " ret:" << idRetPair.second;
    if (idRetPair.first == springId_)
    {
        springExitSignal_();
        springId_ = 0;
        meInGame(false);
    }
    else if (idRetPair.first == prDownloaderId_)
    {
        // remove possible * at end of engine downloads
        if (prDownloadName_.back() == '*')
        {
            prDownloadName_.pop_back();
        }

        downloadDoneSignal_(prDownloadType_, prDownloadName_, idRetPair.second == 0 ? true : false);
        prDownloaderId_ = 0;
    }
    else if (idRetPair.first == curlId_)
    {
        downloadDoneSignal_(DT_CURL, curlDownloadUrl_, idRetPair.second == 0 ? true : false);
        curlId_ = 0;
    }

    // check start of downloaded demo
    if (demoDownloadJobs_.find(idRetPair.first) != demoDownloadJobs_.end())
    {
        // if job failed insert zero to indicate failure
        if (idRetPair.second != 0)
        {
            demoDownloadJobs_.insert(0);
        }

        demoDownloadJobs_.erase(idRetPair.first);
        if (demoDownloadJobs_.empty())
        {
            boost::filesystem::path const pathUrl(start_replay_Args_[0]);
            startDemoSignal_(start_replay_Args_[3], flobbyDemo_ + pathUrl.extension().string());
        }
        else if (demoDownloadJobs_.size() == 1 && demoDownloadJobs_.count(0) == 1)
        {
            serverMsgSignal_("failed to start demo", 1);
            demoDownloadJobs_.clear();
        }
    }
}

void Model::connect(const std::string & host, const std::string & port)
{
    if (!connected_)
    {
        controller_.connect(host, port);
    }
    else
    {
        connectedSignal_(true);
    }
}

void Model::login(const std::string & username, const std::string & password)
{
    if (connected_)
    {
        loginInProgress_ = true;
        userName_ = username;
        password_ = password;

        attemptLogin();
    }
    else
    {
        loginResultSignal_(false, "not connected");
    }
}

void Model::registerAccount(std::string const & userName, std::string const & passwordHash, std::string const & email)
{
    if (connected_)
    {
        LOG_IF(FATAL, userName.empty())<< "userName empty";
        LOG_IF(FATAL, passwordHash.empty())<< "passwordHash empty";

        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["Name"] = userName;
            jv["PasswordHash"] = passwordHash;

            Json::FastWriter writer;
            oss << "Register " << writer.write(jv);
        }
        else
        {
            oss << "REGISTER " << userName << " " << passwordHash;
            if (!email.empty())
            {
                oss << " " << email;
            }
        }
        controller_.send(oss.str());
    }
    else
    {
        registerResultSignal_(false, "not connected");
    }
}

void Model::confirmAgreement()
{
    std::ostringstream oss;
    if (zerok_)
    {
        LOG(FATAL)<< "confirmAgreement should not be needed";
    }
    else
    {
        oss << "CONFIRMAGREEMENT";
    }
    controller_.send(oss.str());
}

std::vector<Battle const *> Model::getBattles()
{
    std::vector<Battle const *> battles;

    for (auto pair : battles_)
    {
        battles.push_back(pair.second.get());
    }

    return battles;
}

const Battle & Model::getBattle(int battleId)
{
    auto it = battles_.find(battleId);
    if (it == battles_.end())
    {
        throw std::invalid_argument("battle not found:" + boost::lexical_cast<std::string>(battleId));
    }
    return *it->second;
}

std::vector<User const *> Model::getUsers()
{
    std::vector<User const *> users;

    for (auto & pair : users_)
    {
        users.push_back(pair.second.get());
    }

    return users;
}

User const & Model::getUser(std::string const & str)
{
    return user(str);
}

User & Model::user(std::string const & str)
{
    auto it = users_.find(str);
    if (it == users_.end())
    {
        throw std::invalid_argument("user not found:" + str);
    }
    return *it->second;
}

Bot & Model::getBot(std::string const & str)
{
    Bots::iterator it = bots_.find(str);
    if (it == bots_.end())
    {
        throw std::invalid_argument("bot not found:" + str);
    }
    return *it->second;
}

Model::Bots const & Model::getBots()
{
    return bots_;
}

User& Model::me()
{
    if (me_ == 0)
    {
        throw std::runtime_error("me_ is zero");
    }
    return *me_;
}

void Model::meSpec(bool spec)
{
    User & u = me();
    UserBattleStatus ubs = u.battleStatus();
    ubs.spectator(spec);
    u.battleStatus(ubs);

    sendMyBattleStatus();
}

void Model::meReady(bool ready)
{
    User & u = me();
    UserBattleStatus ubs = u.battleStatus();
    ubs.ready(ready);
    u.battleStatus(ubs);

    sendMyBattleStatus();
}

void Model::meAllyTeam(int allyTeam)
{
    User & u = me();
    UserBattleStatus ubs = u.battleStatus();
    ubs.allyTeam(allyTeam);
    u.battleStatus(ubs);

    sendMyBattleStatus();
}

void Model::meSide(int side)
{
    User & u = me();
    UserBattleStatus ubs = u.battleStatus();
    ubs.side(side);
    u.battleStatus(ubs);

    sendMyBattleStatus();
}

void Model::meInGame(bool inGame)
{
    User & u = me();
    UserStatus us = u.status();
    us.inGame(inGame);
    u.status(us);

    std::ostringstream oss;
    if (zerok_)
    {
        Json::Value jv;
        jv["IsInGame"] = us.inGame();

        Json::FastWriter writer;
        oss << "ChangeUserStatus " << writer.write(jv);
    }
    else
    {
        oss << "MYSTATUS " << us;
    }
    controller_.send(oss.str());
}

void Model::meAway(bool away)
{
    User & u = me();
    UserStatus us = u.status();
    if (us.away() != away)
    {
        us.away(away);
        u.status(us);

        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["IsAfk"] = us.away();

            Json::FastWriter writer;
            oss << "ChangeUserStatus " << writer.write(jv);
        }
        else
        {
            oss << "MYSTATUS " << us;
        }
        controller_.send(oss.str());
    }
}

void Model::processServerMsg(const std::string & msg)
{
    std::istringstream iss(msg);

    try // catch all message parsing exceptions
    {
        std::string ex;
        LobbyProtocol::extractWord(iss, ex);

        if (checkFirstMsg_)
        {
            std::string const FirstMsg = (zerok_ ? "Welcome" : "TASServer");

            if (FirstMsg == ex)
            {
                checkFirstMsg_ = false;
            }
            else
            {
                LOG(WARNING) << "Disconnecting, first message is not "<< FirstMsg << ":" << msg;
                serverMsgSignal_("Bad first msg from server: " + msg.substr(0, 32), 1);
                disconnect();
                return;
            }
        }

        MessageHandlers& messageHandlers = zerok_ ? messageHandlersZerok_ : messageHandlers_;
        auto res = messageHandlers.find(ex);
        if (res != messageHandlers.end())
        {
            res->second(iss);
        }
        else
        {
            LOG(WARNING) << "Unhandled message:" << msg;
        }
    }
    catch (std::exception const & e)
    {
        LOG(WARNING) << "failed to process server msg:'" << msg
                  << "' (" << e.what() << ")";
    }

}

void Model::joinBattle(int battleId, std::string const & password)
{
    if (joinedBattleId_ != battleId)
    {
        if (joinedBattleId_ != -1)
        {
            leaveBattle();
        }
        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["BattleID"] = battleId;
            jv["Password"] = password;

            Json::FastWriter writer;
            oss << "JoinBattle " << writer.write(jv);
        }
        else
        {
            oss << "JOINBATTLE " << battleId << " " << password << " " << "scriptPassword" << std::rand();
        }
        controller_.send(oss.str());
    }
}

void Model::leaveBattle()
{
    if (zerok_)
    {
        /* TODO BattleID probably not needed
        Json::Value jv;
        jv["BattleID"] = joinedBattleId_;

        Json::FastWriter writer;
        oss << "LeaveBattle " << writer.write(jv);
        */
        controller_.send("LeaveBattle {}");
    }
    else
    {
        controller_.send("LEAVEBATTLE");
    }
}

void Model::sayBattle(std::string const & msg)
{
    if (!msg.empty())
    {
        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["Place"] = 1;
            // Target seem to be not needed
            jv["User"] = userName_;
            jv["IsEmote"] = false;
            jv["Text"] = msg;
            jv["Ring"] = false;

            Json::FastWriter writer;
            oss << "Say " << writer.write(jv);
        }
        else
        {
            oss << "SAYBATTLE " << msg;
        }
        controller_.send(oss.str());
    }
}

void Model::sayPrivate(std::string const & userName, std::string const & msg)
{
    if (msg.empty() || !connected_)
    {
        return;
    }
    if (userName.empty())
    {
        LOG(WARNING)<< "userName.empty()";
        return;
    }
    std::ostringstream oss;
    if (zerok_)
    {
        bool const offline = (users_.find(userName) == users_.end());

        Json::Value jv;
        jv["Place"] = 2;
        jv["Target"] = offline ? "Nightwatch" : userName;
        jv["User"] = userName_;
        jv["IsEmote"] = false;
        jv["Text"] = offline ? "!pm " + userName + " " + msg : msg;
        jv["Ring"] = false;

        Json::FastWriter writer;
        oss << "Say " << writer.write(jv);

        if (offline)
        {
            std::string const offlineMsg = "offline message sent: " + msg;

            sayPrivateSignal_(userName, offlineMsg);
        }
    }
    else
    {
        oss << "SAYPRIVATE " << userName << " " << msg;
    }
    controller_.send(oss.str());
}

void Model::startSpring()
{
    LOG(DEBUG) << "startSpring";

    if (springId_ != 0)
    {
        throw std::runtime_error("spring running");
    }

    if (joinedBattleId_ == -1)
    {
        throw std::runtime_error("battle not joined");
    }

    Battle const & battle = getBattle(joinedBattleId_);

    {
        std::ostringstream oss;
        oss << "GAME/HostIP=" << battle.ip();
        script_.add(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "GAME/HostPort=" << battle.port();
        script_.add(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "GAME/SourcePort=0";
        script_.add(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "GAME/MyPlayerName=" << userName_;
        script_.add(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "GAME/MyPasswd=";
        oss << myScriptPassword_;
        script_.add(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "GAME/IsHost=0";
        script_.add(oss.str());
    }

    // start spring
    {
        std::string const scriptPath = cacheDir() + "flobby_script.txt";
        script_.write(scriptPath);
        std::ostringstream oss;
        oss << "\"" << springPath_ << "\"" << " " << springOptions_ << " " << scriptPath;

        springId_ = controller_.startThread( boost::bind(&Model::runProcess, this, oss.str(), false) );
    }
    meInGame(true);
}

void Model::disconnect()
{
    if (!zerok_)
    {
        sendMessage("EXIT");
    }
    controller_.disconnect();
}

void Model::updateBattleRunningStatus(User const & user)
{
    // TODO should we break when founder found ? yes, if a user cannot be founder of multiple battles
    for (auto & pair : battles_)
    {
        assert(pair.second);
        Battle & b = *pair.second;
        // signal battle changed if founder InGame status differs from battle running status
        if (user.name() ==  b.founder() && b.running(user.status().inGame()) && loggedIn_)
        {
            battleChangedSignal_(b);
        }
    }
}

std::unique_ptr<uint8_t[]>  Model::getMapImage(std::string const & mapName, int mipLevel)
{
    assert(mipLevel >=0 && mipLevel <= 8);

    std::unique_ptr<uint8_t[]> res;
    unsigned short* rgb565 = unitSync_->GetMinimap(mapName.c_str(), mipLevel);
    if (rgb565 != 0)
    {
        int const size = (1024 >> mipLevel)*(1024 >> mipLevel);
        res.reset(new uint8_t[size*3]);
        uint8_t * p = res.get();
        for (int i=0; i<size; ++i)
        {
            unsigned char r5 = (*rgb565 & 0xf800) >> 11;
            unsigned char g6 = (*rgb565 & 0x07e0) >> 5;
            unsigned char b5 = *rgb565 & 0x001f;

            unsigned char r8 = (r5 << 3) | (r5 >> 2);
            unsigned char g8 = (g6 << 2) | (g6 >> 4);
            unsigned char b8 = (b5 << 3) | (b5 >> 2);

            p[0] = r8;
            p[1] = g8;
            p[2] = b8;

            rgb565 += 1;
            p += 3;
        }
    }

    return res;
}

std::unique_ptr<uint8_t[]>  Model::getMetalMap(std::string const & mapName, int & w, int & h)
{
    return getInfoMap(mapName, "metal", w, h);
}

std::unique_ptr<uint8_t[]>  Model::getHeightMap(std::string const & mapName, int & w, int & h)
{
    return getInfoMap(mapName, "height", w, h);
}

std::unique_ptr<uint8_t[]>  Model::getInfoMap(std::string const & mapName, std::string const & type, int & w, int & h)
{
    int size = unitSync_->GetInfoMapSize(mapName.c_str(), type.c_str(), &w, &h);
    if ( size == 0 || w == 0 || h == 0)
    {
        LOG(WARNING) << "GetInfoMapSize failed: " << mapName;
        return 0;
    }
    LOG(DEBUG) << "InfoMapSize: " << mapName << " / " << type << ", " << w << "x" << h;

    std::unique_ptr<uint8_t[]> data(new uint8_t[w*h]);
    int res = unitSync_->GetInfoMap(mapName.c_str(), type.c_str(), data.get(), 1 /* one byte */);
    if (res == 0)
    {
        LOG(WARNING) << "GetInfoMap failed: " << mapName << " / " << type;
        return data; // TODO is it still 0 ???
    }

    return data;
}

void Model::getMapSize(std::string const & mapName, int & w, int & h)
{
    int res = unitSync_->GetInfoMapSize(mapName.c_str(), "metal", &w, &h);
    if (res <= 0 || w == 0 || h == 0)
    {
        throw std::runtime_error("GetInfoMapSize failed:" + mapName);
    }
    LOG(DEBUG) << "InfoMapSize: " << w << "x" << h;
}

void Model::refresh()
{
    if (!unitSync_) return;

    unitSync_->Init(true, 1);
    unitSync_->GetPrimaryModCount();
    initMapIndex();

    updateSync();
}

void Model::updateSync()
{
    if (joinedBattleId_ != -1)
    {
        int const sync = calcSync(getBattle(joinedBattleId_));
        User & u = me();
        if (sync != u.battleStatus().sync())
        {
            LOG(DEBUG) << "sync changed:" << sync;
            u.battleStatus_.sync(sync);
            sendMyBattleStatus();
        }
    }
}

void Model::sendMyInitialBattleStatus(Battle const & battle)
{
/* skip reset/change of my battle status when joining new game, this is to not break matchmaking hosts
    UserBattleStatus ubs;

    // select next free team
    int team;
    for (int i=0; i<16; ++i)
    {
        team = i;
        bool occupied = false;
        for (Battle::BattleUsers::value_type pair : battle.users())
        {
            assert(pair.second);
            User const & u = *pair.second;
            if (u.battleStatus().team() == i && u != me())
            {
                // team number already taken
                occupied = true;
                break;
            }
        }
        if (!occupied)
        {
            for (Bots::value_type pair : getBots())
            {
                assert(pair.second);
                Bot const & bot = *pair.second;
                if (bot.battleStatus().team() == i)
                {
                    // team number already taken
                    occupied = true;
                    break;
                }
            }
        }
        if (!occupied)
        {
            break;
        }
    }
    ubs.team(team);

    // select next free ally team
    int allyTeam;
    for (int i=0; i<16; ++i)
    {
        allyTeam = i;
        bool occupied = false;
        for (Battle::BattleUsers::value_type pair : battle.users())
        {
            assert(pair.second);
            User const & u = *pair.second;
            if (u.battleStatus().allyTeam() == i && u != me())
            {
                // ally team number already taken
                occupied = true;
                break;
            }
        }
        if (!occupied)
        {
            for (Bots::value_type pair : getBots())
            {
                assert(pair.second);
                Bot const & bot = *pair.second;
                if (bot.battleStatus().allyTeam() == i)
                {
                    // team number already taken
                    occupied = true;
                    break;
                }
            }
        }
        if (!occupied)
        {
            break;
        }
    }
    ubs.allyTeam(allyTeam);

    ubs.ready(true);
    ubs.sync(calcSync(battle));

    // update myself
    User & u = me();
    u.battleStatus(ubs);
*/
    sendMyBattleStatus();
}

bool Model::gameExist(std::string const & gameName)
{
    if (!unitSync_) return false;

    return (unitSync_->GetPrimaryModChecksumFromName( gameName.c_str()) != 0 );
}

int Model::calcSync(Battle const & battle)
{
    if (!unitSync_) return 2;

    unsigned int const modChecksum = unitSync_->GetPrimaryModChecksumFromName( battle.modName().c_str() );
    unsigned int const mapChecksum = unitSync_->GetMapChecksumFromName( battle.mapName().c_str() );

    if (modChecksum == 0 || mapChecksum == 0)
    {
        // not synced, we dont have either mod or map
        LOG(WARNING)<< "modChecksum:"<< modChecksum;
        LOG(WARNING)<< "mapChecksum:"<< mapChecksum;
        return 2;
    }
    else if ((battle.modHash() == 0 || battle.modHash() == modChecksum)
            && (battle.mapHash() == 0 || battle.mapHash() == mapChecksum))
    {
        // synced, either host dont know hash or hashes match
        return 1;
    }
    else
    {
        // not synced, log checksum mismatches
        LOG_IF(WARNING, battle.modHash() != modChecksum)<< "mod checksum mismatch: "
                << battle.modHash() << " != " << modChecksum;
        LOG_IF(WARNING, battle.mapHash() != mapChecksum)<< "map checksum mismatch: "
                << battle.mapHash() << " != " << mapChecksum;
        return 2;
    }
}

void Model::sendMyBattleStatus()
{
    std::ostringstream oss;
    if (zerok_)
    {
        Json::Value jv;
        jv["AllyNumber"] = me().battleStatus().allyTeam();
        jv["IsSpectator"] = me().battleStatus().spectator();
        jv["Name"] = userName_;
        jv["Sync"] = me().battleStatus().sync();
        // jv["TeamNumber"] = me().battleStatus().team();

        Json::FastWriter writer;
        oss << "UpdateUserBattleStatus " << writer.write(jv);
    }
    else
    {
        oss << "MYBATTLESTATUS " << me().battleStatus() << " 255"; // TODO color
    }
    controller_.send(oss.str());

}

void Model::handle_TASServer(std::istream & is) // protocolVersion springVersion udpPort serverMode (e.g 0.35 88 8201 0)
{
    using namespace LobbyProtocol;

    ServerInfo si;
    std::string ex;

    extractWord(is, ex);
    si.protocolVersion_ = ex;

    extractWord(is, ex);
    si.springVersion_ = ex;

    extractWord(is, ex);
    si.udpPort_ = boost::lexical_cast<unsigned short>(ex);

    extractWord(is, ex);
    si.serverMode_ = boost::lexical_cast<unsigned short>(ex);

    serverInfo_ = si;
    serverInfoSignal_(serverInfo_);
}

void Model::handle_Welcome(std::istream & is) // Engine Game Version
{
    using namespace LobbyProtocol;

    Json::Value welcome;
    is >> welcome;

    ServerInfo si;

    si.springVersion_ = welcome["Engine"].asString();
    si.game_ = welcome["Game"].asString();
    si.userCount_ = welcome["UserCount"].asInt();
    si.protocolVersion_ = welcome["Version"].asString();
    si.serverMode_ = 0;
    si.udpPort_ = 0;

    serverInfo_ = si;
    serverInfoSignal_(serverInfo_);
}

void Model::handle_LoginResponse(std::istream & is) // ResultCode Reason
{
    Json::Value val;
    is >> val;

    int const resultCode = val["ResultCode"].asInt();

    bool loginSuccess = false;
    std::string reason;

    switch (resultCode)
    {
    case 0:
        loginSuccess = true;
        break;
    case 1:
        reason = "already connected";
        break;
    case 2:
        reason = "invalid name";
        break;
    case 3:
        reason = "invalid password";
        break;
    case 4:
        reason = "banned";
        break;

    }
    reason += val["Reason"].asString();

    if (!loginSuccess)
    {
        loginInProgress_ = false;
        loginResultSignal_(loginSuccess, reason);
    }
}

void Model::handle_User(std::istream & is) // User content
{
    Json::Value jv;
    is >> jv;

    std::string const name = jv["Name"].asString();

    Users::iterator it = users_.find(name);
    if (it != users_.end())
    {
        // existing user, update
        User& user = *it->second;
        auto const pairChangeId = user.updateUser(jv);
        if (loggedIn_)
        {
            if (pairChangeId.first) {
                if (user.joinedBattle() != -1) {
                    Battle& b = battle(user.joinedBattle());
                    b.joined(user);
                    userJoinedBattleSignal_(user, b);
                }
                else {
                    Battle& b = battle(pairChangeId.second);
                    b.left(user);
                    userLeftBattleSignal_(user, b);
                    if (user == me() && b.id () == joinedBattleId_) {
                        joinedBattleId_ = -1;
                        bots_.clear();
                    }
                }
            }
            userChangedSignal_(user);
        }
    }
    else
    {
        // new user, this logic depend on server sending "me" User first
        std::shared_ptr<User> u(new User(jv));
        users_[u->name()] = u;
        if (me_ == 0 && loginInProgress_ && u->name() == userName_)
        {
            me_ = u.get();
            loggedIn_ = true;
            loginInProgress_ = false;
            loginResultSignal_(true, "");
        }
        else if (loggedIn_)
        {
            userJoinedSignal_(*u);
            if (u->joinedBattle() != -1) {
                Battle& b = battle(u->joinedBattle());
                b.joined(*u);
                userJoinedBattleSignal_(*u, b);
            }
        }
        else
        {
            LOG(WARNING)<< "unexpected User message:"<< u->name();
        }
    }
}

void Model::handle_UserDisconnected(std::istream & is) // Name Reason
{
    Json::Value jv;
    is >> jv;

    std::string const name = jv["Name"].asString();

    User const & user = getUser(name);
    userLeftSignal_(user);
    users_.erase(name);
}

void Model::handle_BattleAdded(std::istream & is) // BattleAdded content
{
    Json::Value jv;
    is >> jv;

    std::shared_ptr<Battle> b(new Battle(jv["Header"]));
    battles_[b->id()] = b;

    if (loggedIn_)
    {
       battleOpenedSignal_(*b);
    }

}

void Model::handle_ACCEPTED(std::istream & is) // userName
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    assert(ex == userName_);
}

void Model::handle_DENIED(std::istream & is) // {reason}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractSentence(is, ex);
    loginInProgress_ = false;
    loginResultSignal_(false, ex);
}

void Model::handle_ADDUSER(std::istream & is) // userName country cpu [accountID]
{
    using namespace LobbyProtocol;

    std::shared_ptr<User> u(new User(is));
    users_[u->name()] = u;
    if (me_ == 0 && u->name() == userName_)
    {
        me_ = u.get();
    }

    if (loggedIn_)
    {
        userJoinedSignal_(*u);
    }
}

void Model::handle_REMOVEUSER(std::istream & is) // userName
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    User const & user = getUser(userName);
    userLeftSignal_(user);
    users_.erase(userName);

}

void Model::handle_BATTLEOPENED(std::istream & is)
{
    std::shared_ptr<Battle> b(new Battle(is));
    battles_[b->id()] = b;

    // set running status
    User& founder = user(b->founder());
    b->running(founder.status().inGame());

    founder.joinedBattle(*b);

    b->joined(founder);

    if (loggedIn_)
    {
        battleOpenedSignal_(*b);
        userJoinedBattleSignal_(founder, *b);
        userChangedSignal_(founder);
    }
}

void Model::handle_BATTLECLOSED(std::istream & is) // battleId
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    int const battleId = boost::lexical_cast<int>(ex);
    Battle const & battle = getBattle(battleId);

    // simulate LEFTBATTLE messages since uberserver do not send this before BATTLECLOSED
    auto const users = battle.users(); // we need to a copy here since handle_LEFTBATTLE changes battle users map
    for (auto const& pairNameUser : users)
    {
        std::stringstream ss;
        ss << battle.id() << " " << pairNameUser.first;
        handle_LEFTBATTLE(ss);
    }

    battleClosedSignal_(battle);

    battles_.erase(battleId);
}

void Model::handle_BattleRemoved(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    int const battleId = jv["BattleID"].asInt();

    Battle const & battle = getBattle(battleId);

    // TODO i probably don't need LEFTBATTLE workaround here

    battleClosedSignal_(battle);

    battles_.erase(battleId);
}

void Model::handle_UPDATEBATTLEINFO(std::istream & is) // battleId spectatorCount locked mapHash {mapName}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    Battle & b = getBattle(ex);
    b.updateBattleInfo(is);

    // update self sync
    if (b.id() == joinedBattleId_) {
        updateSync();
    }

    if (loggedIn_) // only inform ui after login sequence is complete
    {
        battleChangedSignal_(b);
    }
}

void Model::handle_BattleUpdate(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    Battle & b = getBattle(jv["Header"]["BattleID"].asString());
    b.updateBattleUpdate(jv["Header"]);

    // update self sync
    if (b.id() == joinedBattleId_) {
        updateSync();
    }

    if (loggedIn_) // only inform ui after login sequence is complete
    {
        battleChangedSignal_(b);
    }
}

void Model::handle_JOINEDBATTLE(std::istream & is) // battleId username [scriptPassword]
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    Battle & b = getBattle(ex);
    extractWord(is, ex);
    User & u = user(ex);
    b.joined(u);
    u.joinedBattle(b);
    if (loggedIn_)
    {
        userJoinedBattleSignal_(u, b);
        userChangedSignal_(u);
    }
    if (u == me())
    {
        try
        {
            extractWord(is, myScriptPassword_);
        }
        catch (std::invalid_argument const & e)
        {
            LOG(WARNING)<< "script password not sent in JOINEDBATTLE";
        }

    }
}

void Model::handle_BattlePoll(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    const std::string msg = (jv["YesNoVote"].asBool() ? "Poll: " : "")
        + jv["Topic"].asString();

    battleChatMsgSignal_("Nightwatch", msg);
}

void Model::handle_BattlePollOutcome(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    const std::string msg = (jv["YesNoVote"].asBool() ? "Poll: " : "")
        + jv["Topic"].asString()
        + " [END:" + (jv["Success"].asBool() ? "SUCCESS" : "FAILED") + "]";

    battleChatMsgSignal_("Nightwatch", msg);
}

void Model::handle_JoinedBattle(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    Battle & b = getBattle(jv["BattleID"].asString());
    User & u = user(jv["User"].asString());
    b.joined(u);
    u.joinedBattle(b);

    assert(loggedIn_);

    userJoinedBattleSignal_(u, b);
    userChangedSignal_(u);

    if (me() == u)
    {
        joinedBattleId_ = b.id();
        sendMyInitialBattleStatus(b);
        battleJoinedSignal_(b);
    }
}

// BattleID, Players=[UpdateUserBattleStatus, ...], Bots=[UpdateBotStatus, ...], Options=Dictionary<string, string>
void Model::handle_JoinBattleSuccess(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    Battle & b = getBattle(jv["BattleID"].asString());

    joinedBattleId_ = b.id();

    Json::Value players = jv["Players"];
    for (Json::Value& userBattleStatus : players)
    {
        User& u = user(userBattleStatus["Name"].asString());
        // probably don't need the joined stuff here but keeping until i know for sure
        b.joined(u);
        u.joinedBattle(b);
        u.updateUserBattleStatus(userBattleStatus);

        userJoinedBattleSignal_(u, b);
        userChangedSignal_(u);
    }

    // join as spectator
    auto bs = me().battleStatus();
    bs.spectator(true);
    me().battleStatus(bs);
    sendMyInitialBattleStatus(b);
    battleJoinedSignal_(b);

    Json::Value bots = jv["Bots"];
    for (Json::Value& updateBotStatus : bots)
    {
        handleUpdateBotStatus(updateBotStatus);
    }

    handleZkOptions(jv["Options"]);
}

void Model::handle_LEFTBATTLE(std::istream & is) // battleId username
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    Battle & b = getBattle(ex);
    extractWord(is, ex);
    User & u = user(ex);
    b.left(u);
    u.leftBattle(b);
    if (loggedIn_)
    {
        userLeftBattleSignal_(u, b);
        userChangedSignal_(u);
    }
    if (u == me() && b.id () == joinedBattleId_)
    {
        joinedBattleId_ = -1;
        bots_.clear();
    }
}

void Model::handle_LeftBattle(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    Battle & b = getBattle(jv["BattleID"].asString());
    User & u = user(jv["User"].asString());

    b.left(u);
    u.leftBattle(b);
    if (loggedIn_)
    {
        userLeftBattleSignal_(u, b);
        userChangedSignal_(u);
    }
    if (u == me() && b.id () == joinedBattleId_)
    {
        joinedBattleId_ = -1;
        bots_.clear();
    }
}

void Model::handle_CLIENTSTATUS(std::istream & is) // userName status
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    User & u = user(ex);
    extractWord(is, ex);
    u.status(UserStatus(ex));
    updateBattleRunningStatus(u);
    if (loggedIn_)
    {
        userChangedSignal_(u);
    }
}

void Model::handle_LOGININFOEND(std::istream & is)
{
    loggedIn_ = true;
    loginInProgress_ = false;
    loginResultSignal_(true, "");
}

void Model::handle_JOINBATTLE(std::istream & is) // battleId hashCode
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    joinedBattleId_ = boost::lexical_cast<int>(ex);
    Battle & b = battle(joinedBattleId_);
    extractWord(is, ex);
    b.modHash( static_cast<unsigned int>( boost::lexical_cast<int64_t>(ex)) );
    script_.clear();
    bots_.clear();
    LOG(DEBUG) << "modHash " << b.modHash();
    LOG(DEBUG) << "mapHash " << b.mapHash();
}

void Model::handle_JOINBATTLEFAILED(std::istream & is) // {reason}
{
    using namespace LobbyProtocol;

    std::string reason;
    extractSentence(is, reason);

    joinBattleFailedSignal_(reason);
}

void Model::handle_SETSCRIPTTAGS(std::istream & is) // {data} [{data} ...]
{
    using namespace LobbyProtocol;
    std::string ex;
    while (!is.eof())
    {
        extractSentence(is, ex);
        auto keyValuePair = script_.getKeyValuePair(ex);
        if (!keyValuePair.first.empty())
        {
            setScriptTagSignal_(keyValuePair.first, keyValuePair.second);
        }
    }
}

void Model::handle_REMOVESCRIPTTAGS(std::istream & is) // key [key ...]
{
    using namespace LobbyProtocol;
    std::string ex;
    while (!is.eof())
    {
        extractWord(is, ex);
        std::string const key = script_.getKey(ex);
        if (!key.empty())
        {
            removeScriptTagSignal_(key);
        }
    }
}

void Model::handleZkOptions(Json::Value const& jv)
{
    // zero-k seem to always send all options in this message, start by removing all
    removeScriptTagSignal_("*");

    for (Json::ValueConstIterator it = jv.begin(); it != jv.end(); ++it)
    {
        setScriptTagSignal_(it.key().asString(), (*it).asString());
    }

}

void Model::handle_SetModOptions(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    handleZkOptions(jv["Options"]);
}

void Model::handle_CLIENTBATTLESTATUS(std::istream & is) // userName battleStatus color
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    User & u = user(ex);
    extractWord(is, ex);
    u.battleStatus(UserBattleStatus(ex));

    extractWord(is, ex);
    u.color(boost::lexical_cast<int>(ex));

    userChangedSignal_(u);
}

void Model::handle_UpdateUserBattleStatus(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    User& u = user(jv["Name"].asString());
    u.updateUserBattleStatus(jv);
    userChangedSignal_(u);
}

void Model::handle_REQUESTBATTLESTATUS(std::istream & is)
{
    Battle const & b = getBattle(joinedBattleId_); // joinedBattleId_ set in JOINBATTLE above
    sendMyInitialBattleStatus(b);
    battleJoinedSignal_(b);
}

void Model::handle_SAIDBATTLE_SAIDBATTLEEX(std::istream & is) // userName {message}
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    std::string ex;
    extractToNewline(is, ex);
    battleChatMsgSignal_(userName, ex);
}

void Model::handle_SAYPRIVATE(std::istream & is) // userName {message}
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    std::string msg;
    extractToNewline(is, msg);
    sayPrivateSignal_(userName, msg);
}

void Model::handle_SAIDPRIVATE(std::istream & is) // userName {message}
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    std::string msg;
    extractToNewline(is, msg);
    saidPrivateSignal_(userName, msg);
}

void Model::handle_ADDBOT(std::istream & is) // battleId name owner battleStatus teamColor {AIDLL}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    int const battleId = boost::lexical_cast<int>(ex);
    if (battleId == joinedBattleId_)
    {
        Bot * b = new Bot(is);
        bots_[b->name()] = b;
        botAddedSignal_(*b);
    }
}

void Model::handle_UpdateBotStatus(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    handleUpdateBotStatus(jv);
}

void Model::handleUpdateBotStatus(Json::Value& jv)
{
    if (-1 != joinedBattleId_)
    {
        std::string const& botName = jv["Name"].asString();

        Bots::iterator it = bots_.find(botName);
        if (it != bots_.end())
        {
            // existing bot, update
            Bot& bot = *it->second;
            bot.updateBotStatus(jv);
            botChangedSignal_(bot);
        }
        else
        {
            // new bot
            Bot* b = new Bot(jv);
            bots_[b->name()] = b;
            botAddedSignal_(*b);
        }
    }
    else
    {
        LOG(WARNING)<< "ignoring UpdateBotStatus since not joined a battle";
    }
}

void Model::handle_REMOVEBOT(std::istream & is) // battleId name
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    int const battleId = boost::lexical_cast<int>(ex);
    if (battleId == joinedBattleId_)
    {
        extractWord(is, ex);
        Bot & b = getBot(ex);
        botRemovedSignal_(b);
        bots_.erase(ex);
    }
}

void Model::handle_RemoveBot(std::istream & is)
{
    if (-1 != joinedBattleId_)
    {
        Json::Value jv;
        is >> jv;

        std::string const name = jv["Name"].asString();
        Bot& b = getBot(name);
        botRemovedSignal_(b);
        bots_.erase(name);
    }
    else
    {
        LOG(WARNING)<< "ignoring RemoveBot since not joined a battle";
    }

}

void Model::handle_UPDATEBOT(std::istream & is) // battleId name battleStatus teamColor
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    int const battleId = boost::lexical_cast<int>(ex);
    if (battleId == joinedBattleId_)
    {
        extractWord(is, ex);
        Bot & b = getBot(ex);
        extractWord(is, ex);
        b.battleStatus(UserBattleStatus(ex));
        extractWord(is, ex);
        b.color(boost::lexical_cast<int>(ex));
        botChangedSignal_(b);
    }
}

void Model::handle_MOTD(std::istream & is) // {message}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractToNewline(is, ex);
    serverMsgSignal_("MOTD: " + ex, 0);
}

void Model::handle_SERVERMSG(std::istream & is) // {message}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractToNewline(is, ex);
    serverMsgSignal_(ex, 1);
}

void Model::handle_SERVERMSGBOX(std::istream & is) // {message} [{url}]
{
    using namespace LobbyProtocol;
    std::string msg;
    extractSentence(is, msg);
    if (!is.eof())
    {
        std::string url;
        extractSentence(is, url);
        msg += " " + url;
    }
    serverMsgSignal_(msg, 1);
}

void Model::handle_CHANNEL(std::istream & is) // channelName userCount [{topic}]
{
    Channel channel(is);
    channels_.push_back(channel);
}

void Model::handle_ENDOFCHANNELS(std::istream & is) // empty
{
    channelsSignal_(channels_);
}

void Model::handle_JOIN(std::istream & is) // channelName
{
    using namespace LobbyProtocol;
    std::string channelName;
    extractWord(is, channelName);
    channelJoinedSignal_(channelName);
}

void Model::handle_JoinChannelResponse(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    std::string const channelName = jv["ChannelName"].asString();
    if (jv["Success"].asBool())
    {
        // TODO add topic info
        channelJoinedSignal_(channelName);

        Json::Value const& jvUsers = jv["Channel"]["Users"];
        for (Json::ValueConstIterator it = jvUsers.begin(); it != jvUsers.end(); ++it)
        {
            userJoinedChannelSignal_(channelName, (*it).asString());
        }
    }
    else
    {
        // TODO
        LOG(WARNING)<< "failed to join channel "<< channelName;
    }
}

void Model::handle_CLIENTS(std::istream & is) // channelName {clients}
{
    using namespace LobbyProtocol;
    std::string channelName;
    extractWord(is, channelName);

    std::vector<std::string> clients;
    std::string userName;
    while (!is.eof())
    {
        extractWord(is, userName);
        clients.push_back(userName);
    }
    channelClientsSignal_(channelName, clients);
}

std::string const & Model::getWriteableDataDir() const
{
    assert(!writeableDataDir_.empty());
    return writeableDataDir_;
}

void Model::getChannels()
{
    channels_.clear();
    if (zerok_)
    {
        // TODO zk do not support getting channels
    }
    else
    {
        controller_.send("CHANNELS");
    }
}

void Model::joinChannel(std::string const & channelName, std::string const & password)
{
    if (!channelName.empty() && connected_)
    {
        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["ChannelName"] = channelName;
            if (!password.empty())
            {
                jv["Password"] = password;
            }
            Json::FastWriter writer;
            oss << "JoinChannel " << writer.write(jv);
        }
        else
        {
            oss << "JOIN " << channelName;
            if (!password.empty())
            {
                oss << " " << password;
            }
        }
        controller_.send(oss.str());
    }
}

void Model::sayChannel(std::string const & channelName, std::string const & message)
{
    if (!channelName.empty() && !message.empty() && connected_)
    {
        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["Place"] = 0;
            jv["Target"] = channelName;
            jv["User"] = userName_;
            jv["IsEmote"] = false;
            jv["Text"] = message;
            jv["Ring"] = false;

            Json::FastWriter writer;
            oss << "Say " << writer.write(jv);
        }
        else
        {
            oss << "SAY " << channelName << " " << message;
        }
        controller_.send(oss.str());
    }
}

void Model::leaveChannel(std::string const & channelName)
{
    if (!channelName.empty() && connected_)
    {
        std::ostringstream oss;
        if (zerok_)
        {
            Json::Value jv;
            jv["ChannelName"] = channelName;

            Json::FastWriter writer;
            oss << "LeaveChannel " << writer.write(jv);
        }
        else
        {
            oss << "LEAVE " << channelName;
        }
        controller_.send(oss.str());
    }
}

void Model::initMapIndex()
{
    mapIndex_.clear();
    int const mapCount = unitSync_->GetMapCount();

    for (int i=0; i<mapCount; ++i)
    {
        mapIndex_[unitSync_->GetMapName(i)] = i;
    }
}

MapInfo Model::getMapInfo(std::string const & mapName)
{
    auto it = mapIndex_.find(mapName);
    if (it == mapIndex_.end())
    {
        throw std::runtime_error("map " + mapName + " not found");
    }
    if (unitSync_.get() == 0)
    {
        throw std::runtime_error("UnitSync not initialized");
    }
    return MapInfo(*unitSync_, it->second);
}

void Model::handle_JOINED(std::istream & is) // channelName userName
{
    using namespace LobbyProtocol;
    std::string channelName;
    extractWord(is, channelName);
    std::string userName;
    extractWord(is, userName);
    userJoinedChannelSignal_(channelName, userName);
}

void Model::handle_ChannelUserAdded(std::istream & is)
{
    Json::Value jv;
    is >> jv;
    userJoinedChannelSignal_(jv["ChannelName"].asString(), jv["UserName"].asString());
}

void Model::handle_LEFT(std::istream & is) // channelName userName [{reason}]
{
    using namespace LobbyProtocol;

    std::string channelName;
    extractWord(is, channelName);

    std::string userName;
    extractWord(is, userName);

    std::string reason;
    if (!is.eof())
    {
        extractSentence(is, reason);
    }

    userLeftChannelSignal_(channelName, userName, reason);
}

void Model::handle_ChannelUserRemoved(std::istream & is)
{
    Json::Value jv;
    is >> jv;
    userLeftChannelSignal_(jv["ChannelName"].asString(), jv["UserName"].asString(), "");
}

void Model::handle_CHANNELTOPIC(std::istream & is) // channelName author changedTime {topic}
{
    using namespace LobbyProtocol;

    std::string channelName;
    extractWord(is, channelName);

    std::string author;
    extractWord(is, author);

    std::string changedTime;
    extractWord(is, changedTime);
    uint64_t const ms = boost::lexical_cast<uint64_t>(changedTime);

    std::string topic;
    extractSentence(is, topic);

    channelTopicSignal_(channelName, author, ms/1000, topic);
}

void Model::handle_CHANNELMESSAGE(std::istream & is) // channelName {message}
{
    using namespace LobbyProtocol;

    std::string channelName;
    extractWord(is, channelName);

    std::string message;
    extractToNewline(is, message);

    channelMessageSignal_(channelName, message);
}

void Model::handle_SAID_SAIDEX(std::istream & is) // channelName userName {message}
{
    using namespace LobbyProtocol;

    std::string channelName;
    extractWord(is, channelName);

    std::string userName;
    extractWord(is, userName);

    std::string msg;
    extractToNewline(is, msg);
    saidChannelSignal_(channelName, userName, msg);
}

bool Model::handle_Nightwatch(Json::Value & jv)
{
    std::string const text = jv["Text"].asString();

    if (text.find("!pm|") == 0)
    {
        NightwatchPm const pm = checkNightwatchPm(text);
        if (pm.valid_ && !pm.user_.empty())
        {
            std::ostringstream ossTimeText;
            ossTimeText << "[" << pm.time_ << "] " << pm.text_;

            if (!pm.channel_.empty())
            {
                saidChannelSignal_(
                    pm.channel_,
                    pm.user_,
                    ossTimeText.str() );
            }
            else
            {
                saidPrivateSignal_(
                    pm.user_,
                    ossTimeText.str() );
            }
            return true;
        }
    }

    return false;
}

void Model::handle_Say(std::istream & is)
{
    Json::Value jv;
    is >> jv;
    int const place = jv["Place"].asInt();

    switch (place)
    {
    case 0: // Channel
        saidChannelSignal_(
            jv["Target"].asString(),
            jv["User"].asString(),
            jv["Text"].asString() );
        break;

    case 2: // User
    {
        std::string const target = jv["Target"].asString();
        std::string const user = jv["User"].asString();
        if (user == "Nightwatch" && handle_Nightwatch(jv))
        {
            // all is done in handle_Nightwatch
        }
        else if (target == userName_)
        {
            saidPrivateSignal_(
                user,
                jv["Text"].asString() );
        }
        else if (user == userName_)
        {
            sayPrivateSignal_(
                target,
                jv["Text"].asString() );
        }
        else
        {
            LOG(WARNING)<< "Say User with wrong Target:"<< target
                        << ", User:"<< jv["User"].asString()
                        << ", Text:"<< jv["Text"].asString();
        }
    }
    break;

    case 1: // Battle
    case 3: // BattlePrivate
        battleChatMsgSignal_(jv["User"].asString(), jv["Text"].asString());
        break;

    case 5: // MessageBox
        serverMsgSignal_(jv["Text"].asString(), 1);
        break;

    default:
        LOG(WARNING)<< "unhandled Say Place "<< place ;
        break;
    }
}

void Model::handle_RING(std::istream & is) // userName
{
    using namespace LobbyProtocol;

    std::string userName;
    extractWord(is, userName);

    ringSignal_(userName);
}

std::vector<std::string> Model::getMaps()
{
    std::vector<std::string> maps;

    for (auto & pair : mapIndex_)
    {
        maps.push_back(pair.first);
    }
    return maps;
}

unsigned int Model::getMapChecksum(std::string const & mapName)
{
    if (!unitSync_) return 0;

    return unitSync_->GetMapChecksumFromName(mapName.c_str());
}

void Model::handle_ADDSTARTRECT(std::istream & is) // allyNo left top right bottom
{
    using namespace LobbyProtocol;

    std::string ex;

    extractWord(is, ex);
    int const ally = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    int const left = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    int const top = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    int const right = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    int const bottom = boost::lexical_cast<int>(ex);

    addStartRectSignal_(StartRect(ally, left, top, right, bottom));
}

// SetRectangle is removed from ZK protocol
void Model::handle_SetRectangle(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    int const number = jv["Number"].asInt();

    if (jv.isMember("Rectangle"))
    {
        // SetRectangle message is a bit of a mess, it is hopefully good to always clear the rect first
        removeStartRectSignal_(number);

        Json::Value const& rect = jv["Rectangle"];

        int const left = rect["Left"].asInt();
        int const top = rect["Top"].asInt();
        int const right = rect["Right"].asInt();
        int const bottom = rect["Bottom"].asInt();

        addStartRectSignal_(StartRect(number, left, top, right, bottom));
    }
    else
    {
        removeStartRectSignal_(number);
    }
}

void Model::handle_REMOVESTARTRECT(std::istream & is) // allyNo
{
    using namespace LobbyProtocol;

    std::string ex;

    extractWord(is, ex);
    int const ally = boost::lexical_cast<int>(ex);

    removeStartRectSignal_(ally);
}

void Model::handle_REGISTRATIONACCEPTED(std::istream & is)
{
    registerResultSignal_(true, "");
}

void Model::handle_REGISTRATIONDENIED(std::istream & is) // {reason}
{
    using namespace LobbyProtocol;

    std::string reason;
    extractSentence(is, reason);

    registerResultSignal_(false, reason);
}

void Model::handle_RegisterResponse(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    int const resultCode = jv["ResultCode"].asInt();
    bool success = false;
    std::string reason;

    switch (resultCode)
    {
    case 0: // Ok
        success = true;
        break;

    case 1: // AlreadyConnected
        reason = "already connected";
        break;

    case 2: // InvalidName
        reason = "name already exists";
        break;

    case 3: // InvalidPassword
        reason = "invalid password";
        break;

    case 4: // Banned
        reason = "banned";
        break;

    case 5: // InvalidCharacters
        reason = "invalid name characters";
        break;

    default:
        LOG(ERROR)<< "unknown RegisterResponse ResultCode "<< resultCode;
        reason = "unknown reason";
        break;
    }

    std::string const zkReason = jv["Reason"].asString();
    if (!zkReason.empty())
    {
        reason += " (" + zkReason + ")";
    }

    registerResultSignal_(success, reason);
}

void Model::handle_AGREEMENT(std::istream & is) // {text}
{
    using namespace LobbyProtocol;

    std::string text;
    extractSentence(is, text);
    agreementStream_ << text << "\n";
}

void Model::handle_AGREEMENTEND(std::istream & is)
{
    std::string a = agreementStream_.str();
    agreementStream_.str("");

    agreementSignal_(a);
}

void Model::handle_PONG(std::istream & is)
{
    using namespace LobbyProtocol;

    waitingForPong_ = 0;
}

void Model::handle_HOSTPORT(std::istream & is)
{
    using namespace LobbyProtocol;

    std::string port;
    extractWord(is, port);

    if (joinedBattleId_ != -1)
    {
        Battle& b = battle(joinedBattleId_);
        b.setPort(port);
    }
    else
    {
        LOG(WARNING)<< "ignoring HOSTPORT since we are not in a battle";
    }
}

void Model::handle_FORCEJOINBATTLE(std::istream & is) // destinationBattleID [destinationBattlePassword]
{
    using namespace LobbyProtocol;

    std::string ex;
    extractWord(is, ex);
    int const battleId = boost::lexical_cast<int>(ex);

    std::string password;
    try
    {
        extractWord(is, password);
    }
    catch (std::invalid_argument const & e)
    {
        // ignore non-existing optional password
    }

    joinBattle(battleId, password);
}

void Model::handle_STARTLISTSUBSCRIPTION(std::istream & is) // empty
{
    using namespace LobbyProtocol;

    serverMsgSignal_("STARTLISTSUBSCRIPTION", 0);
}

void Model::handle_ENDLISTSUBSCRIPTION(std::istream & is) // empty
{
    using namespace LobbyProtocol;

    serverMsgSignal_("ENDLISTSUBSCRIPTION", 0);
}

void Model::handle_LISTSUBSCRIPTION(std::istream & is) // chanName=<NAME>
{
    using namespace LobbyProtocol;

    std::string ex;
    extractWord(is, ex);
    serverMsgSignal_(ex, 0);
}

void Model::handle_OK(std::istream & is) // <command>
{
    using namespace LobbyProtocol;

    std::string msg = "OK: ";
    std::string text;
    std::getline(is, text);
    msg += text;
    serverMsgSignal_(msg, 0);
}

void Model::handle_FAILED(std::istream & is) // <command> <text>
{
    using namespace LobbyProtocol;

    std::string msg = "FAILED: ";
    std::string text;
    std::getline(is, text);
    msg += text;
    serverMsgSignal_(msg, 1);
}

std::vector<AI> Model::getModAIs(std::string const & modName)
{
    std::vector<AI> ais;

    if (!unitSync_) return ais;

    int modIndex = unitSync_->GetPrimaryModIndex(modName.c_str());
    LOG(DEBUG) << "modIndex " << modIndex;

    if (modIndex >= 0)
    {
        const char* archiveName = unitSync_->GetPrimaryModArchive(modIndex);
        LOG(DEBUG) << "archiveName " << archiveName;
        unitSync_->AddAllArchives(archiveName);
        int aiCount = unitSync_->GetSkirmishAICount();
        LOG(DEBUG) << "aiCount " << aiCount;

        for (int i=0; i<aiCount; ++i)
        {
            LOG(DEBUG) << "\tai " << i;
            int infoKeyCount = unitSync_->GetSkirmishAIInfoCount(i);
            AI ai;
            for (int infoKeyIndex=0; infoKeyIndex<infoKeyCount; ++infoKeyIndex)
            {
                std::string infoKeyName = unitSync_->GetInfoKey(infoKeyIndex);
                std::string infoKeyType = unitSync_->GetInfoType(infoKeyIndex);
                if (infoKeyType == "string")
                {
                    std::string infoKeyValue = unitSync_->GetInfoValueString(infoKeyIndex);
                    LOG(DEBUG) << "\t\t" << infoKeyName << "=" << infoKeyValue;
                    ai.info_[infoKeyName] = infoKeyValue;
                }
            }

            if (ai.info_.count("shortName") == 0)
            {
                LOG(WARNING)<< "AI missing shortName";
            }
            else
            {
                ai.name_ = ai.info_["shortName"];
                ais.push_back(ai);
            }
        }
        unitSync_->RemoveAllArchives();
    }

    return ais;
}

std::vector<std::string> Model::getModSideNames(std::string const & modName)
{
    std::vector<std::string> sideNames;

    int modIndex = unitSync_->GetPrimaryModIndex(modName.c_str());
    LOG(DEBUG) << "modIndex " << modIndex;

    if (modIndex >= 0)
    {
        const char* archiveName = unitSync_->GetPrimaryModArchive(modIndex);
        LOG(DEBUG) << "archiveName " << archiveName;
        unitSync_->AddAllArchives(archiveName);
        int sideCount = unitSync_->GetSideCount();
        LOG(DEBUG) << "sideCount " << sideCount;

        for (int i=0; i<sideCount; ++i)
        {
            char const* s = unitSync_->GetSideName(i);
            LOG_IF(FATAL, s == 0)<< "side name null, " << modName << ", " << i;
            sideNames.push_back(s);
        }
        unitSync_->RemoveAllArchives();
    }

    return sideNames;
}

void Model::addBot(Bot const & bot)
{
    std::ostringstream oss;
    if (zerok_)
    {
        Json::Value jv;
        jv["Name"] = bot.name();
        jv["AllyNumber"] = bot.battleStatus().allyTeam();
        // jv["TeamNumber"] = bot.battleStatus().team();
        jv["AiLib"] = bot.aiDll();
        jv["Owner"] = userName_;

        Json::FastWriter writer;
        oss << "UpdateBotStatus " << writer.write(jv);
    }
    else
    {
        oss << "ADDBOT "
            << bot.name() << " "
            << bot.battleStatus() << " "
            << bot.color() << " "
            << bot.aiDll();
    }
    controller_.send(oss.str());
}

void Model::botAllyTeam(std::string const& name, int allyTeam)
{
    try
    {
        Bot const& bot = getBot(name); // throws if bot not found
        UserBattleStatus ubs = bot.battleStatus();
        ubs.allyTeam(allyTeam);
        if (zerok_)
        {
            std::ostringstream oss;
            Json::Value jv;
            jv["Name"] = bot.name();
            jv["AllyNumber"] = ubs.allyTeam();
            // jv["TeamNumber"] = bot.battleStatus().team();
            jv["AiLib"] = bot.aiDll();
            jv["Owner"] = userName_;

            Json::FastWriter writer;
            oss << "UpdateBotStatus " << writer.write(jv);
            controller_.send(oss.str());
        }
        else
        {
            sendUpdateBot(name, ubs, bot.color());
        }
    }
    catch (std::invalid_argument const& e)
    {
        // silently ignore non-existing bot
    }
}

void Model::botSide(std::string const& name, int side)
{
    if (zerok_)
    {
        // zerok protocol do not have side for bots
        return;
    }

    try
    {
        Bot const& bot = getBot(name); // throws if bot not found
        UserBattleStatus ubs = bot.battleStatus();
        ubs.side(side);

        sendUpdateBot(name, ubs, bot.color());
    }
    catch (std::invalid_argument const& e)
    {
        // silently ignore non-existing bot
    }
}

void Model::sendUpdateBot(std::string const& name, UserBattleStatus const& ubs, int color)
{
    std::ostringstream oss;
    oss << "UPDATEBOT "
        << name << " "
        << ubs << " "
        << color;
    controller_.send(oss.str());
}

void Model::removeBot(std::string const & name)
{
    std::ostringstream oss;
    if (zerok_)
    {
        Json::Value jv;
        jv["Name"] = name;

        Json::FastWriter writer;
        oss << "RemoveBot " << writer.write(jv);
    }
    else
    {
        oss << "REMOVEBOT " << name;
    }
    controller_.send(oss.str());
}

void Model::ring(std::string const & userName)
{
    std::ostringstream oss;
    if (zerok_)
    {
        // TODO
    }
    else
    {
        oss << "RING " << userName;
    }
    controller_.send(oss.str());

}

void Model::testThread()
{
    for (int i=0; i<10; ++i)
    {
        controller_.startThread( []() -> int { LOG(ERROR)<< "HEJ"; ::sleep(2); return 0; } );
    }
}


unsigned int Model::downloadPr(std::string const& name, DownloadType type)
{
    if (prDownloaderId_ != 0)
    {
        LOG(WARNING)<< "pr-downloader already running " << prDownloadName_;
        serverMsgSignal_("download of " + name + " failed, pr-downloader currently downloading " + prDownloadName_, 1);
        return 0;
    }

    if (name.empty())
    {
        LOG(ERROR)<< "pr download name empty";
        return 0;
    }

    prDownloadType_ = type;
    prDownloadName_ = name;

    if (useExternalPrDownloader_)
    {
        auto const jobId = prDownloadExternal(name, type);
        if (jobId == 0) {
            serverMsgSignal_("download of " + name + " failed", 1);
        }
        else {
            serverMsgSignal_("downloading " + name + " ...", 0);
        }

        return jobId;
    }
    else
    {
        // TODO pr-d static disabled for now
        assert(false);
        //downloaderId_ = controller_.startThread( boost::bind(&Model::downloadInternal, this, name, type) );
        return 0;
    }
}

unsigned int Model::downloadCurl(std::string const& url, std::string const& file)
{
    if (curlId_ != 0) {
        LOG(WARNING)<< "curl already running " << curlDownloadUrl_;
        serverMsgSignal_("download of " + url + " failed, curl currently downloading " + curlDownloadUrl_, 1);
        return 0;
    }
    serverMsgSignal_("downloading " + url + " to " + file + " ...", 0);
    curlDownloadUrl_ = url;
    std::string url2 = url;
    boost::replace_all(url2, " ", "%20");
    std::ostringstream oss;
    oss << "curl -o " << file << " '" << url2 << "'";
    curlId_ = controller_.startThread( boost::bind(&Model::runProcess, this, oss.str(), true) );
    return curlId_;
}

/* TODO disable pr-d static for now
int Model::prDownloadInternal(std::string const& name, DownloadType type)
{
    category prdType;

    switch (type)
    {
    case DT_MAP:
        prdType = CAT_MAP;
        break;

    case DT_GAME:
        prdType = CAT_GAME;
        break;

    case DT_ENGINE:
        prdType = CAT_ENGINE;
        break;

    default:
        LOG(ERROR)<< "unknown DownloadType " << type;
        return 1;
    }

    int cnt = DownloadSearch(DL_ANY, prdType, name.c_str());
    LOG(INFO)<< "DownloadSearch returned " << cnt;

    if (cnt > 0)
    {
        bool res;
        if (!DownloadAdd(0)) // i hope pr-d puts the best dl alt first
        {
            LOG(ERROR)<< "DownloadAdd failed"<<" ("<<name<<","<<type<<")";
            return 1;
        }
        if (!DownloadStart())
        {
            LOG(ERROR)<< "DownloadStart failed"<<" ("<<name<<","<<type<<")";
            return 1;
        }
    }
    else
    {
        LOG(ERROR)<< "DownloadSearch failed ("<<name<<","<<type<<")";
        return 1;
    }

    LOG(INFO)<< "Download finished";
    return 0;
}
*/

unsigned int Model::prDownloadExternal(std::string const& name, DownloadType type)
{
    if (prDownloaderCmd_.empty())
    {
        LOG(ERROR)<< "pr-downloader path empty";
        serverMsgSignal_("pr-downloader path empty, check your Downloader settings", 1);
        return 0;
    }

    std::ostringstream oss;
    oss << prDownloaderCmd_ << " ";
    switch (type)
    {
    case DT_MAP:
        oss << "--download-map ";
        break;

    case DT_GAME:
        oss << "--download-game ";
        break;

    case DT_ENGINE:
        oss << "--download-engine ";
        break;

    default:
        LOG(ERROR)<< "unknown DownloadType:"<< type;
        return 0;
    }
    oss << "\"" << name << "\"";

    prDownloaderId_ = controller_.startThread( boost::bind(&Model::runProcess, this, oss.str(), true) );

    return prDownloaderId_;
}

void Model::checkPing()
{
    if (zerok_) {
        return;
    }

    uint64_t const timeNow = controller_.timeNow();

    if (timeNow > timePingSent_ + 30000)
    {
        if (waitingForPong_ > 2)
        {
            std::string const msg = "PONG not received in time, disconnecting";
            LOG(WARNING) << msg;
            serverMsgSignal_(msg, 1);
            disconnect();
        }
        else
        {
            controller_.send("PING");
            timePingSent_ = timeNow;
            ++waitingForPong_;
        }
    }
}

std::string Model::calcPasswordHash(std::string const& str)
{
    md5_state_t md5;
    md5_init(&md5);
    md5_append(&md5, (md5_byte_t const *)str.data(), str.size());

    md5_byte_t result[16];
    md5_finish(&md5, result);

    return base64_encode(result, 16);
}

void Model::sendMessage(std::string const& msg)
{
    if (zerok_)
    {
        LOG(WARNING)<< "message not sent: " << msg;
        return;
    }

    if (connected_)
    {
        controller_.send(msg);
    }
}

std::string Model::serverCommand(std::string const& str)
{
    LOG(INFO)<< "serverCommand: '" << str << "'";

    std::string result;

    if (str.empty()) return result;

    if (str[0] == '/' && str.size() > 1)
    {
        result = ServerCommand::process(str.substr(1));
    }
    else
    {
        sendMessage(str);
    }
    return result;
}

void Model::subscribeChannel(std::string const & channelName)
{
    if (zerok_)
    {
        std::string const msg = "!subscribe " + channelName;
        sayPrivate("Nightwatch", msg);
    }
    else
    {
        std::string const msg = "SUBSCRIBE chanName=" + channelName;
        sendMessage(msg);
    }
}

void Model::unsubscribeChannel(std::string const & channelName)
{
    if (zerok_)
    {
        std::string const msg = "!unsubscribe " + channelName;
        sayPrivate("Nightwatch", msg);
    }
    else
    {
        std::string msg = "UNSUBSCRIBE chanName=" + channelName;
        sendMessage(msg);
    }
}

void Model::listChannelSubscriptions()
{
    if (zerok_)
    {
        sayPrivate("Nightwatch", "!listsubscriptions");
    }
    else
    {
        sendMessage("LISTSUBSCRIPTIONS");
    }
}

void Model::handle_SiteToLobbyCommand(std::istream & is)
{
    Json::Value jv;
    is >> jv;

    if (jv.isMember("Command"))
    {
        std::string const command = jv["Command"].asString();
        if (!command.empty() && command[0] == '@')
        {
            auto posColon = command.find(':');
            if (posColon != std::string::npos)
            {
                std::string const action = command.substr(1, posColon-1);
                std::string const arg = command.substr(posColon+1);
                handleZerokAction(action, arg);
            }
        }
    }
}

void Model::handleZerokAction(std::string const& action, std::string const& arg)
{
    if (action == "start_replay")
    {
        if (!demoDownloadJobs_.empty())
        {
            std::string const msg = "download of zk demo is already in progress";
            LOG(WARNING)<< msg;
            serverMsgSignal_("unable to download and start zk demo, " + msg, 1);
            return;
        }

        serverMsgSignal_("handling " + action + ": " + arg, 0);

        // arg is "demoUrl,game,mapName,springVersion" e.g:
        // http://zero-k.info/replays/20151010_003505_DeltaSiegeX_100.sdf,Zero-K v1.3.9.0,DeltaSiegeX,100.0
        start_replay_Args_.clear();
        boost::split(start_replay_Args_, arg, boost::is_any_of(","));

        if (start_replay_Args_.size() != 4)
        {
            LOG(ERROR)<< action << " failed, expected 4 args";
            serverMsgSignal_("unable to download and start zk demo, unexpected data", 1);
            return;
        }

        // download map if needed
        std::string const mapName = start_replay_Args_[2];
        if (0 == getMapChecksum(mapName))
        {
            auto const jobId = downloadPr(mapName, DT_MAP);
            if (jobId)
            {
                demoDownloadJobs_.insert(jobId);
            }
        }

        // attempt download with curl, demo will be placed in CWD, normally ~/.config/spring/
        auto const url = start_replay_Args_[0];
        boost::filesystem::path const pathUrl(start_replay_Args_[0]);
        auto const curlJobId = downloadCurl(url, flobbyDemo_ + pathUrl.extension().string());
        if (curlJobId) {
            demoDownloadJobs_.insert(curlJobId);
        }
    }
}

void Model::startDemo(std::string const& springCmd, std::string const& demoPath)
{
    std::string const cmd = springCmd + " " + demoPath;
    controller_.startThread( boost::bind(&Model::runProcess, this, cmd, false) );
}

void Model::openBattle(int type, std::string const& title, std::string const& password)
{
    if (!connected_) {
        LOG(ERROR)<< __FUNCTION__<< " not connected";
        return;
    }

    if (!isZeroK()) {
        LOG(ERROR)<< __FUNCTION__<< " not zk";
        return;
    }

    std::string const titleStripped = boost::trim_copy(title);
    if (titleStripped.empty()) {
        LOG(ERROR)<< __FUNCTION__<< " title empty";
        return;
    }

    std::string const passwordStripped = boost::trim_copy(password);

    Json::Value jvBattleHeader;
    if (type >= 0) {
        jvBattleHeader["Mode"] = type;
    }
    jvBattleHeader["Title"] = titleStripped;
    jvBattleHeader["Engine"] = serverInfo_.springVersion_;
    jvBattleHeader["Game"] = serverInfo_.game_;
    if (!passwordStripped.empty()) {
        jvBattleHeader["Password"] = passwordStripped;
    }

    Json::Value jv;
    jv["Header"] = jvBattleHeader;

    Json::FastWriter writer;
    std::ostringstream oss;
    oss<< "OpenBattle "<< writer.write(jv);
    controller_.send(oss.str());
}

void Model::requestConnectSpring()
{
    if (!isZeroK()) {
        LOG(ERROR)<< __FUNCTION__<< " not zk";
        return;
    }

    if (joinedBattleId_ == -1) {
        LOG(ERROR)<< __FUNCTION__<< " not in a battle";
        return;
    }

    Battle const & battle = getBattle(joinedBattleId_);

    Json::Value jv;
    jv["BattleID"] = battle.id();

    Json::FastWriter writer;
    std::ostringstream oss;
    oss<< "RequestConnectSpring "<< writer.write(jv);
    controller_.send(oss.str());
    requestedConnectSpring_ = true;
}

void Model::handle_ConnectSpring(std::istream & is)
{
    if (-1 == joinedBattleId_) {
        LOG(ERROR)<< __FUNCTION__<< " no battle joined";
        return;
    }

    Battle& b = battle(joinedBattleId_);

    Json::Value jv;
    is >> jv;

    b.setIp(jv["Ip"].asString());
    b.setPort(jv["Port"].asString());

    if (jv.isMember("ScriptPassword")) {
        myScriptPassword_ = jv["ScriptPassword"].asString();
    }
    else {
        LOG(WARNING)<< "no ScriptPassword, falling back to username";
        myScriptPassword_ = userName_;
    }

    // only start spring here if it was requested, BattleRoom will handle auto start
    if (requestedConnectSpring_) {
        startSpring();
    }
    requestedConnectSpring_ = false;
}

void Model::handle_DefaultGameChanged(std::istream & is) // Game
{
    using namespace LobbyProtocol;

    Json::Value jv;
    is >> jv;

    serverInfo_.game_ = jv["Game"].asString();
    serverMsgSignal_("Default Game changed: " + serverInfo_.game_, 0);
}

