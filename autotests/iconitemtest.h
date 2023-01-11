/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>
#include <QQuickView>
#include <QTest>

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
    QImage waitAndGrabImage(QQuickItem *item, int delay = 50);
    Plasma::Svg *findPlasmaSvg(QQuickItem *item);
    void changeTheme(Plasma::Theme *theme, const QString &themeName);

    QQuickView *m_view;
};
