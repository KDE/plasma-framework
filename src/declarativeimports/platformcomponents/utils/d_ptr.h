/*
    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef D_PTR_H
#define D_PTR_H

#include <memory>

namespace utils
{
template<typename T>
class d_ptr
{
private:
    std::unique_ptr<T> d;

public:
    d_ptr();

    template<typename... Args>
    d_ptr(Args &&...);

    ~d_ptr();

    T *operator->() const;
};

/* clang-format off */
#define D_PTR \
    class Private; \
    friend class Private; \
    const ::utils::d_ptr<Private> d
/* clang-format on */

} // namespace utils

#endif
