#pragma once

#include "model/MapInfo.h"
#include <map>
#include <string>

class Model;
class MapInfo;
class Fl_Image;

class Cache
{
public:
    Cache(Model & model);
    virtual ~Cache();

    MapInfo const & getMapInfo(std::string const & mapName);

    Fl_Image * getMapImage(std::string const & mapName); // returns 0 if map not found
//    Fl_Image * getMetalImage(std::string const & mapName); TODO implement properly
//    Fl_Image * getHeightImage(std::string const & mapName); TODO implement

private:
    Model & model_;
    std::string basePath_;
    std::map<std::string, MapInfo> mapInfos_;

    std::string const & basePath();
};
