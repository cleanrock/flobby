#pragma once

#include <vector>
#include <string>
#include <iosfwd>

class UnitSync;

class MapInfo
{
public:
    MapInfo() {};

    std::string name_;
    std::string fileName_;
    std::string description_;
    std::string author_;
    unsigned int checksum_;
    int width_; // divide with 512 to get "size"
    int height_;
    int tidalStrength_;
    int windMin_;
    int windMax_;
    int gravity_;

    int resourceCount_;
    std::vector<std::string> resourceNames_;
    std::vector<float> resourceMax_;
    std::vector<int> resourceExtractorRadius_;

    int startPosCount_;
    std::vector<std::pair<float,float>> startPos_;

    void serialize(std::ostream & os) const;
    void unserialize(std::istream & is); // throws on error

    bool operator==(MapInfo const & mi) const;

private:
    MapInfo(UnitSync & unitSync, int index);
    friend class Model;

    static std::string const version_; // used for compatibility check in unserialize
    static char const * nullToEmpty(const char * s);

};

std::ostream& operator<<(std::ostream & os, MapInfo const & mapInfo);
std::istream& operator>>(std::istream & os, MapInfo & mapInfo);
