#pragma once

#include "FL/Fl_Image.H"
#include <string>

class Fl_Image;

class MyImage: public Fl_RGB_Image
{
public:
    MyImage(std::string const & fileName);

    static void registerHandler();
    static void write(std::string const & path, uchar const * mapImageData, int w, int h, int d);

private:
    static Fl_Image * check(char const * fileName, uchar * header, int headerSize);

};
