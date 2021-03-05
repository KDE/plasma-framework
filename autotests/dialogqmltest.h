/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DIALOGQMLTEST_H
#define DIALOGQMLTEST_H

#include <QTest>

#include "plasmaquick/dialog.h"

class DialogQmlTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadAndShow();

private:
    bool m_dialogShown;
};

#endif
