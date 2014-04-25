/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHARED_SINGLETON_P_H
#define SHARED_SINGLETON_P_H

#include <memory>

namespace utils
{

template <typename Target>
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

template <typename Target>
std::weak_ptr<Target> SharedSingleton<Target>::s_instance;

} // namespace utils

#endif /* SHARED_SINGLETON_P_H */

