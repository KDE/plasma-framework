/*
 * Copyright 2015 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software desktopFoundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "customcorona.h"
#include <QDebug>
#include <QAction>
#include <KActionCollection>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <Plasma/PluginLoader>

CustomCorona::CustomCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Shell"));
    //applications that want to load a plasma scene would have to load their own shell.. TODO: have a simple shell in plasma-framework for this purpose?
    package.setPath(QStringLiteral("org.kde.plasma.desktop"));
    setKPackage(package);

    qmlRegisterUncreatableType<PlasmaQuick::ContainmentView>("org.kde.plasma.shell", 2, 0, "Desktop", QStringLiteral("It is not possible to create objects of type Desktop"));

    m_view = new PlasmaQuick::ContainmentView(this);
    m_view->setSource(package.fileUrl("views", QStringLiteral("Desktop.qml")));
    m_view->show();
    
    load();
}



QRect CustomCorona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    //TODO?
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
        c->createApplet("org.kde.plasma.analogclock");
        saveLayout(QStringLiteral("exampleplasmashell-appletsrc"));
    }

    //don't let containments to be removed
    for (auto c : containments()) {
        if (c->containmentType() == Plasma::Types::DesktopContainment) {
            //example of a shell without a wallpaper
            c->setWallpaper("null");
            m_view->setContainment(c);
            QAction *removeAction = c->actions()->action(QStringLiteral("remove"));
            if(removeAction) {
                removeAction->deleteLater();
            }
            break;
        }
    }
}

#include "moc_customcorona.cpp"
