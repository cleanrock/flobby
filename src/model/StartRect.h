#pragma once

class StartRect
{
public:
    StartRect(int ally, int left, int top, int right, int bottom);
    ~StartRect();

    int ally() const { return ally_; }
    float left() const { return left_; }
    float top() const { return top_; }
    float right() const { return right_; }
    float bottom() const { return bottom_; }

private:
    int ally_;
    float left_;
    float top_;
    float right_;
    float bottom_;

};
