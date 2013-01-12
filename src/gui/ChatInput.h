#pragma ocne

#include <FL/Fl_Input.H>
#include <boost/signal.hpp>
#include <deque>
#include <string>

class ChatInput: public Fl_Input
{
public:
    ChatInput(int x, int y, int w, int h, size_t historySize = 10);

    // signals
    typedef boost::signal<void (std::string const & text)> TextSignal;
    boost::signals::connection connectText(TextSignal::slot_type subscriber)
    { return textSignal_.connect(subscriber); }

    // complete signal is only fired if cursor is at end of line and last char is a non-space
    typedef boost::signal<void (std::string const & text, std::string & result)> CompleteSignal;
    boost::signals::connection connectComplete(CompleteSignal::slot_type subscriber)
    { assert(completeSignal_.num_slots() == 0); return completeSignal_.connect(subscriber); }

private:
    size_t historySize_;
    std::deque<std::string> history_;
    int pos_;

    TextSignal textSignal_;
    CompleteSignal completeSignal_;

    static void callbackText(Fl_Widget * w, void * data);

    void onText();
    int handle(int event);
    int handleKeyDown();
    void historyDown();
    void historyUp();

};
