#pragma once

#include "model/StartRect.h"
#include <FL/Fl_Box.H>
#include <vector>

class MapImage: public Fl_Box
{
public:
    MapImage(int X, int Y, int W, int H);
    virtual ~MapImage();

    void addStartRect(StartRect const & startRect);
    void removeStartRect(int ally);
    void removeAllStartRects();
    void setAlly(int ally);

private:
    int ally_;
    std::vector<StartRect> startRects_;

    int handle(int event);
    void draw();

    void drawStartRects(int x, int y, int w, int h);

};

