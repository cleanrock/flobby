#include "BattleChat.h"
#include "StringTable.h"
#include "TextDisplay.h"
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
    model_(model)
    // TODO remove deleteLine_(0)
{
    int const ih = 24; // input height

    voteLine_ = new VoteLine(x, y, w, ih, model_);

    voteLine_->deactivate();

    int const m = 0; // margin
    textDisplay_ = new TextDisplay(x+m, y+ih+m, w-2*m, h-2*ih-2*m);

    input_ = new Fl_Input(x, y+h-ih, w, ih);
    input_->callback(BattleChat::onText, this);
    input_->when(FL_WHEN_ENTER_KEY);

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
    //std::string line(userName + ": " + msg);
    std::ostringstream oss;

    // handle messages from host
    if (userName == battleHost_)
    {
        oss << "@C" << FL_DARK2 << "@.";
        voteLine_->processHostMessage(msg);
    }
    oss << userName << ": " << msg;
    textDisplay_->append(oss.str());

}

void BattleChat::clear()
{
    // TODO
}

void BattleChat::onText(Fl_Widget * w, void * data)
{
    BattleChat * bc = static_cast<BattleChat*>(data);

    std::string msg(bc->input_->value());
    boost::trim(msg);

    if (!msg.empty())
    {
        bc->model_.sayBattle(msg);

        // always scroll on input
        // TODO bc->textDisplay_->show_insert_position();
    }
    bc->input_->value("");
}

void BattleChat::addInfo(std::string const & msg)
{
    textDisplay_->append(msg);
}

void BattleChat::battleJoined(Battle const & battle)
{
    battleHost_ = battle.founder();

    // TODO remove if line limit in TextDisplay is ok
//    while (deleteLine_ > 0)
//    {
//        textDisplay_->remove(deleteLine_);
//        --deleteLine_;
//    }
//    deleteLine_ = textDisplay_->size();
}
