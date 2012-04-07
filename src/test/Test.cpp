#include "Test.h"
#include "model/Model.h"
#include "gui/MyImage.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <sstream>
#include <string>


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test );

void Test::setUp()
{
}

void Test::tearDown()
{
}

void Test::testUserStatus()
{
    // test default ctor
    {
        UserStatus us;
        CPPUNIT_ASSERT(!us.inGame());
        CPPUNIT_ASSERT(!us.away());
        CPPUNIT_ASSERT(us.rank() == 0);
        CPPUNIT_ASSERT(!us.moderator());
        CPPUNIT_ASSERT(!us.bot());
    }

    // test when all bits set
    {
        UserStatus us("127");
        CPPUNIT_ASSERT(us.inGame());
        CPPUNIT_ASSERT(us.away());
        CPPUNIT_ASSERT(us.rank() == 7);
        CPPUNIT_ASSERT(us.moderator());
        CPPUNIT_ASSERT(us.bot());

    }

    // test rank is 6
    {
        UserStatus us("24");
        CPPUNIT_ASSERT(!us.inGame());
        CPPUNIT_ASSERT(!us.away());
        CPPUNIT_ASSERT(us.rank() == 6);
        CPPUNIT_ASSERT(!us.moderator());
        CPPUNIT_ASSERT(!us.bot());
    }

    // test set inGame
    {
        UserStatus us;
        us.inGame(true);
        CPPUNIT_ASSERT(us.inGame());
        CPPUNIT_ASSERT(!us.away());
        CPPUNIT_ASSERT(us.rank() == 0);
        CPPUNIT_ASSERT(!us.moderator());
        CPPUNIT_ASSERT(!us.bot());
    }

    // test exception is thrown on bad input to ctor
    {
        CPPUNIT_ASSERT_THROW(UserStatus us(""), boost::bad_lexical_cast);
        CPPUNIT_ASSERT_THROW(UserStatus us("ABC"), boost::bad_lexical_cast);
    }


    // test operator== and operator!=
    {
        UserStatus us1;
        UserStatus us2("24");
        UserStatus us3("24");
        UserStatus us4("127");

        CPPUNIT_ASSERT( us2 == us3 );

        CPPUNIT_ASSERT( us1 != us2 );
        CPPUNIT_ASSERT( us1 != us3 );
        CPPUNIT_ASSERT( us3 != us4 );
    }

}

void Test::testUserBattleStatus()
{
    // test default ctor
    {
        UserBattleStatus ubs;
        CPPUNIT_ASSERT(!ubs.ready());
        CPPUNIT_ASSERT(ubs.team() == 0);
        CPPUNIT_ASSERT(ubs.allyTeam() == 0);
        CPPUNIT_ASSERT(ubs.spectator());
        CPPUNIT_ASSERT(ubs.handicap() == 0);
        CPPUNIT_ASSERT(ubs.sync() == 0);
        CPPUNIT_ASSERT(ubs.side() == 0);
    }

    // test when all bits set
    {
        UserBattleStatus ubs("2147483647");
        CPPUNIT_ASSERT(ubs.ready());
        CPPUNIT_ASSERT(ubs.team() == 15);
        CPPUNIT_ASSERT(ubs.allyTeam() == 15);
        CPPUNIT_ASSERT(!ubs.spectator());
        CPPUNIT_ASSERT(ubs.handicap() == 127);
        CPPUNIT_ASSERT(ubs.sync() == 3);
        CPPUNIT_ASSERT(ubs.side() == 15);
    }

    // test set methods
    {
        UserBattleStatus ubs;
        ubs.ready(true);
        ubs.spectator(false);
        ubs.allyTeam(7);
        ubs.team(3);
        ubs.sync(1);
        CPPUNIT_ASSERT(ubs.ready());
        CPPUNIT_ASSERT(ubs.team() == 3);
        CPPUNIT_ASSERT(ubs.allyTeam() == 7);
        CPPUNIT_ASSERT(!ubs.spectator());
        CPPUNIT_ASSERT(ubs.handicap() == 0);
        CPPUNIT_ASSERT(ubs.sync() == 1);
        CPPUNIT_ASSERT(ubs.side() == 0);
    }

    // test exception is thrown on bad input to ctor
    {
        CPPUNIT_ASSERT_THROW(UserBattleStatus ubs(""), boost::bad_lexical_cast);
        CPPUNIT_ASSERT_THROW(UserBattleStatus ubs("ABC"), boost::bad_lexical_cast);
    }


    // test operator== and operator!=
    {
        UserStatus ubs1;
        UserStatus ubs2("24");
        UserStatus ubs3("24");
        UserStatus ubs4("127");

        CPPUNIT_ASSERT( ubs2 == ubs3 );

        CPPUNIT_ASSERT( ubs1 != ubs2 );
        CPPUNIT_ASSERT( ubs1 != ubs3 );
        CPPUNIT_ASSERT( ubs3 != ubs4 );
    }

}

void Test::testUser()
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

        CPPUNIT_ASSERT_EQUAL(u.name(), name);
        CPPUNIT_ASSERT_EQUAL(u.country(), country);
        CPPUNIT_ASSERT_EQUAL(u.cpu(), cpu);

        // print for ocular inspection
        std::cout << u << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("username CC ");

        CPPUNIT_ASSERT_THROW(User u(ss), std::invalid_argument);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        CPPUNIT_ASSERT_THROW(User u(ss), std::invalid_argument);
    }

    // test operators
    {
        std::stringstream ss1("name1 SE 0");
        User u1(ss1);

        std::stringstream ss2("name2 SE 0");
        User u2(ss2);

        CPPUNIT_ASSERT(u1 == u1);
        CPPUNIT_ASSERT(u1 != u2);
    }

}

void Test::testBattle()
{
    // simple positive
    {
        std::string const opened =
                "8235 0 0 Founder " // id, replay, nat, founder
                "94.23.170.70 8463 32 " // ip, port, maxPlayers
                "0 0 -112462944 " // passw, rank, mapHash
                "Map name\t"
                "Battle title\t"
                "Mod name";

        std::stringstream ssOpened(opened);

        Battle b(ssOpened);

        CPPUNIT_ASSERT(b.id() == 8235);
        CPPUNIT_ASSERT(b.replay() == false);
        CPPUNIT_ASSERT(b.natType() == 0);
        CPPUNIT_ASSERT(b.founder() == "Founder");
        CPPUNIT_ASSERT(b.ip() == "94.23.170.70");
        CPPUNIT_ASSERT(b.port() == "8463");
        CPPUNIT_ASSERT(b.maxPlayers() == 32);
        CPPUNIT_ASSERT(b.passworded() == false);
        CPPUNIT_ASSERT(b.rank() == 0);
        CPPUNIT_ASSERT(b.mapHash() == -112462944);
        CPPUNIT_ASSERT(b.mapName() == "Map name");
        CPPUNIT_ASSERT(b.title() == "Battle title");
        CPPUNIT_ASSERT(b.modName() == "Mod name");
        CPPUNIT_ASSERT(b.spectators() == 0);
        CPPUNIT_ASSERT(b.locked() == false);
        CPPUNIT_ASSERT(b.modHash() == 0);


        std::string const updated =
                "3 1 " // specs, locked
                "-1517218254 " // mapHash
                "New map name";

        std::stringstream ssUpdated(updated);

        b.updateBattleInfo(ssUpdated);
        b.modHash(9786);

        CPPUNIT_ASSERT(b.spectators() == 3);
        CPPUNIT_ASSERT(b.locked() == true);
        CPPUNIT_ASSERT(b.mapHash() == -1517218254);
        CPPUNIT_ASSERT(b.mapName() == "New map name");
        CPPUNIT_ASSERT(b.modHash() == 9786);

        // print for ocular inspection
        std::cout << b << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("id not int");

        CPPUNIT_ASSERT_THROW(Battle b(ss), boost::bad_lexical_cast);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        CPPUNIT_ASSERT_THROW(Battle b(ss), std::invalid_argument);
    }
}

void Test::testScript()
{
    Script script;

    script.add("GAME/Sub1/Sub2/key1=val1");
    script.add("GAME/key1=val1");
    script.add("GAME/Sub1/key1=val1");
    script.add("GAME/Sub1/Sub2/key2=val2");

    script.write("./script.txt");
}

void Test::testBot()
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

        CPPUNIT_ASSERT_EQUAL(b.name(), name);
        CPPUNIT_ASSERT_EQUAL(b.owner(), owner);
        CPPUNIT_ASSERT_EQUAL(b.battleStatus(), battleStatus);
        CPPUNIT_ASSERT_EQUAL(b.color(), color);
        CPPUNIT_ASSERT_EQUAL(b.aiDll(), aiDll);

        // print for ocular inspection
        std::cout << b << std::endl;
    }

    // test exception is thrown on incomplete msg
    {
        std::stringstream ss("123 CC ");

        CPPUNIT_ASSERT_THROW(Bot b(ss), std::invalid_argument);
    }

    // test exception is thrown on empty
    {
        std::stringstream ss("");

        CPPUNIT_ASSERT_THROW(Bot b(ss), std::invalid_argument);
    }
}

void Test::testMyImage()
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

        CPPUNIT_ASSERT_EQUAL(3, image.w());
        CPPUNIT_ASSERT_EQUAL(2, image.h());
        CPPUNIT_ASSERT_EQUAL(1, image.d());
        CPPUNIT_ASSERT_EQUAL(static_cast<uchar>('1'), image.array[0]);
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

        CPPUNIT_ASSERT_EQUAL(2, image.w());
        CPPUNIT_ASSERT_EQUAL(3, image.h());
        CPPUNIT_ASSERT_EQUAL(3, image.d());
        CPPUNIT_ASSERT_EQUAL(static_cast<uchar>('1'), image.array[0]);
    }

    // test exception is thrown if file not found
    {
        CPPUNIT_ASSERT_THROW(MyImage image("non_existing_file"), std::invalid_argument);
    }
}
