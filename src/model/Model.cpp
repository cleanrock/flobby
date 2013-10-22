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

#include "md5/md5.h"
#include "md5/base64.h"

#include "log/Log.h"
#include "FlobbyDirs.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <sstream>

#define ADD_MSG_HANDLER(MSG) \
    messageHandlers_[#MSG] = std::bind(&Model::handle_##MSG, this, std::placeholders::_1);
#define ADD_MSG_HANDLER2(MSG, METHOD) \
    messageHandlers_[#MSG] = std::bind(&Model::handle_##METHOD, this, std::placeholders::_1);

Model::Model(IController & controller):
    controller_(controller),
    connected_(false),
    checkFirstMsg_(false),
    loggedIn_(false),
    timePingSent_(0),
    waitingForPong_(false),
    joinedBattleId_(-1),
    me_(0),
    springId_(0),
    downloaderId_(0)
{
    controller_.setIControllerEvent(*this);
    ServerCommand::init(*this);

    // setup message handlers
    ADD_MSG_HANDLER(ACCEPTED)
    ADD_MSG_HANDLER(DENIED)
    ADD_MSG_HANDLER(ADDUSER)
    ADD_MSG_HANDLER(REMOVEUSER)
    ADD_MSG_HANDLER(BATTLEOPENED)
    ADD_MSG_HANDLER(BATTLEOPENEDEX)
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

}

Model::~Model()
{
}

void Model::setUnitSyncPath(std::string const & path)
{
    unitSyncPath_ = path;
    unitSync_.reset( new UnitSync(unitSyncPath_) );

    unitSync_->Init(true, 1);
    unitSync_->GetPrimaryModCount();
    initMapIndex();

    writeableDataDir_ = unitSync_->GetWritableDataDirectory();
    assert(!writeableDataDir_.empty());

    LOG(DEBUG) << "writeableDataDir_:" << writeableDataDir_;
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
        checkFirstMsg_ = true; // check that first message is TASServer
        timePingSent_ = controller_.timeNow();
    }
    else
    {
        // reset model on disconnect
        loggedIn_ = false;
        waitingForPong_ = false;
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
    oss << "LOGIN " << userName_ << " " << password_ << " 0 * flobby 0.2\t" << userId << "\teb";
    controller_.send(oss.str());
}

void Model::message(std::string const & msg)
{
    LOG(DEBUG) << "message: " << msg;

    processServerMsg(msg);
}

void Model::processDone(std::pair<unsigned int, int> idRetPair)
{
    LOG(DEBUG) << "processDone:" << idRetPair.first << ", " << idRetPair.second;
    if (idRetPair.first == springId_)
    {
        springExitSignal_();
        springId_ = 0;
        meInGame(false);
    }
    else if (idRetPair.first == downloaderId_)
    {
        downloadDoneSignal_(downloadName_, idRetPair.second == 0 ? true : false);
        downloaderId_ = 0;
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
        oss << "REGISTER " << userName << " " << passwordHash;
        if (!email.empty())
        {
            oss << " " << email;
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
    oss << "CONFIRMAGREEMENT";
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

User & Model::me()
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
    oss << "MYSTATUS " << us;
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
        oss << "MYSTATUS " << us;
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
            if (boost::iequals(ex, "TASSERVER"))
            {
                handle_TASSERVER(iss);
                checkFirstMsg_ = false;
            }
            else
            {
                LOG(WARNING) << "Disconnecting, first message is not TASSERVER:" << msg;
                serverMsgSignal_("Bad first msg from server: " + msg.substr(0, 32));
                disconnect();
            }
        }
        else
        {
            auto res = messageHandlers_.find(ex);
            if (res != messageHandlers_.end())
            {
                res->second(iss);
            }
            else
            {
                LOG(WARNING) << "Unhandled message:" << msg;
            }
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
        oss << "JOINBATTLE " << battleId << " " << password << " " << "scriptPassword" << std::rand();
        controller_.send(oss.str());
    }
}

void Model::leaveBattle()
{
    controller_.send("LEAVEBATTLE");
}

void Model::sayBattle(std::string const & msg)
{
    if (!msg.empty())
    {
        std::ostringstream oss;
        oss << "SAYBATTLE " << msg;
        controller_.send(oss.str());
    }
}

void Model::sayPrivate(std::string const & userName, std::string const & msg)
{
    if (userName.empty())
    {
        LOG(WARNING)<< "userName.empty()";
        return;
    }
    if (msg.empty())
    {
        // ignore empty messages
        return;
    }
    std::ostringstream oss;
    oss << "SAYPRIVATE " << userName << " " << msg;
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
        oss << "GAME/MyPasswd=" << myScriptPassword_;
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
        oss << springPath_ << " " << scriptPath;

        springId_ = controller_.startProcess(oss.str());
    }
    meInGame(true);
}

void Model::disconnect()
{
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
    unitSync_->Init(true, 1);
    unitSync_->GetPrimaryModCount();
    initMapIndex();

    // check if sync changed
    if (joinedBattleId_ != -1)
    {
        int const sync = calcSync(getBattle(joinedBattleId_));
        LOG(DEBUG) << "refresh sync:" << sync;
        User & u = me();
        if (sync != u.battleStatus().sync())
        {
            u.battleStatus_.sync(sync);
            sendMyBattleStatus();
        }
    }

}

void Model::sendMyInitialBattleStatus(Battle const & battle)
{
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

    sendMyBattleStatus();
}

bool Model::gameExist(std::string const & gameName)
{
    return (unitSync_->GetPrimaryModChecksumFromName( gameName.c_str()) != 0 );
}

int Model::calcSync(Battle const & battle)
{
    unsigned int const modChecksum = unitSync_->GetPrimaryModChecksumFromName( battle.modName().c_str() );
    unsigned int const mapChecksum = unitSync_->GetMapChecksumFromName( battle.mapName().c_str() );

    if (modChecksum == 0 || mapChecksum == 0)
    {
        // not synced, we dont have either mod or map
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
    oss << "MYBATTLESTATUS " << me().battleStatus() << " 255"; // TODO color
    controller_.send(oss.str());

}

void Model::handle_TASSERVER(std::istream & is) // protocolVersion springVersion udpPort serverMode (e.g 0.35 88 8201 0)
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

    serverInfoSignal_(si);
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
    LOG(WARNING)<< "BATTLEOPENED not supported";
}

void Model::handle_BATTLEOPENEDEX(std::istream & is)
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

void Model::handle_UPDATEBATTLEINFO(std::istream & is) // battleId spectatorCount locked mapHash {mapName}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractWord(is, ex);
    Battle & b = getBattle(ex);
    b.updateBattleInfo(is);

    // update self sync
    if (b.id() == joinedBattleId_)
    {
        int const sync = calcSync(b);
        User & u = me();
        if (sync != u.battleStatus().sync())
        {
            u.battleStatus_.sync(sync);
            sendMyBattleStatus();
        }
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
            // TODO fully OK to ignore optional scriptPassword ?
        }

    }
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
    try
    {
        while (true)
        {
            extractSentence(is, ex); // will break out of loop when it throws
            auto keyValuePair = script_.getKeyValuePair(ex);
            if (!keyValuePair.first.empty())
            {
                setScriptTagSignal_(keyValuePair.first, keyValuePair.second);
            }
        }
    }
    catch (...)
    {
        // no more data
    }
}

void Model::handle_REMOVESCRIPTTAGS(std::istream & is) // key [key ...]
{
    using namespace LobbyProtocol;
    std::string ex;
    try
    {
        while (true)
        {
            extractWord(is, ex); // will break out of loop when it throws
            std::string const key = script_.getKey(ex);
            if (!key.empty())
            {
                removeScriptTagSignal_(key);
            }
        }
    }
    catch (...)
    {
        // no more data
    }
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
    extractSentence(is, ex);
    battleChatMsgSignal_(userName, ex);
}

void Model::handle_SAYPRIVATE(std::istream & is) // userName {message}
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    std::string msg;
    extractSentence(is, msg);
    sayPrivateSignal_(userName, msg);
}

void Model::handle_SAIDPRIVATE(std::istream & is) // userName {message}
{
    using namespace LobbyProtocol;
    std::string userName;
    extractWord(is, userName);
    std::string msg;
    extractSentence(is, msg);
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
    extractSentence(is, ex);
    serverMsgSignal_("MOTD: " + ex);
}

void Model::handle_SERVERMSG(std::istream & is) // {message}
{
    using namespace LobbyProtocol;
    std::string ex;
    extractSentence(is, ex);
    serverMsgSignal_(ex);
}

void Model::handle_SERVERMSGBOX(std::istream & is) // {message} [{url}]
{
    using namespace LobbyProtocol;
    std::string ex;
    extractSentence(is, ex);
    try
    {
        std::string url;
        extractSentence(is, url);
        serverMsgSignal_(ex + " " + url);
    }
    catch (std::invalid_argument const & e)
    {
        serverMsgSignal_(ex);
    }
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

void Model::handle_CLIENTS(std::istream & is) // channelName {clients}
{
    using namespace LobbyProtocol;
    std::string channelName;
    extractWord(is, channelName);

    std::vector<std::string> clients;
    try
    {
        std::string userName;
        while (1) // exits with exception when user list empty
        {
            extractWord(is, userName);
            clients.push_back(userName);
        }
    }
    catch (std::invalid_argument const & e)
    {
        // all user names extracted, do nothing
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
    controller_.send("CHANNELS");
}

void Model::joinChannel(std::string const & channelName)
{
    if (!channelName.empty() && connected_)
    {
        std::ostringstream oss;
        oss << "JOIN " << channelName;
        controller_.send(oss.str());
    }
}

void Model::sayChannel(std::string const & channelName, std::string const & message)
{
    if (!channelName.empty() && !message.empty())
    {
        std::ostringstream oss;
        oss << "SAY " << channelName << " " << message;
        controller_.send(oss.str());
    }
}

void Model::leaveChannel(std::string const & channelName)
{
    if (!channelName.empty() && connected_)
    {
        std::ostringstream oss;
        oss << "LEAVE " << channelName;
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

void Model::handle_LEFT(std::istream & is) // channelName userName [{reason}]
{
    using namespace LobbyProtocol;

    std::string channelName;
    extractWord(is, channelName);

    std::string userName;
    extractWord(is, userName);

    std::string reason;
    try
    {
        extractSentence(is, reason);
    }
    catch (std::invalid_argument const & e)
    {
        // no reason, reason will be empty
    }

    userLeftChannelSignal_(channelName, userName, reason);
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
    extractSentence(is, message);

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
    extractSentence(is, msg);
    saidChannelSignal_(channelName, userName, msg);
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

void Model::handle_AGREEMENT(std::istream & is) // {text}
{
    using namespace LobbyProtocol;

    std::string text;
    extractSentence(is, text);
    agreementStream_ << text << "\n";
}

void Model::handle_AGREEMENTEND(std::istream & is)
{
    // try to remove all RTF stuff since we can't display it with FLTK

    std::string a = agreementStream_.str();
    agreementStream_.str("");

    std::string::size_type pos;

    // remove all up to and first '{'
    if ( (pos = a.find_first_of('{')) != std::string::npos)
    {
        a.replace(0, pos+1, "");
    }
    // remove all after last '}'
    if ( (pos = a.find_last_of('}')) != std::string::npos)
    {
        a.replace(pos, std::string::npos, "");
    }

    // remove everything contained in {}
    pos = 0;
    while ( (pos = a.find_first_of('{', pos)) != std::string::npos)
    {
        int level = 1;
        std::string::size_type posNext = pos;
        while ( (posNext = a.find_first_of("{}", posNext+1)) != std::string::npos)
        {
            if (a[posNext] == '{')
            {
                ++level;
            }
            else
            {
                --level;
                if (level == 0)
                {
                    a.replace(pos, posNext-pos+1, "");
                    break;
                }
            }
        }
        if (level != 0)
        {
            // should not happen, we keep it unchanged
            LOG(WARNING)<< "end of '{' not found:" << a.substr(pos);
            break;
        }
    }

    // remove all "\... " and "\...\n"
    pos = 0;
    while ((pos = a.find_first_of('\\', pos)) != std::string::npos)
    {
            std::string::size_type posEnd;
            if ( (posEnd = a.find_first_of(" \n", pos+1)) != std::string::npos)
            {
                if (a[posEnd] == ' ')
                {
                    a.replace(pos, posEnd-pos+1, "");
                }
                else
                {
                    a.replace(pos, posEnd-pos, "");
                }
            }
            else
            {
                // should not happen, we keep it unchanged
                LOG(WARNING)<< "end of backslash content not found:" << a.substr(pos);
                break;
            }
    }

    agreementSignal_(a);
}

void Model::handle_PONG(std::istream & is)
{
    using namespace LobbyProtocol;

    waitingForPong_ = false;
}

std::vector<AI> Model::getModAIs(std::string const & modName)
{
    std::vector<AI> ais;

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
    oss << "ADDBOT "
        << bot.name() << " "
        << bot.battleStatus() << " "
        << bot.color() << " "
        << bot.aiDll();
    controller_.send(oss.str());
}

void Model::botAllyTeam(std::string const& name, int allyTeam)
{
    try
    {
        Bot const& bot = getBot(name); // throws if bot not found
        UserBattleStatus ubs = bot.battleStatus();
        ubs.allyTeam(allyTeam);

        sendUpdateBot(name, ubs, bot.color());
    }
    catch (std::invalid_argument const& e)
    {
        // silently ignore non-existing bot
    }
}

void Model::botSide(std::string const& name, int side)
{
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
    oss << "REMOVEBOT " << name;
    controller_.send(oss.str());

}

void Model::ring(std::string const & userName)
{
    std::ostringstream oss;
    oss << "RING " << userName;
    controller_.send(oss.str());

}

bool Model::download(std::string const & name, DownloadType type)
{
    if (name.empty())
    {
        LOG(ERROR)<< "download name empty";
        return false;
    }

    // only start pr-downloader if its not running
    if (downloaderId_ != 0)
    {
        LOG(WARNING)<< "downloader already running (" << downloadName_ << ")";
        return false;
    }

    downloadName_ = name;
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

    default:
        LOG(ERROR)<< "unknown DownloadType:"<< type;
        return false;
    }
    oss << "\"" << name << "\"";
    downloaderId_ = controller_.startProcess(oss.str(), true);
    return true;
}

void Model::checkPing()
{
    uint64_t const timeNow = controller_.timeNow();

    if (timeNow > timePingSent_ + 30000)
    {
        if (waitingForPong_)
        {
            std::string const msg = "PONG not received in time, disconnecting";
            LOG(WARNING) << msg;
            serverMsgSignal_(msg);
            disconnect();
        }
        else
        {
            controller_.send("PING");
            timePingSent_ = timeNow;
            waitingForPong_ = true;
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
