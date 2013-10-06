#include "Cache.h"

#include "log/Log.h"
#include "model/Model.h"
#include "FlobbyDirs.h"

#include <Magick++.h>
#include <FL/Fl_Shared_Image.H>

#include <sstream> // ostringstream
#include <fstream>
#include <boost/filesystem.hpp>
#include <cassert>

Cache::Cache(Model & model):
    model_(model)
{
}

Cache::~Cache()
{
}

std::string Cache::mapDir()
{
    std::string basePath = cacheDir() + "map/";
    if (!boost::filesystem::is_directory(basePath.c_str()))
    {
        boost::filesystem::create_directories(basePath.c_str());
    }
    return basePath;
}

std::string Cache::mapPath(std::string const& mapName, std::string const& suffix)
{
    std::string path;

    unsigned int const chksum = model_.getMapChecksum(mapName);
    if (chksum != 0)
    {
        std::ostringstream oss;
        oss << mapDir() << mapName << "_" << chksum << "_" << suffix;
        path = oss.str();
    }

    return path;
}

std::string Cache::pathMapInfo(std::string const& mapName)
{
    return mapPath(mapName, "info.bin");
}

std::string Cache::pathMapImage(std::string const& mapName)
{
    return mapPath(mapName, "minimap_128.png");
}

std::string Cache::pathMetalImage(std::string const& mapName)
{
    return mapPath(mapName, "metal_128.png");
}

std::string Cache::pathHeightImage(std::string const& mapName)
{
    return mapPath(mapName, "height_128.png");
}

std::string Cache::mapInfoKey(std::string const& mapName)
{
    std::string key;

    unsigned int chksum = model_.getMapChecksum(mapName);
    if (chksum == 0)
    {
        LOG(WARNING) << "map not found:" << mapName;
        throw std::runtime_error("map not found: " + mapName);
    }
    std::ostringstream oss;
    oss << mapName << "_" << chksum;
    key = oss.str();
    return key;
}

bool Cache::hasMapInfo(std::string const& mapName)
{
    std::string const key = mapInfoKey(mapName);
    auto it = mapInfos_.find(key);
    if (it != mapInfos_.end())
    {
        // loaded into memory
        return true;
    }

    std::string const path = pathMapInfo(mapName);

    return !path.empty() && boost::filesystem::exists(path);
}

bool Cache::hasMapImage(std::string const & mapName)
{
    std::string const path = pathMapImage(mapName);

    return !path.empty() && (Fl_Shared_Image::find(path.c_str()) || boost::filesystem::exists(path));
}

bool Cache::hasMetalImage(std::string const & mapName)
{
    std::string const path = pathMetalImage(mapName);

    return !path.empty() && (Fl_Shared_Image::find(path.c_str()) || boost::filesystem::exists(path));
}

bool Cache::hasHeightImage(std::string const & mapName)
{
    std::string const path = pathHeightImage(mapName);

    return !path.empty() && (Fl_Shared_Image::find(path.c_str()) || boost::filesystem::exists(path));
}

Fl_Shared_Image * Cache::getMapImage(std::string const & mapName)
{
    std::string const path = pathMapImage(mapName);
    if (path.empty()) return 0;

    Fl_Shared_Image * image = Fl_Shared_Image::get(path.c_str());

    if (image == 0)
    {
        // get 1024x1024 since higher mip levels can result in broken image, e.g. TinySkirmish
        int const mipLevel = 0;
        int const imageSize = 1024 >> mipLevel;

        auto imageData = model_.getMapImage(mapName, mipLevel);
        if (imageData)
        {
            // get real dimensions (minimap is always a square)
            int w,h;
            model_.getMapSize(mapName, w, h);

            createImageFile(imageData.get(), imageSize, imageSize, 3, path, static_cast<double>(w)/h);

            image = Fl_Shared_Image::get(path.c_str());
            if (image == 0)
            {
                throw std::runtime_error("Fl_Shared_Image::get failed:" + path);
            }
        }
    }
    return image;
}

Fl_Shared_Image * Cache::getMetalImage(std::string const & mapName)
{
    std::string const path = pathMetalImage(mapName);
    if (path.empty()) return 0;

    Fl_Shared_Image * image = Fl_Shared_Image::get(path.c_str());

    if (image == 0)
    {
        int w, h;
        auto imageData = model_.getMetalMap(mapName, w, h);
        if (imageData)
        {
            // create RGB data to get a green metal map
            std::unique_ptr<uint8_t[]> rgb(new uint8_t[3*w*h]);
            for (int i=0; i<w*h; ++i)
            {
                rgb[i*3+0] = 0;
                rgb[i*3+1] = imageData[i];
                rgb[i*3+2] = 0;
            }

            createImageFile(rgb.get(), w, h, 3, path);

            image = Fl_Shared_Image::get(path.c_str());
            if (image == 0)
            {
                throw std::runtime_error("Fl_Shared_Image::get failed:" + path);
            }
        }
    }
    return image;
}

Fl_Shared_Image * Cache::getHeightImage(std::string const & mapName)
{
    std::string const path = pathHeightImage(mapName);
    if (path.empty()) return 0;

    Fl_Shared_Image * image = Fl_Shared_Image::get(path.c_str());

    if (image == 0)
    {
        int w, h;
        auto imageData = model_.getHeightMap(mapName, w, h);
        if (imageData)
        {
            createImageFile(imageData.get(), w, h, 1, path);

            image = Fl_Shared_Image::get(path.c_str());
            if (image == 0)
            {
                throw std::runtime_error("Fl_Shared_Image::get failed:" + path);
            }
        }
    }
    return image;
}

void Cache::createImageFile(uint8_t const * data, int w, int h, int d, std::string const & path, double r /* w/h */)
{
    assert(w > 0 && h > 0 && (d == 1 || d == 3) && r > 0);

    // create image, we will resize it below
    Magick::Image image;
    image.read(w, h, d == 1 ? "I" : "RGB", MagickCore::CharPixel, data);
    image.depth(8);

    double const r2 = static_cast<double>(w)/h * r;

    int const maxSize = 128;
    int w2 = maxSize;
    int h2 = maxSize;

    if (r2 < 1)
    {
        w2 *= r2;
    }
    else if (r2 > 1)
    {
        h2 /= r2;
    }
    assert(w2 > 0 && h2 > 0);

    // resize and write
    Magick::Geometry geom(w2, h2);
    geom.aspect(true);
    // TODO remove ? default filter seem good
    // image.filterType(Magick::BoxFilter);
    image.resize(geom);
    image.write(path);
}

MapInfo const & Cache::getMapInfo(std::string const & mapName)
{
    std::string const key = mapInfoKey(mapName);

    auto it = mapInfos_.find(key);
    if (it != mapInfos_.end())
    {
        // loaded into memory
        return it->second;
    }
    else
    {
        std::string const path = pathMapInfo(mapName);

        std::ifstream ifs(path);
        if (ifs.good())
        {
            // map info file found
            MapInfo mapInfo;
            ifs >> mapInfo;
            return mapInfos_[key] = mapInfo;
        }
        else
        {
            // create map info file
            MapInfo const mapInfo = model_.getMapInfo(mapName);
            std::ofstream ofs(path);
            if (!ofs.good())
            {
                LOG(WARNING) << "failed to open mapInfo file for writing:" << path;
                throw std::runtime_error("failed to open mapInfo file for writing: " + path);
            }
            ofs << mapInfo;
            return mapInfos_[key] = mapInfo;
        }
    }
}
