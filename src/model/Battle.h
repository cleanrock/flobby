#pragma once

#include <map>
#include <iosfwd>
#include <string>

// forwards
class User;

class Battle
{
public:
    Battle(std::istream & is); // BATTLEOPENED content
    virtual ~Battle();

    int id() const;
    bool replay() const;
    int natType() const;
    std::string const & founder() const;
    std::string const & ip() const;
    std::string const & port() const;
    int maxPlayers() const;
    bool passworded() const;
    int rank() const;
    int mapHash() const;
    std::string const & engineName() const;
    std::string const & engineVersion() const;
    std::string const & mapName() const;
    std::string const & title() const;
    std::string const & modName() const;
    int players() const;
    int spectators() const;
    bool locked() const;
    bool running() const;
    bool running(bool running); // returns true if running status changed

    void updateBattleInfo(std::istream & is); // UPDATEBATTLEINFO content excluding battle id
    void joined(User const & user);
    void left(User const & user);

    void modHash(int modHash) { modHash_ = modHash; }
    int modHash() const { return modHash_; }

    typedef std::map<std::string, User const *> BattleUsers;
    BattleUsers const & users() const;

    void print(std::ostream & os) const;

private:
    friend class Model;

    int id_;
    bool replay_;
    int natType_;
    std::string founder_;
    std::string ip_;
    std::string port_;
    int maxPlayers_;
    bool passworded_;
    int rank_;
    int mapHash_;
    std::string engineName_;
    std::string engineVersion_;
    std::string mapName_;
    std::string title_;
    std::string modName_;

    int spectators_; // set to 1 in ctor if its a replay battle
    bool locked_;
    bool running_;
    int modHash_;

    BattleUsers users_;
};

// inline methods
//
inline int Battle::id() const
{
    return id_;
}

inline bool Battle::replay() const
{
    return replay_;
}

inline int Battle::natType() const
{
    return natType_;
}

inline std::string const & Battle::founder() const
{
    return founder_;
}

inline std::string const & Battle::ip() const
{
    return ip_;
}

inline std::string const & Battle::port() const
{
    return port_;
}

inline int Battle::maxPlayers() const
{
    return maxPlayers_;
}

inline bool Battle::passworded() const
{
    return passworded_;
}

inline int Battle::rank() const
{
    return rank_;
}

inline int Battle::mapHash() const
{
    return mapHash_;
}

inline std::string const & Battle::engineName() const
{
    return engineName_;
}

inline std::string const & Battle::engineVersion() const
{
    return engineVersion_;
}

inline std::string const & Battle::mapName() const
{
    return mapName_;
}

inline std::string const & Battle::title() const
{
    return title_;
}

inline std::string const & Battle::modName() const
{
    return modName_;
}

inline int Battle::spectators() const
{
    return spectators_;
}

inline bool Battle::locked() const
{
    return locked_;
}

inline int Battle::players() const
{
    return users_.size();
}

inline bool Battle::running() const
{
    return running_;
}

inline Battle::BattleUsers const & Battle::users() const
{
    return users_;
}

// global functions
//
std::ostream& operator<<(std::ostream & os, Battle const & battle);

