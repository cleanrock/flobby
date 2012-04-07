#include "StartRect.h"

#include <stdexcept>

StartRect::StartRect(int ally, int left, int top, int right, int bottom):
    ally_(ally),
    left_(static_cast<float>(left)/200),
    top_(static_cast<float>(top)/200),
    right_(static_cast<float>(right)/200),
    bottom_(static_cast<float>(bottom)/200)
{
    if (ally_ < 0) throw std::out_of_range("StartRect ally");
    if (left_ < 0 || left_ > 1) throw std::out_of_range("StartRect left");
    if (top_ < 0 || top_ > 1) throw std::out_of_range("StartRect top");
    if (right_ < 0 || right_ > 1) throw std::out_of_range("StartRect right");
    if (bottom_ < 0 || bottom_ > 1) throw std::out_of_range("StartRect bottom");
}

StartRect::~StartRect()
{
}

