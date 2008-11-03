/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_CONTROL_SINGLETONDEFS_H
#define SOLID_CONTROL_SINGLETONDEFS_H

#include <kglobal.h>

class QObject;

namespace Solid
{
namespace Control
{
    /**
     * @internal
     */
    template<typename T>
    class SingletonHelper { public: T instance; };

#define SOLID_SINGLETON(Type)                                   \
public:                                                           \
    static Type &self();                                          \
    static Type &selfForceBackend(QObject *backend);            \
private:                                                          \
    friend class Solid::Control::SingletonHelper< Type >;

#define SOLID_SINGLETON_IMPLEMENTATION(Type, Name)              \
    K_GLOBAL_STATIC(Solid::Control::SingletonHelper< Type >, global##Name) \
                                                                  \
    Type &Type::self()                                            \
    {                                                             \
        Solid::Control::SingletonHelper< Type > *singleton = global##Name; \
                                                                  \
        return singleton->instance;                               \
    }
}
}

#endif
