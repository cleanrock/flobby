// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "MapInfo.h"
#include "UnitSync.h"
#include <iostream>
#include <stdexcept>

std::string const MapInfo::version_ = "MapInfo_1";

MapInfo::MapInfo(UnitSync & unitSync, int index):
    name_( nullToEmpty(unitSync.GetMapName(index)) ),
    fileName_( nullToEmpty(unitSync.GetMapFileName(index)) ),
    checksum_(unitSync.GetMapChecksum(index))
{
    int const mapInfos = unitSync.GetMapInfoCount(index);
    for (int i=0; i<mapInfos; ++i)
    {
        const std::string key = unitSync.GetInfoKey(i);

        // not using GetInfoType, assignments below are probably safe enough
        // const std::string type = unitSync.GetInfoType(i);

        if (key == "description") {
            description_ = nullToEmpty(unitSync.GetInfoValueString(i));
            continue;
        }
        if (key == "author") {
            author_ = nullToEmpty(unitSync.GetInfoValueString(i));
            continue;
        }
        if (key == "width") {
            width_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
        if (key == "height") {
            height_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
        if (key == "tidalStrength") {
            tidalStrength_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
        if (key == "minWind") {
            windMin_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
        if (key == "maxWind") {
            windMax_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
        if (key == "gravity") {
            gravity_ = unitSync.GetInfoValueInteger(i);
            continue;
        }
    }
}

void MapInfo::serialize(std::ostream & os) const
{
    os << version_ << '\0'
       << name_ << '\0'
       << fileName_ << '\0'
       << description_ << '\0'
       << author_ << '\0'
       << checksum_ << std::endl
       << width_ << std::endl
       << height_ << std::endl
       << tidalStrength_ << std::endl
       << windMin_ << std::endl
       << windMax_ << std::endl
       << gravity_ << std::endl;
}

void MapInfo::unserialize(std::istream & is)
{
    is.exceptions(std::ios::failbit); // make istream throw on failure

    std::string version;
    std::getline(is, version, '\0');

    if (version != version_)
    {
        throw std::runtime_error("incompatible version: " + version + "!=" + version_);
    }

    std::getline(is, name_, '\0');
    std::getline(is, fileName_, '\0');
    std::getline(is, description_, '\0');
    std::getline(is, author_, '\0');
    is >> checksum_;
    is >> width_;
    is >> height_;
    is >> tidalStrength_;
    is >> windMin_;
    is >> windMax_;
    is >> gravity_;
}

bool MapInfo::operator==(MapInfo const & mi) const
{
    if (name_ != mi.name_) return false;
    if (fileName_ != mi.fileName_) return false;
    if (description_ != mi.description_) return false;
    if (author_ != mi.author_) return false;
    if (width_ != mi.width_) return false;
    if (height_ != mi.height_) return false;
    if (tidalStrength_ != mi.tidalStrength_) return false;
    if (windMin_ != mi.windMin_) return false;
    if (windMax_ != mi.windMax_) return false;
    if (gravity_ != mi.gravity_) return false;

    return true;
}

char const * MapInfo::nullToEmpty(const char * s)
{
    return (s ? s : "");
}

std::ostream& operator<<(std::ostream & os, MapInfo const & mapInfo)
{
    mapInfo.serialize(os);
    return os;
}

std::istream& operator>>(std::istream & is, MapInfo & mapInfo)
{
    mapInfo.unserialize(is);
    return is;
}

