#include "logging.h"
#include "MyImage.h"

#include <FL/Fl_Shared_Image.H>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <iostream> // TODO

MyImage::MyImage(std::string const & fileName):
    Fl_RGB_Image(0,0,0)
{
    std::ifstream ifs(fileName);

    if (!ifs.good())
    {
        throw std::invalid_argument("file not found:" + fileName);
    }

    // header: FLOBBY_IMAGE <W> <H> <D>\n, e.g. "FLOBBY_IMAGE 512 512 3\n"

    std::string id;
    ifs >> id;
    if (!ifs.good() || id != "FLOBBY_IMAGE")
    {
        throw std::runtime_error("expected FLOBBY_IMAGE:" + id);
    }

    int width;
    ifs >> width;
    if (!ifs.good())
    {
        throw std::runtime_error("error extracting width");
    }
    w(width);

    int height;
    ifs >> height;
    if (!ifs.good())
    {
        throw std::runtime_error("error extracting height");
    }
    h(height);

    int depth;
    ifs >> depth;
    if (!ifs.good())
    {
        throw std::runtime_error("error extracting depth");
    }
    d(depth);

    // terminating newline
    char ch;
    ifs.get(ch);
    if (!ifs.good() || ch != '\n')
    {
        throw std::runtime_error("expected newline");
    }

    size_t const size = w()*h()*d();
    array = new uchar[size];
    alloc_array = 1;

    ifs.read((char*)array, size);

    if (!ifs.good())
    {
        throw std::runtime_error("error reading data");
    }

    ifs.get(ch);
    if (!ifs.eof())
    {
        throw std::runtime_error("data left");
    }
}

void MyImage::registerHandler()
{
    Fl_Shared_Image::add_handler(MyImage::check);
}


Fl_Image * MyImage::check(char const * fileName, uchar * header, int headerSize)
{
    if (::memcmp(header, "FLOBBY_IMAGE", 12) == 0)
    {
        return new MyImage(fileName);
    }

    return 0;
}

void MyImage::write(std::string const & path, uchar const * mapImageData, int w, int h, int d)
{
    DLOG(INFO) << "write " << path;
    std::ofstream ofs(path);

    if (!ofs.good())
    {
        throw std::runtime_error("failed to create image file:" + path);
    }
    // sanity check dimension
    if (w < 1 || w > 2048 ||
        h < 1 || h > 2048 ||
        d < 1 || d > 3)
    {
        throw std::runtime_error("bad dimensions");
    }

    ofs << "FLOBBY_IMAGE"
        << " " << w
        << " " << h
        << " " << d
        << '\n';

    ofs.write(reinterpret_cast<char const *>(mapImageData), w*h*d);
}

