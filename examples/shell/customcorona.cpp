/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "customcorona.h"
#include <KActionCollection>
#include <QAction>
#include <QDebug>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <Plasma/PluginLoader>

CustomCorona::CustomCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Shell"));
    // applications that want to load a plasma scene would have to load their own shell.. TODO: have a simple shell in plasma-framework for this purpose?
    package.setPath(QStringLiteral("org.kde.plasma.desktop"));
    setKPackage(package);

    qmlRegisterUncreatableType<PlasmaQuick::ContainmentView>("org.kde.plasma.shell",
                                                             2,
                                                             0,
                                                             "Desktop",
                                                             QStringLiteral("It is not possible to create objects of type Desktop"));

    m_view = new PlasmaQuick::ContainmentView(this);
    m_view->setSource(package.fileUrl("views", QStringLiteral("Desktop.qml")));
    m_view->show();

    load();
}

QRect CustomCorona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    // TODO?
    return QRect();
}

void CustomCorona::load()
{
    loadLayout(QStringLiteral("exampleplasmashell-appletsrc"));

    bool desktopFound = false;
    for (auto c : containments()) {
        if (c->containmentType() == Plasma::Types::DesktopContainment) {
            desktopFound = true;
            break;
        }
    }

    if (!desktopFound) {
        qDebug() << "Loading default layout";
        Plasma::Containment *c = createContainment(QStringLiteral("org.kde.desktopcontainment"));
        c->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
        saveLayout(QStringLiteral("exampleplasmashell-appletsrc"));
    }

    // don't let containments to be removed
    for (auto c : containments()) {
        if (c->containmentType() == Plasma::Types::DesktopContainment) {
            // example of a shell without a wallpaper
            c->setWallpaper(QStringLiteral("null"));
            m_view->setContainment(c);
            if (QAction *removeAction = c->actions()->action(QStringLiteral("remove"))) {
                removeAction->deleteLater();
            }
            break;
        }
    }
}

#include "moc_customcorona.cpp"
