#include "Script.h"

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Script::Script()
{
    root_.level_ = -1;
}

Script::~Script()
{
}

void Script::clear()
{
    root_.nodes_.clear();
    root_.values_.clear();
}

void Script::add(std::string const & str)
{
    // TODO add parsing error handling

    std::istringstream iss(str);

    std::string ex;
    Node * node = &root_;

    // create/find node in tree
    std::getline(iss, ex, '/');
    while (!iss.eof())
    {
        boost::to_upper(ex);

        int level = node->level_;
        node = &node->nodes_[ex];
        node->name_ = ex;
        node->level_ = ++level;

        std::getline(iss, ex, '/');
    }

    // ex now contain key=value
    {
        iss.str(ex);
        iss.seekg(0);
        std::string key;
        std::getline(iss, key, '=');
        std::string value;
        std::getline(iss, value);
        node->values_[key] = value;
    }
}

void Script::write(std::string const & fileName)
{
    std::ofstream ofs(fileName);
    root_.write(ofs);
}

void Script::Node::write(std::ostream & os)
{
    std::string const ind( (level_ >= 0) ? level_ : 0, '\t');

    if (level_ >= 0)
    {
        os << ind << "[" << name_ << "]\n";
        os << ind << "{\n";
    }

    for (std::map<std::string, std::string>::value_type & pair : values_)
    {
        os << ind << '\t' << pair.first << "=" << pair.second << ";\n";
    }

    for (std::map<std::string, Node>::value_type & pair : nodes_)
    {
        pair.second.write(os);
    }

    if (level_ >= 0)
    {
        os << ind << "}\n";
    }
}
