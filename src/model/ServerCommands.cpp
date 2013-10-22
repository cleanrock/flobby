#include "ServerCommands.h"

#include "Model.h"
#include "LobbyProtocol.h"
#include "log/Log.h"


#define SERVER_COMMAND(NAME) \
class SC_##NAME: public ServerCommand \
{ \
public: \
    SC_##NAME(): ServerCommand(#NAME) {} \
private: \
    std::string description(); \
    std::string process(std::vector<std::string> const& args); \
};

SERVER_COMMAND(help)
SERVER_COMMAND(ingame)
SERVER_COMMAND(regdate);
SERVER_COMMAND(uptime);
SERVER_COMMAND(pwhash);
SERVER_COMMAND(rename);
SERVER_COMMAND(changepw);
SERVER_COMMAND(changeuserpw);
SERVER_COMMAND(join);


Model* ServerCommand::model_ = 0;
ServerCommand::Commands ServerCommand::commmands_;

void ServerCommand::init(Model& model)
{
    model_ = &model;

    ServerCommand* sc;

    sc = new SC_help(); commmands_[sc->name_] = sc;
    sc = new SC_ingame(); commmands_[sc->name_] = sc;
    sc = new SC_regdate(); commmands_[sc->name_] = sc;
    sc = new SC_uptime(); commmands_[sc->name_] = sc;
    sc = new SC_pwhash(); commmands_[sc->name_] = sc;
    sc = new SC_rename(); commmands_[sc->name_] = sc;
    sc = new SC_changepw(); commmands_[sc->name_] = sc;
    sc = new SC_changeuserpw(); commmands_[sc->name_] = sc;
    sc = new SC_join(); commmands_[sc->name_] = sc;
}

std::string ServerCommand::process(std::string const& str)
{
    std::string result;

    std::istringstream iss(str);

    std::string cmd;
    LobbyProtocol::extractWord(iss, cmd);
    if (cmd.empty()) return result;

    Commands::iterator itNamePtr = commmands_.find(cmd);
    if (itNamePtr == commmands_.end())
    {
        result = "Command '" + cmd + "' do not exists. Type /help to see what commands exist.";
        return result;
    }

    std::vector<std::string> args;
    try
    {
        // extractWord will throw when no args left
        while (true)
        {
            LobbyProtocol::skipSpaces(iss);
            std::string arg;
            LobbyProtocol::extractWord(iss, arg);
            args.push_back(arg);
        }
    }
    catch (std::invalid_argument const& e)
    {
        // do nothing, no more args
    }

    return itNamePtr->second->process(args);
}


std::string SC_help::description()
{
    return  "/" + name_ + " [cmd1 cmd2 ...] - "
            "show available commands and their usage";
}

std::string SC_help::process(std::vector<std::string> const& args)
{
    std::string result;

    if (args.empty())
    {
        for (auto const& cmd : commmands_)
        {
            result += cmd.second->description() + "\n";
        }
    }
    else
    {
        for (auto const& arg : args)
        {
            Commands::iterator itNamePtr = commmands_.find(arg);
            if (itNamePtr != commmands_.end())
            {
                result += itNamePtr->second->description() + "\n";
            }
            else
            {
                result += "command " + arg + " not found\n";
            }
        }
    }
    return result;
}


std::string SC_ingame::description()
{
    return  "/" + name_ + " [username] - "
            "show your or a user's in-game time";
}

std::string SC_ingame::process(std::vector<std::string> const& args)
{
    std::string result;

    std::string msg = "GETINGAMETIME";
    if (!args.empty())
    {
        msg += " " + args[0];
    }
    model_->sendMessage(msg);

    return result;
}


std::string SC_regdate::description()
{
    return  "/" + name_ + " [username] - "
            "show your or a user's registration date";
}

std::string SC_regdate::process(std::vector<std::string> const& args)
{
    std::string result;

    std::string msg = "GETREGISTRATIONDATE";
    if (!args.empty())
    {
        msg += " " + args[0];
    }
    model_->sendMessage(msg);

    return result;
}


std::string SC_uptime::description()
{
    return  "/" + name_ + " - "
            "show server's uptime";
}

std::string SC_uptime::process(std::vector<std::string> const& args)
{
    std::string result;

    model_->sendMessage("UPTIME");

    return result;
}


std::string SC_pwhash::description()
{
    return  "/" + name_ + " text - "
            "print password hash of text";
}

std::string SC_pwhash::process(std::vector<std::string> const& args)
{
    std::string result;

    if (!args.empty())
    {
        result = model_->calcPasswordHash(args[0]);
    }
    else
    {
        result = name_ + " requires one argument";
    }

    return result;
}


std::string SC_rename::description()
{
    return  "/" + name_ + " newname newname - "
            "change your username";
}

std::string SC_rename::process(std::vector<std::string> const& args)
{
    std::string result;

    if (args.size() == 2)
    {
        if (args[0] == args[1])
        {
            model_->sendMessage("RENAMEACCOUNT " + args[0]);
        }
        else
        {
            result = "new name mismatch";
        }
    }
    else
    {
        result = name_ + " requires two identical new names";
    }

    return result;
}


std::string SC_changepw::description()
{
    return  "/" + name_ + " pw newpw newpw - "
            "change your password";
}

std::string SC_changepw::process(std::vector<std::string> const& args)
{
    std::string result;

    if (args.size() == 3)
    {
        if (args[1] == args[2])
        {
            std::string const pw = model_->calcPasswordHash(args[0]);
            std::string const pwNew = model_->calcPasswordHash(args[1]);
            model_->sendMessage("CHANGEPASSWORD " + pw + " " + pwNew);
        }
        else
        {
            result = "new password mismatch";
        }
    }
    else
    {
        result = name_ + " requires three arguments";
    }

    return result;
}


std::string SC_changeuserpw::description()
{
    return  "/" + name_ + " username newpw newpw - "
            "change a user's password";
}

std::string SC_changeuserpw::process(std::vector<std::string> const& args)
{
    std::string result;

    if (args.size() == 3)
    {
        if (args[1] == args[2])
        {
            std::string const pw = model_->calcPasswordHash(args[1]);
            model_->sendMessage("CHANGEACCOUNTPASS " + args[0] + " " + pw);
        }
        else
        {
            result = "new password mismatch";
        }
    }
    else
    {
        result = name_ + " requires three arguments";
    }

    return result;
}


std::string SC_join::description()
{
    return  "/" + name_ + " channel [pw] - "
            "join a channel";
}

std::string SC_join::process(std::vector<std::string> const& args)
{
    std::string result;

    if (!args.empty())
    {
        std::string msg = "JOIN " + args[0];
        if (args.size() > 1)
        {
            msg += " " + args[1];
        }
        model_->sendMessage(msg);
    }
    else
    {
        result = name_ + " requires one argument";
    }

    return result;
}
