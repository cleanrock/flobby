// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

struct ServerInfo
{
    std::string protocolVersion_;
    std::string springVersion_;
    unsigned short udpPort_;
    unsigned short serverMode_;

    inline void print(std::ostream & os) const;
};

inline void ServerInfo::print(std::ostream & os) const
{
    os << "protocolVersion:" << protocolVersion_;
    os << " springVersion:" << springVersion_;
    os << " udpPort:" << udpPort_;
    os << " serverMode:" << serverMode_;
}

inline std::ostream& operator<<(std::ostream & os, ServerInfo const & si)
{
    si.print(os);
    return os;
}
