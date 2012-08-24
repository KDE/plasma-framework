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

#include <plasma/animations/animation.h>

#include <cmath>
#include <limits>

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QGraphicsGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHostInfo>
#include <QLabel>
#include <QList>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QRegExp>
#include <QSize>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QUiLoader>
#include <QVBoxLayout>
#include <QWidget>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcoreauthorized.h>
#include <kcolorscheme.h>
#include <kdialog.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <kkeysequencewidget.h>
#include <kplugininfo.h>

#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshortcut.h>
#include <kwindowsystem.h>
#include <kpushbutton.h>

#if !PLASMA_NO_KUTILS
#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>
#else
#include <kcmodule.h>
#endif

#if !PLASMA_NO_SOLID
#include <solid/powermanagement.h>
#endif

#include "abstracttoolbox.h"
#include "authorizationrule.h"
#include "configloader.h"
#include "containment.h"
#include "corona.h"
#include "dataenginemanager.h"
#include "dialog.h"
#include "package.h"
#include "plasma.h"
#include "scripting/appletscript.h"
#include "svg.h"
#include "framesvg.h"
#include "popupapplet.h"
#include "private/applethandle_p.h"
#include "private/framesvg_p.h"
#include "remote/authorizationmanager.h"
#include "remote/authorizationmanager_p.h"
#include "theme.h"
#include "view.h"
#include "widgets/iconwidget.h"
#include "widgets/label.h"
#include "tooltipmanager.h"
#include "wallpaper.h"
#include "paintutils.h"
#include "abstractdialogmanager.h"
#include "pluginloader.h"

#include "private/associatedapplicationmanager_p.h"
#include "private/containment_p.h"
#include "private/package_p.h"
#include "private/packages_p.h"
#include "private/plasmoidservice_p.h"
#include "private/popupapplet_p.h"
#include "private/remotedataengine_p.h"
#include "private/service_p.h"
#include "ui_publish.h"


namespace Plasma
{

Applet::Applet(const KPluginInfo &info, QGraphicsItem *parent, uint appletId)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::Ptr(), &info, appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
}

Applet::Applet(QGraphicsItem *parent, const QString &serviceID, uint appletId)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::serviceByStorageId(serviceID), 0, appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
}

Applet::Applet(QGraphicsItem *parent, const QString &serviceID, uint appletId, const QVariantList &args)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::serviceByStorageId(serviceID), 0, appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point

    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();

        if (!mutableArgs.isEmpty()) {
            mutableArgs.removeFirst();
        }
    }

    d->args = mutableArgs;

    d->init();
}

Applet::Applet(QObject *parentObject, const QVariantList &args)
    :  QGraphicsWidget(0),
       d(new AppletPrivate(
             KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString()), 0,
             args.count() > 1 ? args[1].toInt() : 0, this))
{
    // now remove those first two items since those are managed by Applet and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();

        if (!mutableArgs.isEmpty()) {
            mutableArgs.removeFirst();
        }
    }

    d->args = mutableArgs;

    setParent(parentObject);

    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();

    // the brain damage seen in the initialization list is due to the
    // inflexibility of KService::createInstance
}

Applet::Applet(const QString &packagePath, uint appletId, const QVariantList &args)
    : QGraphicsWidget(0),
      d(new AppletPrivate(KService::Ptr(new KService(packagePath + "/metadata.desktop")), 0, appletId, this))
{
    Q_UNUSED(args) // FIXME?
    d->init(packagePath);
}

Applet::~Applet()
{
    //let people know that i will die
    emit appletDestroyed(this);

    // clean up our config dialog, if any
    delete KConfigDialog::exists(d->configDialogId());
    delete d;
}

void Applet::init()
{
    setFlag(ItemIsMovable, true);
    if (d->script) {
        d->setupScriptSupport();

        if (!d->script->init() && !d->failed) {
            setFailedToLaunch(true, i18n("Script initialization failed"));
        }
    }
}

uint Applet::id() const
{
    return d->appletId;
}

void Applet::save(KConfigGroup &g) const
{
    if (d->transient) {
        return;
    }

    KConfigGroup group = g;
    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    //kDebug() << "saving" << pluginName() << "to" << group.name();
    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group.writeEntry("immutability", (int)d->immutability);
    group.writeEntry("plugin", pluginName());

    group.writeEntry("geometry", geometry());
    group.writeEntry("zvalue", zValue());

    if (!d->started) {
        return;
    }

    //kDebug() << pluginName() << "geometry is" << geometry()
    //         << "pos is" << pos() << "bounding rect is" << boundingRect();
    if (transform() == QTransform()) {
        group.deleteEntry("transform");
    } else {
        QList<qreal> m;
        QTransform t = transform();
        m << t.m11() << t.m12() << t.m13() << t.m21() << t.m22() << t.m23() << t.m31() << t.m32() << t.m33();
        group.writeEntry("transform", m);
        //group.writeEntry("transform", transformToString(transform()));
    }

    KConfigGroup appletConfigGroup(&group, "Configuration");
    saveState(appletConfigGroup);

    if (d->configLoader) {
        // we're saving so we know its changed, we don't need or want the configChanged
        // signal bubbling up at this point due to that
        disconnect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
        d->configLoader->writeConfig();
        connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
    }
}

void Applet::restore(KConfigGroup &group)
{
    QList<qreal> m = group.readEntry("transform", QList<qreal>());
    if (m.count() == 9) {
        QTransform t(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        setTransform(t);
    }

    qreal z = group.readEntry("zvalue", 0);

    if (z >= AppletPrivate::s_maxZValue) {
        AppletPrivate::s_maxZValue = z;
    }

    if (z > 0) {
        setZValue(z);
    }

    setImmutability((ImmutabilityType)group.readEntry("immutability", (int)Mutable));

    QRectF geom = group.readEntry("geometry", QRectF());
    if (geom.isValid()) {
        setGeometry(geom);
    }

    KConfigGroup shortcutConfig(&group, "Shortcuts");
    QString shortcutText = shortcutConfig.readEntryUntranslated("global", QString());
    if (!shortcutText.isEmpty()) {
        setGlobalShortcut(KShortcut(shortcutText));
        /*
#ifndef NDEBUG
        kDebug() << "got global shortcut for" << name() << "of" << QKeySequence(shortcutText);
#endif
#ifndef NDEBUG
        kDebug() << "set to" << d->activationAction->objectName()
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

void Applet::setFailedToLaunch(bool failed, const QString &reason)
{
    d->failed = failed;
    d->updateFailedToLaunch(reason);
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

    if (d->isContainment) {
        return *(d->mainConfigGroup());
    }

    return KConfigGroup(d->mainConfigGroup(), "Configuration");
}

KConfigGroup Applet::globalConfig() const
{
    KConfigGroup globalAppletConfig;
    QString group = isContainment() ? "ContainmentGlobals" : "AppletGlobals";

    Corona *corona = qobject_cast<Corona*>(scene());
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
    if (immutability() != Mutable || d->transient || !d->started) {
        return; //don't double delete
    }

    d->transient = true;
    //FIXME: an animation on leave if !isContainment() would be good again .. which should be handled by the containment class
    d->cleanUpAndDelete();
}

bool Applet::destroyed() const
{
    return d->transient;
}



ConfigLoader *Applet::configScheme() const
{
    return d->configLoader;
}

DataEngine *Applet::dataEngine(const QString &name) const
{
    if (d->remoteLocation.isEmpty()) {
        return d->dataEngine(name);
    } else {
        return d->remoteDataEngine(d->remoteLocation, name);
    }
}

Package Applet::package() const
{
    return d->package ? *d->package : Package();
}

QGraphicsView *Applet::view() const
{
    // It's assumed that we won't be visible on more than one view here.
    // Anything that actually needs view() should only really care about
    // one of them anyway though.
    if (!scene()) {
        return 0;
    }

    QGraphicsView *found = 0;
    QGraphicsView *possibleFind = 0;
    //kDebug() << "looking through" << scene()->views().count() << "views";
    foreach (QGraphicsView *view, scene()->views()) {
        //kDebug() << "     checking" << view << view->sceneRect()
        //         << "against" << sceneBoundingRect() << scenePos();
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            //kDebug() << "     found something!" << view->isActiveWindow();
            if (view->isActiveWindow()) {
                found = view;
            } else {
                possibleFind = view;
            }
        }
    }

    return found ? found : possibleFind;
}

QRectF Applet::mapFromView(const QGraphicsView *view, const QRect &rect) const
{
    // Why is this adjustment needed? Qt calculation error?
    return mapFromScene(view->mapToScene(rect)).boundingRect().adjusted(0, 0, 1, 1);
}

QRect Applet::mapToView(const QGraphicsView *view, const QRectF &rect) const
{
    // Why is this adjustment needed? Qt calculation error?
    return view->mapFromScene(mapToScene(rect)).boundingRect().adjusted(0, 0, -1, -1);
}

QPoint Applet::popupPosition(const QSize &s) const
{
    return popupPosition(s, Qt::AlignLeft);
}

QPoint Applet::popupPosition(const QSize &s, Qt::AlignmentFlag alignment) const
{
    Corona * corona = qobject_cast<Corona*>(scene());
    Q_ASSERT(corona);

    return corona->popupPosition(this, s, alignment);
}

void Applet::updateConstraints(Plasma::Constraints constraints)
{
    d->scheduleConstraintsUpdate(constraints);
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    //NOTE: do NOT put any code in here that reacts to constraints updates
    //      as it will not get called for any applet that reimplements constraintsEvent
    //      without calling the Applet:: version as well, which it shouldn't need to.
    //      INSTEAD put such code into flushPendingConstraintsEvents
    Q_UNUSED(constraints)
    //kDebug() << constraints << "constraints are FormFactor: " << formFactor()
    //         << ", Location: " << location();
    if (d->script) {
        d->script->constraintsEvent(constraints);
    }
}

void Applet::setBusy(bool busy)
{
    d->setBusy(busy);
}

bool Applet::isBusy() const
{
    return d->isBusy();
}

QString Applet::name() const
{
    if (d->isContainment) {
        const Containment *c = qobject_cast<const Containment*>(this);
        if (c && c->d->isPanelContainment()) {
            return i18n("Panel");
        } else if (!d->appletDescription.isValid()) {
            return i18n("Unknown");
        } else {
            return d->appletDescription.name();
        }
    } else if (!d->appletDescription.isValid()) {
        return i18n("Unknown Widget");
    }

    return d->appletDescription.name();
}

QFont Applet::font() const
{
    return QApplication::font();
}

QString Applet::icon() const
{
    if (!d->appletDescription.isValid()) {
        return QString();
    }

    return d->appletDescription.icon();
}

QString Applet::pluginName() const
{
    if (!d->appletDescription.isValid()) {
        return d->mainConfigGroup()->readEntry("plugin", QString());
    }

    return d->appletDescription.pluginName();
}

bool Applet::shouldConserveResources() const
{
#if !PLASMA_NO_SOLID
    return Solid::PowerManagement::appShouldConserveResources();
#else
    return true;
#endif
}

QString Applet::category() const
{
    if (!d->appletDescription.isValid()) {
        return i18nc("misc category", "Miscellaneous");
    }

    return d->appletDescription.category();
}

QString Applet::category(const KPluginInfo &applet)
{
    return applet.property("X-KDE-PluginInfo-Category").toString();
}

QString Applet::category(const QString &appletName)
{
    if (appletName.isEmpty()) {
        return QString();
    }

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        return QString();
    }

    return offers.first()->property("X-KDE-PluginInfo-Category").toString();
}

ImmutabilityType Applet::immutability() const
{
    // if this object is itself system immutable, then just return that; it's the most
    // restrictive setting possible and will override anything that might be happening above it
    // in the Corona->Containment->Applet hierarchy
    if (d->transient || (d->mainConfig && d->mainConfig->isImmutable())) {
        return SystemImmutable;
    }

    //Returning the more strict immutability between the applet immutability, Containment and Corona
    ImmutabilityType upperImmutability = Mutable;
    Containment *cont = d->isContainment ? 0 : containment();

    if (cont) {
        upperImmutability = cont->immutability();
    } else if (Corona *corona = qobject_cast<Corona*>(scene())) {
        upperImmutability = corona->immutability();
    }

    if (upperImmutability != Mutable) {
        // it's either system or user immutable, and we already check for local system immutability,
        // so upperImmutability is guaranteed to be as or more severe as this object's immutability
        return upperImmutability;
    } else {
        return d->immutability;
    }
}

void Applet::setImmutability(const ImmutabilityType immutable)
{
    if (d->immutability == immutable || immutable == Plasma::SystemImmutable) {
        // we do not store system immutability in d->immutability since that gets saved
        // out to the config file; instead, we check with
        // the config group itself for this information at all times. this differs from
        // corona, where SystemImmutability is stored in d->immutability.
        return;
    }

    d->immutability = immutable;
    updateConstraints(ImmutableConstraint);
}

BackgroundHints Applet::backgroundHints() const
{
    return d->backgroundHints;
}

void Applet::setBackgroundHints(const Plasma::BackgroundHints hints)
{
    if (d->backgroundHints == hints) {
        return;
    }

    d->backgroundHints = hints;
    d->preferredBackgroundHints = hints;

    //Draw the standard background?
    if ((hints & StandardBackground) || (hints & TranslucentBackground)) {
        if (!d->background) {
            d->background = new Plasma::FrameSvg(this);
            QObject::connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(themeChanged()));
        }

        if ((hints & TranslucentBackground) &&
            Plasma::Theme::defaultTheme()->currentThemeHasImage("widgets/translucentbackground")) {
            d->background->setImagePath("widgets/translucentbackground");
        } else {
            d->background->setImagePath("widgets/background");
        }

        d->background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        qreal left, top, right, bottom;
        d->background->getMargins(left, top, right, bottom);
        setContentsMargins(left, right, top, bottom);
        QSizeF fitSize(left + right, top + bottom);
        d->background->resizeFrame(boundingRect().size());

        //if the background has an "overlay" element decide a random position for it and then save it so it's consistent across plasma starts
        if (d->background->hasElement("overlay")) {
            QSize overlaySize = d->background->elementSize("overlay");

            //position is in the boundaries overlaySize.width()*2, overlaySize.height()
            qsrand(id());
            d->background->d->overlayPos.rx() = - (overlaySize.width() /2) + (overlaySize.width() /4) * (qrand() % (4 + 1));
            d->background->d->overlayPos.ry() = (- (overlaySize.height() /2) + (overlaySize.height() /4) * (qrand() % (4 + 1)))/2;
        }
    } else if (d->background) {
        qreal left, top, right, bottom;
        d->background->getMargins(left, top, right, bottom);

        delete d->background;
        d->background = 0;
        setContentsMargins(0, 0, 0, 0);
    }

    update();
}

bool Applet::hasFailedToLaunch() const
{
    return d->failed;
}

bool Applet::configurationRequired() const
{
    return d->needsConfig;
}

void Applet::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    if (d->needsConfig == needsConfig) {
        return;
    }

    d->needsConfig = needsConfig;
    d->showConfigurationRequiredMessage(needsConfig, reason);
}

void Applet::showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons)
{
    d->showMessage(icon, message, buttons);
}

QVariantList Applet::startupArguments() const
{
    return d->args;
}

ItemStatus Applet::status() const
{
    return d->itemStatus;
}

void Applet::setStatus(const ItemStatus status)
{
    d->itemStatus = status;
    emit newStatus(status);
}

void Applet::flushPendingConstraintsEvents()
{
    if (d->pendingConstraints == NoConstraint) {
        return;
    }

    if (d->constraintsTimer.isActive()) {
        d->constraintsTimer.stop();
    }

    //kDebug() << "fushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Plasma::Constraints c = d->pendingConstraints;
    d->pendingConstraints = NoConstraint;

    if (c & Plasma::StartupCompletedConstraint) {
        //common actions
        bool unlocked = immutability() == Mutable;
        QAction *closeApplet = d->actions->action("remove");
        if (closeApplet) {
            closeApplet->setEnabled(unlocked);
            closeApplet->setVisible(unlocked);
            connect(closeApplet, SIGNAL(triggered(bool)), this, SLOT(selectItemToDestroy()), Qt::UniqueConnection);
        }

        QAction *configAction = d->actions->action("configure");
        if (configAction) {
            if (d->isContainment) {
                connect(configAction, SIGNAL(triggered(bool)), this, SLOT(requestConfiguration()), Qt::UniqueConnection);
            } else {
                connect(configAction, SIGNAL(triggered(bool)), this, SLOT(showConfigurationInterface()), Qt::UniqueConnection);
            }

            if (d->hasConfigurationInterface) {
                bool canConfig = unlocked || KAuthorized::authorize("plasma/allow_configure_when_locked");
                configAction->setVisible(canConfig);
                configAction->setEnabled(canConfig);
            }
        }

        QAction *runAssociatedApplication = d->actions->action("run associated application");
        if (runAssociatedApplication) {
            connect(runAssociatedApplication, SIGNAL(triggered(bool)), this, SLOT(runAssociatedApplication()), Qt::UniqueConnection);
        }

        d->updateShortcuts();
        Corona * corona = qobject_cast<Corona*>(scene());
        if (corona) {
            connect(corona, SIGNAL(shortcutsChanged()), this, SLOT(updateShortcuts()), Qt::UniqueConnection);
        }
    }

    if (c & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Mutable;
        QAction *action = d->actions->action("remove");
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }

        action = d->actions->action("configure");
        if (action && d->hasConfigurationInterface) {
            bool canConfig = unlocked || KAuthorized::authorize("plasma/allow_configure_when_locked");
            action->setVisible(canConfig);
            action->setEnabled(canConfig);
        }

        if (!unlocked && d->handle) {
            AppletHandle *h = d->handle.data();
            disconnect(this);

            QGraphicsScene *s = scene();
            if (s && h->scene() == s) {
                s->removeItem(h);
            }

            h->deleteLater();
        }

        emit immutabilityChanged(immutability());
    }

    if (c & Plasma::SizeConstraint) {
        d->positionMessageOverlay();

        if (d->started && layout()) {
            layout()->updateGeometry();
        }
    }

    if (c & Plasma::FormFactorConstraint) {
        FormFactor f = formFactor();
        if (!d->isContainment && f != Vertical && f != Horizontal) {
            setBackgroundHints(d->preferredBackgroundHints);
        } else {
            BackgroundHints hints = d->preferredBackgroundHints;
            setBackgroundHints(NoBackground);
            d->preferredBackgroundHints = hints;
        }

        if (d->failed) {
            if (f == Vertical || f == Horizontal) {
                QGraphicsLayoutItem *item = layout()->itemAt(1);
                layout()->removeAt(1);
                delete item;
            }
        }

        // avoid putting rotated applets in panels
        if (f == Vertical || f == Horizontal) {
            QTransform at;
            at.rotateRadians(0);
            setTransform(at);
        }

        //was a size saved for a particular form factor?
        if (d->sizeForFormFactor.contains(f)) {
            resize(d->sizeForFormFactor.value(f));
        }
    }

    if (!size().isEmpty() &&
        ((c & Plasma::StartupCompletedConstraint) || (c & Plasma::SizeConstraint && !(c & Plasma::FormFactorConstraint)))) {
        d->sizeForFormFactor[formFactor()] = size();
    }

    if (c & Plasma::SizeConstraint || c & Plasma::FormFactorConstraint) {
        if (aspectRatioMode() == Plasma::Square || aspectRatioMode() == Plasma::ConstrainedSquare) {
            // enforce square size in panels
            //save the old size policy. since ignored doesn't (yet) have a valid use case in containments, use it as special unset value
            if (d->preferredSizePolicy == QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored)) {
                d->preferredSizePolicy = sizePolicy();
            }
            if (formFactor() == Horizontal) {
                setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
            } else if (formFactor() == Vertical) {
                setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
            } else if (d->preferredSizePolicy != QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored)) {
                setSizePolicy(d->preferredSizePolicy);
            }
        }
        updateGeometry();
    }

    // now take care of constraints in special subclasses: Contaiment and PopupApplet
    Containment* containment = qobject_cast<Plasma::Containment*>(this);
    if (d->isContainment && containment) {
        containment->d->containmentConstraintsEvent(c);
    }

    PopupApplet* popup = qobject_cast<Plasma::PopupApplet*>(this);
    if (popup) {
        popup->d->popupConstraintsEvent(c);
    }

    // pass the constraint on to the actual subclass
    constraintsEvent(c);

    if (c & StartupCompletedConstraint) {
        // start up is done, we can now go do a mod timer
        if (d->modificationsTimer) {
            if (d->modificationsTimer->isActive()) {
                d->modificationsTimer->stop();
            }
        } else {
            d->modificationsTimer = new QBasicTimer;
        }
    }
}

int Applet::type() const
{
    return Type;
}

QList<QAction*> Applet::contextualActions()
{
    //kDebug() << "empty context actions";
    return d->script ? d->script->contextualActions() : QList<QAction*>();
}

QAction *Applet::action(QString name) const
{
    return d->actions->action(name);
}

void Applet::addAction(QString name, QAction *action)
{
    d->actions->addAction(name, action);
}

void Applet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!d->started) {
        //kDebug() << "not started";
        return;
    }

    if (transform().isRotating()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->setRenderHint(QPainter::Antialiasing);
    }

    if (d->background &&
        formFactor() != Plasma::Vertical &&
        formFactor() != Plasma::Horizontal) {
        //kDebug() << "option rect is" << option->rect;
        d->background->paintFrame(painter);
    }

    if (d->failed) {
        //kDebug() << "failed!";
        return;
    }

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect contentsRect = QRectF(QPointF(0, 0),
                                boundingRect().size()).adjusted(left, top, -right, -bottom).toRect();

    if (widget && d->isContainment) {
        // note that the widget we get is actually the viewport of the view, not the view itself
        View* v = qobject_cast<Plasma::View*>(widget->parent());
        Containment* c = qobject_cast<Plasma::Containment*>(this);

        if (!v || v->isWallpaperEnabled()) {

            // paint the wallpaper
            if (c && c->drawWallpaper() && c->wallpaper()) {
                Wallpaper *w = c->wallpaper();
                if (!w->isInitialized()) {
                    // delayed paper initialization
                    KConfigGroup wallpaperConfig = c->config();
                    wallpaperConfig = KConfigGroup(&wallpaperConfig, "Wallpaper");
                    wallpaperConfig = KConfigGroup(&wallpaperConfig, w->pluginName());
                    w->restore(wallpaperConfig);
                    disconnect(w, SIGNAL(update(QRectF)), this, SLOT(updateRect(QRectF)));
                    connect(w, SIGNAL(update(QRectF)), this, SLOT(updateRect(QRectF)));
                }

                painter->save();
                c->wallpaper()->paint(painter, option->exposedRect);
                painter->restore();
            }

            // .. and now paint the actual containment interface, but with
            //  a Containment style option based on the one we get
            //  the view must be assigned only if its containment is actually our own
            Containment::StyleOption coption(*option);
            if (v && v->containment() == containment()) {
                coption.view = v;
            }
            paintInterface(painter, &coption, contentsRect);
        }
    } else {
        //kDebug() << "paint interface of" << (QObject*) this;
        // paint the applet's interface
        paintInterface(painter, option, contentsRect);
    }
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    if (d->script) {
        d->script->paintInterface(painter, option, contentsRect);
    } else {
        //kDebug() << "Applet::paintInterface() default impl";
    }
}

FormFactor Applet::formFactor() const
{
    Containment *c = containment();
    QGraphicsWidget *pw = qobject_cast<QGraphicsWidget *>(parent());
    if (!pw) {
        pw = dynamic_cast<QGraphicsWidget *>(parentItem());
    }
    Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(pw);
    //assumption: this loop is usually is -really- short or doesn't run at all
    while (!parentApplet && pw && pw->parentWidget()) {
        QGraphicsWidget *parentWidget = qobject_cast<QGraphicsWidget *>(pw->parent());
        if (!parentWidget) {
            parentWidget = dynamic_cast<QGraphicsWidget *>(pw->parentItem());
        }
        pw = parentWidget;
        parentApplet = qobject_cast<Plasma::Applet *>(pw);
    }


    const PopupApplet *pa = dynamic_cast<const PopupApplet *>(this);

    //if the applet is in a widget that isn't a containment
    //try to retrieve the formFactor from the parent size
    //we can't use our own sizeHint here because it needs formFactor, so endless recursion.
    // a popupapplet can always be constrained.
    // a normal applet should to but
    //FIXME: not always constrained to not break systemmonitor
    if (parentApplet && parentApplet != c && c != this && (pa || layout())) {
        if (pa || (parentApplet->size().height() < layout()->effectiveSizeHint(Qt::MinimumSize).height())) {
            return Plasma::Horizontal;
        } else if (pa || (parentApplet->size().width() < layout()->effectiveSizeHint(Qt::MinimumSize).width())) {
            return Plasma::Vertical;
        }
        return parentApplet->formFactor();
    }

    return c ? c->d->formFactor : Plasma::Planar;
}

Containment *Applet::containment() const
{
    if (d->isContainment) {
        Containment *c = qobject_cast<Containment*>(const_cast<Applet*>(this));
        if (c) {
            return c;
        }
    }

    QGraphicsItem *parent = parentItem();
    Containment *c = 0;

    while (parent) {
        Containment *possibleC = dynamic_cast<Containment*>(parent);
        if (possibleC && possibleC->Applet::d->isContainment) {
            c = possibleC;
            break;
        }
        parent = parent->parentItem();
    }

    if (!c) {
        //if the applet is an offscreen widget its parentItem will be 0, while its parent
        //will be its parentWidget, so here we check the QObject hierarchy.
        QObject *objParent = this->parent();
        while (objParent) {
            Containment *possibleC = qobject_cast<Containment*>(objParent);
            if (possibleC && possibleC->Applet::d->isContainment) {
                c = possibleC;
                break;
            }
            objParent = objParent->parent();
        }
    }

    return c;
}

void Applet::setGlobalShortcut(const KShortcut &shortcut)
{
    if (!d->activationAction) {
        d->activationAction = new KAction(this);
        d->activationAction->setText(i18n("Activate %1 Widget", name()));
        d->activationAction->setObjectName(QString("activate widget %1").arg(id())); // NO I18N
        connect(d->activationAction, SIGNAL(triggered()), this, SIGNAL(activate()));
        connect(d->activationAction, SIGNAL(globalShortcutChanged(QKeySequence)),
                this, SLOT(globalShortcutChanged()));

        QList<QWidget *> widgets = d->actions->associatedWidgets();
        foreach (QWidget *w, widgets) {
            w->addAction(d->activationAction);
        }
    } else if (d->activationAction->globalShortcut() == shortcut) {
        return;
    }

    //kDebug() << "before" << shortcut.primary() << d->activationAction->globalShortcut().primary();
    d->activationAction->setGlobalShortcut(
        shortcut,
        KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
        KAction::NoAutoloading);
    d->globalShortcutChanged();
}

KShortcut Applet::globalShortcut() const
{
    if (d->activationAction) {
        return d->activationAction->globalShortcut();
    }

    return KShortcut();
}

bool Applet::isPopupShowing() const
{
    return false;
}

void Applet::addAssociatedWidget(QWidget *widget)
{
    d->actions->addAssociatedWidget(widget);
}

void Applet::removeAssociatedWidget(QWidget *widget)
{
    d->actions->removeAssociatedWidget(widget);
}

Location Applet::location() const
{
    Containment *c = containment();
    return c ? c->d->location : Plasma::Desktop;
}

Plasma::AspectRatioMode Applet::aspectRatioMode() const
{
    return d->aspectRatioMode;
}

void Applet::setAspectRatioMode(Plasma::AspectRatioMode mode)
{
    PopupApplet *popup = qobject_cast<PopupApplet *>(this);
    if (popup && popup->d->dialogPtr) {
        popup->d->dialogPtr.data()->setAspectRatioMode(mode);
        popup->d->savedAspectRatio = mode;
    }

    d->aspectRatioMode = mode;
}

void Applet::registerAsDragHandle(QGraphicsItem *item)
{
    if (!item || d->registeredAsDragHandle.contains(item)) {
        return;
    }

    d->registeredAsDragHandle.insert(item);
    item->installSceneEventFilter(this);
}

void Applet::unregisterAsDragHandle(QGraphicsItem *item)
{
    if (!item) {
        return;
    }

    if (d->registeredAsDragHandle.remove(item)) {
        if (item != this) {
            item->removeSceneEventFilter(this);
        }
    }
}

bool Applet::isRegisteredAsDragHandle(QGraphicsItem *item)
{
    return d->registeredAsDragHandle.contains(item);
}

bool Applet::hasConfigurationInterface() const
{
    return d->hasConfigurationInterface;
}

void Applet::publish(AnnouncementMethods methods, const QString &resourceName)
{
    if (!d->remotingService) {
        d->remotingService = new PlasmoidService(this);
    }

    const QString resName = resourceName.isEmpty() ?  i18nc("%1 is the name of a plasmoid, %2 the name of the machine that plasmoid is published on",
                                                         "%1 on %2", name(), QHostInfo::localHostName())
                                                   : resourceName;
#ifndef NDEBUG
    kDebug() << "publishing package under name " << resName;
#endif
    if (d->package && d->package->isValid()) {
        d->remotingService->d->publish(methods, resName, d->package->metadata());
    } else if (!d->package && d->appletDescription.isValid()) {
        d->remotingService->d->publish(methods, resName, d->appletDescription);
    } else {
        delete d->remotingService;
        d->remotingService  = 0;
#ifndef NDEBUG
        kDebug() << "Can not publish invalid applets.";
#endif
    }
}

void Applet::unpublish()
{
    if (d->remotingService) {
        d->remotingService->d->unpublish();
    }
}

bool Applet::isPublished() const
{
    return d->remotingService && d->remotingService->d->isPublished();
}

void Applet::setHasConfigurationInterface(bool hasInterface)
{
    if (hasInterface == d->hasConfigurationInterface) {
        return;
    }

    QAction *configAction = d->actions->action("configure");
    if (configAction) {
        bool enable = hasInterface;
        if (enable) {
            const bool unlocked = immutability() == Mutable;
            enable = unlocked || KAuthorized::authorize("plasma/allow_configure_when_locked");
        }
        configAction->setEnabled(enable);
    }

    d->hasConfigurationInterface = hasInterface;
}

bool Applet::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
            case QEvent::GraphicsSceneHoverEnter:
                //kDebug() << "got hoverenterEvent" << immutability() << " " << immutability();
                if (immutability() == Mutable) {
                    QGraphicsWidget *pw = this;
                    //This is for the rare case of applet in applet (systray)
                    //if the applet is in an applet that is not a containment, don't create the handle BUG:301648
                    while (pw = pw->parentWidget()) {
                        if (qobject_cast<Containment *>(pw)) {
                            break;
                        } else if (qobject_cast<Applet *>(pw)) {
                            return false;
                        }
                    }

                    QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent*>(event);
                    if (d->handle) {
                        d->handle.data()->setHoverPos(he->pos());
                    } else {
                        //kDebug() << "generated applet handle";
                        AppletHandle *handle = new AppletHandle(containment(), this, he->pos());
                        connect(handle, SIGNAL(disappearDone(AppletHandle*)),
                                this, SLOT(handleDisappeared(AppletHandle*)));
                        connect(this, SIGNAL(geometryChanged()),
                                handle, SLOT(appletResized()));
                        d->handle = handle;
                    }
                }
            break;

            case QEvent::GraphicsSceneHoverMove:
                if (d->handle && !d->handle.data()->shown() && immutability() == Mutable) {
                    QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent*>(event);
                    d->handle.data()->setHoverPos(he->pos());
                }
            break;

        default:
            break;
        }

    }

    switch (event->type()) {
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    {
        // don't move when the containment is not mutable,
        // in the rare case the containment doesn't exists consider it as mutable
        if ((flags() & ItemIsMovable) && d->registeredAsDragHandle.contains(watched)) {
            Containment *c = containment();
            if (!c || c->immutability() == Mutable) {
                scene()->sendEvent(this, event);
                return false;
            }
        }
        break;
    }

    default:
        break;
    }

    return QGraphicsItem::sceneEventFilter(watched, event);
}

void Applet::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (immutability() == Mutable && formFactor() == Plasma::Planar && (flags() & ItemIsMovable)) {
        QGraphicsWidget::mouseMoveEvent(event);
    }
}

void Applet::focusInEvent(QFocusEvent *event)
{
    if (!isContainment() && containment()) {
        //focusing an applet may trigger this event again, but we won't be here more than twice
        containment()->d->focusApplet(this);
    }

    QGraphicsWidget::focusInEvent(event);
}

void Applet::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);

    if (d->background) {
        d->background->resizeFrame(boundingRect().size());
    }

    updateConstraints(Plasma::SizeConstraint);

    d->scheduleModificationNotification();
    emit geometryChanged();
}

bool Applet::isUserConfiguring() const
{
    return KConfigDialog::exists(d->configDialogId());
}

void Applet::showConfigurationInterface()
{
    if (!hasConfigurationInterface()) {
        return;
    }

    if (immutability() != Mutable && !KAuthorized::authorize("plasma/allow_configure_when_locked")) {
        return;
    }

    KConfigDialog *dlg = KConfigDialog::exists(d->configDialogId());

    if (dlg) {
        KWindowSystem::setOnDesktop(dlg->winId(), KWindowSystem::currentDesktop());
        dlg->show();
        KWindowSystem::activateWindow(dlg->winId());
        return;
    }

    d->publishUI.publishCheckbox = 0;
    if (d->package) {
        KConfigDialog *dialog = 0;

        const QString uiFile = d->package->filePath("mainconfigui");
        KDesktopFile df(d->package->path() + "/metadata.desktop");
        const QStringList kcmPlugins = df.desktopGroup().readEntry("X-Plasma-ConfigPlugins", QStringList());
        if (!uiFile.isEmpty() || !kcmPlugins.isEmpty()) {
            KConfigSkeleton *configLoader = d->configLoader ? d->configLoader : new KConfigSkeleton(0);
            dialog = new AppletConfigDialog(0, d->configDialogId(), configLoader);

            if (!d->configLoader) {
                // delete the temporary when this dialog is done
                configLoader->setParent(dialog);
            }

            dialog->setWindowTitle(d->configWindowTitle());
            dialog->setAttribute(Qt::WA_DeleteOnClose, true);
            bool hasPages = false;

            QFile f(uiFile);
            QUiLoader loader;
            QWidget *w = loader.load(&f);
            if (w) {
                dialog->addPage(w, i18n("Settings"), icon(), i18n("%1 Settings", name()));
                hasPages = true;
            }

            foreach (const QString &kcm, kcmPlugins) {
#if !PLASMA_NO_KUTILS
                KCModuleProxy *module = new KCModuleProxy(kcm);
                if (module->realModule()) {
                    //preemptively load modules to prevent save() crashing on some kcms, like powerdevil ones
                    module->load();
                    connect(module, SIGNAL(changed(bool)), dialog, SLOT(settingsModified(bool)));
                    connect(dialog, SIGNAL(okClicked()),
                            module->realModule(), SLOT(save()));
                    connect(dialog, SIGNAL(applyClicked()),
                            module->realModule(), SLOT(save()));
                    dialog->addPage(module, module->moduleInfo().moduleName(), module->moduleInfo().icon());
                    hasPages = true;
                } else {
                    delete module;
                }
#else
                KService::Ptr service = KService::serviceByStorageId(kcm);
                if (service) {
                    QString error;
                    KCModule *module = service->createInstance<KCModule>(dialog, QVariantList(), &error);
                    if (module) {
                        module->load();
                        connect(module, SIGNAL(changed(bool)), dialog, SLOT(settingsModified(bool)));
                        connect(dialog, SIGNAL(okClicked()),
                                module, SLOT(save()));
                        connect(dialog, SIGNAL(applyClicked()), 
                                module, SLOT(save()));
                        dialog->addPage(module, service->name(), service->icon());
                        hasPages = true;
                    } else {
#ifndef NDEBUG
                        kDebug() << "failed to load kcm" << kcm << "for" << name();
#endif
                    }
                }
#endif
            }

            if (hasPages) {
                d->addGlobalShortcutsPage(dialog);
                d->addPublishPage(dialog);
                dialog->show();
            } else {
                delete dialog;
                dialog = 0;
            }
        }

        if (!dialog && d->script) {
            d->script->showConfigurationInterface();
        }
    } else if (d->script) {
        d->script->showConfigurationInterface();
    } else {
        KConfigDialog *dialog = d->generateGenericConfigDialog();
        d->addStandardConfigurationPages(dialog);
        showConfigurationInterface(dialog);
    }

    emit releaseVisualFocus();
}

void Applet::showConfigurationInterface(QWidget *widget)
{
    if (!containment() || !containment()->corona() ||
        !containment()->corona()->dialogManager()) {
        widget->show();
        return;
    }

    QMetaObject::invokeMethod(containment()->corona()->dialogManager(), "showDialog", Q_ARG(QWidget *, widget), Q_ARG(Plasma::Applet *, this));
}

void Applet::configChanged()
{
    if (d->script) {
        if (d->configLoader) {
            d->configLoader->readConfig();
        }
        d->script->configChanged();
    }
}

void Applet::createConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
    // virtual method reimplemented by subclasses.
    // do not put anything here ...
}

bool Applet::hasAuthorization(const QString &constraint) const
{
    KConfigGroup constraintGroup(KSharedConfig::openConfig(), "Constraints");
    return constraintGroup.readEntry(constraint, true);
}

void Applet::setAssociatedApplication(const QString &string)
{
    AssociatedApplicationManager::self()->setApplication(this, string);

    QAction *runAssociatedApplication = d->actions->action("run associated application");
    if (runAssociatedApplication) {
        bool valid = AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
        valid = valid && hasAuthorization("LaunchApp"); //obey security!
        runAssociatedApplication->setVisible(valid);
        runAssociatedApplication->setEnabled(valid);
    }
}

void Applet::setAssociatedApplicationUrls(const QList<QUrl> &urls)
{
    AssociatedApplicationManager::self()->setUrls(this, urls);

    QAction *runAssociatedApplication = d->actions->action("run associated application");
    if (runAssociatedApplication) {
        bool valid = AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
        valid = valid && hasAuthorization("LaunchApp"); //obey security!
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
    if (hasAuthorization("LaunchApp")) {
        AssociatedApplicationManager::self()->run(this);
    }
}

bool Applet::hasValidAssociatedApplication() const
{
    return AssociatedApplicationManager::self()->appletHasValidAssociatedApplication(this);
}

KPluginInfo::List Applet::listAppletInfo(const QString &category, const QString &parentApp)
{
   return PluginLoader::self()->listAppletInfo(category, parentApp);
}

KPluginInfo::List Applet::listAppletInfoForMimeType(const QString &mimeType)
{
    QString constraint = AppletPrivate::parentAppConstraint();
    constraint.append(QString(" and '%1' in [X-Plasma-DropMimeTypes]").arg(mimeType));
    //kDebug() << "listAppletInfoForMimetype with" << mimeType << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    AppletPrivate::filterOffers(offers);
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Applet::listAppletInfoForUrl(const QUrl &url)
{
    QString constraint = AppletPrivate::parentAppConstraint();
    constraint.append(" and exist [X-Plasma-DropUrlPatterns]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    AppletPrivate::filterOffers(offers);

    KPluginInfo::List allApplets = KPluginInfo::fromServices(offers);
    KPluginInfo::List filtered;
    foreach (const KPluginInfo &info, allApplets) {
        QStringList urlPatterns = info.property("X-Plasma-DropUrlPatterns").toStringList();
        foreach (const QString &glob, urlPatterns) {
            QRegExp rx(glob);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(url.toString())) {
#ifndef NDEBUG
                kDebug() << info.name() << "matches" << glob << url;
#endif
                filtered << info;
            }
        }
    }

    return filtered;
}

QStringList Applet::listCategories(const QString &parentApp, bool visibleOnly)
{
    QString constraint = AppletPrivate::parentAppConstraint(parentApp);
    constraint.append(" and exist [X-KDE-PluginInfo-Category]");

    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    foreach (const QString &category, excluded) {
        constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    AppletPrivate::filterOffers(offers);

    QStringList categories;
    QSet<QString> known = AppletPrivate::knownCategories();
    foreach (const KService::Ptr &applet, offers) {
        QString appletCategory = applet->property("X-KDE-PluginInfo-Category").toString();
        if (visibleOnly && applet->noDisplay()) {
            // we don't want to show the hidden category
            continue;
        }

        //kDebug() << "   and we have " << appletCategory;
        if (!appletCategory.isEmpty() && !known.contains(appletCategory.toLower())) {
#ifndef NDEBUG
            kDebug() << "Unknown category: " << applet->name() << "says it is in the"
                     << appletCategory << "category which is unknown to us";
#endif
            appletCategory.clear();
        }

        if (appletCategory.isEmpty()) {
            if (!categories.contains(i18nc("misc category", "Miscellaneous"))) {
                categories << i18nc("misc category", "Miscellaneous");
            }
        } else  if (!categories.contains(appletCategory)) {
            categories << appletCategory;
        }
    }

    categories.sort();
    return categories;
}

void Applet::setCustomCategories(const QStringList &categories)
{
    AppletPrivate::s_customCategories = QSet<QString>::fromList(categories);
}

QStringList Applet::customCategories()
{
    return AppletPrivate::s_customCategories.toList();
}

Applet *Applet::loadPlasmoid(const QString &path, uint appletId, const QVariantList &args)
{
    if (QFile::exists(path + "/metadata.desktop")) {
        KService service(path + "/metadata.desktop");
        const QStringList &types = service.serviceTypes();

        if (types.contains("Plasma/Containment")) {
            return new Containment(path, appletId, args);
        } else if (types.contains("Plasma/PopupApplet")) {
            return new PopupApplet(path, appletId, args);
        } else {
            return new Applet(path, appletId, args);
        }
    }

    return 0;
}

QVariant Applet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant ret = QGraphicsWidget::itemChange(change, value);

    //kDebug() << change;
    switch (change) {
    case ItemSceneHasChanged: {
        Corona *newCorona = qobject_cast<Corona *>(qvariant_cast<QGraphicsScene*>(value));
        if (newCorona && newCorona->immutability() != Mutable) {
            updateConstraints(ImmutableConstraint);
        }
    }
        break;
    case ItemParentChange:
        if (!d->isContainment) {
            Containment *c = containment();
            if (d->mainConfig && !c) {
                kWarning() << "Configuration object was requested prior to init(), which is too early. "
                    "Please fix this item:" << parentItem() << value.value<QGraphicsItem *>()
                    << name();

                Applet *newC = dynamic_cast<Applet*>(value.value<QGraphicsItem *>());
                if (newC) {
                    // if this is an applet, and we've just been assigned to our first containment,
                    // but the applet did something stupid like ask for the config() object prior to
                    // this happening (e.g. inits ctor) then let's repair that situation for them.
                    KConfigGroup *old = d->mainConfig;
                    KConfigGroup appletConfig = newC->config();
                    appletConfig = KConfigGroup(&appletConfig, "Applets");
                    d->mainConfig = new KConfigGroup(&appletConfig, QString::number(d->appletId));
                    old->copyTo(d->mainConfig);
                    old->deleteGroup();
                    delete old;
                }
            }
        }
        break;
    case ItemParentHasChanged:
        {
            if (isContainment()) {
                removeSceneEventFilter(this);
            } else {
                Containment *c = containment();
                if (c && c->containmentType() == Containment::DesktopContainment) {
                    installSceneEventFilter(this);
                } else {
                    removeSceneEventFilter(this);
                }
            }
        }
        break;
    case ItemPositionHasChanged:
        emit geometryChanged();
        // fall through!
    case ItemTransformHasChanged:
        d->scheduleModificationNotification();
        break;
    default:
        break;
    };

    return ret;
}

QPainterPath Applet::shape() const
{
    if (d->script) {
        return d->script->shape();
    }

    return QGraphicsWidget::shape();
}

QSizeF Applet::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);
    const FormFactor ff = formFactor();

    // in panels make sure that the contents won't exit from the panel
    if (which == Qt::MinimumSize) {
        if (ff == Horizontal) {
            hint.setHeight(0);
        } else if (ff == Vertical) {
            hint.setWidth(0);
        }
    }

    // enforce a square size in panels
    if (d->aspectRatioMode == Plasma::Square) {
        if (ff == Horizontal) {
            hint.setWidth(size().height());
        } else if (ff == Vertical) {
            hint.setHeight(size().width());
        }
    } else if (d->aspectRatioMode == Plasma::ConstrainedSquare) {
        //enforce a size not wider than tall
        if (ff == Horizontal) {
            hint.setWidth(size().height());
        //enforce a size not taller than wide
        } else if (ff == Vertical && (which == Qt::MaximumSize || size().width() <= KIconLoader::SizeLarge)) {
            hint.setHeight(size().width());
        }
    }

    return hint;
}

void Applet::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
}

void Applet::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
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
        if(!(d->pendingConstraints & Plasma::StartupCompletedConstraint)) {
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

QRect Applet::screenRect() const
{
    QGraphicsView *v = view();

    if (v) {
        QPointF bottomRight = pos();
        bottomRight.rx() += size().width();
        bottomRight.ry() += size().height();

        QPoint tL = v->mapToGlobal(v->mapFromScene(pos()));
        QPoint bR = v->mapToGlobal(v->mapFromScene(bottomRight));
        return QRect(QPoint(tL.x(), tL.y()), QSize(bR.x() - tL.x(), bR.y() - tL.y()));
    }

    //The applet doesn't have a view on it.
    //So a screenRect isn't relevant.
    return QRect(QPoint(0, 0), QSize(0, 0));
}

void Applet::raise()
{
    setZValue(++AppletPrivate::s_maxZValue);
}

void Applet::lower()
{
    setZValue(--AppletPrivate::s_minZValue);
}

bool Applet::isContainment() const
{
    return d->isContainment;
}

// PRIVATE CLASS IMPLEMENTATION



void ContainmentPrivate::checkRemoveAction()
{
    q->enableAction("remove", q->immutability() == Mutable);
}


} // Plasma namespace

#include "moc_applet.cpp"
#include "private/moc_applet_p.cpp"
