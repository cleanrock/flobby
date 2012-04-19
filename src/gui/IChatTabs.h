#pragma once

class IChatTabs
{
public:
    virtual void redrawTabs() = 0;
protected:
    ~IChatTabs() {}
};
