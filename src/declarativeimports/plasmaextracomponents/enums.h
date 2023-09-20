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

#endif // ENUMS_H
