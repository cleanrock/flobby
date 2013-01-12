#pragma once

#include "StringTable.h"
#include "FL/Fl_Tile.H"
#include <string>
#include <memory>

class User;
class Battle;
class BattleChat;
class Model;
class StartRect;
class Bot;
class Cache;
class ITabs;
class MapImage;
class AddBotDialog;
class GameSettings;

class Fl_Widget;
class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;
class Fl_RGB_Image;
class Fl_Multiline_Output;

class BattleRoom: public Fl_Tile
{
public:
    BattleRoom(int x, int y, int w, int h, Model & model, Cache & cache, ITabs & iTabs);
    virtual ~BattleRoom();

    void initTiles();

    void joined(Battle const & battle); // call when user joins a battle
    void userLeft(User const & user, Battle const & battle);

    // model signal handlers
    void battleChanged(Battle const & battle);
    void battleClosed(Battle const & battle);
    void userJoinedBattle(User const & user, const Battle & battle);
    void userLeftBattle(User const & user, const Battle & battle);
    void userChanged(User const & user);
    void botAdded(Bot const & bot);
    void botChanged(Bot const & bot);
    void botRemoved(Bot const & bot);
    void addStartRect(StartRect const & startRect);
    void removeStartRect(int ally);
    void springExit();
    void connected(bool connected);

    int battleId() const;
    void addUser(User const & user);

    void refresh();

private:
    Model & model_;
    Cache & cache_;
    ITabs & iTabs_;
    int battleId_;
    std::string founder_;
    bool lastRunning_;

    Fl_Group * top_;
    Fl_Group * header_;
    Fl_Multiline_Output * headerText_;
    Fl_Button * downloadGameBtn_;
    Fl_Check_Button * specBtn_;
    Fl_Check_Button * readyBtn_;
    Fl_Choice * teamBtn_;
    Fl_Button * addBotBtn_;
    Fl_Button * startBtn_;

    Fl_Group * topRight_;
    MapImage * mapImageBox_;
    Fl_Multiline_Output * mapInfo_;
    GameSettings * settings_;
    std::string currentMapImage_; // optimization, indicates what map image is currently shown to avoid setting the same image

    StringTable * playerList_;

    BattleChat * battleChat_;
    AddBotDialog * addBotDialog_;

    static void onSpec(Fl_Widget* w, void* data);
    static void onReady(Fl_Widget* w, void* data);
    static void onAllyTeam(Fl_Widget* w, void* data);
    static void onAddBot(Fl_Widget* w, void* data);
    static void onStart(Fl_Widget* w, void* data);
    static void onLeave(Fl_Widget* w, void* data);
    static void onMapImage(Fl_Widget* w, void* data);
    static void onDownloadGame(Fl_Widget* w, void* data);

    void close(); // call when user (me) left the battle

    void setMapImage(Battle const & battle);
    void setHeaderText(Battle const & battle);
    std::string statusString(User const & user);
    std::string syncString(User const & user);
    StringTableRow makeRow(User const & user);
    StringTableRow makeRow(Bot const & bot);
    void handleOnMapImage();
    void handleOnDownloadGame();
    void hideDownloadGameButton();
    void showDownloadGameButton();

    void playerClicked(int rowIndex, int button);
    void playerDoubleClicked(int rowIndex, int button);

    void onComplete(std::string const& text, std::string& result);

    void menuUser(User const & user);
    void menuBot(std::string const& botName, std::string const& ownerName);

};

inline int BattleRoom::battleId() const
{
    return battleId_;
}
