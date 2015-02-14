// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <boost/signals2/signal.hpp>
#include <boost/filesystem.hpp>
#include <string>

class Model;
class Fl_Hold_Browser;
class Fl_Input;
class Fl_File_Input;
class Fl_Button;
class Fl_Return_Button;

class SpringDialog: public Fl_Window
{
public:
    SpringDialog(Model & model);
    virtual ~SpringDialog();

    void removeNonExistingProfiles();
    void addFoundProfiles();
    bool addProfile(std::string const& engineVersion); // returns true if new profile was added successfully

    void show();
    bool setPaths(); // returns true on success
    void setProfile(std::string const& engineVersion);

    // signals
    typedef boost::signals2::signal<void (std::string const & text)> ProfileSetSignal;
    boost::signals2::connection connectProfileSet(ProfileSetSignal::slot_type subscriber)
    { return profileSetSignal_.connect(subscriber); }

private:
    Model & model_;
    ProfileSetSignal profileSetSignal_;

    Fl_Preferences prefs_;
    Fl_Hold_Browser * list_;
    Fl_Input * name_;
    Fl_File_Input * springPath_;
    Fl_File_Input * unitSyncPath_;
    Fl_Button * save_;
    Fl_Button * delete_;
    Fl_Button * add_;
    Fl_Return_Button * select_;

    static void callbackList(Fl_Widget*, void*);
    static void callbackSave(Fl_Widget*, void*);
    static void callbackDelete(Fl_Widget*, void*);
    static void callbackAdd(Fl_Widget*, void*);
    static void callbackSelect(Fl_Widget*, void*);
    static void callbackBrowseSpring(Fl_Widget*, void*);
    static void callbackBrowseUnitSync(Fl_Widget*, void*);

    void initList(bool selectCurrent = false);
    void clearInputFields();
    void populate(char const * name);
    void onList();
    void onSave();
    void onDelete();
    void onAdd();
    void onSelect();
    void onBrowseSpring();
    void onBrowseUnitSync();
    bool openFileDialog(char const * title, char const * fileName, std::string & result); // returns false on cancel
    boost::filesystem::path findEngineDir(boost::filesystem::path const& engineDir, std::string const& engineVersion);
};
