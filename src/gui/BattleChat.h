#pragma once

#include "LogFile.h"
#include <FL/Fl_Group.H>
#include <string>

class User;
class Battle;
class Model;
class VoteLine;
class TextDisplay;
class Fl_Input;
class Fl_Widget;

class BattleChat: public Fl_Group
{
public:
    BattleChat(int x, int y, int w, int h, Model & model);
    virtual ~BattleChat();

    void battleJoined(Battle const & battle); // call when joining a battle
    void addInfo(std::string const & msg);
    void close(); // call when "me" left battle

private:
    Model & model_;
    VoteLine * voteLine_;
    TextDisplay * textDisplay_;
    Fl_Input * input_;
    std::string battleHost_;
    LogFile logFile_;

    void battleChatMsg(std::string const & userName, std::string const & msg);
    static void onText(Fl_Widget * w, void * data);
    bool inGameMessage(std::string const & msg, std::string & userNameOut, std::string & msgOut);
};

