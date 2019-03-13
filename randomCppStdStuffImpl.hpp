#pragma once

#include <type_traits>

namespace ravier
{

// Random C++20 stuff
template<class T> struct type_identity
{
    using type = T;
};

template<class T> struct remove_cvref
{
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<class inputIt, class T> T accumulate(inputIt first, inputIt last, T init)
{
    for (; first != last; ++first)
        init = std::move(init) + *first; // std::move since C++20
    return init;
}

template<class inputIt, class T, class binOp> T accumulate(inputIt first, inputIt last, T init, binOp op)
{
    for (; first != last; ++first)
        init = op(std::move(init), *first); // std::move since C++20
    return init;
}

}