#include "GameSettings.h"

#include "model/Model.h"

#include <boost/bind.hpp>

GameSettings::GameSettings(int x, int y, int w, int h, Model & model):
    StringTable(x, y, w, h, "GameSettings", {"setting", "value"}),
    model_(model)
{
//    connectRowClicked( boost::bind(&GameSettings::rowClicked, this, _1, _2) );
//    connectRowDoubleClicked( boost::bind(&GameSettings::rowDoubleClicked, this, _1, _2) );

    model_.connectSetScriptTag( boost::bind(&GameSettings::setScriptTag, this, _1, _2) );
    model_.connectRemoveScriptTag( boost::bind(&GameSettings::removeScriptTag, this, _1) );
}

void GameSettings::setScriptTag(std::string const & key, std::string const & value)
{
    StringTableRow row(key, { key, value });
    if (rowExist(key))
    {
        updateRow(row);
    }
    else
    {
        addRow(row);
    }
}

void GameSettings::removeScriptTag(std::string const & key)
{
    removeRow(key);
}

/* TODO
void GameSettings::rowClicked(int rowIndex, int button)
{
    StringTableRow const & row = getRow(static_cast<std::size_t>(rowIndex));
}

void GameSettings::rowDoubleClicked(int rowIndex, int button)
{
    StringTableRow const & row = getRow(static_cast<std::size_t>(rowIndex));
}
*/
