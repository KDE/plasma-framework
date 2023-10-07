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
#include <QGuiApplication>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>

#include <KConfigLoader>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <kpackage/packageloader.h>

#include "containment.h"
#include "corona.h"
#include "debug_p.h"
#include "pluginloader.h"
#include "private/containment_p.h"
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
    , pendingConstraints(Applet::NoConstraint)
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
    Q_ASSERT(appletDescription.isValid());
    if (appletId == 0) {
        appletId = ++s_maxAppletId;
    } else if (appletId > s_maxAppletId) {
        s_maxAppletId = appletId;
    }
    QObject::connect(actions.value(QStringLiteral("configure")), SIGNAL(triggered()), q, SLOT(requestConfiguration()));
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PLASMA_TRACK_STARTUP")) {
        new TimeTracker(q);
    }
#endif

    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        QAction *action = it.value();
        const QString name = it.key();
        QObject::connect(action, &QObject::destroyed, q, [this, name]() {
            actions.remove(name);
        });
    }
}

AppletPrivate::~AppletPrivate()
{
    if (deleteNotification) {
        deleteNotification->close();
    }

    delete configLoader;
    configLoader = nullptr;
    delete mainConfig;
    mainConfig = nullptr;
    delete modificationsTimer;
}

void AppletPrivate::init(const QVariantList &args)
{
    startupArguments = args;

    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a Corona, which is not available at this point
    q->setHasConfigurationInterface(true);

    QAction *closeApplet = actions.value(QStringLiteral("remove"));
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove %1", q->title()));
    }

    QAction *configAction = actions.value(QStringLiteral("configure"));
    if (configAction) {
        configAction->setText(i18nc("%1 is the name of the applet", "Configure %1...", q->title().replace(QLatin1Char('&'), QStringLiteral("&&"))));
    }

    // A constructor may have set a valid package already
    if (!package.isValid()) {
        const QString packagePath = QFileInfo(appletDescription.fileName()).dir().path();
        QString path = appletDescription.value(QStringLiteral("X-Plasma-RootPath"));
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

    if (!q->isContainment()) {
        QAction *a = new QAction(QIcon::fromTheme(QStringLiteral("widget-alternatives")), i18n("Show Alternatives..."), q);
        a->setVisible(false);
        q->setInternalAction(QStringLiteral("alternatives"), a);
        QObject::connect(a, &QAction::triggered, q, [this] {
            if (q->containment()) {
                Q_EMIT q->containment()->appletAlternativesRequested(q);
            }
        });

        QObject::connect(q, &Applet::contextualActionsAboutToShow, a, [=]() {
            bool hasAlternatives = false;

            const QStringList provides = q->pluginMetaData().value(QStringLiteral("X-Plasma-Provides"), QStringList());
            if (!provides.isEmpty() && q->immutability() == Types::Mutable) {
                auto filter = [&provides](const KPluginMetaData &md) -> bool {
                    const QStringList provided = md.value(QStringLiteral("X-Plasma-Provides"), QStringList());
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

    if (qApp->closingDown()) {
        // If we are closing down, the actual delete will be executed when the corona will be destroyed
        // which crashes
        delete q;
    } else {
        q->deleteLater();
    }
}

void AppletPrivate::setDestroyed(bool destroyed)
{
    transient = destroyed;
    if (destroyed) {
        mainConfig->writeEntry(QStringLiteral("transient"), true);
    } else {
        mainConfig->deleteEntry(QStringLiteral("transient"));
    }
    Q_EMIT q->destroyedChanged(destroyed);
    // when an applet gets transient, it's "systemimmutable"
    Q_EMIT q->immutabilityChanged(q->immutability());

    Plasma::Containment *asContainment = qobject_cast<Plasma::Containment *>(q);
    if (asContainment) {
        const auto lstApplets = asContainment->applets();
        for (Applet *a : lstApplets) {
            a->d->setDestroyed(destroyed);
        }
    }
    Q_EMIT q->configNeedsSaving();
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
                   && (asContainment->containmentType() == Containment::Type::Panel //
                       || asContainment->containmentType() == Containment::Type::CustomPanel)) {
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
                Q_EMIT q->containment()->appletAboutToBeAdded(q, QRectF());
                q->containment()->d->applets.insert(position, q);
                Q_EMIT q->containment()->appletAdded(q, QRectF());
                Q_EMIT q->containment()->appletsChanged();
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
            Q_EMIT q->containment()->appletAboutToBeRemoved(q);
            q->containment()->d->applets.removeAll(q);
            Q_EMIT q->containment()->appletRemoved(q);
            Q_EMIT q->containment()->appletsChanged();
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

QMap<QString, QAction *> AppletPrivate::defaultActions(QObject *parent)
{
    QMap<QString, QAction *> actions;

    QAction *configAction = new QAction(parent);
    actions[QStringLiteral("configure")] = configAction;
    configAction->setAutoRepeat(false);
    configAction->setText(i18n("Widget Settings"));
    configAction->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    configAction->setShortcut(QKeySequence(QStringLiteral("alt+d, s")));

    QAction *closeApplet = new QAction(parent);
    actions[QStringLiteral("remove")] = closeApplet;
    closeApplet->setAutoRepeat(false);
    closeApplet->setText(i18n("Remove this Widget"));
    closeApplet->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    closeApplet->setShortcut(QKeySequence(QStringLiteral("alt+d, r")));

    return actions;
}

void AppletPrivate::contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    a->d->contextualActions.append(action);
    QObject::connect(action, &QObject::destroyed, a, [a, action]() {
        if (a->destroyed())
            return;
        a->d->contextualActions.removeAll(action);
        Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
    });
    Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
};

qsizetype AppletPrivate::contextualActions_count(QQmlListProperty<QAction> *prop)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    return a->d->contextualActions.count();
}

QAction *AppletPrivate::contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    return a->d->contextualActions.value(idx);
}

void AppletPrivate::contextualActions_clear(QQmlListProperty<QAction> *prop)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    a->d->contextualActions.clear();
    Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
}

void AppletPrivate::contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    a->d->contextualActions.replace(idx, action);
    QObject::connect(action, &QObject::destroyed, a, [a, action]() {
        a->d->contextualActions.removeAll(action);
        Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
    });
    Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
}

void AppletPrivate::contextualActions_removeLast(QQmlListProperty<QAction> *prop)
{
    Applet *a = static_cast<Plasma::Applet *>(prop->object);
    a->d->contextualActions.pop_back();
    Q_EMIT a->contextualActionsChanged(a->d->contextualActions);
}

void AppletPrivate::requestConfiguration()
{
    if (q->containment()) {
        Q_EMIT q->containment()->configureRequested(q);
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

QString AppletPrivate::globalName() const
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.pluginId();
}

void AppletPrivate::scheduleConstraintsUpdate(Applet::Constraints c)
{
    // Don't start up a timer if we're just starting up
    // flushPendingConstraints will be called by Corona
    if (started && !constraintsTimer.isActive() && !(c & Applet::StartupCompletedConstraint)) {
        constraintsTimer.start(0, q);
    }

    if (c & Applet::StartupCompletedConstraint) {
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
            qCDebug(LOG_PLASMA) << "requesting config for" << q->title() << "without a containment!";
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
