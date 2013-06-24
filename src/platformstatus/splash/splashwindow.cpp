/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "splashwindow.h"
#include <utils/d_ptr_implementation.h>

#include <QGuiApplication>
#include <QScreen>
#include <QHash>
#include <QDebug>

#include <kwindoweffects.h>
#include <shell/lookandfeelpackage.h>

class SplashWindow::Private {
public:
    static
    QHash<QString, SplashWindow*> windows;

    LookAndFeelPackageStructure * packageStructure;
    Plasma::Package * package;
};

QHash<QString, SplashWindow*> SplashWindow::Private::windows;

SplashWindow::SplashWindow()
    : QQuickView(Q_NULLPTR)
{
    d->packageStructure = new LookAndFeelPackageStructure();
    d->package = new Plasma::Package(d->packageStructure);
    d->package->setPath(
        QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            d->package->defaultPackageRoot() + "org.kde.lookandfeel", QStandardPaths::LocateDirectory)
        );
    setSource(QUrl::fromLocalFile(d->package->filePath("splash", "PlatformChangeSplash.qml")));
    setResizeMode(QQuickView::SizeRootObjectToView);
}

SplashWindow::~SplashWindow()
{
}

void SplashWindow::splashScreen(QScreen * screen)
{
    // TODO: This could be a bit more like the screen locker
    auto & window = Private::windows[screen->name()];

    if (!window) {
        window = new SplashWindow();
    }

    window->setScreen(screen);
    window->setFlags(Qt::WindowStaysOnTopHint);
    window->showFullScreen();

    // TODO: Add some effect as well
}

void SplashWindow::splashAllScreens()
{
    foreach (auto screen, QGuiApplication::screens()) {
        splashScreen(screen);
    }
}

