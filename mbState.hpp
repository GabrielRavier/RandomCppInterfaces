#pragma once
#include <cwchar>

namespace ravier
{

struct mbState
{
    mbstate_t state = { 0 };    // 0 is the initial conversion state

    bool representsInitialShiftState()
    {
        return mbsinit(&state) != 0;
    }
}

}