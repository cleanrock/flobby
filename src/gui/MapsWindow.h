// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Window.H>

#include <vector>
#include <string>


class Model;
class Cache;
class Fl_Shared_Image;
class Fl_Box;
class Fl_Scrollbar;

class MapsWindow: public Fl_Double_Window
{
public:
    MapsWindow(Model & model, Cache& cache);
    virtual ~MapsWindow();

private:
    Model& model_;
    Cache& cache_;
    Fl_Preferences prefs_;

    struct MapArea: public Fl_Widget
    {
        class MapInfoWin: public Fl_Menu_Window
        {
        public:
            MapInfoWin();
            void info(std::string const& info);
            void draw();
        private:
            std::string info_;
        };

        std::vector<std::string> names_;
        std::vector<Fl_Shared_Image*> images_;
        static const int SIZE_ = 128+2;
        int pos_;

        MapArea::MapInfoWin* mapInfoWin_;
        Model& model_;

        MapArea(int x, int y, int w, int h, Model& model);
        void draw();
        int handle(int event);

        int mousePosToMapIndex(int x, int y); // returns -1 if not on a map
        void updateMapInfoWin(int x, int y);
        void showMapInfoMenu(int x, int y);
        int mapsPerLine() const;
        int linesVisible() const;
        int lines() const;
    };
    MapArea* mapArea_;
    Fl_Scrollbar* scrollbar_;

    static void callbackScrollbar(Fl_Widget*, void*);
    void onScrollbar();
    int handle(int event);
    void draw();
    void resize(int x, int y, int w, int h);
};
