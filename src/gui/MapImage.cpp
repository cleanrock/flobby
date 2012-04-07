#include "MapImage.h"

#include "FL/fl_draw.H"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cmath>

MapImage::MapImage(int X, int Y, int W, int H, char const * l):
    Fl_Box(X, Y, W, H, l),
    ally_(-1)
{
}

MapImage::~MapImage()
{
}


void MapImage::draw()
{
    draw_box();

    Fl_Image * img = image();
    if (img != 0)
    {
        int X = x() + w()/2 - img->w()/2;
        int Y = y() + h()/2 - img->h()/2;

        img->draw(X, Y);
        drawStartRects(X, Y, img->w(), img->h());
    }
    else
    {
        draw_label();
    }
}

void MapImage::drawStartRects(int x, int y, int w, int h)
{
    for (StartRect const & sr : startRects_)
    {
        fl_color( (sr.ally() == ally_) ? FL_GREEN : FL_RED );

        int X = x + std::round( sr.left()*w );
        int Y = y + std::round( sr.top()*h );
        int W = std::round( (sr.right() - sr.left())*w );
        int H = std::round( (sr.bottom() - sr.top())*h );
        fl_rect(X, Y, W, H);
        std::string const ally = boost::lexical_cast<std::string>(sr.ally()+1);
        fl_draw(ally.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
    }
}

void MapImage::addStartRect(StartRect const & startRect)
{
    startRects_.push_back(startRect);
    redraw();
}

void MapImage::removeStartRect(int ally)
{
    startRects_.erase(std::remove_if(startRects_.begin(), startRects_.end(), boost::bind(&StartRect::ally, _1) == ally));
    redraw();
}

void MapImage::removeAllStartRects()
{
    startRects_.clear();
    redraw();
}

void MapImage::setAlly(int ally)
{
    if (ally != ally_)
    {
        ally_ = ally;
        redraw();
    }
}
