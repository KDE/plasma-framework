/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>

class DialogStatus : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Opening,
        Open,
        Closing,
        Closed,
    };
    Q_ENUM(Status)
};

class PageOrientation : public QObject
{
    Q_OBJECT

public:
    enum Orientation {
        Automatic,
        LockPortrait,
        LockLandscape,
        LockPrevious,
        Manual,
    };
    Q_ENUM(Orientation)
};

class PageStatus : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Inactive,
        Activating,
        Active,
        Deactivating,
    };
    Q_ENUM(Status)
};

#endif // ENUMS_H
