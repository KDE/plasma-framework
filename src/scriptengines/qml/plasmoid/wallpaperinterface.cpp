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
#include <kdeclarative/qmlobjectsharedengine.h>

#include <kactioncollection.h>
#include <kdesktopfile.h>
#include <KConfigLoader>

#include <QDebug>
#include <QQmlExpression>
#include <QQmlContext>
#include <QQmlProperty>

#include <Plasma/PluginLoader>
#include <kpackage/packageloader.h>

QHash<QObject *, WallpaperInterface *> WallpaperInterface::s_rootObjects = QHash<QObject *, WallpaperInterface *>();

WallpaperInterface::WallpaperInterface(ContainmentInterface *parent)
    : QQuickItem(parent),
      m_containmentInterface(parent),
      m_qmlObject(nullptr),
      m_configuration(nullptr),
      m_configLoader(nullptr)
{
    m_actions = new KActionCollection(this);

    //resize at the beginning to avoid as much resize events as possible
    if (parent) {
        setSize(QSizeF(parent->width(), parent->height()));
    }

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

QList<KPluginMetaData> WallpaperInterface::listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor)
{
    auto filter = [&mimetype, &formFactor](const KPluginMetaData &md) -> bool
    {
        if (!formFactor.isEmpty() && !md.value(QStringLiteral("X-Plasma-FormFactors")).contains(formFactor)) {
            return false;
        }
        return KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimetype);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Wallpaper"), QString(), filter);
}

KPackage::Package WallpaperInterface::kPackage() const
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
        const QString xmlPath = m_pkg.filePath("config", QStringLiteral("main.xml"));

        KConfigGroup cfg = m_containmentInterface->containment()->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");
        cfg = KConfigGroup(&cfg, m_wallpaperPlugin);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
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
        m_qmlObject = new KDeclarative::QmlObjectSharedEngine(this);
        s_rootObjects[m_qmlObject->engine()] = this;
        m_qmlObject->setInitializationDelayed(true);
        connect(m_qmlObject, &KDeclarative::QmlObject::finished, this, &WallpaperInterface::loadFinished);
    }

    m_actions->clear();
    m_pkg = Plasma::PluginLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper")).kPackage();
    m_pkg.setPath(m_wallpaperPlugin);
    if (!m_pkg.isValid()) {
        qWarning() << "Error loading the wallpaper, no valid package loaded";
        return;
    }

    if (m_configLoader) m_configLoader->deleteLater();
    if (m_configuration) m_configuration->deleteLater();
    m_configLoader = nullptr;
    m_configuration = nullptr;
    if (configScheme()) {
        m_configuration = new KDeclarative::ConfigPropertyMap(configScheme(), this);
    }

    m_qmlObject->rootContext()->setContextProperty(QStringLiteral("wallpaper"), this);
    m_qmlObject->setSource(m_pkg.fileUrl("mainscript"));

    const QString rootPath = m_pkg.metadata().value(QStringLiteral("X-Plasma-RootPath"));
    if (!rootPath.isEmpty()) {
        m_qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + rootPath);
    } else {
        m_qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + m_pkg.metadata().pluginId());
    }

    //initialize with our size to avoid as much resize events as possible
    QVariantHash props;
    props[QStringLiteral("width")] = width();
    props[QStringLiteral("height")] = height();
    m_qmlObject->completeInitialization(props);
}

void WallpaperInterface::loadFinished()
{
    if (m_qmlObject->mainComponent() &&
            m_qmlObject->rootObject() &&
            !m_qmlObject->mainComponent()->isError()) {
        m_qmlObject->rootObject()->setProperty("z", -1000);
        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), QStringLiteral("parent"));
        QQmlProperty prop(m_qmlObject->rootObject(), QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());

    } else if (m_qmlObject->mainComponent()) {
        qWarning() << "Error loading the wallpaper" << m_qmlObject->mainComponent()->errors();
        s_rootObjects.remove(m_qmlObject->engine());
        m_qmlObject->deleteLater();
        m_qmlObject = nullptr;

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
    return KPluginMetaData::readStringList(m_pkg.metadata().rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimetype);
}

void WallpaperInterface::setUrl(const QUrl &url)
{
    if (m_qmlObject->rootObject()) {
        QMetaObject::invokeMethod(m_qmlObject->rootObject(), "setUrl", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(url)));
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

        connect(action, &QAction::triggered, this, [this, name] {
            executeAction(name);
        });
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
        const QByteArray actionName("action_" + name.toUtf8());
        QMetaObject::invokeMethod(m_qmlObject->rootObject(), actionName.constData(), Qt::DirectConnection);
    }
}

WallpaperInterface * WallpaperInterface::qmlAttachedProperties(QObject* object)
{
    //at the moment of the attached object creation, the root item is the only one that hasn't a parent
    //only way to avoid creation of this attached for everybody but the root item
    return object->parent() ? nullptr : s_rootObjects.value(QtQml::qmlEngine(object));
}

#include "moc_wallpaperinterface.cpp"
