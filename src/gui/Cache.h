#pragma once

#include "model/MapInfo.h"
#include <map>
#include <string>

class Model;
class Fl_Image;

class Cache
{
public:
    Cache(Model & model);
    virtual ~Cache();

    MapInfo const & getMapInfo(std::string const & mapName);

    Fl_Image * getMapImage(std::string const & mapName); // returns 0 if map not found
    Fl_Image * getMetalImage(std::string const & mapName);
    Fl_Image * getHeightImage(std::string const & mapName);

private:
    Model & model_;
    std::string basePath_;
    std::map<std::string, MapInfo> mapInfos_;

    std::string const & basePath();
    void createImageFile(uint8_t const * data, int w, int h, int d, std::string const & path, double r = 1 /* w/h */);
};
