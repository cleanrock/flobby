// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "model/Model.h"
#include "gui/MyImage.h"
#include "gui/TextFunctions.h"
#include "log/Log.h"
#include "FlobbyDirs.h"
#include "model/Nightwatch.h"
#include "model/LobbyProtocol.h"

#include <boost/lexical_cast.hpp>
#define BOOST_TEST_DYN_LINK // this will define BOOST_TEST_ALTERNATIVE_INIT_API in boost/test/detail/config.hpp
#define BOOST_TEST_ALTERNATIVE_INIT_API // here for clarity
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <functional>
#include <thread>
#include <stdexcept>
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

static
bool init_unit_test()
{
    std::string logFileName = "unittest.log";
    // reset log file
    {
        std::ofstream ofs(logFileName);
    }
    Log::logFile(logFileName);
    Log::minSeverity(DEBUG);

    return true;
}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}


BOOST_AUTO_TEST_CASE(testUserStatus)
{
    // test default ctor
    {
        UserStatus us;
        BOOST_CHECK(!us.inGame());
        BOOST_CHECK(!us.away());
        BOOST_CHECK(us.rank() == 0);
        BOOST_CHECK(!us.moderator());
        BOOST_CHECK(!us.bot());
    }

    // test when all bits set
    {
        UserStatus us("127");
        BOOST_CHECK(us.inGame());
        BOOST_CHECK(us.away());
        BOOST_CHECK(us.rank() == 7);
        BOOST_CHECK(us.moderator());
        BOOST_CHECK(us.bot());

    }

    // test rank is 6
    {
        UserStatus us("24");
        BOOST_CHECK(!us.inGame());
        BOOST_CHECK(!us.away());
        BOOST_CHECK(us.rank() == 6);
        BOOST_CHECK(!us.moderator());
        BOOST_CHECK(!us.bot());
    }

    // test set inGame
    {
        UserStatus us;
        us.inGame(true);
        BOOST_CHECK(us.inGame());
        BOOST_CHECK(!us.away());
        BOOST_CHECK(us.rank() == 0);
        BOOST_CHECK(!us.moderator());
        BOOST_CHECK(!us.bot());
    }

    // test set away
    {
        UserStatus us;
        us.away(true);
        BOOST_CHECK(!us.inGame());
        BOOST_CHECK(us.away());
        BOOST_CHECK(us.rank() == 0);
        BOOST_CHECK(!us.moderator());
        BOOST_CHECK(!us.bot());
    }

    // test exception is thrown on bad input to ctor
    {
        BOOST_CHECK_THROW(UserStatus us(""), boost::bad_lexical_cast);
        BOOST_CHECK_THROW(UserStatus us("ABC"), boost::bad_lexical_cast);
    }


    // test operator== and operator!=
    {
        UserStatus us1;
        UserStatus us2("24");
        UserStatus us3("24");
        UserStatus us4("127");

        BOOST_CHECK( us2 == us3 );

        BOOST_CHECK( us1 != us2 );
        BOOST_CHECK( us1 != us3 );
        BOOST_CHECK( us3 != us4 );
    }

}

BOOST_AUTO_TEST_CASE(testUserBattleStatus)
{
    // test default ctor
    {
        UserBattleStatus ubs;
        BOOST_CHECK(!ubs.ready());
        BOOST_CHECK(ubs.team() == 0);
        BOOST_CHECK(ubs.allyTeam() == 0);
        BOOST_CHECK(ubs.spectator());
        BOOST_CHECK(ubs.handicap() == 0);
        BOOST_CHECK(ubs.sync() == 0);
        BOOST_CHECK(ubs.side() == 0);
    }

    // test when all bits set
    {
        UserBattleStatus ubs("2147483647");
        BOOST_CHECK(ubs.ready());
        BOOST_CHECK(ubs.team() == 15);
        BOOST_CHECK(ubs.allyTeam() == 15);
        BOOST_CHECK(!ubs.spectator());
        BOOST_CHECK(ubs.handicap() == 127);
        BOOST_CHECK(ubs.sync() == 3);
        BOOST_CHECK(ubs.side() == 15);
    }

    // test set methods
    {
        UserBattleStatus ubs;
        ubs.ready(true);
        ubs.spectator(false);
        ubs.allyTeam(7);
        ubs.team(3);
        ubs.sync(1);
        BOOST_CHECK(ubs.ready());
        BOOST_CHECK(ubs.team() == 3);
        BOOST_CHECK(ubs.allyTeam() == 7);
        BOOST_CHECK(!ubs.spectator());
        BOOST_CHECK(ubs.handicap() == 0);
        BOOST_CHECK(ubs.sync() == 1);
        BOOST_CHECK(ubs.side() == 0);
    }

    // test exception is thrown on bad input to ctor
    {
        BOOST_CHECK_THROW(UserBattleStatus ubs(""), boost::bad_lexical_cast);
        BOOST_CHECK_THROW(UserBattleStatus ubs("ABC"), boost::bad_lexical_cast);
    }


    // test operator== and operator!=
    {
        UserStatus ubs1;
        UserStatus ubs2("24");
        UserStatus ubs3("24");
        UserStatus ubs4("127");

        BOOST_CHECK( ubs2 == ubs3 );

        BOOST_CHECK( ubs1 != ubs2 );
        BOOST_CHECK( ubs1 != ubs3 );
        BOOST_CHECK( ubs3 != ubs4 );
    }

}

BOOST_AUTO_TEST_CASE(testUser)
{
    // simple positive
    {
        std::string const name = "cleanrock";
        std::string const country = "SE";
        std::string const cpu = "3000";
        std::stringstream ss;
        ss << name << " "
           << country << " "
           << cpu;
        User u(ss);

        BOOST_CHECK_EQUAL(u.name(), name);
        BOOST_CHECK_EQUAL(u.country(), country);
        BOOST_CHECK_EQUAL(u.cpu(), cpu);

        // print for ocular inspection
        std::cout << u << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("username CC ");

        BOOST_CHECK_THROW(User u(ss), std::invalid_argument);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        BOOST_CHECK_THROW(User u(ss), std::invalid_argument);
    }

    // test operators
    {
        std::stringstream ss1("name1 SE 0");
        User u1(ss1);

        std::stringstream ss2("name1 SE 0");
        User u2(ss2);

        std::stringstream ss3("name2 SE 0");
        User u3(ss3);

        BOOST_CHECK(u1 == u2);
        BOOST_CHECK(u1 != u3);
    }

}

BOOST_AUTO_TEST_CASE(testBattle)
{
    // simple positive
    {
        std::string const opened =
                "8235 0 0 Founder " // id, replay, nat, founder
                "94.23.170.70 8463 32 " // ip, port, maxPlayers
                "0 0 -112462944 " // passw, rank, mapHash
                "engineName\t"
                "engineVersion\t"
                "Map name\t"
                "Battle title\t"
                "Mod name";

        std::stringstream ssOpened(opened);

        Battle b(ssOpened);

        BOOST_CHECK(b.id() == 8235);
        BOOST_CHECK(b.replay() == false);
        BOOST_CHECK(b.natType() == 0);
        BOOST_CHECK(b.founder() == "Founder");
        BOOST_CHECK(b.ip() == "94.23.170.70");
        BOOST_CHECK(b.port() == "8463");
        BOOST_CHECK(b.maxPlayers() == 32);
        BOOST_CHECK(b.passworded() == false);
        BOOST_CHECK(b.rank() == 0);
        BOOST_CHECK(b.mapHash() == -112462944);
        BOOST_CHECK(b.mapHash() == static_cast<unsigned int>(-112462944));
        BOOST_CHECK(b.engineName() == "engineName");
        BOOST_CHECK(b.engineVersion() == "engineVersion");
        BOOST_CHECK(b.engineBranch() == "");
        BOOST_CHECK(b.engineVersionLong() == "engineVersion");
        BOOST_CHECK(b.mapName() == "Map name");
        BOOST_CHECK(b.title() == "Battle title");
        BOOST_CHECK(b.modName() == "Mod name");
        BOOST_CHECK(b.spectators() == 0);
        BOOST_CHECK(b.locked() == false);
        BOOST_CHECK(b.modHash() == 0);


        std::string const updated =
                "3 1 " // specs, locked
                "-1517218254 " // mapHash
                "New map name";

        std::stringstream ssUpdated(updated);

        b.updateBattleInfo(ssUpdated);
        b.modHash(9786);

        BOOST_CHECK(b.spectators() == 3);
        BOOST_CHECK(b.locked() == true);
        BOOST_CHECK(b.mapHash() == -1517218254);
        BOOST_CHECK(b.mapName() == "New map name");
        BOOST_CHECK(b.modHash() == 9786);

        // print for ocular inspection
        std::cout << b << std::endl;
    }

    // engine with branch name
    {
        std::string const opened =
                "8235 0 0 Founder " // id, replay, nat, founder
                "94.23.170.70 8463 32 " // ip, port, maxPlayers
                "0 0 -112462944 " // passw, rank, mapHash
                "engineName\t"
                "engineVersion  develop\t" // extra space
                "Map name\t"
                "Battle title\t"
                "Mod name";

        std::stringstream ssOpened(opened);

        Battle b(ssOpened);

        BOOST_CHECK(b.id() == 8235);
        BOOST_CHECK(b.replay() == false);
        BOOST_CHECK(b.natType() == 0);
        BOOST_CHECK(b.founder() == "Founder");
        BOOST_CHECK(b.ip() == "94.23.170.70");
        BOOST_CHECK(b.port() == "8463");
        BOOST_CHECK(b.maxPlayers() == 32);
        BOOST_CHECK(b.passworded() == false);
        BOOST_CHECK(b.rank() == 0);
        BOOST_CHECK(b.mapHash() == -112462944);
        BOOST_CHECK(b.mapHash() == static_cast<unsigned int>(-112462944));
        BOOST_CHECK(b.engineName() == "engineName");
        BOOST_CHECK(b.engineVersion() == "engineVersion");
        BOOST_CHECK(b.engineBranch() == "develop");
        BOOST_CHECK(b.engineVersionLong() == "engineVersion (develop)");
        BOOST_CHECK(b.mapName() == "Map name");
        BOOST_CHECK(b.title() == "Battle title");
        BOOST_CHECK(b.modName() == "Mod name");
        BOOST_CHECK(b.spectators() == 0);
        BOOST_CHECK(b.locked() == false);
        BOOST_CHECK(b.modHash() == 0);


        std::string const updated =
                "3 1 " // specs, locked
                "-1517218254 " // mapHash
                "New map name";

        std::stringstream ssUpdated(updated);

        b.updateBattleInfo(ssUpdated);
        b.modHash(9786);

        BOOST_CHECK(b.spectators() == 3);
        BOOST_CHECK(b.locked() == true);
        BOOST_CHECK(b.mapHash() == -1517218254);
        BOOST_CHECK(b.mapName() == "New map name");
        BOOST_CHECK(b.modHash() == 9786);

        // print for ocular inspection
        std::cout << b << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("id not int");

        BOOST_CHECK_THROW(Battle b(ss), boost::bad_lexical_cast);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        BOOST_CHECK_THROW(Battle b(ss), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(testScript)
{
    Script script;

    {
        auto keyValuePair = script.getKeyValuePair("GAME/Players/[CLAN]PlayerName/skill=(35)");
        BOOST_CHECK_EQUAL( keyValuePair.first, std::string("Players/[CLAN]PlayerName/skill") );
        BOOST_CHECK_EQUAL( keyValuePair.second, std::string("(35)") );
    }

    {
        auto keyValuePair = script.getKeyValuePair("Players/[CLAN]PlayerName/skill=(35)");
        BOOST_CHECK_EQUAL( keyValuePair.first, std::string("") );
        BOOST_CHECK_EQUAL( keyValuePair.second, std::string("") );
    }

    {
        auto keyValuePair = script.getKeyValuePair("GAME/Players/[CLAN]PlayerName/crap");
        BOOST_CHECK_EQUAL( keyValuePair.first, std::string("") );
        BOOST_CHECK_EQUAL( keyValuePair.second, std::string("") );
    }

    {
        auto key = script.getKey("GAME/removed");
        BOOST_CHECK_EQUAL( std::string("removed"), key );
    }

    {
        auto key = script.getKey("GAME/removed=bla");
        BOOST_CHECK_EQUAL( std::string("removed"), key );
    }

    {
        auto key = script.getKey("GAME_missing");
        BOOST_CHECK_EQUAL( key, std::string("") );
    }

    script.add("GAME/Sub1/Sub2/key1=val1");
    script.add("GAME/key1=val1");
    script.add("GAME/Sub1/key1=val1");
    script.add("GAME/Sub1/Sub2/key2=val2");

    auto keyValuePair = script.add("GAME/Sub2/removed=removedValue");
    BOOST_CHECK_EQUAL( keyValuePair.first, std::string("removed") );
    BOOST_CHECK_EQUAL( keyValuePair.second, std::string("removedValue") );

    auto key =  script.remove("GAME/Sub2/removed");
    BOOST_CHECK_EQUAL( key, std::string("removed") );

    script.write("./script.txt");
}

BOOST_AUTO_TEST_CASE(testBot)
{
    // simple positive
    {
        std::string const name = "botName";
        std::string const owner = "botOwner";
        UserBattleStatus const battleStatus;
        int const color = 255;
        std::string const aiDll = "the ai dll name";
        std::stringstream ss;
        ss << name << " "
           << owner << " "
           << battleStatus << " "
           << color << " "
           << aiDll;
        Bot b(ss);

        BOOST_CHECK_EQUAL(b.name(), name);
        BOOST_CHECK_EQUAL(b.owner(), owner);
        BOOST_CHECK_EQUAL(b.battleStatus(), battleStatus);
        BOOST_CHECK_EQUAL(b.color(), color);
        BOOST_CHECK_EQUAL(b.aiDll(), aiDll);

        // print for ocular inspection
        std::cout << b << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("123 CC ");

        BOOST_CHECK_THROW(Bot b(ss), std::invalid_argument);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        BOOST_CHECK_THROW(Bot b(ss), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(testMyImage)
{
    // 3x2x1
    {
        std::string const fileName("MyImageTestFile");
        std::string content("FLOBBY_IMAGE 3 2 1\n"
                "111"
                "222");

        {
            std::ofstream ofs(fileName);
            ofs << content;
        }

        MyImage image(fileName);

        BOOST_CHECK_EQUAL(3, image.w());
        BOOST_CHECK_EQUAL(2, image.h());
        BOOST_CHECK_EQUAL(1, image.d());
        BOOST_CHECK_EQUAL(static_cast<uchar>('1'), image.array[0]);
    }

    // 2x3x3
    {
        std::string const fileName("MyImageTestFile");
        std::string content("FLOBBY_IMAGE 2 3 3\n"
                "111111"
                "222222"
                "333333");

        {
            std::ofstream ofs(fileName);
            ofs << content;
        }

        MyImage image(fileName);

        BOOST_CHECK_EQUAL(2, image.w());
        BOOST_CHECK_EQUAL(3, image.h());
        BOOST_CHECK_EQUAL(3, image.d());
        BOOST_CHECK_EQUAL(static_cast<uchar>('1'), image.array[0]);
    }

    // test exception is thrown if file not found
    {
        BOOST_CHECK_THROW(MyImage image("non_existing_file"), std::invalid_argument);
    }
}

static
void logThread(int id)
{
    for (int i=0; i<100; ++i)
    {
        LOG(DEBUG) << "test_thread_" << id; // change to INFO to see how it looks on std::cout
    }
}


BOOST_AUTO_TEST_CASE(testLog)
{
    // 10 threads write 100 lines each, output is analyzed manually

    int const cnt = 10;
    std::vector< std::unique_ptr<std::thread>> threads(cnt);

    for (int i=0; i<cnt; ++i)
    {
        threads[i].reset( new std::thread(std::bind(logThread, i)) );
    }

    for (int i=0; i<cnt; ++i)
    {
        threads[i]->join();
    }
}

BOOST_AUTO_TEST_CASE(testTextFunctions)
{
    typedef std::vector<std::string> StringVector;

    StringVector strings = { "Habc", "abc", "ABC", "Hagf", "aGF" };

    std::pair<MatchResult, std::string> result;

    result = findMatch(strings, "");
    BOOST_CHECK_EQUAL(MR_NO_MATCH, result.first);

    result = findMatch(strings, "GHabc");
    BOOST_CHECK_EQUAL(MR_NO_MATCH, result.first);

    result = findMatch(strings, "ab");
    BOOST_CHECK_EQUAL(MR_BEGINS_I, result.first);
    BOOST_CHECK("abc" == result.second);

//    result = findMatch(strings, "ab");
//    BOOST_CHECK_EQUAL(MR_BEGINS_C, result.first);
//    BOOST_CHECK("abc" == result.second);

//    result = findMatch(strings, "A");
//    BOOST_CHECK_EQUAL(MR_BEGINS_C, result.first);
//    BOOST_CHECK("ABC" == result.second);

    result = findMatch(strings, "ag");
    BOOST_CHECK_EQUAL(MR_BEGINS_I, result.first);
    BOOST_CHECK("aGF" == result.second);

    result = findMatch(strings, "BC");
    BOOST_CHECK_EQUAL(MR_CONTAINS_I, result.first);
    BOOST_CHECK("Habc" == result.second);

    result = findMatch(strings, "gf");
    BOOST_CHECK_EQUAL(MR_CONTAINS_I, result.first);
    BOOST_CHECK("Hagf" == result.second);

}

BOOST_AUTO_TEST_CASE(testFlobbyDirs)
{
    {
        std::string res = wordExpand("~");
        std::cout << res << std::endl;
        BOOST_CHECK(res.size() > 1);
    }

    {
        std::string res = wordExpand("");
        BOOST_CHECK(res.empty());
    }

    {
        std::string const str = "asdasd";
        std::string res = wordExpand(str);
        BOOST_CHECK(res == str);
    }
}

BOOST_AUTO_TEST_CASE(testNightwatch)
{
    // ok simple pm channel message
    {
        NightwatchPm const res = checkNightwatchPm("!pm|chan1|user1|07/01/2015 03:32:14|text1");
        BOOST_CHECK(res.valid_ == true);
        BOOST_CHECK(res.channel_ == "chan1");
        BOOST_CHECK(res.user_ == "user1");
        BOOST_CHECK(res.time_ == "07/01/2015 03:32:14");
        BOOST_CHECK(res.text_ == "text1");
    }

    // ok private chat message with || in text
    {
        NightwatchPm const res = checkNightwatchPm("!pm||user2|07/01/2015 03:32:15|text2||");
        BOOST_CHECK(res.valid_ == true);
        BOOST_CHECK(res.channel_ == "");
        BOOST_CHECK(res.user_ == "user2");
        BOOST_CHECK(res.time_ == "07/01/2015 03:32:15");
        BOOST_CHECK(res.text_ == "text2||");
    }

    // bad pm
    {
        NightwatchPm const res = checkNightwatchPm("!pm|crap");
        BOOST_CHECK(res.valid_ == false);
    }

    // bad pm missing initial !
    {
        NightwatchPm const res = checkNightwatchPm("pm||User2|07/01/2015 03:32:14|crap||");
        BOOST_CHECK(res.valid_ == false);
    }

}

BOOST_AUTO_TEST_CASE(testLobbyProtocol)
{
    using namespace LobbyProtocol;

    // extractWord
    {
        std::istringstream iss("word1 word2  word3");
        std::string ex;

        extractWord(iss, ex);
        BOOST_CHECK(ex == "word1");

        extractWord(iss, ex);
        BOOST_CHECK(ex == "word2");

        extractWord(iss, ex);
        BOOST_CHECK(ex == "word3");
    }

    // extractSentence
    {
        std::istringstream iss("sentence 1\tsentence 2");
        std::string ex;

        extractSentence(iss, ex);
        BOOST_CHECK(ex == "sentence 1");

        extractSentence(iss, ex);
        BOOST_CHECK(ex == "sentence 2");
    }

    // extractToNewline
    {
        std::istringstream iss("a b\tc d\nremaining");
        std::string ex;

        extractToNewline(iss, ex);
        BOOST_CHECK(ex == "a b\tc d");
    }

    // skipSpaces
    {
        std::istringstream iss(" a b");

        skipSpaces(iss);
        std::string content(std::istreambuf_iterator<char>(iss), {});
        BOOST_CHECK(content == "a b");
    }
}
