// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma ocne

#include <FL/Fl_Input.H>
#include <boost/signals2/signal.hpp>
#include <deque>
#include <string>

class ChatInput: public Fl_Input
{
public:
    ChatInput(int x, int y, int w, int h, size_t historySize = 10);

    // signals
    typedef boost::signals2::signal<void (std::string const & text)> TextSignal;
    boost::signals2::connection connectText(TextSignal::slot_type subscriber)
    { return textSignal_.connect(subscriber); }

    // result contain completed word and new insert position, completed word is unchanged if not found
    typedef boost::signals2::signal<void (std::string const & text, std::size_t pos, std::pair<std::string, std::size_t>& result)> CompleteSignal;
    boost::signals2::connection connectComplete(CompleteSignal::slot_type subscriber)
    { assert(completeSignal_.num_slots() == 0); return completeSignal_.connect(subscriber); }

private:
    size_t historySize_;
    std::deque<std::string> history_;
    int pos_;
    std::string msg_; // stores unsent message when browsing history

    TextSignal textSignal_;
    CompleteSignal completeSignal_;

    static void callbackText(Fl_Widget * w, void * data);

    void onText();
    int handle(int event);
    int handleKeyDown();
    void pushHistory(std::string const& text);
    void historyDown();
    void historyUp();

};
