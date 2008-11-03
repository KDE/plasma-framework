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

#ifndef SOLID_SOLIDDEFS_P_H
#define SOLID_SOLIDDEFS_P_H

#include <QObject>



#define return_SOLID_CALL(Type, Object, Default, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=0) \
    { \
         return t->Method; \
    } \
    else \
    { \
         return Default; \
    }



#define SOLID_CALL(Type, Object, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=0) \
    { \
         t->Method; \
    }



#endif
