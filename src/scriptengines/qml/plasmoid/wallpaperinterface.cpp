/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#include "wallpaperinterface.h"

#include "containmentinterface.h"
#include "declarative/configpropertymap.h"
#include "declarative/qmlobject.h"

#include <QDebug>
#include <QQmlExpression>
#include <QQmlContext>
#include <QQmlProperty>

#include <Plasma/ConfigLoader>
#include <Plasma/PluginLoader>

WallpaperInterface::WallpaperInterface(ContainmentInterface *parent)
    : QQuickItem(parent),
      m_containmentInterface(parent),
      m_qmlObject(0),
      m_configLoader(0)
{
    connect(m_containmentInterface->containment(), &Plasma::Containment::wallpaperChanged,
            this, &WallpaperInterface::syncWallpaperPackage);

    syncWallpaperPackage();
}

WallpaperInterface::~WallpaperInterface()
{}


Plasma::Package WallpaperInterface::package() const
{
    return m_pkg;
}

QObject* WallpaperInterface::configuration() const
{
    return m_configuration;
}

Plasma::ConfigLoader *WallpaperInterface::configScheme()
{
    if (!m_configLoader) {
        const QString xmlPath = m_pkg.filePath("mainconfigxml");

        KConfigGroup cfg = m_containmentInterface->containment()->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");

        if (xmlPath.isEmpty()) {
            m_configLoader = new Plasma::ConfigLoader(&cfg, 0);
        } else {
            QFile file(xmlPath);
            m_configLoader = new Plasma::ConfigLoader(&cfg, &file);
        }
    }

    return m_configLoader;
}

void WallpaperInterface::syncWallpaperPackage()
{
    if (!m_qmlObject) {
        m_qmlObject = new QmlObject(this);
        m_qmlObject->setInitializationDelayed(true);
    }

    m_pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_pkg.setDefaultPackageRoot("plasma/wallpapers");
    m_pkg.setPath(m_containmentInterface->containment()->wallpaper());

    if (configScheme()) {
        m_configuration = new ConfigPropertyMap(configScheme(), this);
    }

    m_qmlObject->setSource(QUrl::fromLocalFile(m_pkg.filePath("mainscript")));
    m_qmlObject->engine()->rootContext()->setContextProperty("wallpaper", this);
    m_qmlObject->completeInitialization();

    if (m_qmlObject->mainComponent() &&
        m_qmlObject->rootObject() &&
        !m_qmlObject->mainComponent()->isError()) {
        m_qmlObject->rootObject()->setProperty("z", -1000);
        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "parent");
        QQmlProperty prop(m_qmlObject->rootObject(), "anchors.fill");
        prop.write(expr.evaluate());

    } else if (m_qmlObject->mainComponent()) {
        qWarning() << "Error loading the wallpaper" << m_qmlObject->mainComponent()->errors();
        m_qmlObject->deleteLater();
        m_qmlObject = 0;

    } else {
        qWarning() << "Error loading the wallpaper, package not found";
    }

    emit packageChanged();
}

#include "moc_wallpaperinterface.cpp"
