#pragma once

#include <map>
#include <string>

class Script
{
public:
    Script();
    virtual ~Script();

    void clear();
    void add(std::string const & str); // e.g. GAME/MODOPTIONS/maxunits=3000
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
};
