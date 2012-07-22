#pragma once

#include "LogFile.h"
#include <FL/Fl_Group.H>
#include <string>

class User;
class Battle;
class Model;
class VoteLine;
class TextDisplay;
class ChatInput;

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
    ChatInput * input_;
    std::string battleHost_;
    LogFile logFile_;

    void battleChatMsg(std::string const & userName, std::string const & msg);
    void onText(std::string const & text);
    bool inGameMessage(std::string const & msg, std::string & userNameOut, std::string & msgOut);
};

