#pragma once
#include <cwchar>
#include <memory>

namespace ravier
{

struct wideCString
{
    std::unique_ptr<wchar_t []> pStr;
    size_t size;

    void makeCopy(const wideCString& src)
    {
        size_t srcLen = src.size;
        if (this->size < srcLen)
        {
            this->size = srcLen;
            pStr = std::make_unique<wchar_t []>(this->size);
        }
        this->unsafeCopy(src);
    }

    void unsafeCopy(const wideCString& src) const
    {
        wcscpy(this->pStr.get(), src.pStr.get());
    }

    void unsafeCopyN(const wideCString& src, size_t count) const
    {
        wcsncpy(this->pStr.get(), src.pStr.get(), count);
    }

    void unsafeAppend(const wideCString& src) const
    {
        wcscat(this->pStr.get(), src.pStr.get());
    }

    void unsafeAppendN(const wideCString& src, size_t count) const
    {
        wcsncat(this->pStr.get(), src.pStr.get(), count);
    }

    size_t len() const
    {
        return wcslen(this->pStr.get());
    }

    size_t unsafeXfrm(const wideCString& src, size_t count) const
    {
        wcsxfrm(this->pStr.get(), src.pStr.get(), count);
    }

    void makeXfrm(const wideCString& src)
    {
        size_t reqLen = wcsxfrm(nullptr, src.pStr.get(), 0);
        if (this->size < reqLen)
        {
            this->size = reqLen;
            this->pStr = std::make_unique<wchar_t []>(this->size);
        }
        this->unsafeXfrm(src, this->size);
    }

    int compare(const wideCString& other) const
    {
        return wcscmp(this->pStr.get(), other.pStr.get());
    }

    int compareN(const wideCString& other, size_t count) const
    {
        return wcsncmp(this->pStr.get(), other.pStr.get(), count);
    }

    int localeCompare(const wideCString& other) const
    {
        return wcscoll(this->pStr.get(), other.pStr.get());
    }
};

}