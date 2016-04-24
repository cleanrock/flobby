// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "ChatInput.h"
#include "Sound.h"

#include "FL/Fl.H"
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

ChatInput::ChatInput(int x, int y, int w, int h, size_t historySize)
    : Fl_Input(x, y, w, h)
    , historySize_(historySize)
    , pos_(-1)
    , lastCompletePos_(0)
    , lastPosAfterComplete_(0)
{
    box(FL_THIN_DOWN_BOX);
    callback(ChatInput::callbackText, this);
    when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
}

void ChatInput::callbackText(Fl_Widget * w, void * data)
{
    ChatInput * o = static_cast<ChatInput*>(data);
    o->onText();
}

void ChatInput::onText()
{
    std::string msg(value());
    if (!msg.empty())
    {
        pushHistory(msg);
        pos_ = -1;

        // split into multiple messages if newlines
        std::vector<std::string> lines;
        boost::algorithm::split(lines, msg, boost::is_any_of("\n"));
        for (auto& line : lines)
        {
            boost::replace_all(line, "\r", ""); //  remove all '\r'
            boost::replace_all(line, "\t", "    "); // replace tabs with four spaces
            if (!line.empty())
            {
                textSignal_(line);
            }
        }
    }

    // reset lastComplete on enter, probably not needed though
    lastCompleteMatch_.clear();

    value(0);
}

void ChatInput::pushHistory(std::string const& text)
{
    if (history_.empty() || text != history_.front()) // avoid adding duplicates
    {
        history_.push_front(text);
        if (history_.size() > historySize_)
        {
            history_.pop_back();
        }
    }
}

int ChatInput::handle(int event)
{
    switch (event)
    {
    case FL_KEYDOWN:
        return handleKeyDown();
    }
    return Fl_Input::handle(event);
}

int ChatInput::handleKeyDown()
{
    int const mods = Fl::event_state() & (FL_META|FL_CTRL|FL_ALT);

    switch (Fl::event_key())
    {
        case FL_Down:
            historyDown();
            return 1;

        case FL_Up:
            historyUp();
            return 1;

        case FL_Tab:
        {
            // do tab completion if unmodified tab and char before cursor is not space
            if (mods == 0 && position() > 0 && value()[position()-1] != ' ')
            {
                // use previous input if text and pos is same as after last match
                std::string ignore;
                std::string text = value();
                int pos = position();
                if (text == lastCompleteText_ && pos == lastPosAfterComplete_) {
                    text = lastCompleteTextBeforeCompletion_;
                    pos = lastCompletePos_;
                    ignore = lastCompleteMatch_;
                }

                CompleteResult result;
                completeSignal_(text, pos, ignore, result);
                lastCompleteMatch_ = result.match_;
                lastCompletePos_ = pos;
                if (!result.match_.empty())
                {
                    lastCompleteTextBeforeCompletion_ = text;

                    value(result.newText_.c_str());
                    position(result.newPos_);

                    lastCompleteText_ = value();
                    lastPosAfterComplete_ = position();
                }
                else
                {
                    // no match found
                    lastCompleteTextBeforeCompletion_.clear();
                    lastCompleteText_.clear();
                    lastPosAfterComplete_ = 0;
                    Sound::beep();
                }
                return 1;
            }
        }
        break;

        default:
            break;
    }

    // skip the Ctrl+[HIJLM] handling in FL_Input to not hide command shortcuts
    if (mods & FL_CTRL)
    {
        switch (Fl::event_key())
        {
        case 'h':
        case 'i':
        case 'j':
        case 'l':
        case 'm':
            return 0;
        }
    }

    return Fl_Input::handle(FL_KEYDOWN);
}

void ChatInput::historyDown()
{
    --pos_;
    if (pos_ >= 0 && pos_ < history_.size())
    {
        replace(0, size(), history_[pos_].c_str());
    }
    else
    {
        pos_ = -1;
        value(msg_.c_str());
    }
}

void ChatInput::historyUp()
{
    // save current line
    if (pos_ == -1)
    {
        msg_ = value();
    }

    ++pos_;

    if (pos_ >= history_.size())
    {
        pos_ = history_.size() - 1;
    }
    if (pos_ >= 0)
    {
        replace(0, size(), history_[pos_].c_str());
    }
}

