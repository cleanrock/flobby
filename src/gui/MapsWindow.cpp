// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "MapsWindow.h"
#include "Prefs.h"
#include "Cache.h"

#include "model/Model.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Tooltip.H>

#include <algorithm>
#include <boost/algorithm/string.hpp>

static char const * PrefWindowX = "WindowX";
static char const * PrefWindowY = "WindowY";
static char const * PrefWindowW  = "WindowW";
static char const * PrefWindowH = "WindowH";

// Fl_Tooltip::margin_width/height() not available in FLTK 1.3.0
static int const MARGIN = 3;

MapsWindow::MapsWindow(Model & model, Cache& cache):
    Fl_Double_Window(100, 100, "Maps"),
    model_(model),
    cache_(cache),
    prefs_(prefs(), label())
{
    int const scrollW = Fl::scrollbar_size();
    mapArea_ = new MapArea(0, 0, w()-scrollW, h());
    scrollbar_ = new Fl_Scrollbar(w()-scrollW, 0, scrollW, h());
    resizable(mapArea_);
    end();

    scrollbar_->callback(&callbackScrollbar, this);
    scrollbar_->linesize(MapArea::SIZE_);

    int x, y, w, h;
    prefs_.get(PrefWindowX, x, 0);
    prefs_.get(PrefWindowY, y, 0);
    prefs_.get(PrefWindowW, w, 600);
    prefs_.get(PrefWindowH, h, 600);
    resize(x,y,w,h);
    size_range(MapArea::SIZE_ + scrollW, MapArea::SIZE_, 0, 0, 0, 0, 0);
}

MapsWindow::~MapsWindow()
{
    prefs_.set(PrefWindowX, x_root());
    prefs_.set(PrefWindowY, y_root());
    prefs_.set(PrefWindowW, w());
    prefs_.set(PrefWindowH, h());
}

void MapsWindow::callbackScrollbar(Fl_Widget*, void *data)
{
    MapsWindow* mw = static_cast<MapsWindow*>(data);
    mw->onScrollbar();
}

void MapsWindow::onScrollbar()
{
    mapArea_->pos_ = scrollbar_->value();
    mapArea_->updateMapInfoWin(Fl::event_x(), Fl::event_y());
    mapArea_->redraw();
}

int MapsWindow::handle(int event)
{
    switch (event)
    {
    case FL_SHOW: {
        mapArea_->names_ = model_.getMaps();

        auto comp=[](const std::string& a, const std::string& b){
           return boost::ilexicographical_compare
                               <std::string, std::string>(a,b);
        };

        std::sort(mapArea_->names_.begin(), mapArea_->names_.end(), comp);

        mapArea_->images_.clear();
        for (auto const& name : mapArea_->names_)
        {
            Fl_Shared_Image* im = cache_.getMapImage(name);
            mapArea_->images_.push_back(im);
        }
        scrollbar_->value(0, mapArea_->h(), 0, mapArea_->lines()*MapArea::SIZE_);

    } break;

    }

    return Fl_Double_Window::handle(event);
}

void MapsWindow::draw()
{
    Fl_Double_Window::draw();
    return;
}

void MapsWindow::resize(int x, int y, int w, int h)
{
    Fl_Double_Window::resize(x, y, w, h);
    int const lines = mapArea_->lines()*MapArea::SIZE_;
    scrollbar_->value(0, mapArea_->h(), 0, lines);
    onScrollbar();
}

///////////////
// MapInfoWin

MapsWindow::MapArea::MapInfoWin::MapInfoWin(): Fl_Menu_Window(1,1)
{
    set_override();
    end();
}

void MapsWindow::MapArea::MapInfoWin::info(std::string const& info)
{
    info_ = info;
    fl_font(Fl_Tooltip::font(), Fl_Tooltip::size());
    int W = 0;
    int H = 0;
    fl_measure(info_.c_str(), W, H, 0);
    size(W + 2*MARGIN, H + 2*MARGIN);
    redraw();
}

void MapsWindow::MapArea::MapInfoWin::draw()
{
    draw_box(FL_BORDER_BOX, 0, 0, w(), h(), Fl_Tooltip::color());
    fl_color(Fl_Tooltip::textcolor());
    fl_font(Fl_Tooltip::font(), Fl_Tooltip::size());

    int X = MARGIN;
    int Y = MARGIN;
    int W = w() - (MARGIN*2);
    int H = h() - (MARGIN*2);
    fl_draw(info_.c_str(), X, Y, W, H, Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_WRAP));
}

////////////
// MapArea

MapsWindow::MapArea::MapArea(int x, int y, int w, int h)
    : Fl_Widget(x, y, w, h)
    , pos_(0)
{
    Fl_Group *save = Fl_Group::current();
    mapInfoWin_ = new MapInfoWin();
    mapInfoWin_->hide();
    Fl_Group::current(save);
}

void MapsWindow::MapArea::draw()
{
    fl_color(FL_BACKGROUND_COLOR);
    fl_rectf(0, 0, w(), h());

    int line = pos_/SIZE_;
    int first = line*mapsPerLine();

    int x = 0;
    int y = line*SIZE_ - pos_;
    for (int i=first; i<images_.size(); ++i)
    {
        auto im = images_[i];
        if (x > (w()-SIZE_) && x != 0)
        {
            x = 0;
            y += SIZE_;
        }

        if (y > h())
            break;

        if (im)
            im->draw(x + SIZE_/2 - im->w()/2, y + SIZE_/2 - im->h()/2);

        x += SIZE_;
    }
}

int MapsWindow::MapArea::handle(int event)
{
    switch (event)
    {
    case FL_ENTER: {
        return 1; // to get FL_MOVE and FL_LEAVE events
    } break;

    case FL_LEAVE:
    case FL_HIDE:
        mapInfoWin_->hide();
        break;

    case FL_MOVE:
        updateMapInfoWin(Fl::event_x(), Fl::event_y());

    }

    Fl_Widget::handle(event);
}

void MapsWindow::MapArea::updateMapInfoWin(int x, int y)
{
    int offsetX = x/SIZE_; // offsetX is map index on line
    if (offsetX < mapsPerLine())
    {
        int index = mapsPerLine()*((pos_ + y)/SIZE_) + offsetX;
        if (index < names_.size())
        {
            std::ostringstream oss;
            oss << names_[index];
            mapInfoWin_->position(Fl::event_x_root(), Fl::event_y_root()+20);
            mapInfoWin_->info(oss.str());
            mapInfoWin_->show();
            return;
        }
    }

    mapInfoWin_->hide();
}

int MapsWindow::MapArea::mapsPerLine() const
{
    return std::max(w()/SIZE_, 1);
}

int MapsWindow::MapArea::linesVisible() const
{
    int const lines = h()/SIZE_;
    return std::max(lines, 1);
}

int MapsWindow::MapArea::lines() const
{
    int const lines = 1 + names_.size()/mapsPerLine();
    return lines;
}
