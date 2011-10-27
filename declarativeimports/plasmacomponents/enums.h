/*
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>

#include <QtDeclarative/qdeclarative.h>

class DialogStatus : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)

public:
    enum Status {
        Opening,
        Open,
        Closing,
        Closed
    };
};

class PageOrientation : public QObject
{
    Q_OBJECT
    Q_ENUMS(Orientation)

public:
    enum Orientation {
        Automatic,
        LockPortrait,
        LockLandscape,
        LockPrevious,
        Manual
    };
};

class PageStatus : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)

public:
    enum Status {
        Inactive,
        Activating,
        Active,
        Deactivating
    };
};


#endif // ENUMS_H
