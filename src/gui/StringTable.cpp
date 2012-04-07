#include "StringTable.h"
#include "Prefs.h"

#include <FL/fl_draw.H>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>            // STL sort
#include <cassert>
#include <stdexcept>
#include <iostream> // TODO remove

// Prefs
static char const * PrefColWidth = "ColWidth";
static char const * PrefSortCol = "SortCol";
static char const * PrefSortReverse = "SortReverse";

StringTable::StringTable(int x, int y, int w, int h, std::string const & name, std::vector<std::string> const & headers) :
    Fl_Table_Row(x,y,w,h, name.c_str()),
    selectedRow_(-1),
    headers_(headers),
    prefs_(prefs, label())
{
    labeltype(FL_NO_LABEL);
    end();
    callback(event_callback, (void*)this);

    cols((int)headers_.size());

    color(FL_WHITE);
    selection_color(FL_YELLOW);
    col_header(1);
    col_resize(1);
    col_resize_min(10);
    type(Fl_Table_Row::SELECT_NONE);

    // setup column widths
    int const defaultColWidth = w/headers_.size();
    for (int c = 0; c < cols(); ++c)
    {
        std::ostringstream oss;
        oss << PrefColWidth << c;
        int val;
        prefs_.get(oss.str().c_str(), val, defaultColWidth);
        col_width(c, val);
    }

    // setup sorting
    prefs_.get(PrefSortCol, sort_lastcol_, 0);
    prefs_.get(PrefSortReverse, sort_reverse_, 0);
}

StringTable::~StringTable()
{
    // store column widths
    for (int c = 0; c < cols(); ++c)
    {
        std::ostringstream oss;
        oss << PrefColWidth << c;
        prefs_.set(oss.str().c_str(), col_width(c));
    }

    // store sorting prefs
    prefs_.set(PrefSortCol, sort_lastcol_);
    prefs_.set(PrefSortReverse, sort_reverse_);

}

void StringTable::sort()
{
    sort_column(sort_lastcol_, sort_reverse_);
}

// Sort a column up or down
void StringTable::sort_column(int col, int reverse)
{
    std::string id;
    if (selectedRow_ != -1)
    {
        assert(selectedRow_ >= 0 && selectedRow_ < rows());
        id = rows_[selectedRow_].id_;
    }
    std::stable_sort(rows_.begin(), rows_.end(), SortColumn(col, reverse));

    if (!id.empty())
    {
        selectRow(id);
    }
    redraw();
}

// Draw sort arrow
void StringTable::draw_sort_arrow(int X,int Y,int W,int H,int sort) {
    int xlft = X+(W-6)-8;
    int xctr = X+(W-6)-4;
    int xrit = X+(W-6)-0;
    int ytop = Y+(H/2)-4;
    int ybot = Y+(H/2)+4;
    if ( sort_reverse_ ) {
        // Engraved down arrow
        fl_color(FL_WHITE);
        fl_line(xrit, ytop, xctr, ybot);
        fl_color(41);                   // dark gray
        fl_line(xlft, ytop, xrit, ytop);
        fl_line(xlft, ytop, xctr, ybot);
    } else {
        // Engraved up arrow
        fl_color(FL_WHITE);
        fl_line(xrit, ybot, xctr, ytop);
        fl_line(xrit, ybot, xlft, ybot);
        fl_color(41);                   // dark gray
        fl_line(xlft, ybot, xctr, ytop);
    }
}

// Handle drawing all cells in table
void StringTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
    const char *s = ""; 
    if ( R < (int)rows_.size() && C < (int)rows_[R].data_.size() )
        s = rows_[R].data_[C].c_str();
    switch ( context ) {
        case CONTEXT_COL_HEADER:
            fl_push_clip(X,Y,W,H); {
                fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, FL_BACKGROUND_COLOR);
                if ( C < 9 ) {
                    fl_font(FL_HELVETICA_BOLD, 12);
                    fl_color(active_r() ? FL_BLACK : FL_INACTIVE_COLOR);
                    fl_draw(headers_[C].c_str(), X+2,Y,W,H, FL_ALIGN_LEFT, 0, 0);         // +2=pad left
                    // Draw sort arrow
                    if ( C == sort_lastcol_ ) {
                        draw_sort_arrow(X,Y,W,H, sort_reverse_);
                    }
                }
            }
            fl_pop_clip();
            return; 
        case CONTEXT_CELL: {
            fl_push_clip(X,Y,W,H); {
                // Bg color
                Fl_Color bgcolor = (selectedRow_ == R) ? selection_color() : FL_WHITE;
                // TODO remove Fl_Color bgcolor = row_selected(R) ? selection_color() : FL_WHITE;
                fl_color(bgcolor); fl_rectf(X,Y,W,H); 
                fl_font(FL_HELVETICA, 12);
                fl_color(FL_BLACK); fl_draw(s, X+2,Y,W,H, FL_ALIGN_LEFT);     // +2=pad left
                // Border
                fl_color(FL_LIGHT2); fl_line(X,Y+H-1, X+W, Y+H-1); // fl_rect(X,Y,W,H);
            }
            fl_pop_clip();
            return;
        }
        default:
            return;
    }
}

StringTableRow const & StringTable::getRow(std::size_t rowIndex)
{
    if (rowIndex < rows_.size())
    {
        return rows_[rowIndex];
    }
    throw std::runtime_error("rowIndex out of bounds");
}

void StringTable::addRow(const StringTableRow & row)
{
    assert(row.data_.size() == headers_.size());

    // check row does not exist
    for (StringTableRow const & r : rows_)
    {
        if (row.id_ == r.id_)
        {
            throw std::runtime_error("row already exist");
        }
    }

    rows_.push_back(row);
    rows( static_cast<int>(rows_.size()) );
    row_height(rows()-1, col_header_height()+2);

    // instant sort TODO
    sort();

    // TODO not needed, rows(n) will always redraw table
    //redraw_range(rows()-1, rows()-1, 0, cols());
}

void StringTable::updateRow(const StringTableRow & row)
{
    int i = 0;
    for (StringTableRow & r : rows_)
    {
        if (row.id_ == r.id_)
        {
            // only redraw if content changed
            if (r.data_ != row.data_)
            {
                r.data_ = row.data_;

                // instant sort TODO
                sort();

                //redraw_range(i, i, 0, cols());
            }
            return;
        }
        ++i;
    }

    // only reached if row not found
    throw std::runtime_error("row not found:" + row.id_);
}

void StringTable::removeRow(std::string const & id)
{
    int row = 0;
    for (std::vector<StringTableRow>::iterator it = rows_.begin(); it != rows_.end(); ++it)
    {
        if (id == it->id_)
        {
            if (selectedRow_ == row)
            {
                selectedRow_ = -1;
            }
            if (selectedRow_ > row)
            {
                selectedRow_ -= 1;
            }
            rows_.erase(it);
            rows(rows_.size());

            return;
        }
        ++row;
    }

    // only reached if row not found
    throw std::runtime_error("row not found:" + id);
}

void StringTable::clear()
{
    selectedRow_ = -1;
    rows_.clear();
    rows(0);
}

int StringTable::handle(int event)
{
    // calc rows per page, a bit ugly but good enough
    int const rowsPerPage = (h() - col_header_height()) / col_header_height();

    // make mousewheel scroll half a page
    if (event == FL_MOUSEWHEEL)
    {
        Fl::e_dy *= std::max(1, rowsPerPage/2);
    }

    switch (event)
    {
    case FL_FOCUS:
        return 1;

    case FL_KEYDOWN:
    {
        int const key = Fl::event_key();
        switch (key)
        {
        case FL_Up:
            selectRow(selectedRow_- 1);
            if (selectedRow_ < (toprow + 1))
            {
                row_position(selectedRow_ - 1);
            }
            return 1;

        case FL_Down:
            selectRow(selectedRow_ + 1);
            if (selectedRow_ > (botrow - 1))
            {
                row_position(toprow + (selectedRow_ + 1 - botrow));
            }
            return 1;

        case FL_Page_Up:
        {
            selectRow(selectedRow_ - rowsPerPage);
            if (selectedRow_ < (toprow + 1))
            {
                row_position(selectedRow_);
            }
            return 1;
        }
        case FL_Page_Down:
        {
            selectRow(selectedRow_ + rowsPerPage);
            if (selectedRow_ > (botrow - 1))
            {
                row_position(selectedRow_);
            }
            return 1;
        }
        case FL_Home:
            selectRow(0);
            row_position(selectedRow_);
            return 1;

        case FL_End:
            selectRow(rows());
            row_position(selectedRow_);
            return 1;
        default:
            return 0;
        }
    }
    break;

    default:
        return Fl_Table_Row::handle(event);
    }
}

// Callback whenever someone clicks on different parts of the table
void StringTable::event_callback(Fl_Widget*, void *data) {
    StringTable *o = static_cast<StringTable*>(data);
    o->event_callback2();
}

void StringTable::event_callback2()
{
    int ROW = callback_row();
    int COL = callback_col();
    TableContext context = callback_context();

    switch (context)
    {
    case CONTEXT_COL_HEADER: // someone clicked on column header
        if (Fl::event() == FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE)
        {
            if (sort_lastcol_ == COL)
            { // Click same column? Toggle sort
                sort_reverse_ ^= 1;
            }
            else
            { // Click diff column? Up sort
                sort_reverse_ = 0;
            }
            sort_column(COL, sort_reverse_);
            sort_lastcol_ = COL;
        }
        break;

    case CONTEXT_CELL:
        if (Fl::event() == FL_PUSH)
        {
            int const button = Fl::event_button();
            if (Fl::event_clicks())
            {
                // row will be selected before the double-click is seen, i.e. selectRow in else below has been done
                rowDoubleClickedSignal_(ROW, button);
            }
            else
            {
                selectRow(ROW);
                rowClickedSignal_(ROW, button);
            }
        }
        break;

    case CONTEXT_TABLE:
        if (Fl::event() == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE)
        {
            selectRow(-1);
        }
        break;

    default:
        return;
    }
}

void StringTable::selectRow(int rowIndex)
{
    // clamp rowIndex, will result in -1 if table is empty
    rowIndex = std::min(std::max(rowIndex, 0), rows()-1);

    if (rowIndex >= 0 && rowIndex < rows() && rowIndex != selectedRow_)
    {   // new selected row
        redraw_range(rowIndex, rowIndex, 0, cols());
    }

    if (rowIndex != selectedRow_)
    {
        if (selectedRow_ >= 0 && selectedRow_ < rows())
        {
            redraw_range(selectedRow_, selectedRow_, 0, cols());
        }
        selectedRow_ = rowIndex;
        selectedRowSignal_(selectedRow_);
    }
}

void StringTable::selectRow(std::string const & id)
{
    // do nothing if selectedRow_ is correct
    if (selectedRow_ != 0 && rows_[selectedRow_].id_ == id)
    {
        assert(selectedRow_ < rows());
        return;
    }

    int row = 0;
    for (std::vector<StringTableRow>::iterator it = rows_.begin(); it != rows_.end(); ++it)
    {
        if (id == it->id_)
        {
            selectedRow_ = row;
            return;
        }
        ++row;
    }

    // row not found
    selectedRow_ = -1;
}

StringTable::SortColumn::SortColumn(int col, int reverse)
{
    col_ = col;
    reverse_ = reverse;
}

bool StringTable::SortColumn::operator()(const StringTableRow &a, const StringTableRow &b)
{
    int const aCols = static_cast<int>(a.data_.size());
    int const bCols = static_cast<int>(b.data_.size());

    assert(col_ < aCols && col_ < bCols);

    // copy since we case convert and do case-insensitive comparison
    std::string as(a.data_[col_]);
    std::string bs(b.data_[col_]);
    boost::to_upper(as);
    boost::to_upper(bs);

    return ( reverse_ ? as > bs : as < bs );
}
