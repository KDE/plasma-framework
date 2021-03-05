/*
    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef D_PTR_IMPLEMENTATION_H
#define D_PTR_IMPLEMENTATION_H

#include <utility>

namespace utils
{
template<typename T>
d_ptr<T>::d_ptr()
    : d(new T())
{
}

template<typename T>
template<typename... Args>
d_ptr<T>::d_ptr(Args &&...args)
    : d(new T(std::forward<Args>(args)...))
{
}

template<typename T>
d_ptr<T>::~d_ptr()
{
}

template<typename T>
T *d_ptr<T>::operator->() const
{
    return d.get();
}

} // namespace utils

#endif
