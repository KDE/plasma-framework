/******************************************************************************
*   Copyright 2016 David Rosca <nowrep@gmail.com>                             *
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

#pragma once

#include <QQuickItem>
#include <QQuickView>
#include <QtTest/QtTest>

namespace Plasma
{
class Svg;
class Theme;
}

class IconItemTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void loadPixmap();
    void loadImage();

    void invalidIcon();
    void usesPlasmaTheme();
    void animation();
    void animationAfterHide();
    void bug_359388();
    void loadSvg();
    void themeChange();
    void qiconFromTheme();
    void changeColorGroup();
    void animatingActiveChange();
    void animatingEnabledChange();
    void windowChanged();
    void paintedSize();
    void implicitSize();
    void nonSquareImplicitSize();
    void roundToIconSize();

private:
    QQuickItem *createIconItem();
    QImage grabImage(QQuickItem *item);
    Plasma::Svg *findPlasmaSvg(QQuickItem *item);
    void changeTheme(Plasma::Theme *theme, const QString &themeName);

    QQuickView *m_view;
};

