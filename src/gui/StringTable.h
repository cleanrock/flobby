// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Preferences.H>

#include <boost/signals2/signal.hpp>
#include <string>
#include <vector>
#include <array>

struct StringTableColumnDef
{
    std::string name_;
    int defaultWidth_;
    StringTableColumnDef(const std::string& name, int defaultWidth):
        name_(name),
        defaultWidth_(defaultWidth)
    {
    }
};

struct StringTableRow
{
    StringTableRow(std::string const & id, std::vector<std::string> const & data): id_(id), data_(data) {}

    std::string id_;
    std::vector<std::string> data_;

    bool operator==(StringTableRow const & other)
    {
        return (id_ == other.id_);
    }
    bool operator==(std::string const & id)
    {
        return (id == id_);
    }
};

class StringTable : public Fl_Table_Row
{
public:
    StringTable(int x, int y, int w, int h,
            std::string const & name,
            std::vector<StringTableColumnDef> const& headers,
            int defaultSortColumn,
            bool savePrefs = true);
    virtual ~StringTable();

    // signals
    //
    typedef boost::signals2::signal<void (int rowIndex)> SelectedRowChangedSignal;
    boost::signals2::connection connectSelectedRowChanged(SelectedRowChangedSignal::slot_type subscriber) { return selectedRowSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (int rowIndex, int button)> RowClickedSignal; // button is FL_LEFT/MIDDLE/RIGHT_MOUSE (1,2,3)
    boost::signals2::connection connectRowClicked(RowClickedSignal::slot_type subscriber) { return rowClickedSignal_.connect(subscriber); }

    typedef boost::signals2::signal<void (int rowIndex, int button)> RowDoubleClickedSignal;
    boost::signals2::connection connectRowDoubleClicked(RowDoubleClickedSignal::slot_type subscriber) { return rowDoubleClickedSignal_.connect(subscriber); }

    StringTableRow const & getRow(std::size_t rowIndex);
    void addRow(StringTableRow const & row);
    void updateRow(StringTableRow const & row);
    void removeRow(std::string const & id);
    bool rowExist(std::string const & id);
    void sort();
    void clear();

protected:
    std::vector<StringTableRow> rows_;
    int selectedRow_;
    int handle(int event);
    void selectRow(int rowIndex);
    void selectRow(std::string const & id); // used to restore selectedRow_ after sort

private:
    void draw_cell(TableContext context, int R=0, int C=0,      // table cell drawing
                   int X=0, int Y=0, int W=0, int H=0);

    Fl_Color fltkColor(std::string const& text); // text is spring color, e.g 0xBBGGRR as int

    void sort_column(int col, int reverse=0);                   // sort table by a column
    void draw_sort_arrow(int X,int Y,int W,int H,int sort);
    void savePrefs();


    struct SortColumn
    {
        SortColumn(int col, int reverse);
        bool operator()(const StringTableRow &a, const StringTableRow &b);
        int col_, reverse_;
    };

    std::vector<StringTableColumnDef> headers_;
    int sort_reverse_;
    int sort_lastcol_;
    Fl_Preferences prefs_;
    bool savePrefs_;

    static void event_callback(Fl_Widget*, void*);
    void event_callback2();

    SelectedRowChangedSignal selectedRowSignal_;
    RowClickedSignal rowClickedSignal_;
    RowDoubleClickedSignal rowDoubleClickedSignal_;
};
