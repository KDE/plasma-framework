/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <qactiongroup.h>

class ExclusiveGroup : public QActionGroup {
    Q_OBJECT

public:
    ExclusiveGroup(QObject* parent = nullptr) : QActionGroup(parent)
    {
        setExclusive(true);
    }
};
