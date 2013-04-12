#pragma once

#include "model/MapInfo.h"

#include <map>
#include <string>

class Model;
class Fl_Shared_Image;

class Cache
{
public:
    Cache(Model & model);
    virtual ~Cache();

    MapInfo const & getMapInfo(std::string const & mapName);

    bool hasMapImage(std::string const & mapName); // returns true if map cache file exist
    bool hasMetalImage(std::string const & mapName);
    bool hasHeightImage(std::string const & mapName);

    Fl_Shared_Image * getMapImage(std::string const & mapName); // returns 0 if map not found
    Fl_Shared_Image * getMetalImage(std::string const & mapName);
    Fl_Shared_Image * getHeightImage(std::string const & mapName);

private:
    Model & model_;
    std::map<std::string, MapInfo> mapInfos_;

    std::string basePath();
    std::string pathMapImage(std::string const& mapName);
    std::string pathMetalImage(std::string const& mapName);
    std::string pathHeightImage(std::string const& mapName);
    std::string getPath(std::string const& mapName, std::string const& type);

    void createImageFile(uint8_t const * data, int w, int h, int d, std::string const & path, double r = 1 /* w/h */);
};
