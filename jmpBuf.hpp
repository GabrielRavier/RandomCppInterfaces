#pragma once
#include <csetjmp>

namespace ravier
{

struct jumpBuffer
{
    jmp_buf jumpBuffer;

    int set()
    {
        return setjmp(jumpBuffer);
    }

    [[noreturn]] void jump(int status)
    {
        longjmp(jumpBuffer, status);
    }
};

}