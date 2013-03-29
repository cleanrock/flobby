#pragma once

#include <map>
#include <string>

class Script
{
public:
    Script();
    virtual ~Script();

    void clear();
    std::pair<std::string, std::string> getKeyValuePair(std::string const & str); // e.g. str=GAME/MODOPTIONS/maxunits=3000, returns MODOPTIONS/maxunits,3000
    std::string getKey(std::string const & str); // e.g. str=GAME/MODOPTIONS/maxunits=3000, returns MODOPTIONS/maxunits
    std::pair<std::string, std::string> add(std::string const & str); // e.g. str=GAME/MODOPTIONS/maxunits=3000, returns maxunits,3000
    std::string remove(std::string const & key); // e.g. GAME/MODOPTIONS/maxunits, returns maxunits
    void write(std::string const & fileName);

private:
    struct Node
    {
        std::string name_;
        int level_;
        std::map<std::string, Node> nodes_;
        std::map<std::string, std::string> values_;

        void write(std::ostream & os);
    };

    Node root_;

    Node & parse(std::string const & str, std::string & key, std::string & value); // key and value are output parameters

};
