/*
 *   Copyright 2008-2013 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010-2013 Marco Martin <mart@kde.org>
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
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QSignalMapper>
#include <QTimer>

#include <KActionCollection>
#include <QDebug>
#include <KGlobalSettings>
#include <KService>
#include <KServiceTypeTrader>
#include <KLocalizedString>

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include "containmentinterface.h"
#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>
#include "declarative/packageaccessmanagerfactory.h"
#include "declarative/packageurlinterceptor.h"

Q_DECLARE_METATYPE(AppletInterface*)

AppletInterface::AppletInterface(DeclarativeAppletScript *script, QQuickItem *parent)
    : QQuickItem(parent),
      m_appletScriptEngine(script),
      m_actionSignals(0),
      m_backgroundHints(Plasma::Types::StandardBackground),
      m_busy(false),
      m_expanded(false)
{
    qmlRegisterType<AppletInterface>();
    qmlRegisterType<QAction>();

    connect(this, &AppletInterface::configNeedsSaving,
            applet(), &Plasma::Applet::configNeedsSaving);
    connect(applet(), &Plasma::Applet::immutabilityChanged,
            this, &AppletInterface::immutableChanged);
    connect(applet(), &Plasma::Applet::userConfiguringChanged,
            this, &AppletInterface::userConfiguringChanged);

    connect(applet(), &Plasma::Applet::statusChanged,
            this, &AppletInterface::statusChanged);

    connect(m_appletScriptEngine, &DeclarativeAppletScript::formFactorChanged,
            this, &AppletInterface::formFactorChanged);
    connect(m_appletScriptEngine, &DeclarativeAppletScript::locationChanged,
            this, &AppletInterface::locationChanged);
    connect(m_appletScriptEngine, &DeclarativeAppletScript::contextChanged,
            this, &AppletInterface::contextChanged);

    if (applet()->containment()) {
        connect(applet()->containment(), &Plasma::Containment::screenChanged,
                this, &ContainmentInterface::screenChanged);
    }

    m_qmlObject = new QmlObject(this);
    m_qmlObject->setInitializationDelayed(true);

    m_collapseTimer = new QTimer(this);
    m_collapseTimer->setSingleShot(true);
    connect(m_collapseTimer, &QTimer::timeout, this, &AppletInterface::compactRepresentationCheck);
}

AppletInterface::~AppletInterface()
{
}

void AppletInterface::init()
{
    if (m_qmlObject->rootObject()) {
        return;
    }

    m_configuration = new ConfigPropertyMap(applet()->configScheme(), this);

    //use our own custom network access manager that will access Plasma packages and to manage security (i.e. deny access to remote stuff when the proper extension isn't enabled
    QQmlEngine *engine = m_qmlObject->engine();
    QQmlNetworkAccessManagerFactory *factory = engine->networkAccessManagerFactory();
    engine->setNetworkAccessManagerFactory(0);
    delete factory;
    engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory(m_appletScriptEngine->package()));

    //Hook generic url resolution to the applet package as well
    //TODO: same thing will have to be done for every qqmlengine: PackageUrlInterceptor is material for plasmaquick?
    engine->setUrlInterceptor(new PackageUrlInterceptor(engine, m_appletScriptEngine->package()));

    m_qmlObject->setSource(QUrl::fromLocalFile(m_appletScriptEngine->mainScript()));

    if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
        }
        reason = i18n("Error loading QML file: %1", reason);

        m_qmlObject->setSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("appleterror")));
        m_qmlObject->completeInitialization();


        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return;
        } else {
            m_qmlObject->rootObject()->setProperty("reason", reason);
        }

        m_appletScriptEngine->setLaunchErrorMessage(reason);
    }


    m_qmlObject->engine()->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    initialProperties["width"] = width();
    initialProperties["height"] = height();
    m_qmlObject->completeInitialization(initialProperties);

    qDebug() << "Graphic object created:" << applet() << applet()->property("graphicObject");

    //Create the ToolBox
    Plasma::Containment *pc = qobject_cast<Plasma::Containment *>(applet());
    if (pc && !qobject_cast<Plasma::Applet *>(pc->parent())) {
        KConfigGroup defaults;
        if (pc->containmentType() == Plasma::Types::DesktopContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(pc->corona()->package().filePath("defaults")), "Desktop");
        } else if (pc->containmentType() == Plasma::Types::PanelContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(pc->corona()->package().filePath("defaults")), "Panel");
        }

        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        if (defaults.isValid()) {
            pkg.setPath(defaults.readEntry("ToolBox", "org.kde.desktoptoolbox"));
        } else {
            pkg.setPath("org.kde.desktoptoolbox");
        }

        if (pkg.isValid()) {
            QObject *toolBoxObject = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(pkg.filePath("mainscript")));

            QObject *containmentGraphicObject = m_qmlObject->rootObject();

            if (containmentGraphicObject && toolBoxObject) {
                toolBoxObject->setProperty("parent", QVariant::fromValue(containmentGraphicObject));

                containmentGraphicObject->setProperty("toolBox", QVariant::fromValue(toolBoxObject));
            } else {
                delete toolBoxObject;
            }
            qDebug() << "Loaded toolbox package" << pkg.path();
        } else {
            qWarning() << "Could not load toolbox package." << pkg.path();
        }
    }

    //set parent, both as object hyerarchy and visually
    if (m_qmlObject->rootObject()) {
        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "parent");
        QQmlProperty prop(m_qmlObject->rootObject(), "anchors.fill");
        prop.write(expr.evaluate());
    }
    geometryChanged(QRectF(), QRectF(x(), y(), width(), height()));
    emit busyChanged();

    applet()->updateConstraints(Plasma::Types::UiReadyConstraint);
}

Plasma::Types::FormFactor AppletInterface::formFactor() const
{
    return applet()->formFactor();
}

Plasma::Types::Location AppletInterface::location() const
{
    return applet()->location();
}

QString AppletInterface::currentActivity() const
{
    return applet()->containment()->activity();
}

QObject* AppletInterface::configuration() const
{
    return m_configuration;
}

QString AppletInterface::icon() const
{
    return applet()->icon();
}

QString AppletInterface::title() const
{
    return applet()->title();
}

uint AppletInterface::id() const
{
    return applet()->id();
}

void AppletInterface::setTitle(const QString &title)
{
    if (applet()->title() == title) {
        return;
    }

    applet()->setTitle(title);
    emit titleChanged();
}

bool AppletInterface::isBusy() const
{
    return !m_qmlObject->rootObject() || m_busy;
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

Plasma::Types::BackgroundHints AppletInterface::backgroundHints() const
{
    return m_backgroundHints;
}

void AppletInterface::setBackgroundHints(Plasma::Types::BackgroundHints hint)
{
    if (m_backgroundHints == hint) {
        return;
    }

    m_backgroundHints = hint;
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
        qWarning() << "Couldn't find a configuration entry";
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
        QAction *action = a->actions()->action(name);

        if (action) {
            actions << action;
        }
    }

    return actions;
}

void AppletInterface::setActionSeparator(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

    if (action) {
        action->setSeparator(true);
    } else {
        action = new QAction(this);
        action->setSeparator(true);
        a->actions()->addAction(name, action);
        m_actions.append(name);
    }
}

void AppletInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

    if (action) {
        action->setText(text);
    } else {
        action = new QAction(text, this);
        a->actions()->addAction(name, action);

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
        action->setIcon(QIcon::fromTheme(icon));
    }

    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }

    action->setObjectName(name);
}

void AppletInterface::removeAction(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

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
    return applet()->actions()->action(name);
}

bool AppletInterface::immutable() const
{
    return applet()->immutability() != Plasma::Types::Mutable;
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

bool AppletInterface::fillWidth() const
{
    if (!m_qmlObject->rootObject()) {
        return false;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property("fillWidth");
    } else {
        prop = m_qmlObject->rootObject()->property("fillWidth");
    }

    if (prop.isValid() && prop.canConvert<bool>()) {
        return prop.toBool();
    } else {
        return false;
    }
}

bool AppletInterface::fillHeight() const
{
    if (!m_qmlObject->rootObject()) {
        return false;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property("fillHeight");
    } else {
        prop = m_qmlObject->rootObject()->property("fillHeight");
    }

    if (prop.isValid() && prop.canConvert<bool>()) {
        return prop.toBool();
    } else {
        return false;
    }
}

//private api, just an helper
qreal AppletInterface::readGraphicsObjectSizeHint(const char *hint) const
{
    if (!m_qmlObject->rootObject()) {
        return -1;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property(hint);
    } else {
        prop = m_qmlObject->rootObject()->property(hint);
    }

    if (prop.isValid() && prop.canConvert<qreal>()) {
        return prop.toReal();
    } else {
        return -1;
    }
}

qreal AppletInterface::minimumWidth() const
{
    return readGraphicsObjectSizeHint("minimumWidth");
}

qreal AppletInterface::minimumHeight() const
{
    return readGraphicsObjectSizeHint("minimumHeight");
}

qreal AppletInterface::maximumWidth() const
{
    return readGraphicsObjectSizeHint("maximumWidth");
}

qreal AppletInterface::maximumHeight() const
{
    return readGraphicsObjectSizeHint("maximumHeight");
}

qreal AppletInterface::implicitWidth() const
{
    return readGraphicsObjectSizeHint("implicitWidth");
}

qreal AppletInterface::implicitHeight() const
{
    return readGraphicsObjectSizeHint("implicitHeight");
}

void AppletInterface::setAssociatedApplication(const QString &string)
{
    applet()->setAssociatedApplication(string);
}

QString AppletInterface::associatedApplication() const
{
    return applet()->associatedApplication();
}

void AppletInterface::setStatus(const Plasma::Types::ItemStatus &status)
{
    applet()->setStatus(status);
}

Plasma::Types::ItemStatus AppletInterface::status() const
{
    return applet()->status();
}

int AppletInterface::screen() const
{
    if (applet()->containment()) {
        return applet()->containment()->screen();
    }

    return -1;
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
    m_collapseTimer->start(100);
}

void AppletInterface::compactRepresentationCheck()
{
    if (width() <= 0 || height() <= 0 || !m_qmlObject->rootObject() ||
        qobject_cast<ContainmentInterface *>(this)) {
        return;
    }

    //Read the minimum width of the full representation, not our own, since we could be in collapsed mode
    QSizeF minHint(-1, -1);
    if (m_qmlObject->rootObject()->property("minimumWidth").canConvert<qreal>()) {
        minHint.setWidth(m_qmlObject->rootObject()->property("minimumWidth").toReal());
    }

    if (m_qmlObject->rootObject()->property("minimumHeight").canConvert<qreal>()) {
        minHint.setHeight(m_qmlObject->rootObject()->property("minimumHeight").toReal());
    }

    //Make it an icon
    if (width() < minHint.width() || height() < minHint.height()) {
        m_expanded = false;

        //we are already an icon: nothing to do
        if (m_compactUiObject) {
            return;
        }

        m_compactUiObject = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("compactapplet")));

        QObject *compactRepresentation = 0;

        //build the icon representation
        if (m_compactUiObject) {
            QQmlComponent *compactComponent = m_qmlObject->rootObject()->property("compactRepresentation").value<QQmlComponent *>();

            if (compactComponent) {
                compactRepresentation = compactComponent->create(compactComponent->creationContext());
            } else {
                compactRepresentation = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("defaultcompactrepresentation")));
            }

            if (compactRepresentation && compactComponent) {
                compactComponent->setParent(compactRepresentation);
            } else {
                delete compactComponent;
            }
        }

        if (m_compactUiObject && compactRepresentation) {
            //put compactRepresentation in the icon place
            compactRepresentation->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));
            m_compactUiObject.data()->setProperty("compactRepresentation", QVariant::fromValue(compactRepresentation));

            //replace the full applet with the collapsed view
            m_compactUiObject.data()->setProperty("visible", true);
            m_compactUiObject.data()->setProperty("parent", QVariant::fromValue(this));

            {
                //set anchors
                QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_compactUiObject.data(), "parent");
                QQmlProperty prop(m_compactUiObject.data(), "anchors.fill");
                prop.write(expr.evaluate());
            }

            m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));


            {
                //reset all the anchors
                QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "anchors.fill=undefined;anchors.left=undefined;anchors.right=undefined;anchors.top=undefined;anchors.bottom=undefined;");
                expr.evaluate();
            }

            KConfigGroup cg = applet()->config();
            cg = KConfigGroup(&cg, "PopupApplet");
            int width = cg.readEntry("DialogWidth", 0);
            int height = cg.readEntry("DialogHeight", 0);

            m_qmlObject->rootObject()->setProperty("width", width);
            m_qmlObject->rootObject()->setProperty("height", height);

            m_compactUiObject.data()->setProperty("applet", QVariant::fromValue(m_qmlObject->rootObject()));

            //hook m_compactUiObject size hints to this size hint
            //Here we have to use the old connect syntax, because we don't have access to the class type
            if (m_qmlObject->rootObject()) {
                disconnect(m_qmlObject->rootObject(), 0, this, 0);
            }

            //resize of the root object means popup resize when iconified
            connect(m_qmlObject->rootObject(), SIGNAL(widthChanged()),
                    this, SLOT(updatePopupSize()));
            connect(m_qmlObject->rootObject(), SIGNAL(heightChanged()),
                    this, SLOT(updatePopupSize()));

            if (m_compactUiObject.data()->property("minimumWidth").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(minimumWidthChanged()),
                        this, SIGNAL(minimumWidthChanged()));
            }
            if (m_compactUiObject.data()->property("minimumHeight").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(minimumHeightChanged()),
                        this, SIGNAL(minimumHeightChanged()));
            }

            if (m_compactUiObject.data()->property("maximumWidth").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(maximumWidthChanged()),
                        this, SIGNAL(maximumWidthChanged()));
            }
            if (m_compactUiObject.data()->property("maximumHeight").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(maximumHeightChanged()),
                        this, SIGNAL(maximumHeightChanged()));
            }

            if (m_compactUiObject.data()->property("implicitWidth").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(implicitWidthChanged()),
                        this, SIGNAL(implicitWidthChanged()));
            }
            if (m_compactUiObject.data()->property("implicitHeight").isValid()) {
                connect(m_compactUiObject.data(), SIGNAL(implicitHeightChanged()),
                        this, SIGNAL(implicitHeightChanged()));
            }

            emit fillWidthChanged();
            emit fillHeightChanged();
            emit minimumWidthChanged();
            emit minimumHeightChanged();
            emit implicitWidthChanged();
            emit implicitHeightChanged();
            emit maximumWidthChanged();
            emit maximumHeightChanged();
        //failed to create UI, don't do anything, return in expanded status
        } else {
            m_expanded = true;
        }

        emit expandedChanged();

    //show the full UI
    } else {
        m_expanded = true;
        emit expandedChanged();

        //we are already expanded: nothing to do
        if (m_compactUiObject) {
            disconnect(m_compactUiObject.data(), 0, this, 0);
        }

        disconnect(m_qmlObject->rootObject(), SIGNAL(widthChanged()),
                    this, SLOT(updatePopupSize()));
        disconnect(m_qmlObject->rootObject(), SIGNAL(heightChanged()),
                this, SLOT(updatePopupSize()));

        //Here we have to use the old connect syntax, because we don't have access to the class type
        if (m_qmlObject->rootObject()->property("minimumWidth").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(minimumWidthChanged()),
                    this, SIGNAL(minimumWidthChanged()));
        }
        if (m_qmlObject->rootObject()->property("minimumHeight").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(minimumHeightChanged()),
                    this, SIGNAL(minimumHeightChanged()));
        }

        if (m_qmlObject->rootObject()->property("maximumWidth").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(maximumWidthChanged()),
                    this, SIGNAL(maximumWidthChanged()));
        }
        if (m_qmlObject->rootObject()->property("maximumHeight").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(maximumHeightChanged()),
                    this, SIGNAL(maximumHeightChanged()));
        }

        if (m_qmlObject->rootObject()->property("implicitWidth").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(implicitWidthChanged()),
                    this, SIGNAL(implicitWidthChanged()));
        }
        if (m_qmlObject->rootObject()->property("implicitHeight").isValid()) {
            connect(m_qmlObject->rootObject(), SIGNAL(implicitHeightChanged()),
                    this, SIGNAL(implicitHeightChanged()));
        }

        emit fillWidthChanged();
        emit fillHeightChanged();
        emit minimumWidthChanged();
        emit minimumHeightChanged();
        emit implicitWidthChanged();
        emit implicitHeightChanged();
        emit maximumWidthChanged();
        emit maximumHeightChanged();

        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));
        if (m_compactUiObject) {
            m_compactUiObject.data()->deleteLater();
        }

        //set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "parent");
        QQmlProperty prop(m_qmlObject->rootObject(), "anchors.fill");
        prop.write(expr.evaluate());
    }
}

void AppletInterface::updatePopupSize()
{
    KConfigGroup cg = applet()->config();
    cg = KConfigGroup(&cg, "PopupApplet");
    cg.writeEntry("DialogWidth", m_qmlObject->rootObject()->property("width").toInt());
    cg.writeEntry("DialogHeight", m_qmlObject->rootObject()->property("height").toInt());
}

void AppletInterface::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the 
        if (value.window && !m_qmlObject->rootObject()) {
            init();
        }
    }
    QQuickItem::itemChange(change, value);
}

QmlObject *AppletInterface::qmlObject()
{
    return m_qmlObject;
}

#include "moc_appletinterface.cpp"
