/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DIALOGNATIVETEST_H
#define DIALOGNATIVETEST_H

#include <QQuickItem>
#include <QQuickView>
#include <QTest>

#include "plasmaquick/dialog.h"

class DialogNativeTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void size();
    void position();

private:
    QQuickView *m_panel;
    QQuickView *m_panel2;
    QQuickView *m_panel3;
    QQuickItem *m_content;
    QQuickItem *m_content2;
    PlasmaQuick::Dialog *m_dialog;
    QDir m_cacheDir;
};

#endif
