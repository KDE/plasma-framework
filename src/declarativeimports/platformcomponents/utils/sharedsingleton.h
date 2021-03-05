/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHARED_SINGLETON_P_H
#define SHARED_SINGLETON_P_H

#include <memory>

namespace utils
{
template<typename Target>
class SharedSingleton
{
public:
    static std::shared_ptr<Target> instance()
    {
        // TODO: Make this thread safe

        auto ptr = s_instance.lock();

        if (!ptr) {
            ptr = std::make_shared<Target>();
            s_instance = ptr;
        }

        return ptr;
    }

    operator bool() const
    {
        return !s_instance.expired();
    }

private:
    static std::weak_ptr<Target> s_instance;
};

template<typename Target>
std::weak_ptr<Target> SharedSingleton<Target>::s_instance;

} // namespace utils

#endif /* SHARED_SINGLETON_P_H */
