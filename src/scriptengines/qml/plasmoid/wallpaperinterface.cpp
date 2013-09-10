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
#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>

#include <KActionCollection>

#include <QDebug>
#include <QQmlExpression>
#include <QQmlContext>
#include <QQmlProperty>
#include <QSignalMapper>

#include <Plasma/ConfigLoader>
#include <Plasma/PluginLoader>

WallpaperInterface::WallpaperInterface(ContainmentInterface *parent)
    : QQuickItem(parent),
      m_containmentInterface(parent),
      m_qmlObject(0),
      m_configLoader(0),
      m_configuration(0),
      m_actionSignals(0)
{
    m_actions = new KActionCollection(this);

    if (!m_containmentInterface->containment()->wallpaper().isEmpty()) {
        syncWallpaperPackage();
    }
    connect(m_containmentInterface->containment(), &Plasma::Containment::wallpaperChanged,
            this, &WallpaperInterface::syncWallpaperPackage);

}

WallpaperInterface::~WallpaperInterface()
{}


Plasma::Package WallpaperInterface::package() const
{
    return m_pkg;
}

ConfigPropertyMap *WallpaperInterface::configuration() const
{
    return m_configuration;
}

Plasma::ConfigLoader *WallpaperInterface::configScheme()
{
    if (!m_configLoader) {
        //FIXME: do we need "mainconfigxml" in wallpaper packagestructures?
        const QString xmlPath = m_pkg.filePath("config", "main.xml");

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
    if (m_wallpaperPlugin == m_containmentInterface->containment()->wallpaper()) {
        return;
    }

    m_wallpaperPlugin = m_containmentInterface->containment()->wallpaper();
    
    if (!m_qmlObject) {
        m_qmlObject = new QmlObject(this);
        m_qmlObject->setInitializationDelayed(true);
    }

    m_actions->clear();
    m_pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/QmlWallpaper");
    m_pkg.setPath(m_wallpaperPlugin);

    m_configLoader->deleteLater();
    m_configuration->deleteLater();
    m_configLoader = 0;
    m_configuration = 0;
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
    emit configurationChanged();
}

QList<QAction*> WallpaperInterface::contextualActions() const
{
    return m_actions->actions();
}

void WallpaperInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    QAction *action = m_actions->action(name);

    if (action) {
        action->setText(text);
    } else {
        action = new QAction(text, this);
        m_actions->addAction(name, action);

        Q_ASSERT(!m_actions->actions().contains(name));
        m_actions->addAction(name, action);

        if (!m_actionSignals) {
            m_actionSignals = new QSignalMapper(this);
            connect(m_actionSignals, SIGNAL(mapped(QString)),
                    this, SLOT(executeAction(QString)));
        }

        connect(action, SIGNAL(triggered()), m_actionSignals, SLOT(map()));
        m_actionSignals->setMapping(action, name);
    }

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }

    action->setObjectName(name);
}

void WallpaperInterface::removeAction(const QString &name)
{
    QAction *action = m_actions->action(name);

    if (action) {
        if (m_actionSignals) {
            m_actionSignals->removeMappings(action);
        }
        m_actions->removeAction(action);

        delete action;
    }
}

QAction *WallpaperInterface::action(QString name) const
{
    return m_actions->action(name);
}

void WallpaperInterface::executeAction(const QString &name)
{
    if (m_qmlObject->rootObject()) {
         QMetaObject::invokeMethod(m_qmlObject->rootObject(), QString("action_" + name).toLatin1(), Qt::DirectConnection);
    }
}

#include "moc_wallpaperinterface.cpp"
