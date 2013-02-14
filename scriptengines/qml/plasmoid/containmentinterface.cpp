/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "containmentinterface.h"

#include <QQmlExpression>
#include <QQmlProperty>

#include <KDebug>

#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include "declarative/qmlobject.h"

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent)
    : AppletInterface(parent)
{
    qmlRegisterType<ContainmentInterface>();

    connect(containment(), SIGNAL(appletRemoved(Plasma::Applet *)), this, SLOT(appletRemovedForward(Plasma::Applet *)));
    connect(containment(), SIGNAL(appletAdded(Plasma::Applet *, const QPointF &)), this, SLOT(appletAddedForward(Plasma::Applet *, const QPointF &)));
    connect(containment(), SIGNAL(screenChanged(int, int, Plasma::Containment*)), this, SIGNAL(screenChanged()));
    connect(containment(), SIGNAL(activityChanged()), this, SIGNAL(activityChanged()));
    connect(containment(), SIGNAL(wallpaperChanged()), this, SLOT(loadWallpaper()));

     if (containment()->corona()) {
         connect(containment()->corona(), SIGNAL(availableScreenRegionChanged()),
                 this, SIGNAL(availableScreenRegionChanged()));
     }
     loadWallpaper();
}

QVariantList ContainmentInterface::applets()
{
    QVariantList list;
    int i = 0;
    foreach (Plasma::Applet *applet, containment()->applets()) {
        list << QVariant::fromValue(applet);
        ++i;
    }
    return list;
}

void ContainmentInterface::setDrawWallpaper(bool drawWallpaper)
{
    if (drawWallpaper == m_appletScriptEngine->drawWallpaper()) {
        return;
    }

    m_appletScriptEngine->setDrawWallpaper(drawWallpaper);

    loadWallpaper();
}

bool ContainmentInterface::drawWallpaper()
{
    return m_appletScriptEngine->drawWallpaper();
}

ContainmentInterface::Type ContainmentInterface::containmentType() const
{
    return (ContainmentInterface::Type)m_appletScriptEngine->containmentType();
}

void ContainmentInterface::setContainmentType(ContainmentInterface::Type type)
{
    m_appletScriptEngine->setContainmentType((Plasma::Containment::Type)type);
}

int ContainmentInterface::screen() const
{
    return containment()->screen();
}

QRectF ContainmentInterface::screenGeometry(int id) const
{
    QRectF rect;
    if (containment()->corona()) {
        rect = QRectF(containment()->corona()->screenGeometry(id));
    }

    return rect;
}

QVariantList ContainmentInterface::availableScreenRegion(int id) const
{
    QRegion reg;
    if (containment()->corona()) {
        reg = containment()->corona()->availableScreenRegion(id);
    }

    QVariantList regVal;
    foreach (QRect rect, reg.rects()) {
        regVal << QVariant::fromValue(QRectF(rect));
    }
    return regVal;
}

void ContainmentInterface::appletAddedForward(Plasma::Applet *applet, const QPointF &pos)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    QObject *contGraphicObject = containment()->property("graphicObject").value<QObject *>();

    qDebug() << "Applet added:" << applet << applet->title() << appletGraphicObject;

    if (applet && contGraphicObject && appletGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));

    //if an appletGraphicObject is not set, we have to display some error message
    } else if (applet && contGraphicObject) {
        QQmlComponent *component = new QQmlComponent(m_appletScriptEngine->engine(), applet);
        component->loadUrl(QUrl::fromLocalFile(containment()->corona()->package().filePath("ui", "AppletError.qml")));
        QObject *errorUi = component->create();

        if (errorUi) {
            errorUi->setProperty("visible", false);
            errorUi->setProperty("parent", QVariant::fromValue(contGraphicObject));
            errorUi->setProperty("reason", applet->launchErrorMessage());
            appletGraphicObject = errorUi;
        }
    }

    emit appletAdded(appletGraphicObject, pos);
}

void ContainmentInterface::appletRemovedForward(Plasma::Applet *applet)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    emit appletRemoved(appletGraphicObject);
}

void ContainmentInterface::loadWallpaper()
{
    if (m_appletScriptEngine->drawWallpaper()) {
        if (m_wallpaperQmlObject) {
            return;
        }

        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        pkg.setPath("org.kde.wallpaper.image");

        m_wallpaperQmlObject = new QmlObject(this);
        m_wallpaperQmlObject->setQmlPath(pkg.filePath("mainscript"));

        if (m_wallpaperQmlObject->mainComponent() &&
            m_wallpaperQmlObject->rootObject() &&
            !m_wallpaperQmlObject->mainComponent()->isError()) {
            m_wallpaperQmlObject->rootObject()->setProperty("z", -1000);
            m_wallpaperQmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

            //set anchors
            QQmlExpression expr(m_appletScriptEngine->engine()->rootContext(), m_wallpaperQmlObject->rootObject(), "parent");
            QQmlProperty prop(m_wallpaperQmlObject->rootObject(), "anchors.fill");
            prop.write(expr.evaluate());

        } else if (m_wallpaperQmlObject->mainComponent()) {
            qWarning() << "Error loading the wallpaper" << m_wallpaperQmlObject->mainComponent()->errors();
            m_wallpaperQmlObject->deleteLater();
            m_wallpaperQmlObject = 0;

        } else {
            qWarning() << "Error loading the wallpaper, package not found";
        }

    } else {
        m_wallpaperQmlObject->deleteLater();
        m_wallpaperQmlObject = 0;
    }
}

QString ContainmentInterface::activityId() const
{
    return containment()->activity();
}

#include "moc_containmentinterface.cpp"
