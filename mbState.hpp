#pragma once
#include <cwchar>
#include <cstring>

namespace ravier
{

struct mbState
{
    mbstate_t state;

    mbState()
    {
        this->setInitial();    // Initialize mbstate_t properly
    }

    void setInitial()
    {
        if (!this->representsInitialShiftState())
            memset(&state, '\0', sizeof(state));
    }

    bool representsInitialShiftState()
    {
        return mbsinit(&state) != 0;
    }

    size_t nextCharacterSize(const char *str, size_t maxBytes)
    {
        return mbrlen(str, maxBytes, &state);
    }

    size_t multiCharToWideChar(wchar_t *destination, const char *source, size_t limit)
    {
        return mbrtowc(destination, source, limit, &state);
    }

    size_t wideCharToMultiChar(char *destination, wchar_t charToConvert)
    {
        return wcrtomb(destination, charToConvert, &state);
    }

    size_t multiCharCStringToWideCharCString(wchar_t *destination, const char **source, size_t limit)
    {
        return mbsrtowcs(destination, source, limit, &state);
    }

    size_t wideCharCStringToMultiCharCString(char *destination, const wchar_t **source, size_t limit)
    {
        return wcsrtombs(destination, source, limit, &state);
    }
};

}