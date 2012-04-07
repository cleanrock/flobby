#include "Cache.h"
#include "MyImage.h"
#include "logging.h"

#include "model/Model.h"

#include <FL/Fl_Shared_Image.H>
#include <sstream> // ostringstream
#include <fstream>
#include <boost/filesystem.hpp>

Cache::Cache(Model & model):
    model_(model)
{
}

Cache::~Cache()
{
}

std::string const & Cache::basePath()
{
    if (basePath_.empty())
    {
        basePath_ = model_.getWriteableDataDir() + "flobby/cache/";
        using namespace boost::filesystem;
        if (!is_directory(basePath_.c_str()))
        {
            create_directories(basePath_.c_str());
        }
    }
    return basePath_;
}

Fl_Image * Cache::getMapImage(std::string const & mapName)
{
    unsigned int chksum = model_.getMapChecksum(mapName);
    if (chksum == 0)
    {
        return 0;
    }

    std::ostringstream oss;
    oss << basePath() << mapName << "_" << chksum << "_minimap_128.bin";
    std::string const path = oss.str();

    Fl_Shared_Image * image = Fl_Shared_Image::get(path.c_str());

    if (image == 0)
    {
        // setup for 128x128
        int const mipLevel = 3;
        int const imageSize = 1024 >> mipLevel;

        auto imageData = model_.getMapImage(mapName, mipLevel);
        if (imageData)
        {
            // hack to resize always square minimap image
            int w,h;
            model_.getMapSize(mapName, w, h);
            if (w == h)
            {
                MyImage::write(path, imageData.get(), imageSize, imageSize, 3);
            }
            else
            {
                int wp = imageSize;
                int hp = imageSize;
                Fl_RGB_Image * imSquare = new Fl_RGB_Image(imageData.get(), imageSize, imageSize, 3);
                if (w > h)
                {
                    hp *= (float)h/(float)w;
                }
                else
                {
                    wp *= (float)w/(float)h;
                }
                assert(wp > 0 && hp > 0);
                Fl_RGB_Image * imRect = static_cast<Fl_RGB_Image *>(imSquare->copy(wp, hp));
                MyImage::write(path, imRect->array, wp, hp, 3);
                delete imSquare;
                delete imRect;
            }

            image = Fl_Shared_Image::get(path.c_str());
            if (image == 0)
            {
                throw std::runtime_error("Fl_Shared_Image::get failed:" + path);
            }
        }
    }
    return image;
}

/*
Fl_Image * Cache::getMetalImage(std::string const & mapName)
{
    std::string const path(basePath() + mapName + "_metal_128.bin");
    Fl_Shared_Image * image = Fl_Shared_Image::get(path.c_str());

    if (image == 0)
    {
        // setup for 128x128
        int w, h;
        int const imageSize = 128;

        auto imageData = model_.getMetalMap(mapName, w, h);
        if (imageData)
        {
            Fl_RGB_Image * imUnscaled = new Fl_RGB_Image(imageData.get(), w, h, 1);

            int wp = imageSize;
            int hp = imageSize;
            if (w > h)
            {
                hp *= (float)h/(float)w;
            }
            else
            {
                wp *= (float)w/(float)h;
            }
            assert(wp > 0 && hp > 0);

            // cast needed here to get access to Fl_RGB_Image::array below
            Fl_RGB_Image * imScaled = static_cast<Fl_RGB_Image *>(imUnscaled->copy(wp, hp));

            MyImage::write(path, imScaled->array, wp, hp, 1);

            delete imUnscaled;
            delete imScaled;

            image = Fl_Shared_Image::get(path.c_str());
            if (image == 0)
            {
                throw std::runtime_error("Fl_Shared_Image::get failed:" + path);
            }
        }
    }
    return image;
}
*/

MapInfo const & Cache::getMapInfo(std::string const & mapName)
{
    unsigned int chksum = model_.getMapChecksum(mapName);
    if (chksum == 0)
    {
        LOG(WARNING) << "map not found:" << mapName;
        throw std::runtime_error("map not found: " + mapName);
    }

    std::ostringstream oss;
    oss << mapName << "_" << chksum;
    std::string const key = oss.str();

    auto it = mapInfos_.find(key);
    if (it != mapInfos_.end())
    {
        // loaded into memory
        return it->second;
    }
    else
    {
        std::ostringstream oss;
        oss << basePath() << key << "_info.bin";
        std::string const path = oss.str();

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
