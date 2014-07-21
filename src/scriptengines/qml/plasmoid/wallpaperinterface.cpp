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

#include <kactioncollection.h>
#include <kservicetypetrader.h>
#include <kdesktopfile.h>
#include <KConfigLoader>

#include <QDebug>
#include <QQmlExpression>
#include <QQmlContext>
#include <QQmlProperty>
#include <QSignalMapper>

#include <Plasma/PluginLoader>

QHash<QObject *, WallpaperInterface *> WallpaperInterface::s_rootObjects = QHash<QObject *, WallpaperInterface *>();

WallpaperInterface::WallpaperInterface(ContainmentInterface *parent)
    : QQuickItem(parent),
      m_containmentInterface(parent),
      m_qmlObject(0),
      m_configuration(0),
      m_configLoader(0),
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
{
    if (m_qmlObject) {
        s_rootObjects.remove(m_qmlObject->engine());
    }
}

KPluginInfo::List WallpaperInterface::listWallpaperInfoForMimetype(const QString &mimetype, const QString &formFactor)
{
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    qDebug() << offers.count() << constraint;
    return KPluginInfo::fromServices(offers);
}

Plasma::Package WallpaperInterface::package() const
{
    return m_pkg;
}

QString WallpaperInterface::pluginName() const
{
    return m_wallpaperPlugin;
}

KDeclarative::ConfigPropertyMap *WallpaperInterface::configuration() const
{
    return m_configuration;
}

KConfigLoader *WallpaperInterface::configScheme()
{
    if (!m_configLoader) {
        //FIXME: do we need "mainconfigxml" in wallpaper packagestructures?
        const QString xmlPath = m_pkg.filePath("config", "main.xml");

        KConfigGroup cfg = m_containmentInterface->containment()->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, 0, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }

    return m_configLoader;
}

void WallpaperInterface::syncWallpaperPackage()
{
    if (m_wallpaperPlugin == m_containmentInterface->containment()->wallpaper() &&
            m_qmlObject->rootObject()) {
        return;
    }

    m_wallpaperPlugin = m_containmentInterface->containment()->wallpaper();

    if (!m_qmlObject) {
        m_qmlObject = new KDeclarative::QmlObject(this);
        s_rootObjects[m_qmlObject->engine()] = this;
        m_qmlObject->setInitializationDelayed(true);
    }

    m_actions->clear();
    m_pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Wallpaper");
    if (!m_pkg.isValid()) {
        qWarning() << "Error loading the wallpaper, no valid package loaded";
        return;
    }
    m_pkg.setPath(m_wallpaperPlugin);

    if (m_configLoader) m_configLoader->deleteLater();
    if (m_configuration) m_configuration->deleteLater();
    m_configLoader = 0;
    m_configuration = 0;
    if (configScheme()) {
        m_configuration = new KDeclarative::ConfigPropertyMap(configScheme(), this);
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
        s_rootObjects.remove(m_qmlObject->engine());
        m_qmlObject->deleteLater();
        m_qmlObject = 0;

    } else {
        qWarning() << "Error loading the wallpaper, package not found";
    }

    emit packageChanged();
    emit configurationChanged();
}

QList<QAction *> WallpaperInterface::contextualActions() const
{
    return m_actions->actions();
}

bool WallpaperInterface::supportsMimetype(const QString &mimetype) const
{
    //FIXME: a less brutal way? packages should have valid KService :/
    KDesktopFile desktop(m_pkg.path() + "/" + "metadata.desktop");
    return desktop.desktopGroup().readEntry<QStringList>("X-Plasma-DropMimeTypes", QStringList()).contains(mimetype);
}

void WallpaperInterface::setUrl(const QUrl &url)
{
    if (m_qmlObject->rootObject()) {
        QMetaObject::invokeMethod(m_qmlObject->rootObject(), QString("setUrl").toLatin1(), Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(url)));
    }
}

void WallpaperInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    QAction *action = m_actions->action(name);

    if (action) {
        action->setText(text);
    } else {
        Q_ASSERT(!m_actions->action(name));
        action = new QAction(text, this);
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
    setProperty("contextualActions", QVariant::fromValue(contextualActions()));
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
    setProperty("contextualActions", QVariant::fromValue(contextualActions()));
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
