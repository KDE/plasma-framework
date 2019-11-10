/******************************************************************************
*   Copyright 2014 Marco Martin <mart@kde.org>                                *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/
#ifndef DIALOGNATIVETEST_H
#define DIALOGNATIVETEST_H

#include <QtTest>
#include <QQuickView>
#include <QQuickItem>

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

