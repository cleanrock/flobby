#include "BattleChat.h"
#include "StringTable.h"
#include "TextDisplay2.h"
#include "ChatInput.h"
#include "VoteLine.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <ctime>

BattleChat::BattleChat(int x, int y, int w, int h, Model & model):
    Fl_Group(x, y, w, h),
    model_(model),
    logFile_("battlechat")
{
    int const ih = 24; // input height

    voteLine_ = new VoteLine(x, y, w, ih, model_);

    voteLine_->deactivate();

    int const m = 0; // margin
    textDisplay_ = new TextDisplay2(x+m, y+ih+m, w-2*m, h-2*ih-2*m);

    input_ = new ChatInput(x, y+h-ih, w, ih);
    input_->connectText( boost::bind(&BattleChat::onText, this, _1) );
    input_->deactivate();

    resizable(textDisplay_);
    end();

    // model signals
    model_.connectBattleChatMsg( boost::bind(&BattleChat::battleChatMsg, this, _1, _2) );
}

BattleChat::~BattleChat()
{
}

void BattleChat::battleChatMsg(std::string const & userName, std::string const & msg)
{
    logFile_.log(userName + ": " + msg);

    std::ostringstream oss;

    bool interesting = true;

    // handle messages from host
    if (userName == battleHost_)
    {
        voteLine_->processHostMessage(msg);

        // detect relayed in-game messages, e.g. "[userName] bla"
        std::string inGameUserName;
        std::string inGameMsg;

        if (inGameMessage(msg, inGameUserName, inGameMsg))
        {
            oss << inGameUserName << ": " << inGameMsg;
        }
        else
        {
            oss << userName << ": " << msg;
            interesting = false;
        }
    }
    else
    {
        oss << userName << ": " << msg;
    }
    textDisplay_->append(oss.str(), interesting);

}

bool BattleChat::inGameMessage(std::string const & msg, std::string & userNameOut, std::string & msgOut)
{
    if (msg.size() > 0 && msg[0] == '[')
    {
        int level = 1;
        int i = 1;
        while (i < msg.size())
        {
            if (msg[i] == '[')
            {
                ++level;
            }
            else if (msg[i] == ']')
            {
                --level;
                if (level == 0)
                {
                    userNameOut = msg.substr(1, i-1);
                    msgOut = msg.substr(i+1);
                    return true;
                }
            }
            ++i;
        }
    }
    return false;
}

void BattleChat::close()
{
    voteLine_->label(0);
    voteLine_->deactivate();

    // add empty line
    addInfo("");

    input_->deactivate();
}

void BattleChat::onText(std::string const & text)
{
    model_.sayBattle(text);
}

void BattleChat::addInfo(std::string const & msg)
{
    if (!msg.empty())
    {
        logFile_.log(msg);
    }

    textDisplay_->append(msg, false);
}

void BattleChat::battleJoined(Battle const & battle)
{
    battleHost_ = battle.founder();

    input_->activate();
}
