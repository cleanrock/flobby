#include "SpringDialog.h"
#include "Prefs.h"
#include "model/Model.h"

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <boost/filesystem.hpp>

// prefs
char const * const PrefSpringProfile = "SpringProfile";
char const * const PrefSpringPath = "SpringPath";
char const * const PrefUnitSyncPath = "UnitSyncPath";

SpringDialog::SpringDialog(Model & model) :
        model_(model), prefs_(prefs(), "SpringProfiles"), Fl_Window(600, 400,
                "Spring")
{
    set_modal();

    Fl_Button * btn; // used for anonymous buttons below

    list_ = new Fl_Hold_Browser(10, 30, 200, 360, "Spring profiles");
    list_->align(FL_ALIGN_TOP_LEFT);
    list_->callback(SpringDialog::callbackList, this);

    name_ = new Fl_Input(220, 30, 370, 30, "Name");
    name_->align(FL_ALIGN_TOP_LEFT);

    springPath_ = new Fl_File_Input(220, 90, 350, 40, "Spring (e.g. /usr/bin/spring [options])");
    springPath_->align(FL_ALIGN_TOP_LEFT);

    btn = new Fl_Button(570, 90, 20, 40, "...");
    btn->callback(SpringDialog::callbackBrowseSpring, this);

    unitSyncPath_ = new Fl_File_Input(220, 160, 350, 40, "UnitSync (e.g. /usr/lib/libunitsync.so)");
    unitSyncPath_->align(FL_ALIGN_TOP_LEFT);

    btn = new Fl_Button(570, 160, 20, 40, "...");
    btn->callback(SpringDialog::callbackBrowseUnitSync, this);

    save_ = new Fl_Button(500, 290, 90, 30, "Save");
    save_->callback(SpringDialog::callbackSave, this);

    delete_ = new Fl_Button(400, 290, 90, 30, "Delete");
    delete_->callback(SpringDialog::callbackDelete, this);

    select_ = new Fl_Return_Button(500, 350, 90, 30, "Select");
    select_->callback(SpringDialog::callbackSelect, this);
    select_->deactivate();

    end();
}

SpringDialog::~SpringDialog()
{
}

void SpringDialog::initList(bool selectCurrent)
{
    list_->clear();
    // setup default if no entries exist
    if (prefs_.groups() == 0)
    {
        std::string springDefault;
        std::string unitsyncDefault;

        using namespace boost::filesystem;

        // test a few default paths

        // arch, fedora
        if (springDefault.empty())
        {
            std::string spring("/usr/bin/spring");
            std::string unitsync("/usr/lib/libunitsync.so");
            if (is_regular_file(spring) &&
                is_regular_file(unitsync) )
            {
                springDefault = spring;
                unitsyncDefault = unitsync;
            }
        }

        // ubuntu
        if (springDefault.empty())
        {
            std::string spring("/usr/games/spring");
            std::string unitsync("/usr/lib/spring/libunitsync.so");
            if (is_regular_file(spring) &&
                is_regular_file(unitsync) )
            {
                springDefault = spring;
                unitsyncDefault = unitsync;
            }
        }

        Fl_Preferences p(prefs_, "default");
        p.set(PrefSpringPath, springDefault.c_str());
        p.set(PrefUnitSyncPath, unitsyncDefault.c_str());
        prefs_.set(PrefSpringProfile, p.name());
    }

    for (int i = 0; i < prefs_.groups(); ++i)
    {
        list_->add(prefs_.group(i));
    }
    list_->sort(FL_SORT_ASCENDING);

    if (selectCurrent)
    {
        char * str;
        prefs_.get(PrefSpringProfile, str, "UNKNOWN");
        std::string const profile(str);
        ::free(str);

        for (int i = 1; i <= list_->size(); ++i)
        {
            if (profile == list_->text(i))
            {
                list_->select(i);
                populate(profile.c_str());
            }
        }
    }
}

void SpringDialog::clearInputFields()
{
    name_->value(0);
    springPath_->value(0);
    unitSyncPath_->value(0);
}

void SpringDialog::populate(char const * name)
{
    if (prefs_.groupExists(name))
    {
        Fl_Preferences p(prefs_, name);
        name_->value(p.name());

        char * str;

        p.get(PrefSpringPath, str, "");
        springPath_->value(str);
        ::free(str);

        p.get(PrefUnitSyncPath, str, "");
        unitSyncPath_->value(str);
        ::free(str);
    }
}

void SpringDialog::callbackList(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onList();
}

void SpringDialog::callbackSave(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onSave();
}

void SpringDialog::callbackDelete(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onDelete();
}

void SpringDialog::callbackSelect(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onSelect();
}

void SpringDialog::callbackBrowseSpring(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onBrowseSpring();
}

void SpringDialog::callbackBrowseUnitSync(Fl_Widget*, void *data)
{
    SpringDialog * o = static_cast<SpringDialog*>(data);
    o->onBrowseUnitSync();
}

void SpringDialog::onList()
{
    int const line = list_->value();
    if (line == 0)
    {
        clearInputFields();
        select_->deactivate();
        return;
    }
    select_->activate();

    populate(list_->text(line));

    // select if double click
    if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() != 0)
    {
        onSelect();
    }
}

void SpringDialog::onSave()
{
    char const * name = name_->value();
    assert(name != 0);
    if (::strlen(name) > 0)
    {
        // delete if we change an entry
        int line = list_->value();
        if (line > 0)
        {
            prefs_.deleteGroup(list_->text(line));
        }

        Fl_Preferences p(prefs_, name);
        p.set(PrefSpringPath, springPath_->value());
        p.set(PrefUnitSyncPath, unitSyncPath_->value());
        initList();
    }

    // flush prefs to make debugging easier
    prefs().flush();
}

void SpringDialog::onDelete()
{
    char const * name = name_->value();
    assert(name != 0);
    if (::strlen(name) > 0)
    {
        prefs_.deleteGroup(name);
        clearInputFields();
        initList();
    }
}

void SpringDialog::onSelect()
{
    int const line = list_->value();
    if (line > 0)
    {
        prefs_.set(PrefSpringProfile, list_->text(line));
        if (setPaths())
        {
            hide();
        }
    }
}

void SpringDialog::onBrowseSpring()
{
    std::string fileName;
    if (openFileDialog("Select spring executable", springPath_->value(), fileName))
    {
        springPath_->value(fileName.c_str());
    }
}

void SpringDialog::onBrowseUnitSync()
{
    std::string fileName;
    if (openFileDialog("Select UnitSync library", unitSyncPath_->value(), fileName))
    {
        unitSyncPath_->value(fileName.c_str());
    }
}

bool SpringDialog::setPaths()
{
    char * str;
    prefs_.get(PrefSpringProfile, str, "UNKNOWN");
    std::string const profile(str);
    ::free(str);

    if (prefs_.groupExists(profile.c_str()))
    {
        Fl_Preferences p(prefs_, profile.c_str());

        // Spring
        p.get(PrefSpringPath, str, "UNKNOWN");
        std::string springPath(str);
        ::free(str);

        model_.setSpringPath(springPath);

        // UnitSync
        p.get(PrefUnitSyncPath, str, "UNKNOWN");
        std::string unitSyncPath(str);
        ::free(str);

        try
        {
            model_.setUnitSyncPath(unitSyncPath);
        } catch (std::exception const & e)
        {
            fl_alert("problem loading UnitSync: %s", e.what());
            show();
            return false;
        }

        profileSetSignal_(profile);
        return true;
    }
    else
    {
        show();
        return false;
    }

}

void SpringDialog::show()
{
    clearInputFields();
    initList(true);
    Fl_Window::show();
}

bool SpringDialog::openFileDialog(char const * title, char const * fileName, std::string & result)
{
    Fl_Native_File_Chooser fc;
    fc.options(Fl_Native_File_Chooser::NO_OPTIONS);
    fc.title(title);
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);

    if (fileName != 0 && ::strlen(fileName) > 0)
    {
        fc.preset_file(fileName);
    }
    else
    {
        fc.directory("/");
    }

    if (fc.show() == 0)
    {
        result = fc.filename();
        return true;
    }
    return false;
}
