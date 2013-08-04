#pragma once

#include <cstdint>

// UserId is used for generating the userID sent in LOGIN message

class UserId
{
public:
    static uint32_t get();

private:
    UserId() {}
};
