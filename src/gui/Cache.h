// This file is part of flobby (GPL v2 or later), see the LICENSE file

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

    // has* methods below returns true if cache entry is loaded or cache file exist
    bool hasMapInfo(std::string const& mapName);
    bool hasMapImage(std::string const& mapName);
    bool hasMetalImage(std::string const& mapName);
    bool hasHeightImage(std::string const& mapName);

    MapInfo const&   getMapInfo(std::string const& mapName);
    Fl_Shared_Image* getMapImage(std::string const& mapName); // returns 0 if map not found
    Fl_Shared_Image* getMetalImage(std::string const& mapName);
    Fl_Shared_Image* getHeightImage(std::string const& mapName);

private:
    Model & model_;
    std::map<std::string, MapInfo> mapInfos_;

    std::string mapDir();
    std::string mapInfoKey(std::string const& mapName); // returns "<mapname>_<chksum>", throws if map not found

    std::string pathMapInfo(std::string const& mapName);
    std::string pathMapImage(std::string const& mapName);
    std::string pathMetalImage(std::string const& mapName);
    std::string pathHeightImage(std::string const& mapName);
    std::string mapPath(std::string const& mapName, std::string const& suffix); // returns empty string if map do not exist

    void createImageFile(uint8_t const* data, int w, int h, int d, std::string const& path, double r = 1 /* w/h */);
};
