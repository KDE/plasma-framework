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

#include "appletinterface.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QSignalMapper>
#include <QTimer>

#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KService>
#include <KServiceTypeTrader>

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>

Q_DECLARE_METATYPE(AppletInterface*)

AppletInterface::AppletInterface(DeclarativeAppletScript *script, QQuickItem *parent)
    : QQuickItem(parent),
      m_appletScriptEngine(script),
      m_actionSignals(0),
      m_backgroundHints(Plasma::StandardBackground),
      m_busy(false),
      m_expanded(false)
{
    qmlRegisterType<AppletInterface>();
    connect(this, SIGNAL(releaseVisualFocus()), applet(), SIGNAL(releaseVisualFocus()));
    connect(this, SIGNAL(configNeedsSaving()), applet(), SIGNAL(configNeedsSaving()));
    connect(applet(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SIGNAL(immutableChanged()));
    connect(applet(), SIGNAL(statusChanged(Plasma::ItemStatus)), this, SIGNAL(statusChanged()));
    connect(m_appletScriptEngine, SIGNAL(formFactorChanged()),
            this, SIGNAL(formFactorChanged()));
    connect(m_appletScriptEngine, SIGNAL(locationChanged()),
            this, SIGNAL(locationChanged()));
    connect(m_appletScriptEngine, SIGNAL(contextChanged()),
            this, SIGNAL(contextChanged()));
}

AppletInterface::~AppletInterface()
{
}

void AppletInterface::setUiObject(QObject *object)
{
    if (m_uiObject.data() == object) {
        return;
    }

    m_uiObject = object;

    //set parent, both as object hyerarchy and visually
    object->setProperty("parent", QVariant::fromValue(this));

    //set anchors
    QQmlExpression expr(m_appletScriptEngine->engine()->rootContext(), object, "parent");
    QQmlProperty prop(object, "anchors.fill");
    prop.write(expr.evaluate());
}

QObject *AppletInterface::uiObject() const
{
    return m_uiObject.data();
}

AppletInterface::FormFactor AppletInterface::formFactor() const
{
    return static_cast<FormFactor>(applet()->formFactor());
}

AppletInterface::Location AppletInterface::location() const
{
    return static_cast<Location>(applet()->location());
}

QString AppletInterface::currentActivity() const
{
    return applet()->containment()->activity();
}

bool AppletInterface::isBusy() const
{
    return m_busy;
}

void AppletInterface::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

bool AppletInterface::isExpanded() const
{
    return m_expanded;
}

void AppletInterface::setExpanded(bool expanded)
{
    //if there is no compact representation it means it's always expanded
    //Containnments are always expanded
    if (!m_compactUiObject || qobject_cast<ContainmentInterface *>(this) || m_expanded == expanded) {
        return;
    }

    m_expanded = expanded;
    emit expandedChanged();
}

AppletInterface::BackgroundHints AppletInterface::backgroundHints() const
{
    return (BackgroundHints)m_backgroundHints;
}

void AppletInterface::setBackgroundHints(BackgroundHints hint)
{
    if (m_backgroundHints == (Plasma::BackgroundHints)hint) {
        return;
    }

    m_backgroundHints = (Plasma::BackgroundHints)hint;
    emit backgroundHintsChanged();
}

void AppletInterface::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    m_appletScriptEngine->setConfigurationRequired(needsConfiguring, reason);
}

QString AppletInterface::activeConfig() const
{
    return m_currentConfig.isEmpty() ? "main" : m_currentConfig;
}

void AppletInterface::setActiveConfig(const QString &name)
{
    if (name == "main") {
        m_currentConfig.clear();
        return;
    }

    Plasma::ConfigLoader *loader = m_configs.value(name, 0);

    if (!loader) {
        QString path = m_appletScriptEngine->filePath("config", name + ".xml");
        if (path.isEmpty()) {
            return;
        }

        QFile f(path);
        KConfigGroup cg = applet()->config();
        loader = new Plasma::ConfigLoader(&cg, &f, this);
        m_configs.insert(name, loader);
    }

    m_currentConfig = name;
}

void AppletInterface::writeConfig(const QString &entry, const QVariant &value)
{
    Plasma::ConfigLoader *config = 0;
    if (m_currentConfig.isEmpty()) {
        config = applet()->configScheme();
    } else {
        config = m_configs.value(m_currentConfig, 0);
    }

    if (config) {
        KConfigSkeletonItem *item = config->findItemByName(entry);
        if (item) {
            item->setProperty(value);
            config->blockSignals(true);
            config->writeConfig();
            config->blockSignals(false);
            m_appletScriptEngine->configNeedsSaving();
        }
    } else
        kWarning() << "Couldn't find a configuration entry";
}

QVariant AppletInterface::readConfig(const QString &entry) const
{
    Plasma::ConfigLoader *config = 0;
    QVariant result;

    if (m_currentConfig.isEmpty()) {
        config = applet()->configScheme();
    } else {
        config = m_configs.value(m_currentConfig, 0);
    }

    if (config) {
        result = config->property(entry);
    }

    return result;
}

QString AppletInterface::file(const QString &fileType)
{
    return m_appletScriptEngine->filePath(fileType, QString());
}

QString AppletInterface::file(const QString &fileType, const QString &filePath)
{
    return m_appletScriptEngine->filePath(fileType, filePath);
}

QList<QAction*> AppletInterface::contextualActions() const
{
    QList<QAction*> actions;
    Plasma::Applet *a = applet();
    if (a->failedToLaunch()) {
        return actions;
    }

    foreach (const QString &name, m_actions) {
        QAction *action = a->action(name);

        if (action) {
            actions << action;
        }
    }

    return actions;
}

void AppletInterface::setActionSeparator(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->action(name);

    if (action) {
        action->setSeparator(true);
    } else {
        action = new QAction(this);
        action->setSeparator(true);
        a->addAction(name, action);
        m_actions.append(name);
    }
}

void AppletInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    Plasma::Applet *a = applet();
    QAction *action = a->action(name);

    if (action) {
        action->setText(text);
    } else {
        action = new QAction(text, this);
        a->addAction(name, action);

        Q_ASSERT(!m_actions.contains(name));
        m_actions.append(name);

        if (!m_actionSignals) {
            m_actionSignals = new QSignalMapper(this);
            connect(m_actionSignals, SIGNAL(mapped(QString)),
                    m_appletScriptEngine, SLOT(executeAction(QString)));
        }

        connect(action, SIGNAL(triggered()), m_actionSignals, SLOT(map()));
        m_actionSignals->setMapping(action, name);
    }

    if (!icon.isEmpty()) {
        action->setIcon(KIcon(icon));
    }

    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }

    action->setObjectName(name);
}

void AppletInterface::removeAction(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->action(name);

    if (action) {
        if (m_actionSignals) {
            m_actionSignals->removeMappings(action);
        }

        delete action;
    }

    m_actions.removeAll(name);
}

QAction *AppletInterface::action(QString name) const
{
    return applet()->action(name);
}

bool AppletInterface::immutable() const
{
    return applet()->immutability() != Plasma::Mutable;
}

bool AppletInterface::userConfiguring() const
{
    return applet()->isUserConfiguring();
}

int AppletInterface::apiVersion() const
{
    const QString constraint("[X-Plasma-API] == 'declarative' and 'Applet' in [X-Plasma-ComponentTypes]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    if (offers.isEmpty()) {
        return -1;
    }

    return offers.first()->property("X-KDE-PluginInfo-Version", QVariant::Int).toInt();
}

void AppletInterface::debug(const QString &msg)
{
    kDebug() << msg;
}

void AppletInterface::setAssociatedApplication(const QString &string)
{
    applet()->setAssociatedApplication(string);
}

QString AppletInterface::associatedApplication() const
{
    return applet()->associatedApplication();
}

void AppletInterface::setStatus(const AppletInterface::ItemStatus &status)
{
    applet()->setStatus((Plasma::ItemStatus)status);
}

AppletInterface::ItemStatus AppletInterface::status() const
{
    return (AppletInterface::ItemStatus)((int)(applet()->status()));
}

QString AppletInterface::downloadPath(const QString &file)
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';

    if (!QFile::exists(downloadDir)) {
        QDir dir(QChar('/'));
        dir.mkpath(downloadDir);
    }

    return downloadDir;
}

QStringList AppletInterface::downloadedFiles() const
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';
    QDir dir(downloadDir);
    return dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
}

void AppletInterface::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (!m_uiObject || qobject_cast<ContainmentInterface *>(this)) {
        return;
    }

    //TODO: completely arbitrary for now
    if (newGeometry.width() < 100 || newGeometry.height() < 100) {
        m_expanded = false;

        //we are already an icon: nothing to do
        if (m_compactUiObject) {
            return;
        }

        QQmlComponent *component = new QQmlComponent(m_appletScriptEngine->engine(), this);
        component->loadUrl(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("ui", "CompactApplet.qml")));
        m_compactUiObject = component->create(m_appletScriptEngine->engine()->rootContext());

        QObject *compactRepresentation = 0;

        //build the icon representation
        if (m_compactUiObject) {
            QQmlComponent *compactComponent = m_uiObject.data()->property("compactRepresentation").value<QQmlComponent *>();
            
            if (!compactComponent) {
                compactComponent = new QQmlComponent(m_appletScriptEngine->engine(), this);
                compactComponent->loadUrl(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("ui", "DefaultCompactRepresentation.qml")));
            }
            compactRepresentation = compactComponent->create(m_appletScriptEngine->engine()->rootContext());
            if (compactRepresentation) {
                compactComponent->setParent(compactRepresentation);
            } else {
                delete compactComponent;
            }
        }

        if (m_compactUiObject && compactRepresentation) {
            //for memory management
            component->setParent(m_compactUiObject.data());

            //put compactRepresentation in the icon place
            compactRepresentation->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));
            m_compactUiObject.data()->setProperty("compactRepresentation", QVariant::fromValue(compactRepresentation));

            //replace the full applet with the collapsed view
            m_compactUiObject.data()->setProperty("visible", true);
            m_compactUiObject.data()->setProperty("parent", QVariant::fromValue(this));
            //set anchors
            QQmlExpression expr(m_appletScriptEngine->engine()->rootContext(), m_compactUiObject.data(), "parent");
            QQmlProperty prop(m_compactUiObject.data(), "anchors.fill");
            prop.write(expr.evaluate());
            
            m_uiObject.data()->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));
            m_compactUiObject.data()->setProperty("applet", QVariant::fromValue(m_uiObject.data()));
        
        //failed to create UI, don't do anything, return in expanded status
        } else {
            qWarning() << component->errors();
            m_expanded = true;
            delete component;
        }

        emit expandedChanged();

    } else {
        m_expanded = true;
        emit expandedChanged();

        //we are already expanded: nothing to do
        if (!m_compactUiObject) {
            return;
        }

        m_uiObject.data()->setProperty("parent", QVariant::fromValue(this));
        m_compactUiObject.data()->deleteLater();
    }
}








///////////// ContainmentInterface

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent)
    : AppletInterface(parent),
      m_movableApplets(true)
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
    m_appletScriptEngine->setDrawWallpaper(drawWallpaper);
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

}

QString ContainmentInterface::activityId() const
{
    return containment()->activity();
}

#include "moc_appletinterface.cpp"
