/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/applet_p.h"

#include <config-plasma.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>

#include <KConfigLoader>
#include <KGlobalAccel>
#include <KKeySequenceWidget>
#include <KLocalizedString>
#include <kpackage/packageloader.h>

#include "containment.h"
#include "corona.h"
#include "debug_p.h"
#include "pluginloader.h"
#include "private/containment_p.h"
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
#include "private/package_p.h"
#endif
#include "scripting/appletscript.h"
#include "scripting/scriptengine.h"
#include "timetracker.h"

namespace Plasma
{
AppletPrivate::AppletPrivate(const KPluginMetaData &info, int uniqueID, Applet *applet)
    : appletId(uniqueID)
    , q(applet)
    , immutability(Types::Mutable)
    , oldImmutability(Types::Mutable)
    , appletDescription(info)
    , icon(appletDescription.iconName())
    , mainConfig(nullptr)
    , pendingConstraints(Types::NoConstraint)
    , script(nullptr)
    , package(nullptr)
    , configLoader(nullptr)
    , actions(AppletPrivate::defaultActions(applet))
    , activationAction(nullptr)
    , itemStatus(Types::UnknownStatus)
    , modificationsTimer(nullptr)
    , deleteNotificationTimer(nullptr)
    , hasConfigurationInterface(false)
    , failed(false)
    , transient(false)
    , needsConfig(false)
    , started(false)
    , globalShortcutEnabled(false)
    , userConfiguring(false)
    , busy(false)
{
    if (appletId == 0) {
        appletId = ++s_maxAppletId;
    } else if (appletId > s_maxAppletId) {
        s_maxAppletId = appletId;
    }
    QObject::connect(actions->action(QStringLiteral("configure")), SIGNAL(triggered()), q, SLOT(requestConfiguration()));
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PLASMA_TRACK_STARTUP")) {
        new TimeTracker(q);
    }
#endif
}

AppletPrivate::~AppletPrivate()
{
    if (deleteNotification) {
        deleteNotification->close();
    }

    delete script;
    script = nullptr;
    delete configLoader;
    configLoader = nullptr;
    delete mainConfig;
    mainConfig = nullptr;
    delete modificationsTimer;
}

void AppletPrivate::init(const QString &_packagePath, const QVariantList &args)
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a Corona, which is not available at this point
    q->setHasConfigurationInterface(true);

    QAction *closeApplet = actions->action(QStringLiteral("remove"));
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove %1", q->title()));
    }

    QAction *configAction = actions->action(QStringLiteral("configure"));
    if (configAction) {
        configAction->setText(i18nc("%1 is the name of the applet", "Configure %1...", q->title().replace(QLatin1Char('&'), QStringLiteral("&&"))));
    }

    if (!appletDescription.isValid()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Check your constructor! "
        //         << "You probably want to be passing in a Service::Ptr "
        //         << "or a QVariantList with a valid storageid as arg[0].";
#endif
        return;
    }

    QString api = appletDescription.rawData().value(QStringLiteral("X-Plasma-API")).toString();

    if (api.isEmpty()) {
        q->setLaunchErrorMessage(i18n("The %1 widget did not define which ScriptEngine to use.", appletDescription.name()));
        return;
    }

    // A constructor may have set a valid package already
    if (!package.isValid()) {
        const QString packagePath = _packagePath.isEmpty() && !appletDescription.metaDataFileName().isEmpty()
            ? QFileInfo(appletDescription.metaDataFileName()).dir().path()
            : _packagePath;
        QString path = appletDescription.rawData().value(QStringLiteral("X-Plasma-RootPath")).toString();
        if (path.isEmpty()) {
            path = packagePath.isEmpty() ? appletDescription.pluginId() : packagePath;
        }

        package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"));
        package.setPath(path);

        if (!package.isValid()) {
            q->setLaunchErrorMessage(i18nc("Package file, name of the widget",
                                           "Could not open the %1 package required for the %2 widget.",
                                           appletDescription.pluginId(),
                                           appletDescription.name()));
            return;
        }
    }

    // now we try and set up the script engine.
    // it will be parented to this applet and so will get
    // deleted when the applet does
    script = Plasma::loadScriptEngine(api, q, args);

    // It's valid, let's try to load the icon from within the package
    if (script) {
        // use the absolute path of the in-package icon as icon name
        if (appletDescription.iconName().startsWith(QLatin1Char('/'))) {
            icon = package.filePath({}, appletDescription.iconName());
        }
        // package not valid, get rid of it
    } else {
        q->setLaunchErrorMessage(i18nc("API or programming language the widget was written in, name of the widget",
                                       "Could not create a %1 ScriptEngine for the %2 widget.",
                                       api,
                                       appletDescription.name()));
    }

    if (!q->isContainment()) {
        QAction *a = new QAction(QIcon::fromTheme(QStringLiteral("widget-alternatives")), i18n("Show Alternatives..."), q);
        a->setVisible(false);
        q->actions()->addAction(QStringLiteral("alternatives"), a);
        QObject::connect(a, &QAction::triggered, q, [this] {
            if (q->containment()) {
                Q_EMIT q->containment()->appletAlternativesRequested(q);
            }
        });

        QObject::connect(q, &Applet::contextualActionsAboutToShow, a, [=]() {
            bool hasAlternatives = false;

            const QStringList provides = KPluginMetaData::readStringList(q->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));
            if (!provides.isEmpty() && q->immutability() == Types::Mutable) {
                auto filter = [&provides](const KPluginMetaData &md) -> bool {
                    const QStringList provided = KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-Provides"));
                    for (const QString &p : provides) {
                        if (provided.contains(p)) {
                            return true;
                        }
                    }
                    return false;
                };
                QList<KPluginMetaData> applets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);

                if (applets.count() > 1) {
                    hasAlternatives = true;
                }
            }
            a->setVisible(hasAlternatives);
        });
    }
}

void AppletPrivate::cleanUpAndDelete()
{
    // reimplemented in the UI specific library
    if (configLoader) {
        configLoader->clearItems();
    }

    resetConfigurationObject();

    if (activationAction && globalShortcutEnabled) {
        // qCDebug(LOG_PLASMA) << "resetting global action for" << q->title() << activationAction->objectName();
        KGlobalAccel::self()->removeAllShortcuts(activationAction);
    }

    if (q->isContainment()) {
        // prematurely emit our destruction if we are a Containment,
        // giving Corona a chance to remove this Containment from its collection
        Q_EMIT q->QObject::destroyed(q);
    }

    q->deleteLater();
}

void AppletPrivate::setDestroyed(bool destroyed)
{
    transient = destroyed;
    Q_EMIT q->destroyedChanged(destroyed);
    // when an applet gets transient, it's "systemimmutable"
    Q_EMIT q->immutabilityChanged(q->immutability());

    Plasma::Containment *asContainment = qobject_cast<Plasma::Containment *>(q);
    if (asContainment) {
        const auto lstApplets = asContainment->applets();
        for (Applet *a : lstApplets)
            a->d->setDestroyed(destroyed);
    }
}

void AppletPrivate::askDestroy()
{
    if (q->immutability() != Types::Mutable || !started) {
        return; // don't double delete
    }

    if (transient) {
        cleanUpAndDelete();
    } else {
        // There is no confirmation anymore for panels removal:
        // this needs users feedback
        setDestroyed(true);
        // no parent, but it won't leak, since it will be closed both in case of timeout
        // or direct action
        deleteNotification = new KNotification(QStringLiteral("plasmoidDeleted"));
        deleteNotification->setFlags(KNotification::Persistent | KNotification::SkipGrouping);

        deleteNotification->setComponentName(QStringLiteral("plasma_workspace"));
        QStringList actions;
        deleteNotification->setIconName(q->icon());
        Plasma::Containment *asContainment = qobject_cast<Plasma::Containment *>(q);

        if (!q->isContainment()) {
            deleteNotification->setTitle(i18n("Widget Removed"));
            deleteNotification->setText(i18n("The widget \"%1\" has been removed.", q->title().toHtmlEscaped()));
        } else if (asContainment
                   && (asContainment->containmentType() == Types::PanelContainment //
                       || asContainment->containmentType() == Types::CustomPanelContainment)) {
            deleteNotification->setTitle(i18n("Panel Removed"));
            deleteNotification->setText(i18n("A panel has been removed."));
            // This will never happen with our current shell, but could with a custom one
        } else {
            deleteNotification->setTitle(i18n("Desktop Removed"));
            deleteNotification->setText(i18n("A desktop has been removed."));
        }

        actions.append(i18n("Undo"));
        deleteNotification->setActions(actions);
        QObject::connect(deleteNotification.data(), &KNotification::action1Activated, q, [=]() {
            setDestroyed(false);
            if (!q->isContainment() && q->containment()) {
                Plasma::Applet *containmentApplet = static_cast<Plasma::Applet *>(q->containment());
                if (containmentApplet && containmentApplet->d->deleteNotificationTimer) {
                    Q_EMIT containmentApplet->destroyedChanged(false);
                    // when an applet gets transient, it's "systemimmutable"
                    Q_EMIT q->immutabilityChanged(q->immutability());
                    delete containmentApplet->d->deleteNotificationTimer;
                    containmentApplet->d->deleteNotificationTimer = nullptr;
                }

                // make sure the applets are sorted by id
                auto position =
                    std::lower_bound(q->containment()->d->applets.begin(), q->containment()->d->applets.end(), q, [](Plasma::Applet *a1, Plasma::Applet *a2) {
                        return a1->id() < a2->id();
                    });
                q->containment()->d->applets.insert(position, q);
                Q_EMIT q->containment()->appletAdded(q);
            }
            if (deleteNotification) {
                deleteNotification->close();
            } else if (deleteNotificationTimer) {
                deleteNotificationTimer->stop();
                deleteNotificationTimer->deleteLater();
                deleteNotificationTimer = nullptr;
            }
        });
        QObject::connect(deleteNotification.data(), &KNotification::closed, q, [=]() {
            // If the timer still exists, it means the undo action was NOT triggered
            if (transient) {
                cleanUpAndDelete();
            }
            if (deleteNotificationTimer) {
                deleteNotificationTimer->stop();
                deleteNotificationTimer->deleteLater();
                deleteNotificationTimer = nullptr;
            }
        });

        deleteNotification->sendEvent();
        if (!deleteNotificationTimer) {
            deleteNotificationTimer = new QTimer(q);
            // really delete after a minute
            deleteNotificationTimer->setInterval(60 * 1000);
            deleteNotificationTimer->setSingleShot(true);
            QObject::connect(deleteNotificationTimer, &QTimer::timeout, q, [=]() {
                transient = true;
                if (deleteNotification) {
                    deleteNotification->close();
                } else {
                    Q_EMIT q->destroyedChanged(true);
                    cleanUpAndDelete();
                }
            });
            deleteNotificationTimer->start();
        }
        if (!q->isContainment() && q->containment()) {
            q->containment()->d->applets.removeAll(q);
            Q_EMIT q->containment()->appletRemoved(q);
        }
    }
}

void AppletPrivate::globalShortcutChanged()
{
    if (!activationAction) {
        return;
    }
    KConfigGroup shortcutConfig(mainConfigGroup(), "Shortcuts");
    QString newShortCut = activationAction->shortcut().toString();
    QString oldShortCut = shortcutConfig.readEntry("global", QString());
    if (newShortCut != oldShortCut) {
        shortcutConfig.writeEntry("global", newShortCut);
        scheduleModificationNotification();
    }
    // qCDebug(LOG_PLASMA) << "after" << shortcut.primary() << d->activationAction->globalShortcut().primary();
}

KActionCollection *AppletPrivate::defaultActions(QObject *parent)
{
    KActionCollection *actions = new KActionCollection(parent);
    actions->setConfigGroup(QStringLiteral("Shortcuts-Applet"));

    QAction *configAction = actions->add<QAction>(QStringLiteral("configure"));
    configAction->setAutoRepeat(false);
    configAction->setText(i18n("Widget Settings"));
    configAction->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    configAction->setShortcut(QKeySequence(QStringLiteral("alt+d, s")));
    configAction->setData(Plasma::Types::ConfigureAction);

    QAction *closeApplet = actions->add<QAction>(QStringLiteral("remove"));
    closeApplet->setAutoRepeat(false);
    closeApplet->setText(i18n("Remove this Widget"));
    closeApplet->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    closeApplet->setShortcut(QKeySequence(QStringLiteral("alt+d, r")));
    closeApplet->setData(Plasma::Types::DestructiveAction);

    QAction *runAssociatedApplication = actions->add<QAction>(QStringLiteral("run associated application"));
    runAssociatedApplication->setAutoRepeat(false);
    runAssociatedApplication->setText(i18n("Run the Associated Application"));
    runAssociatedApplication->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    runAssociatedApplication->setShortcut(QKeySequence(QStringLiteral("alt+d, t")));
    runAssociatedApplication->setVisible(false);
    runAssociatedApplication->setEnabled(false);
    runAssociatedApplication->setData(Plasma::Types::ControlAction);

    return actions;
}

void AppletPrivate::requestConfiguration()
{
    if (q->containment()) {
        Q_EMIT q->containment()->configureRequested(q);
    }
}

void AppletPrivate::updateShortcuts()
{
    if (q->isContainment()) {
        // a horrible hack to avoid clobbering corona settings
        // we pull them out, then read, then put them back
        QList<QAction *> qactions;
        const QList<QString> names = {QStringLiteral("add sibling containment"), QStringLiteral("configure shortcuts"), QStringLiteral("lock widgets")};
        for (const QString &name : names) {
            QAction *a = actions->action(name);
            actions->takeAction(a); // FIXME this is stupid, KActionCollection needs a takeAction(QString) method
            qactions << a;
        }

        actions->readSettings();

        for (int i = 0; i < names.size(); ++i) {
            QAction *a = qactions.at(i);
            if (a) {
                actions->addAction(names.at(i), a);
            }
        }
    } else {
        actions->readSettings();
    }
}

void AppletPrivate::propagateConfigChanged()
{
    Containment *c = qobject_cast<Containment *>(q);
    if (c) {
        c->d->configChanged();
    }
    q->configChanged();
}

void AppletPrivate::setUiReady()
{
    // am i the containment?
    Containment *c = qobject_cast<Containment *>(q);
    if (c && c->isContainment()) {
        c->d->setUiReady();
    } else if (Containment *cc = q->containment()) {
        cc->d->appletLoaded(q);
    }
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void AppletPrivate::setupPackage()
{
    if (!package.isValid()) {
        return;
    }

#ifndef NDEBUG
    // qCDebug(LOG_PLASMA) << "setting up script support, package is in" << package->path()
    //         << ", main script is" << package->filePath("mainscript");
#endif

    // FIXME: Replace with ki18n functionality once semantics is clear.
    // const QString translationsPath = package->filePath("translations");
    // if (!translationsPath.isEmpty()) {
    //     KGlobal::dirs()->addResourceDir("locale", translationsPath);
    // }

    if (!package.filePath("mainconfigui").isEmpty()) {
        q->setHasConfigurationInterface(true);
    }
}

void AppletPrivate::setupScripting()
{
    if (script) {
        if (!script->init() && !failed) {
            q->setLaunchErrorMessage(i18n("Script initialization failed"));
        }
    }
}

QString AppletPrivate::globalName() const
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.pluginId();
}

void AppletPrivate::scheduleConstraintsUpdate(Plasma::Types::Constraints c)
{
    // Don't start up a timer if we're just starting up
    // flushPendingConstraints will be called by Corona
    if (started && !constraintsTimer.isActive() && !(c & Plasma::Types::StartupCompletedConstraint)) {
        constraintsTimer.start(0, q);
    }

    if (c & Plasma::Types::StartupCompletedConstraint) {
        started = true;
        if (q->isContainment()) {
            qobject_cast<Containment *>(q)->d->setStarted();
        }
    }

    pendingConstraints |= c;
}

void AppletPrivate::scheduleModificationNotification()
{
    // modificationsTimer is not allocated until we get our notice of being started
    if (modificationsTimer) {
        // schedule a save
        modificationsTimer->start(1000, q);
    }
}

KConfigGroup *AppletPrivate::mainConfigGroup()
{
    if (mainConfig) {
        return mainConfig;
    }

    Containment *c = q->containment();
    Plasma::Applet *parentApplet = nullptr;
    if (c) {
        parentApplet = qobject_cast<Plasma::Applet *>(c->parent());
    }

    if (q->isContainment()) {
        Corona *corona = static_cast<Containment *>(q)->corona();
        KConfigGroup containmentConfig;
        // qCDebug(LOG_PLASMA) << "got a corona, baby?" << (QObject*)corona << (QObject*)q;

        if (parentApplet) {
            containmentConfig = parentApplet->config();
            containmentConfig = KConfigGroup(&containmentConfig, "Containments");
        } else if (corona) {
            containmentConfig = KConfigGroup(corona->config(), "Containments");
        } else {
            containmentConfig = KConfigGroup(KSharedConfig::openConfig(), "Containments");
        }

        mainConfig = new KConfigGroup(&containmentConfig, QString::number(appletId));
    } else {
        KConfigGroup appletConfig;

        if (c) {
            // applet directly in a Containment, as usual
            appletConfig = c->config();
            appletConfig = KConfigGroup(&appletConfig, "Applets");
        } else {
            qCWarning(LOG_PLASMA) << "requesting config for" << q->title() << "without a containment!";
            appletConfig = KConfigGroup(KSharedConfig::openConfig(), "Applets");
        }

        mainConfig = new KConfigGroup(&appletConfig, QString::number(appletId));
    }

    if (configLoader) {
        configLoader->setSharedConfig(KSharedConfig::openConfig(mainConfig->config()->name()));
        configLoader->load();
    }

    return mainConfig;
}

void AppletPrivate::resetConfigurationObject()
{
    // make sure mainConfigGroup exists in all cases
    mainConfigGroup();
    mainConfig->deleteEntry("plugin");
    mainConfig->deleteEntry("formfactor");
    mainConfig->deleteEntry("immutability");
    mainConfig->deleteEntry("location");
    // if it's not a containment, deleting the non existing activityId entry does nothing
    mainConfig->deleteEntry("activityId");
    mainConfig->deleteGroup();
    delete mainConfig;
    mainConfig = nullptr;

    Containment *cont = qobject_cast<Containment *>(q);

    if (cont && cont->corona()) {
        cont->corona()->requireConfigSync();
    } else {
        if (!q->containment()) {
            return;
        }
        Corona *corona = q->containment()->corona();
        if (corona) {
            corona->requireConfigSync();
        }
    }
}

uint AppletPrivate::s_maxAppletId = 0;

} // namespace Plasma
