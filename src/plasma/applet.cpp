/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#include "applet.h"
#include "private/applet_p.h"

#include "config-plasma.h"

#include <cmath>
#include <limits>

#include <QFile>
#include <QList>
#include <QAbstractButton>
#include <QMessageBox>

#include <kactioncollection.h>
#include <kauthorized.h>
#include <kcolorscheme.h>
#include <kdesktopfile.h>
#include <QDebug>
#include <kglobalaccel.h>
#include <kplugininfo.h>
#include <klocalizedstring.h>
#include <kservice.h>
#include <KConfigLoader>
#include <kwindowsystem.h>

#include "containment.h"
#include "corona.h"
#include "package.h"
#include "plasma.h"
#include "scripting/appletscript.h"
#include "pluginloader.h"

#include "private/associatedapplicationmanager_p.h"
#include "private/containment_p.h"
#include "private/package_p.h"
#include "debug_p.h"

namespace Plasma
{

static KPluginMetaData appletMetadataForDirectory(const QString &path)
{
    return QFile::exists(path + QLatin1String("/metadata.json"))
                ? KPluginMetaData(path + QLatin1String("/metadata.json"))
                : KPluginMetaData::fromDesktopFile(path + QLatin1String("/metadata.desktop"), { QStringLiteral("plasma-applet.desktop") });
}

Applet::Applet(const KPluginMetaData &info, QObject *parent, uint appletId)
    :  QObject(parent),
       d(new AppletPrivate(info, appletId, this))
{
    qCDebug(LOG_PLASMA) << " From KPluginMetaData, valid? " << info.isValid();
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
    d->setupPackage();
}

Applet::Applet(const KPluginInfo &info, QObject *parent, uint appletId)
    :  Applet(info.toMetaData(), parent, appletId)
{
}

Applet::Applet(QObject *parent, const QString &serviceID, uint appletId)
    :  QObject(parent),
       d(new AppletPrivate(KPluginMetaData(serviceID), appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
    d->setupPackage();
}

Applet::Applet(QObject *parentObject, const QVariantList &args)
    :  QObject(0),
       d(new AppletPrivate(KPluginMetaData(), args.count() > 2 ? args[2].toInt() : 0, this))
{
    setParent(parentObject);
    if (args.count() > 0) {
        const QVariant first = args.first();
        if (first.canConvert<KPackage::Package>()) {
            d->package = first.value<KPackage::Package>();
        }
    }
    if (args.count() > 1) {
        const QVariant second = args[1];
        if (second.canConvert<QString>()) {
            d->appletDescription = KPluginMetaData(second.toString());
        } else if (second.canConvert<QVariantMap>()) {
            auto metadata = second.toMap().value(QStringLiteral("MetaData")).toMap();
            d->appletDescription = KPluginMetaData(QJsonObject::fromVariantMap(metadata), {});
        }
    }
    d->icon = d->appletDescription.iconName();

    if (args.contains(QVariant::fromValue(QStringLiteral("org.kde.plasma:force-create")))) {
        setProperty("org.kde.plasma:force-create", true);
    }

    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init(QString(), args.mid(3));
    d->setupPackage();
}

Applet::Applet(const QString &packagePath, uint appletId)
    : QObject(0),
      d(new AppletPrivate(appletMetadataForDirectory(packagePath), appletId, this))
{
    d->init(packagePath);
    d->setupPackage();
}

Applet::~Applet()
{
    if (d->transient) {
        d->resetConfigurationObject();
    }
    //let people know that i will die
    emit appletDeleted(this);

    delete d;
}

void Applet::init()
{
    //Don't implement anything here, it will be overridden by subclasses
}

uint Applet::id() const
{
    return d->appletId;
}

void Applet::save(KConfigGroup &g) const
{
    if (d->transient || !d->appletDescription.isValid()) {
        return;
    }

    KConfigGroup group = g;
    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    //qCDebug(LOG_PLASMA) << "saving" << pluginName() << "to" << group.name();
    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group.writeEntry("immutability", (int)d->immutability);
    group.writeEntry("plugin", d->appletDescription.pluginId());

    if (!d->started) {
        return;
    }

    KConfigGroup appletConfigGroup(&group, "Configuration");
    saveState(appletConfigGroup);

    if (d->configLoader) {
        // we're saving so we know its changed, we don't need or want the configChanged
        // signal bubbling up at this point due to that
        disconnect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
        d->configLoader->save();
        connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
    }
}

void Applet::restore(KConfigGroup &group)
{

    setImmutability((Types::ImmutabilityType)group.readEntry("immutability", (int)Types::Mutable));

    KConfigGroup shortcutConfig(&group, "Shortcuts");
    QString shortcutText = shortcutConfig.readEntryUntranslated("global", QString());
    if (!shortcutText.isEmpty()) {
        setGlobalShortcut(QKeySequence(shortcutText));
        /*
        #ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "got global shortcut for" << name() << "of" << QKeySequence(shortcutText);
        #endif
        #ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "set to" << d->activationAction->objectName()
        #endif
                 << d->activationAction->globalShortcut().primary();
                 */
    }

    // local shortcut, if any
    //TODO: implement; the shortcut will need to be registered with the containment
    /*
    #include "accessmanager.h"
    #include "private/plasmoidservice_p.h"
    #include "authorizationmanager.h"
    #include "authorizationmanager.h"
    shortcutText = shortcutConfig.readEntryUntranslated("local", QString());
    if (!shortcutText.isEmpty()) {
        //TODO: implement; the shortcut
    }
    */
}

void Applet::setLaunchErrorMessage(const QString &message)
{
    if (message == d->launchErrorMessage) {
        return;
    }

    d->failed = true;
    d->launchErrorMessage = message;
}

void Applet::saveState(KConfigGroup &group) const
{
    if (d->script) {
        emit d->script->saveState(group);
    }

    if (group.config()->name() != config().config()->name()) {
        // we're being saved to a different file!
        // let's just copy the current values in our configuration over
        KConfigGroup c = config();
        c.copyTo(&group);
    }
}

KConfigGroup Applet::config() const
{
    if (d->transient) {
        return KConfigGroup(KSharedConfig::openConfig(), "PlasmaTransientsConfig");
    }

    if (isContainment()) {
        return *(d->mainConfigGroup());
    }

    return KConfigGroup(d->mainConfigGroup(), "Configuration");
}

KConfigGroup Applet::globalConfig() const
{
    KConfigGroup globalAppletConfig;
    QString group = isContainment() ? QStringLiteral("ContainmentGlobals") : QStringLiteral("AppletGlobals");

    Containment *cont = containment();
    Corona *corona = 0;
    if (cont) {
        corona = cont->corona();
    }
    if (corona) {
        KSharedConfig::Ptr coronaConfig = corona->config();
        globalAppletConfig = KConfigGroup(coronaConfig, group);
    } else {
        globalAppletConfig = KConfigGroup(KSharedConfig::openConfig(), group);
    }

    return KConfigGroup(&globalAppletConfig, d->globalName());
}

void Applet::destroy()
{
    if (immutability() != Types::Mutable || d->transient || !d->started) {
        return; //don't double delete
    }

    d->setDestroyed(true);
    //FIXME: an animation on leave if !isContainment() would be good again .. which should be handled by the containment class
    d->cleanUpAndDelete();
}

bool Applet::destroyed() const
{
    return d->transient;
}

KConfigLoader *Applet::configScheme() const
{
    if (!d->configLoader) {
        const QString xmlPath = d->package.isValid() ? d->package.filePath("mainconfigxml") : QString();
        KConfigGroup cfg = config();
        if (xmlPath.isEmpty()) {
            d->configLoader = new KConfigLoader(cfg, 0);
        } else {
            QFile file(xmlPath);
            d->configLoader = new KConfigLoader(cfg, &file);
            QObject::connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
        }
    }

    return d->configLoader;
}

Package Applet::package() const
{
    Package p;
    p.d->internalPackage = new KPackage::Package(d->package);
    return p;
}

KPackage::Package Applet::kPackage() const
{
    return d->package;
}

void Applet::updateConstraints(Plasma::Types::Constraints constraints)
{
    d->scheduleConstraintsUpdate(constraints);
}

void Applet::constraintsEvent(Plasma::Types::Constraints constraints)
{
    //NOTE: do NOT put any code in here that reacts to constraints updates
    //      as it will not get called for any applet that reimplements constraintsEvent
    //      without calling the Applet:: version as well, which it shouldn't need to.
    //      INSTEAD put such code into flushPendingConstraintsEvents
    Q_UNUSED(constraints)
    //qCDebug(LOG_PLASMA) << constraints << "constraints are FormFactor: " << formFactor()
    //         << ", Location: " << location();
    if (d->script) {
        d->script->constraintsEvent(constraints);
    }
}

QString Applet::title() const
{
    if (!d->customTitle.isEmpty()) {
        return d->customTitle;
    }

    if (d->appletDescription.isValid()) {
        return d->appletDescription.name();
    }

    return i18n("Unknown");
}

void Applet::setTitle(const QString &title)
{
    if (title == d->customTitle) {
        return;
    }

    d->customTitle = title;
    emit titleChanged(title);
}

QString Applet::icon() const
{
    return d->icon;
}

void Applet::setIcon(const QString &icon)
{
    if (icon == d->icon) {
        return;
    }

    d->icon = icon;
    emit iconChanged(icon);
}

bool Applet::isBusy() const
{
    return d->busy;
}

void Applet::setBusy(bool busy)
{
    if (busy == d->busy) {
        return;
    }

    d->busy = busy;
    emit busyChanged(busy);
}

KPluginInfo Applet::pluginInfo() const
{
    return KPluginInfo(d->appletDescription);
}

KPluginMetaData Applet::pluginMetaData() const
{
    return d->appletDescription;
}

Types::ImmutabilityType Applet::immutability() const
{
    // if this object is itself system immutable, then just return that; it's the most
    // restrictive setting possible and will override anything that might be happening above it
    // in the Corona->Containment->Applet hierarchy
    if (d->transient || (d->mainConfig && d->mainConfig->isImmutable())) {
        return Types::SystemImmutable;
    }

    //Returning the more strict immutability between the applet immutability, Containment and Corona
    Types::ImmutabilityType upperImmutability = Types::Mutable;

    if (isContainment()) {
        Corona *cor = static_cast<Containment *>(const_cast<Applet *>(this))->corona();
        if (cor) {
            upperImmutability = cor->immutability();
        }
    } else {
        const Containment *cont = containment();
        if (cont) {
            if (cont->corona()) {
                upperImmutability = cont->corona()->immutability();
            } else {
                upperImmutability = cont->immutability();
            }
        }
    }

    if (upperImmutability != Types::Mutable) {
        // it's either system or user immutable, and we already check for local system immutability,
        // so upperImmutability is guaranteed to be as or more severe as this object's immutability
        return upperImmutability;
    } else {
        return d->immutability;
    }
}

void Applet::setImmutability(const Types::ImmutabilityType immutable)
{
    if (d->immutability == immutable || immutable == Types::SystemImmutable) {
        // we do not store system immutability in d->immutability since that gets saved
        // out to the config file; instead, we check with
        // the config group itself for this information at all times. this differs from
        // corona, where SystemImmutability is stored in d->immutability.
        return;
    }

    d->immutability = immutable;
    updateConstraints(Types::ImmutableConstraint);
}

QString Applet::launchErrorMessage() const
{
    return d->launchErrorMessage;
}

bool Applet::failedToLaunch() const
{
    return d->failed;
}

bool Applet::configurationRequired() const
{
    return d->needsConfig;
}

QString Applet::configurationRequiredReason() const
{
    return d->configurationRequiredReason;
}

void Applet::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    if (d->needsConfig == needsConfig && reason == d->configurationRequiredReason) {
        return;
    }

    d->needsConfig = needsConfig;
    d->configurationRequiredReason = reason;

    emit configurationRequiredChanged(needsConfig, reason);
}

bool Applet::isUserConfiguring() const
{
    return d->userConfiguring;
}

void Applet::setUserConfiguring(bool configuring)
{
    if (configuring == d->userConfiguring) {
        return;
    }

    d->userConfiguring = configuring;
    emit userConfiguringChanged(configuring);
}

Types::ItemStatus Applet::status() const
{
    return d->itemStatus;
}

void Applet::setStatus(const Types::ItemStatus status)
{
    if (status == d->itemStatus) {
        return;
    }
    d->itemStatus = status;
    emit statusChanged(status);
}

void Applet::flushPendingConstraintsEvents()
{
    if (d->pendingConstraints == Types::NoConstraint) {
        return;
    }

    if (d->constraintsTimer.isActive()) {
        d->constraintsTimer.stop();
    }

    //qCDebug(LOG_PLASMA) << "fushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Plasma::Types::Constraints c = d->pendingConstraints;
    d->pendingConstraints = Types::NoConstraint;

    if (c & Plasma::Types::UiReadyConstraint) {
        d->setUiReady();
    }

    if (c & Plasma::Types::StartupCompletedConstraint) {
        //common actions
        bool unlocked = immutability() == Types::Mutable;
        QAction *closeApplet = d->actions->action(QStringLiteral("remove"));
        if (closeApplet) {
            closeApplet->setEnabled(unlocked);
            closeApplet->setVisible(unlocked);
            connect(closeApplet, SIGNAL(triggered(bool)), this, SLOT(askDestroy()), Qt::UniqueConnection);
        }

        QAction *configAction = d->actions->action(QStringLiteral("configure"));
        if (configAction) {

            if (d->hasConfigurationInterface) {
                bool canConfig = unlocked || KAuthorized::authorize(QStringLiteral("plasma/allow_configure_when_locked"));
                configAction->setVisible(canConfig);
                configAction->setEnabled(canConfig);
            }
        }

        QAction *runAssociatedApplication = d->actions->action(QStringLiteral("run associated application"));
        if (runAssociatedApplication) {
            connect(runAssociatedApplication, SIGNAL(triggered(bool)), this, SLOT(runAssociatedApplication()), Qt::UniqueConnection);
        }

        d->updateShortcuts();
    }

    if (c & Plasma::Types::ImmutableConstraint) {
        bool unlocked = immutability() == Types::Mutable;
        QAction *action = d->actions->action(QStringLiteral("remove"));
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }

        action = d->actions->action(QStringLiteral("configure"));
        if (action && d->hasConfigurationInterface) {
            bool canConfig = unlocked || KAuthorized::authorize(QStringLiteral("plasma/allow_configure_when_locked"));
            action->setVisible(canConfig);
            action->setEnabled(canConfig);
        }

        //an immutable constraint will alwasy happen at startup
        //make sure don't emit a change signal for nothing
        if (d->oldImmutability != immutability()) {
            emit immutabilityChanged(immutability());
        }
        d->oldImmutability = immutability();
    }

    // now take care of constraints in special subclass: Contaiment
    Containment *containment = qobject_cast<Plasma::Containment *>(this);
    if (containment) {
        containment->d->containmentConstraintsEvent(c);
    }

    // pass the constraint on to the actual subclass
    constraintsEvent(c);

    if (c & Types::StartupCompletedConstraint) {
        // start up is done, we can now go do a mod timer
        if (d->modificationsTimer) {
            if (d->modificationsTimer->isActive()) {
                d->modificationsTimer->stop();
            }
        } else {
            d->modificationsTimer = new QBasicTimer;
        }
    }

    if (c & Plasma::Types::FormFactorConstraint) {
        emit formFactorChanged(formFactor());
    }

    if (c & Plasma::Types::LocationConstraint) {
        emit locationChanged(location());
    }
}

QList<QAction *> Applet::contextualActions()
{
    //qCDebug(LOG_PLASMA) << "empty context actions";
    return d->script ? d->script->contextualActions() : QList<QAction *>();
}

KActionCollection *Applet::actions() const
{
    return d->actions;
}

Types::FormFactor Applet::formFactor() const
{
    Containment *c = containment();
    QObject *pw = qobject_cast<QObject *>(parent());
    Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(pw);
    //assumption: this loop is usually is -really- short or doesn't run at all
    while (!parentApplet && pw && pw->parent()) {
        pw = pw->parent();
        parentApplet = qobject_cast<Plasma::Applet *>(pw);
    }

    return c ? c->d->formFactor : Plasma::Types::Planar;
}

Containment *Applet::containment() const
{
    Containment *c = qobject_cast<Containment *>(const_cast<Applet *>(this));
    if (c && c->isContainment()) {
        return c;
    } else {
        c = 0;
    }

    QObject *parent = this->parent();

    while (parent) {
        Containment *possibleC = qobject_cast<Containment *>(parent);

        if (possibleC && possibleC->isContainment()) {
            c = possibleC;
            break;
        }
        parent = parent->parent();
    }

    return c;
}

void Applet::setGlobalShortcut(const QKeySequence &shortcut)
{
    if (!d->activationAction) {
        d->activationAction = new QAction(this);
        d->activationAction->setText(i18n("Activate %1 Widget", title()));
        d->activationAction->setObjectName(QStringLiteral("activate widget %1").arg(id())); // NO I18N
        connect(d->activationAction, SIGNAL(triggered()), this, SIGNAL(activated()));
        connect(d->activationAction, SIGNAL(changed()),
                this, SLOT(globalShortcutChanged()));
    } else if (d->activationAction->shortcut() == shortcut) {
        return;
    }

    d->activationAction->setShortcut(shortcut);
    d->globalShortcutEnabled = true;
    QList<QKeySequence> seqs;
    seqs << shortcut;
    KGlobalAccel::self()->setShortcut(d->activationAction, seqs, KGlobalAccel::NoAutoloading);
    d->globalShortcutChanged();
}

QKeySequence Applet::globalShortcut() const
{
    if (d->activationAction) {
        QList<QKeySequence> shortcuts = KGlobalAccel::self()->shortcut(d->activationAction);
        if (!shortcuts.isEmpty()) {
            return shortcuts.first();
        }
    }

    return QKeySequence();
}

Types::Location Applet::location() const
{
    Containment *c = containment();
    return c ? c->d->location : Plasma::Types::Desktop;
}

bool Applet::hasConfigurationInterface() const
{
    return d->hasConfigurationInterface;
}

void Applet::setHasConfigurationInterface(bool hasInterface)
{
    if (hasInterface == d->hasConfigurationInterface) {
        return;
    }

    QAction *configAction = d->actions->action(QStringLiteral("configure"));
    if (configAction) {
        bool enable = hasInterface;
        if (enable) {
            const bool unlocked = immutability() == Types::Mutable;
            enable = unlocked || KAuthorized::authorize(QStringLiteral("plasma/allow_configure_when_locked"));
        }
        configAction->setEnabled(enable);
    }

    d->hasConfigurationInterface = hasInterface;
}

void Applet::configChanged()
{
    if (d->script) {
        if (d->configLoader) {
            d->configLoader->load();
        }
        d->script->configChanged();
    }
}

void Applet::setAssociatedApplication(const QString &string)
{
    AssociatedApplicationManager::self()->setApplication(this, string);

    QAction *runAssociatedApplication = d->actions->action(QStringLiteral("run associated application"));
    if (runAssociatedApplication) {
        bool valid = AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
        runAssociatedApplication->setVisible(valid);
        runAssociatedApplication->setEnabled(valid);
    }
}

void Applet::setAssociatedApplicationUrls(const QList<QUrl> &urls)
{
    AssociatedApplicationManager::self()->setUrls(this, urls);

    QAction *runAssociatedApplication = d->actions->action(QStringLiteral("run associated application"));
    if (runAssociatedApplication) {
        bool valid = AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
        runAssociatedApplication->setVisible(valid);
        runAssociatedApplication->setEnabled(valid);
    }
}

QString Applet::associatedApplication() const
{
    return AssociatedApplicationManager::self()->application(this);
}

QList<QUrl> Applet::associatedApplicationUrls() const
{
    return AssociatedApplicationManager::self()->urls(this);
}

void Applet::runAssociatedApplication()
{
    AssociatedApplicationManager::self()->run(this);
}

bool Applet::hasValidAssociatedApplication() const
{
    return AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
}

Applet *Applet::loadPlasmoid(const QString &path, uint appletId)
{
    const KPluginMetaData md = appletMetadataForDirectory(path);
    if (md.isValid()) {
        QStringList types = md.serviceTypes();

        if (types.contains(QStringLiteral("Plasma/Containment"))) {
            return new Containment(md, appletId);
        } else {
            return new Applet(md, nullptr, appletId);
        }
    }

    return 0;
}

void Applet::timerEvent(QTimerEvent *event)
{
    if (d->transient) {
        d->constraintsTimer.stop();
        if (d->modificationsTimer) {
            d->modificationsTimer->stop();
        }
        return;
    }

    if (event->timerId() == d->constraintsTimer.timerId()) {
        d->constraintsTimer.stop();

        // Don't flushPendingConstraints if we're just starting up
        // flushPendingConstraints will be called by Corona
        if (!(d->pendingConstraints & Plasma::Types::StartupCompletedConstraint)) {
            flushPendingConstraintsEvents();
        }
    } else if (d->modificationsTimer && event->timerId() == d->modificationsTimer->timerId()) {
        d->modificationsTimer->stop();
        // invalid group, will result in save using the default group
        KConfigGroup cg;

        save(cg);
        emit configNeedsSaving();
    }
}

bool Applet::isContainment() const
{
    //HACK: this is a special case for the systray
    //containment in an applet that is not a containment
    Applet *pa = qobject_cast<Applet *>(parent());
    if (pa && !pa->isContainment()) {
        return true;
    }
    //normal "acting as a containment" condition
    return qobject_cast<const Containment *>(this) && qobject_cast<Corona *>(parent());
}

} // Plasma namespace

#include "moc_applet.cpp"
