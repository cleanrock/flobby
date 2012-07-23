#pragma once

#include "model/MapInfo.h"

#include <FL/Fl_Shared_Image.H> // included here so user can easily cast to Fl_Image
#include <map>
#include <string>

class Model;

class Cache
{
public:
    Cache(Model & model);
    virtual ~Cache();

    MapInfo const & getMapInfo(std::string const & mapName);

    Fl_Shared_Image * getMapImage(std::string const & mapName); // returns 0 if map not found
    Fl_Shared_Image * getMetalImage(std::string const & mapName);
    Fl_Shared_Image * getHeightImage(std::string const & mapName);

private:
    Model & model_;
    std::string basePath_;
    std::map<std::string, MapInfo> mapInfos_;

    std::string const & basePath();
    void createImageFile(uint8_t const * data, int w, int h, int d, std::string const & path, double r = 1 /* w/h */);
};
