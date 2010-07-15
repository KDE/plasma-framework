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
#include <kauthorized.h>
#include <kcolorscheme.h>
#include <kdialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kkeysequencewidget.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshortcut.h>
#include <kwindowsystem.h>
#include <kpushbutton.h>
#include <krun.h>

#ifndef PLASMA_NO_SOLID
#include <solid/powermanagement.h>
#endif

#include "abstracttoolbox.h"
#include "authorizationmanager.h"
#include "authorizationrule.h"
#include "configloader.h"
#include "containment.h"
#include "corona.h"
#include "dataenginemanager.h"
#include "dialog.h"
#include "extenders/extender.h"
#include "extenders/extenderitem.h"
#include "package.h"
#include "plasma.h"
#include "scripting/appletscript.h"
#include "svg.h"
#include "framesvg.h"
#include "popupapplet.h"
#include "private/applethandle_p.h"
#include "private/extenderitem_p.h"
#include "private/framesvg_p.h"
#include "theme.h"
#include "view.h"
#include "widgets/iconwidget.h"
#include "widgets/label.h"
#include "widgets/pushbutton.h"
#include "widgets/busywidget.h"
#include "tooltipmanager.h"
#include "wallpaper.h"
#include "paintutils.h"
#include "abstractdialogmanager.h"
#include "pluginloader.h"

#include "private/associatedapplicationmanager_p.h"
#include "private/authorizationmanager_p.h"
#include "private/containment_p.h"
#include "private/extenderapplet_p.h"
#include "private/package_p.h"
#include "private/packages_p.h"
#include "private/plasmoidservice_p.h"
#include "private/popupapplet_p.h"
#include "private/remotedataengine_p.h"
#include "private/service_p.h"
#include "ui_publish.h"


namespace Plasma
{

Applet::Applet(QGraphicsItem *parent, const QString &serviceID, uint appletId)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::serviceByStorageId(serviceID), appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
}

Applet::Applet(QGraphicsItem *parent,
               const QString &serviceID,
               uint appletId,
               const QVariantList &args)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::serviceByStorageId(serviceID), appletId, this))
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
             KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString()),
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
      d(new AppletPrivate(KService::Ptr(new KService(packagePath + "/metadata.desktop")), appletId, this))
{
    Q_UNUSED(args) // FIXME?
    d->init(packagePath);
}

Applet::~Applet()
{
    //let people know that i will die
    emit appletDestroyed(this);

    if (!d->transient && d->extender) {
        //This would probably be nicer if it was located in extender. But in it's dtor, this won't
        //work since when that get's called, the applet's config() isn't accessible anymore. (same
        //problem with calling saveState(). Doing this in saveState() might be a possibility, but
        //that would require every extender savestate implementation to call it's parent function,
        //which isn't very nice.
        d->extender.data()->saveState();

        foreach (ExtenderItem *item, d->extender.data()->attachedItems()) {
            if (item->autoExpireDelay()) {
                //destroy temporary extender items, or items that aren't detached, so their
                //configuration won't linger after a plasma restart.
                item->destroy();
            }
        }
    }

    // clean up our config dialog, if any
    delete KConfigDialog::exists(d->configDialogId());
    delete d;
}

PackageStructure::Ptr Applet::packageStructure()
{
    if (!AppletPrivate::packageStructure) {
        AppletPrivate::packageStructure = new PlasmoidPackage();
    }

    return AppletPrivate::packageStructure;
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

    //kDebug() << "saving to" << group.name();
    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group.writeEntry("immutability", (int)d->immutability);
    group.writeEntry("plugin", pluginName());

    group.writeEntry("geometry", geometry());
    group.writeEntry("zvalue", zValue());

    if (!d->started) {
        return;
    }

    //FIXME: for containments, we need to have some special values here w/regards to
    //       screen affinity (e.g. "bottom of screen 0")
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
        disconnect(d->configLoader, SIGNAL(configChanged()), this, SLOT(configChanged()));
        d->configLoader->writeConfig();
        connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(configChanged()));
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
        kDebug() << "got global shortcut for" << name() << "of" << QKeySequence(shortcutText);
        kDebug() << "set to" << d->activationAction->objectName()
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

void AppletPrivate::setFocus()
{
    //kDebug() << "setting focus";
    q->setFocus(Qt::ShortcutFocusReason);
}

void Applet::setFailedToLaunch(bool failed, const QString &reason)
{
    if (d->failed == failed) {
        if (failed && !reason.isEmpty()) {
            foreach (QGraphicsItem *item, QGraphicsItem::children()) {
                Label *l = dynamic_cast<Label *>(item);
                if (l) {
                    l->setText(d->visibleFailureText(reason));
                }
            }
        }
        return;
    }

    d->failed = failed;
    prepareGeometryChange();

    foreach (QGraphicsItem *item, childItems()) {
        if (!dynamic_cast<AppletHandle *>(item)) {
            delete item;
        }
    }

    d->messageOverlay = 0;
    if (d->messageDialog) {
        d->messageDialog.data()->deleteLater();
        d->messageDialog.clear();
    }

    setLayout(0);

    if (failed) {
        setBackgroundHints(d->backgroundHints|StandardBackground);

        QGraphicsLinearLayout *failureLayout = new QGraphicsLinearLayout(this);
        failureLayout->setContentsMargins(0, 0, 0, 0);

        IconWidget *failureIcon = new IconWidget(this);
        failureIcon->setIcon(KIcon("dialog-error"));
        failureLayout->addItem(failureIcon);

        Label *failureWidget = new Plasma::Label(this);
        failureWidget->setText(d->visibleFailureText(reason));
        QLabel *label = failureWidget->nativeWidget();
        label->setWordWrap(true);
        failureLayout->addItem(failureWidget);

        Plasma::ToolTipManager::self()->registerWidget(failureIcon);
        Plasma::ToolTipContent data(i18n("Unable to load the widget"), reason,
                                    KIcon("dialog-error"));
        Plasma::ToolTipManager::self()->setContent(failureIcon, data);

        setLayout(failureLayout);
        resize(300, 250);
        d->background->resizeFrame(geometry().size());
    }

    update();
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

KConfigGroup Applet::config(const QString &group) const
{
    if (d->transient) {
        return KConfigGroup(KGlobal::config(), "PlasmaTransientsConfig");
    }

    KConfigGroup cg = config();
    return KConfigGroup(&cg, group);
}

KConfigGroup Applet::config() const
{
    if (d->transient) {
        return KConfigGroup(KGlobal::config(), "PlasmaTransientsConfig");
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
        globalAppletConfig = KConfigGroup(KGlobal::config(), group);
    }

    return KConfigGroup(&globalAppletConfig, d->globalName());
}

void Applet::destroy()
{
    if (immutability() != Mutable || d->transient || !d->started) {
        return; //don't double delete
    }

    d->transient = true;

    if (isContainment()) {
        d->cleanUpAndDelete();
    } else {
        Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
        connect(zoomAnim, SIGNAL(finished()), this, SLOT(appletAnimationComplete()));
        zoomAnim->setTargetWidget(this);
        zoomAnim->start();
    }
}

bool Applet::destroyed() const
{
    return d->transient;
}

void AppletPrivate::appletAnimationComplete()
{
    cleanUpAndDelete();
}

void AppletPrivate::selectItemToDestroy()
{
    //FIXME: this will not work nicely with multiple screens and being zoomed out!
    if (isContainment) {
        QGraphicsView *view = q->view();
        if (view && view->transform().isScaling() &&
            q->scene()->focusItem() != q) {
            QGraphicsItem *focus = q->scene()->focusItem();

            if (focus) {
                Containment *toDestroy = dynamic_cast<Containment*>(focus->topLevelItem());

                if (toDestroy) {
                    toDestroy->destroy();
                    return;
                }
            }
        }
    }

    q->destroy();
}

void AppletPrivate::updateRect(const QRectF &rect)
{
    q->update(rect);
}

void AppletPrivate::cleanUpAndDelete()
{
    //kDebug() << "???????????????? DESTROYING APPLET" << q->name() << q->scene() << " ???????????????????????????";
    QGraphicsWidget *parent = dynamic_cast<QGraphicsWidget *>(q->parentItem());
    //it probably won't matter, but right now if there are applethandles, *they* are the parent.
    //not the containment.

    //is the applet in a containment and does the containment have a layout?
    //if yes, we remove the applet in the layout
    if (parent && parent->layout()) {
        QGraphicsLayout *l = parent->layout();
        for (int i = 0; i < l->count(); ++i) {
            if (q == l->itemAt(i)) {
                l->removeAt(i);
                break;
            }
        }
    }

    if (configLoader) {
        configLoader->setDefaults();
    }

    resetConfigurationObject();

    q->scene()->removeItem(q);
    q->deleteLater();
}

void AppletPrivate::createMessageOverlay(bool usePopup)
{
    if (messageOverlay) {
        qDeleteAll(messageOverlay->children());
        messageOverlay->setLayout(0);
    }

    PopupApplet *popup = qobject_cast<Plasma::PopupApplet*>(q);

    if (!messageOverlay) {
        if (usePopup && popup) {
            if (popup->widget()) {
                messageOverlayProxy = new QGraphicsProxyWidget(q);
                messageOverlayProxy->setWidget(popup->widget());
                messageOverlay = new AppletOverlayWidget(messageOverlayProxy);
            } else if (popup->graphicsWidget() &&
                       popup->graphicsWidget() != extender.data()) {
                messageOverlay = new AppletOverlayWidget(popup->graphicsWidget());
            }
        }

        if (!messageOverlay) {
            messageOverlay = new AppletOverlayWidget(q);
        }
    }

    positionMessageOverlay();
}

void AppletPrivate::positionMessageOverlay()
{
    if (!messageOverlay) {
        return;
    }

    PopupApplet *popup = qobject_cast<Plasma::PopupApplet*>(q);
    const bool usePopup = popup && (messageOverlay->parentItem() != q);
    QGraphicsItem *topItem = q;

    if (usePopup && popup->widget()) {
        // popupapplet with widget()
        topItem = popup->d->proxy.data();
        messageOverlay->setGeometry(popup->widget()->contentsRect());
    } else if (usePopup && popup->graphicsWidget() && popup->graphicsWidget() != extender.data()) {
        // popupapplet with graphicsWidget()
        topItem = popup->graphicsWidget();
        QGraphicsWidget *w = dynamic_cast<QGraphicsWidget *>(topItem);
        messageOverlay->setGeometry(w ? w->contentsRect() : topItem->boundingRect());
    } else {
        // normal applet
        messageOverlay->setGeometry(q->contentsRect());
    }

    // raise the overlay above all the other children!
    int zValue = 100;
    foreach (QGraphicsItem *child, topItem->children()) {
        if (child->zValue() > zValue) {
            zValue = child->zValue() + 1;
        }
    }
    messageOverlay->setZValue(zValue);
}

void AppletPrivate::destroyMessageOverlay()
{
    if (messageDialog) {
        messageDialog.data()->animatedHide(Plasma::locationToInverseDirection(q->location()));
        //messageDialog.data()->deleteLater();
        messageDialog.clear();
    }

    if (!messageOverlay) {
        return;
    }

    messageOverlay->destroy();
    messageOverlay = 0;

    if (messageOverlayProxy) {
        messageOverlayProxy->setWidget(0);
        delete messageOverlayProxy;
        messageOverlayProxy = 0;
    }

    MessageButton buttonCode = ButtonNo;
    //find out if we're disappearing because of a button press
    PushButton *button = qobject_cast<PushButton *>(q->sender());
    if (button) {
        if (button->text() == i18n("Ok")) {
            buttonCode = ButtonOk;
        }
        if (button->text() == i18n("Yes")) {
            buttonCode = ButtonYes;
        }
        if (button->text() == i18n("No")) {
            buttonCode = ButtonNo;
        }
        if (button->text() == i18n("Cancel")) {
        }

        emit q->messageButtonPressed(buttonCode);
    }
}

ConfigLoader *Applet::configScheme() const
{
    return d->configLoader;
}

DataEngine *Applet::dataEngine(const QString &name) const
{
    if (!d->remoteLocation.isEmpty()) {
        return d->remoteDataEngine(KUrl(d->remoteLocation), name);
    }

    if (!package() || package()->metadata().remoteLocation().isEmpty()) {
        return d->dataEngine(name);
    } else {
        return d->remoteDataEngine(KUrl(package()->metadata().remoteLocation()), name);
    }
}

const Package *Applet::package() const
{
    return d->package;
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

void Applet::initExtenderItem(ExtenderItem *item)
{
    if (d->script) {
        emit extenderItemRestored(item);
    } else {
        kWarning() << "Missing implementation of initExtenderItem in the applet "
                   << item->config().readEntry("SourceAppletPluginName", "")
                   << "!\n Any applet that uses extenders should implement initExtenderItem to "
                   << "instantiate a widget. Destroying the item...";
        item->destroy();
    }
}

Extender *Applet::extender() const
{
    if (!d->extender) {
        new Extender(const_cast<Applet*>(this));
    }

    return d->extender.data();
}

void Applet::setBusy(bool busy)
{
    if (busy) {
        if (!d->busyWidget && !d->busyWidgetTimer.isActive()) {
            d->busyWidgetTimer.start(500, this);
        }
    } else {
        d->busyWidgetTimer.stop();
        if (d->busyWidget) {
            d->busyWidget = 0;
            d->destroyMessageOverlay();
        }
    }
}

bool Applet::isBusy() const
{
    return d->busyWidgetTimer.isActive() || (d->busyWidget && d->busyWidget->isVisible());
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
        return QString();
    }

    return d->appletDescription.pluginName();
}

bool Applet::shouldConserveResources() const
{
#ifndef PLASMA_NO_SOLID
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
    //Returning the more strict immutability between the applet immutability and Corona one
    ImmutabilityType coronaImmutability = Mutable;

    if (qobject_cast<Corona*>(scene())) {
        coronaImmutability = static_cast<Corona*>(scene())->immutability();
    }

    if (coronaImmutability == SystemImmutable) {
        return SystemImmutable;
    } else if (coronaImmutability == UserImmutable && d->immutability != SystemImmutable) {
        return UserImmutable;
    } else {
        return d->immutability;
    }
}

void Applet::setImmutability(const ImmutabilityType immutable)
{
    if (d->immutability == immutable || d->immutability == Plasma::SystemImmutable) {
        return;
    }

    d->immutability = immutable;
    updateConstraints(ImmutableConstraint);
}

Applet::BackgroundHints Applet::backgroundHints() const
{
    return d->backgroundHints;
}

void Applet::setBackgroundHints(const BackgroundHints hints)
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

void Applet::paintWindowFrame(QPainter *painter,
                              const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
    //Here come the code for the window frame
    //kDebug() << windowFrameGeometry();
    //painter->drawRoundedRect(windowFrameGeometry(), 5, 5);
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

    if (!needsConfig) {
        d->destroyMessageOverlay();
        return;
    }

    d->createMessageOverlay(true);
    d->messageOverlay->opacity = 0.4;

    QGraphicsGridLayout *configLayout = new QGraphicsGridLayout(d->messageOverlay);
    configLayout->setContentsMargins(0, 0, 0, 0);

  //  configLayout->addStretch();
    configLayout->setColumnStretchFactor(0, 5);
    configLayout->setColumnStretchFactor(2, 5);
    configLayout->setRowStretchFactor(0, 5);
    configLayout->setRowStretchFactor(3, 5);

    int row = 1;
    if (!reason.isEmpty()) {
        Label *explanation = new Label(d->messageOverlay);
        explanation->setText(reason);
        configLayout->addItem(explanation, row, 1);
        configLayout->setColumnStretchFactor(1, 5);
        ++row;
        configLayout->setAlignment(explanation, Qt::AlignBottom | Qt::AlignCenter);
    }

    PushButton *configWidget = new PushButton(d->messageOverlay);
    if (!qobject_cast<Plasma::PopupApplet *>(this) && (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical)) {
        configWidget->setImage("widgets/configuration-icons", "configure");
        configWidget->setMaximumSize(24,24);
        configWidget->setMinimumSize(24,24);
    } else {
        configWidget->setText(i18n("Configure..."));
    }
    connect(configWidget, SIGNAL(clicked()), this, SLOT(showConfigurationInterface()));
    configLayout->addItem(configWidget, row, 1);

    //configLayout->setAlignment(configWidget, Qt::AlignTop | Qt::AlignCenter);
    //configLayout->addStretch();

    d->messageOverlay->show();
}

void Applet::showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons)
{
    if (message.isEmpty()) {
        d->destroyMessageOverlay();
        return;
    }

    Corona *corona = qobject_cast<Corona *>(scene());
    QGraphicsWidget *mainWidget = new QGraphicsWidget;

    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(mainWidget);
    mainLayout->setOrientation(Qt::Vertical);
    mainLayout->addStretch();

    QGraphicsLinearLayout *messageLayout = new QGraphicsLinearLayout();
    messageLayout->setOrientation(Qt::Horizontal);

    QGraphicsLinearLayout *buttonLayout = new QGraphicsLinearLayout();
    buttonLayout->setOrientation(Qt::Horizontal);

    mainLayout->addItem(messageLayout);
    mainLayout->addItem(buttonLayout);
    mainLayout->addStretch();

    IconWidget *messageIcon = new IconWidget(mainWidget);
    Label *messageText = new Label(mainWidget);
    messageText->nativeWidget()->setWordWrap(true);

    messageLayout->addStretch();
    messageLayout->addItem(messageIcon);
    messageLayout->addItem(messageText);
    messageLayout->addStretch();

    messageIcon->setIcon(icon);
    messageText->setText(message);

    buttonLayout->addStretch();

    if (buttons & ButtonOk) {
        PushButton *ok = new PushButton(mainWidget);
        ok->setText(i18n("OK"));
        ok->setIcon(KIcon("dialog-ok"));
        buttonLayout->addItem(ok);
        connect(ok, SIGNAL(clicked()), this, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonYes) {
        PushButton *yes = new PushButton(mainWidget);
        yes->setText(i18n("Yes"));
        buttonLayout->addItem(yes);
        connect(yes, SIGNAL(clicked()), this, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonNo) {
        PushButton *no = new PushButton(mainWidget);
        no->setText(i18n("No"));
        buttonLayout->addItem(no);
        connect(no, SIGNAL(clicked()), this, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonCancel) {
        PushButton *cancel = new PushButton(mainWidget);
        cancel->setText(i18n("Cancel"));
        cancel->setIcon(KIcon("dialog-cancel"));
        buttonLayout->addItem(cancel);
        connect(cancel, SIGNAL(clicked()), this, SLOT(destroyMessageOverlay()));
    }

    buttonLayout->addStretch();

    mainWidget->adjustSize();
    QSizeF hint = mainWidget->preferredSize();
    if (hint.height() > size().height() || hint.width() > size().width()) {
        // either a collapsed popup in h/v form factor or just too small,
        // so show it in a dialog associated with ourselves
        if (corona) {
            corona->addOffscreenWidget(mainWidget);
        }

        if (d->messageDialog) {
            delete d->messageDialog.data()->graphicsWidget();
        } else {
            d->messageDialog = new Plasma::Dialog;
        }

        ToolTipManager::self()->hide(this);
        KWindowSystem::setOnAllDesktops(d->messageDialog.data()->winId(), true);
        KWindowSystem::setState(d->messageDialog.data()->winId(), NET::SkipTaskbar | NET::SkipPager);
        d->messageDialog.data()->setGraphicsWidget(mainWidget);
        connect(d->messageDialog.data(), SIGNAL(destroyed(QObject*)), mainWidget, SLOT(deleteLater()));

        // if we are going to show it in a popup, then at least make sure it can be dismissed
        if (buttonLayout->count() < 1) {
            PushButton *ok = new PushButton(mainWidget);
            ok->setText(i18n("OK"));
            ok->setIcon(KIcon("dialog-ok"));
            buttonLayout->addItem(ok);
            connect(ok, SIGNAL(clicked()), this, SLOT(destroyMessageOverlay()));
        }
    } else {
        delete d->messageDialog.data();
        d->createMessageOverlay();
        d->messageOverlay->opacity = 0.8;
        mainWidget->setParentItem(d->messageOverlay);
        QGraphicsLinearLayout *l = new QGraphicsLinearLayout(d->messageOverlay);
        l->addItem(mainWidget);
    }

    if (d->messageDialog) {
        QPoint pos = geometry().topLeft().toPoint();
        if (corona) {
            pos = corona->popupPosition(this, d->messageDialog.data()->size());
        }

        d->messageDialog.data()->move(pos);
        d->messageDialog.data()->animatedShow(locationToDirection(location()));
    } else {
        d->messageOverlay->show();
    }
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
        //FIXME desktop containments can't be removed while in use.
        //it's kinda silly to have a keyboard shortcut for something that can only be used when the
        //shortcut isn't active.
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

            if (configAction->isEnabled()) {
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
        if (action && action->isEnabled()) {
            bool canConfig = unlocked || KAuthorized::authorize("plasma/allow_configure_when_locked");
            action->setVisible(canConfig);
            action->setEnabled(canConfig);
        }

        if (d->extender) {
            foreach (ExtenderItem *item, d->extender.data()->attachedItems()) {
                item->d->setMovable(unlocked);
            }
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

    if ((c & Plasma::StartupCompletedConstraint) || (c & Plasma::SizeConstraint && !(c & Plasma::FormFactorConstraint))) {
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
                    disconnect(w, SIGNAL(update(const QRectF&)), this, SLOT(updateRect(const QRectF&)));
                    connect(w, SIGNAL(update(const QRectF&)), this, SLOT(updateRect(const QRectF&)));
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
    }

    //kDebug() << "before" << shortcut.primary() << d->activationAction->globalShortcut().primary();
    d->activationAction->setGlobalShortcut(
        shortcut,
        KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
        KAction::NoAutoloading);
    d->globalShortcutChanged();
}

void AppletPrivate::globalShortcutChanged()
{
    if (!activationAction) {
        return;
    }

    KConfigGroup shortcutConfig(mainConfigGroup(), "Shortcuts");
    shortcutConfig.writeEntry("global", activationAction->globalShortcut().toString());
    scheduleModificationNotification();
    //kDebug() << "after" << shortcut.primary() << d->activationAction->globalShortcut().primary();
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

Context *Applet::context() const
{
    Containment *c = containment();
    Q_ASSERT(c);
    return c->d->context();
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
        item->removeSceneEventFilter(this);
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
    if (d->package) {
        d->package->d->publish(methods);
    } else {
        if (!d->service) {
            d->service = new PlasmoidService(this);
        }

        kDebug() << "publishing package under name " << resourceName;
        d->service->d->publish(methods, resourceName);
    }
}

void Applet::unpublish()
{
    if (d->package) {
        d->package->d->unpublish();
    } else {
        if (d->service) {
            d->service->d->unpublish();
        }
    }
}

bool Applet::isPublished() const
{
    if (d->package) {
        return d->package->d->isPublished();
    } else {
        if (d->service) {
            return d->service->d->isPublished();
        } else {
            return false;
        }
    }
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

KActionCollection* AppletPrivate::defaultActions(QObject *parent)
{
    KActionCollection *actions = new KActionCollection(parent);
    actions->setConfigGroup("Shortcuts-Applet");

    KAction *configAction = actions->addAction("configure");
    configAction->setAutoRepeat(false);
    configAction->setText(i18n("Widget Settings"));
    configAction->setIcon(KIcon("configure"));
    configAction->setShortcut(KShortcut("alt+d, s"));
    configAction->setData(AbstractToolBox::ConfigureTool);

    KAction *closeApplet = actions->addAction("remove");
    closeApplet->setAutoRepeat(false);
    closeApplet->setText(i18n("Remove this Widget"));
    closeApplet->setIcon(KIcon("edit-delete"));
    closeApplet->setShortcut(KShortcut("alt+d, r"));
    closeApplet->setData(AbstractToolBox::DestructiveTool);

    KAction *runAssociatedApplication = actions->addAction("run associated application");
    runAssociatedApplication->setAutoRepeat(false);
    runAssociatedApplication->setText(i18n("Run the Associated Application"));
    runAssociatedApplication->setIcon(KIcon("system-run"));
    runAssociatedApplication->setShortcut(KShortcut("alt+d, t"));
    runAssociatedApplication->setVisible(false);
    runAssociatedApplication->setEnabled(false);
    runAssociatedApplication->setData(AbstractToolBox::ControlTool);

    return actions;
}

bool Applet::eventFilter(QObject *o, QEvent *e)
{
    return QObject::eventFilter(o, e);
}

bool Applet::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMouseMove:
    {
        // don't move when the containment is not mutable,
        // in the rare case the containment doesn't exists consider it as mutable
        if ((flags() & ItemIsMovable) && d->registeredAsDragHandle.contains(watched)) {
            Containment *c = containment();
            if (!c || c->immutability() == Mutable) {
                mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
                return true;
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
        QGraphicsItem *parent = parentItem();
        Plasma::Applet *applet = qgraphicsitem_cast<Plasma::Applet*>(parent);

        if (applet && applet->isContainment()) {
            // our direct parent is a containment. just move ourselves.
            QPointF curPos = event->pos();
            QPointF lastPos = event->lastPos();

            QTransform appletTransform = transform();
            //we need to discard translation from the transform
            QTransform t(appletTransform.m11(), appletTransform.m12(), appletTransform.m21(), appletTransform.m22(), 0, 0);
            QPointF delta = t.map(curPos - lastPos);

            moveBy(delta.x(), delta.y());
        } else if (parent) {
            //don't move the icon as well because our parent
            //(usually an appletHandle) will do it for us
            //parent->moveBy(delta.x(),delta.y());
            QPointF curPos = parent->transform().map(event->pos());
            QPointF lastPos = parent->transform().map(event->lastPos());
            QPointF delta = curPos - lastPos;

            parent->setPos(parent->pos() + delta);
        }
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
        //FIXME: in 4.3 add an explanatory dialog
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
    if (d->package && d->configLoader) {
        KConfigDialog *dialog = 0;

        QString uiFile = d->package->filePath("mainconfigui");
        if (!uiFile.isEmpty()) {
            QFile f(uiFile);
            QUiLoader loader;
            QWidget *w = loader.load(&f);
            if (w) {
                dialog = new AppletConfigDialog(0, d->configDialogId(), d->configLoader);
                dialog->setWindowTitle(d->configWindowTitle());
                dialog->setAttribute(Qt::WA_DeleteOnClose, true);
                dialog->addPage(w, i18n("Settings"), icon(), i18n("%1 Settings", name()));
                d->addGlobalShortcutsPage(dialog);
                d->addPublishPage(dialog);
                dialog->show();
            }
        }

        if (!dialog && d->script) {
            d->script->showConfigurationInterface();
        }
    } else if (d->script) {
        d->script->showConfigurationInterface();
    } else {
        KConfigDialog *dialog = d->generateGenericConfigDialog();
        //createConfigurationInterface(dialog);
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

QString AppletPrivate::configDialogId() const
{
    return QString("%1settings%2").arg(appletId).arg(q->name());
}

QString AppletPrivate::configWindowTitle() const
{
    return i18nc("@title:window", "%1 Settings", q->name());
}

QSet<QString> AppletPrivate::knownCategories()
{
    // this is to trick the tranlsation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    categories << QString(I18N_NOOP("Accessibility")).toLower()
               << QString(I18N_NOOP("Application Launchers")).toLower()
               << QString(I18N_NOOP("Astronomy")).toLower()
               << QString(I18N_NOOP("Date and Time")).toLower()
               << QString(I18N_NOOP("Development Tools")).toLower()
               << QString(I18N_NOOP("Education")).toLower()
               << QString(I18N_NOOP("Environment and Weather")).toLower()
               << QString(I18N_NOOP("Examples")).toLower()
               << QString(I18N_NOOP("File System")).toLower()
               << QString(I18N_NOOP("Fun and Games")).toLower()
               << QString(I18N_NOOP("Graphics")).toLower()
               << QString(I18N_NOOP("Language")).toLower()
               << QString(I18N_NOOP("Mapping")).toLower()
               << QString(I18N_NOOP("Miscellaneous")).toLower()
               << QString(I18N_NOOP("Multimedia")).toLower()
               << QString(I18N_NOOP("Online Services")).toLower()
               << QString(I18N_NOOP("System Information")).toLower()
               << QString(I18N_NOOP("Utilities")).toLower()
               << QString(I18N_NOOP("Windows and Tasks")).toLower();
    return categories;
}

KConfigDialog *AppletPrivate::generateGenericConfigDialog()
{
    KConfigSkeleton *nullManager = new KConfigSkeleton(0);
    KConfigDialog *dialog = new AppletConfigDialog(0, configDialogId(), nullManager);
    dialog->setFaceType(KPageDialog::Auto);
    dialog->setWindowTitle(configWindowTitle());
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    q->createConfigurationInterface(dialog);
    //TODO: Apply button does not correctly work for now, so do not show it
    dialog->showButton(KDialog::Apply, false);
    dialog->showButton(KDialog::Default, false);
    QObject::connect(dialog, SIGNAL(applyClicked()), q, SLOT(configDialogFinished()));
    QObject::connect(dialog, SIGNAL(okClicked()), q, SLOT(configDialogFinished()));
    QObject::connect(dialog, SIGNAL(finished()), nullManager, SLOT(deleteLater()));
    return dialog;
}

void AppletPrivate::addStandardConfigurationPages(KConfigDialog *dialog)
{
    addGlobalShortcutsPage(dialog);
    addPublishPage(dialog);
}

void AppletPrivate::addGlobalShortcutsPage(KConfigDialog *dialog)
{
    if (isContainment) {
        return;
    }

    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    if (!shortcutEditor) {
        shortcutEditor = new KKeySequenceWidget(page);
        QObject::connect(shortcutEditor, SIGNAL(destroyed(QObject*)), q, SLOT(clearShortcutEditorPtr()));
    }

    shortcutEditor->setKeySequence(q->globalShortcut().primary());
    layout->addWidget(shortcutEditor);
    layout->addStretch();
    dialog->addPage(page, i18n("Keyboard Shortcut"), "preferences-desktop-keyboard");

    //TODO: Apply button does not correctly work for now, so do not show it
    dialog->showButton(KDialog::Apply, false);
    QObject::connect(dialog, SIGNAL(applyClicked()), q, SLOT(configDialogFinished()), Qt::UniqueConnection);
    QObject::connect(dialog, SIGNAL(okClicked()), q, SLOT(configDialogFinished()), Qt::UniqueConnection);
}

void AppletPrivate::addPublishPage(KConfigDialog *dialog)
{
#ifdef ENABLE_REMOTE_WIDGETS
    QWidget *page = new QWidget;
    publishUI.setupUi(page);
    publishUI.publishCheckbox->setChecked(q->isPublished());
    publishUI.allUsersCheckbox->setEnabled(q->isPublished());

    QString resourceName =
    i18nc("%1 is the name of a plasmoid, %2 the name of the machine that plasmoid is published on",
          "%1 on %2", q->name(), QHostInfo::localHostName());
    if (AuthorizationManager::self()->d->matchingRule(resourceName, Credentials())) {
        publishUI.allUsersCheckbox->setChecked(true);
    } else {
        publishUI.allUsersCheckbox->setChecked(false);
    }

    q->connect(publishUI.publishCheckbox, SIGNAL(stateChanged(int)),
               q, SLOT(publishCheckboxStateChanged(int)));
    dialog->addPage(page, i18n("Share"), "applications-internet");
#endif
}

void AppletPrivate::publishCheckboxStateChanged(int state)
{
    if (state == Qt::Checked) {
        publishUI.allUsersCheckbox->setEnabled(true);
    } else {
        publishUI.allUsersCheckbox->setEnabled(false);
    }
}

void AppletPrivate::clearShortcutEditorPtr()
{
    shortcutEditor = 0;
}

void AppletPrivate::configDialogFinished()
{
    if (shortcutEditor) {
        QKeySequence sequence = shortcutEditor->keySequence();
        if (sequence != q->globalShortcut().primary()) {
            q->setGlobalShortcut(KShortcut(sequence));
            emit q->configNeedsSaving();
        }
    }

#ifdef ENABLE_REMOTE_WIDGETS
    if (publishUI.publishCheckbox) {
        q->config().writeEntry("Share", publishUI.publishCheckbox->isChecked());

        if (publishUI.publishCheckbox->isChecked()) {
            QString resourceName =
                i18nc("%1 is the name of a plasmoid, %2 the name of the machine that plasmoid is published on",
                        "%1 on %2", q->name(), QHostInfo::localHostName());
            q->publish(Plasma::ZeroconfAnnouncement, resourceName);
            if (publishUI.allUsersCheckbox->isChecked()) {
                if (!AuthorizationManager::self()->d->matchingRule(resourceName, Credentials())) {
                    AuthorizationRule *rule = new AuthorizationRule(resourceName, "");
                    rule->setPolicy(AuthorizationRule::Allow);
                    rule->setTargets(AuthorizationRule::AllUsers);
                    AuthorizationManager::self()->d->rules.append(rule);
                }
            } else {
                AuthorizationRule *matchingRule =
                    AuthorizationManager::self()->d->matchingRule(resourceName, Credentials());
                if (matchingRule) {
                    AuthorizationManager::self()->d->rules.removeAll(matchingRule);
                }
            }
        } else {
            q->unpublish();
        }
    }
#endif

    if (!configLoader) {
        // the config loader will trigger this for us, so we don't need to.
        q->configChanged();
    }
}

void AppletPrivate::updateShortcuts()
{
    if (isContainment) {
        //a horrible hack to avoid clobbering corona settings
        //we pull them out, then read, then put them back
        QList<QString> names;
        QList<QAction*> qactions;
        names << "add sibling containment" << "configure shortcuts" << "lock widgets";
        foreach (const QString &name, names) {
            QAction *a = actions->action(name);
            actions->takeAction(a); //FIXME this is stupid, KActionCollection needs a takeAction(QString) method
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

void Applet::configChanged()
{
    if (d->script) {
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
    KConfigGroup constraintGroup(KGlobal::config(), "Constraints");
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

void Applet::setAssociatedApplicationUrls(const KUrl::List &urls)
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

KUrl::List Applet::associatedApplicationUrls() const
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

void AppletPrivate::filterOffers(QList<KService::Ptr> &offers)
{
    KConfigGroup constraintGroup(KGlobal::config(), "Constraints");
    foreach (const QString &key, constraintGroup.keyList()) {
        //kDebug() << "security constraint" << key;
        if (constraintGroup.readEntry(key, true)) {
            continue;
        }

        //ugh. a qlist of ksharedptr<kservice>
        QMutableListIterator<KService::Ptr> it(offers);
        while (it.hasNext()) {
            KService::Ptr p = it.next();
            QString prop = QString("X-Plasma-Requires-").append(key);
            QVariant req = p->property(prop, QVariant::String);
            //valid values: Required/Optional/Unused
            QString reqValue;
            if (req.isValid()) {
                reqValue = req.toString();
            } else if (p->property("X-Plasma-API").toString().toLower() == "javascript") {
                //TODO: be able to check whether or not a script engine provides "controled"
                //bindings; for now we just give a pass to the qscript ones
                reqValue = "Unused";
            }

            if (!(reqValue == "Optional" || reqValue == "Unused")) {
            //if (reqValue == "Required") {
                it.remove();
            }
        }
    }
}

QString AppletPrivate::parentAppConstraint(const QString &parentApp)
{
    if (parentApp.isEmpty()) {
        return QString("((not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '') or [X-KDE-ParentApp] == '%1')")
                      .arg(KGlobal::mainComponent().aboutData()->appName());
    }

    return QString("[X-KDE-ParentApp] == '%1'").arg(parentApp);
}

KPluginInfo::List Applet::listAppletInfo(const QString &category, const QString &parentApp)
{
    QString constraint = AppletPrivate::parentAppConstraint(parentApp);

    //note: constraint guaranteed non-empty from here down
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KGlobal::config(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
        foreach (const QString &category, excluded) {
            constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
        }
    } else { //specific category (this could be an excluded one - is that bad?)
        constraint.append(" and ").append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    //now we have to do some manual filtering because the constraint can't handle everything
    AppletPrivate::filterOffers(offers);

    //kDebug() << "Applet::listAppletInfo constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Applet::listAppletInfoForMimetype(const QString &mimetype)
{
    QString constraint = AppletPrivate::parentAppConstraint();
    constraint.append(QString(" and '%1' in [X-Plasma-DropMimeTypes]").arg(mimetype));
    //kDebug() << "listAppletInfoForMimetype with" << mimetype << constraint;
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
                kDebug() << info.name() << "matches" << glob << url;
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

    KConfigGroup group(KGlobal::config(), "General");
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
            kDebug() << "Unknown category: " << applet->name() << "says it is in the"
                     << appletCategory << "category which is unknown to us";
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
        const QStringList& types = service.serviceTypes();

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

Applet *Applet::load(const QString &appletName, uint appletId, const QVariantList &args)
{
    return PluginLoader::pluginLoader()->loadApplet(appletName, appletId, args);
}

Applet *Applet::load(const KPluginInfo &info, uint appletId, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }

    return load(info.pluginName(), appletId, args);
}

QVariant Applet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant ret = QGraphicsWidget::itemChange(change, value);

    //kDebug() << change;
    switch (change) {
    case ItemSceneHasChanged:
    {
        QGraphicsScene *newScene = qvariant_cast<QGraphicsScene*>(value);
        if (newScene) {
            d->checkImmutability();
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
    case ItemPositionChange:
        return (immutability() == Mutable || isContainment() || formFactor() == Horizontal || formFactor() == Vertical || (parentLayoutItem() && parentLayoutItem()->isLayout()) || !d->started) ? value : pos();
        break;
    case ItemTransformChange:
        return immutability() == Mutable ? value : transform();
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

    //in panels make sure that the contents won't exit from the panel
    if (formFactor() == Horizontal && which == Qt::MinimumSize) {
        hint.setHeight(0);
    } else if (formFactor() == Vertical && which == Qt::MinimumSize) {
        hint.setWidth(0);
    }

    // enforce a square size in panels
    if (d->aspectRatioMode == Plasma::Square) {
        if (formFactor() == Horizontal) {
            hint.setWidth(size().height());
        } else if (formFactor() == Vertical) {
            hint.setHeight(size().width());
        }
    } else if (d->aspectRatioMode == Plasma::ConstrainedSquare) {
        //enforce a size not wider than tall
        if (formFactor() == Horizontal &&
            (which == Qt::MaximumSize || size().height() <= KIconLoader::SizeLarge)) {
            hint.setWidth(size().height());
        //enforce a size not taller than wide
        } else if (formFactor() == Vertical &&
                   (which == Qt::MaximumSize || size().width() <= KIconLoader::SizeLarge)) {
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
        d->busyWidgetTimer.stop();
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
    } else if (event->timerId() == d->busyWidgetTimer.timerId()) {
        if (!d->busyWidget) {
            d->createMessageOverlay(false);
            d->messageOverlay->opacity = 0;

            QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(d->messageOverlay);
            d->busyWidget = new Plasma::BusyWidget(d->messageOverlay);
            d->busyWidget->setAcceptHoverEvents(false);
            d->busyWidget->setAcceptedMouseButtons(Qt::NoButton);
            d->messageOverlay->setAcceptHoverEvents(false);

            mainLayout->addStretch();
            mainLayout->addItem(d->busyWidget);
            mainLayout->addStretch();
        }
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

void AppletPrivate::setIsContainment(bool nowIsContainment, bool forceUpdate)
{
    if (isContainment == nowIsContainment && !forceUpdate) {
        return;
    }

    isContainment = nowIsContainment;
    //FIXME I do not like this function.
    //currently it's only called before ctmt/applet init, with (true,true), and I'm going to assume it stays that way.
    //if someone calls it at some other time it'll cause headaches. :P

    delete mainConfig;
    mainConfig = 0;

    Containment *c = q->containment();
    if (c) {
        c->d->checkContainmentFurniture();
    }
}

bool Applet::isContainment() const
{
    return d->isContainment;
}

// PRIVATE CLASS IMPLEMENTATION

AppletPrivate::AppletPrivate(KService::Ptr service, int uniqueID, Applet *applet)
        : appletId(uniqueID),
          q(applet),
          service(0),
          preferredBackgroundHints(Applet::StandardBackground),
          backgroundHints(Applet::NoBackground),
          aspectRatioMode(Plasma::KeepAspectRatio),
          immutability(Mutable),
          appletDescription(service),
          background(0),
          mainConfig(0),
          pendingConstraints(NoConstraint),
          messageOverlay(0),
          messageOverlayProxy(0),
          busyWidget(0),
          script(0),
          package(0),
          configLoader(0),
          actions(AppletPrivate::defaultActions(applet)),
          activationAction(0),
          shortcutEditor(0),
          itemStatus(UnknownStatus),
          preferredSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored),
          modificationsTimer(0),
          hasConfigurationInterface(false),
          failed(false),
          isContainment(false),
          transient(false),
          needsConfig(false),
          started(false)
{
    if (appletId == 0) {
        appletId = ++s_maxAppletId;
    } else if (appletId > s_maxAppletId) {
        s_maxAppletId = appletId;
    }
}

AppletPrivate::~AppletPrivate()
{
    if (activationAction && activationAction->isGlobalShortcutEnabled()) {
        //kDebug() << "reseting global action for" << q->name() << activationAction->objectName();
        activationAction->forgetGlobalShortcut();
    }

    delete extender.data();

    delete script;
    script = 0;
    delete package;
    package = 0;
    delete configLoader;
    configLoader = 0;
    delete mainConfig;
    mainConfig = 0;
    delete modificationsTimer;
}

void AppletPrivate::init(const QString &packagePath)
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    q->setCacheMode(Applet::DeviceCoordinateCache);
    q->setAcceptsHoverEvents(true);
    q->setFlag(QGraphicsItem::ItemIsFocusable, true);
    q->setFocusPolicy(Qt::ClickFocus);
    // FIXME: adding here because nothing seems to be doing it in QGraphicsView,
    // but it doesn't actually work anyways =/
    q->setLayoutDirection(qApp->layoutDirection());

    if (!appletDescription.isValid()) {
        kDebug() << "Check your constructor! "
                 << "You probably want to be passing in a Service::Ptr "
                 << "or a QVariantList with a valid storageid as arg[0].";
        return;
    }

    QString api = appletDescription.property("X-Plasma-API").toString();

    // we have a scripted plasmoid
    if (!api.isEmpty()) {
        // find where the Package is
        QString path = packagePath;
        if (path.isEmpty()) {
            QString subPath = q->packageStructure()->defaultPackageRoot() + '/' + appletDescription.pluginName() + '/';
            path = KStandardDirs::locate("data", subPath + "metadata.desktop");
            if (path.isEmpty()) {
                path = KStandardDirs::locate("data", subPath);
            } else {
                path.remove(QString("metadata.desktop"));
            }
        } else if (!path.endsWith('/')) {
            path.append('/');
        }

        if (path.isEmpty()) {
            q->setFailedToLaunch(
                true,
                i18nc("Package file, name of the widget",
                      "Could not locate the %1 package required for the %2 widget.",
                      appletDescription.pluginName(), appletDescription.name()));
        } else {
            // create the package and see if we have something real
            //kDebug() << "trying for" << path;
            PackageStructure::Ptr structure = Plasma::packageStructure(api, Plasma::AppletComponent);
            structure->setPath(path);
            package = new Package(path, structure);

            if (package->isValid()) {
                // now we try and set up the script engine.
                // it will be parented to this applet and so will get
                // deleted when the applet does

                script = Plasma::loadScriptEngine(api, q);
                if (!script) {
                    delete package;
                    package = 0;
                    q->setFailedToLaunch(true,
                                         i18nc("API or programming language the widget was written in, name of the widget",
                                               "Could not create a %1 ScriptEngine for the %2 widget.",
                                               api, appletDescription.name()));
                }
            } else {
                q->setFailedToLaunch(true, i18nc("Package file, name of the widget",
                                                 "Could not open the %1 package required for the %2 widget.",
                                                 appletDescription.pluginName(), appletDescription.name()));
                delete package;
                package = 0;
            }
        }
    }

    //set a default size before any saved settings are read
    QSize size = appletDescription.property("X-Plasma-DefaultSize").toSize();
    if (size.isEmpty()) {
        size = QSize(200, 200);
    }
    //kDebug() << "size" << size;
    q->resize(size);

    q->setBackgroundHints(Applet::DefaultBackground);
    q->setHasConfigurationInterface(true); //FIXME why not default it to true in the constructor?

    QAction *closeApplet = actions->action("remove");
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove this %1", q->name()));
    }

    QAction *configAction = actions->action("configure");
    if (configAction) {
        configAction->setText(i18nc("%1 is the name of the applet", "%1 Settings", q->name()));
    }

    QObject::connect(q, SIGNAL(activate()), q, SLOT(setFocus()));
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void AppletPrivate::setupScriptSupport()
{
    if (!package) {
        return;
    }

    kDebug() << "setting up script support, package is in" << package->path()
             << "which is a" << package->structure()->type() << "package"
             << ", main script is" << package->filePath("mainscript");

    QString translationsPath = package->filePath("translations");
    if (!translationsPath.isEmpty()) {
        //FIXME: we should _probably_ use a KComponentData to segregate the applets
        //       from each other; but I want to get the basics working first :)
        KGlobal::dirs()->addResourceDir("locale", translationsPath);
        KGlobal::locale()->insertCatalog(package->metadata().pluginName());
    }

    QString xmlPath = package->filePath("mainconfigxml");
    if (!xmlPath.isEmpty()) {
        QFile file(xmlPath);
        KConfigGroup config = q->config();
        configLoader = new ConfigLoader(&config, &file);
        QObject::connect(configLoader, SIGNAL(configChanged()), q, SLOT(configChanged()));
    }

    if (!package->filePath("mainconfigui").isEmpty()) {
        q->setHasConfigurationInterface(true);
    }
}

QString AppletPrivate::globalName() const
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.service()->library();
}

QString AppletPrivate::instanceName()
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.service()->library() + QString::number(appletId);
}

void AppletPrivate::scheduleConstraintsUpdate(Plasma::Constraints c)
{
    // Don't start up a timer if we're just starting up
    // flushPendingConstraints will be called by Corona
    if (started && !constraintsTimer.isActive() && !(c & Plasma::StartupCompletedConstraint)) {
        constraintsTimer.start(0, q);
    }

    if (c & Plasma::StartupCompletedConstraint) {
        started = true;
    }

    pendingConstraints |= c;
}

void AppletPrivate::scheduleModificationNotification()
{
    // modificationsTimer is not allocated until we get our notice of being started
    if (modificationsTimer) {
        // schedule a save
        if (modificationsTimer->isActive()) {
            modificationsTimer->stop();
        }

        modificationsTimer->start(1000, q);
    }
}

KConfigGroup *AppletPrivate::mainConfigGroup()
{
    if (mainConfig) {
        return mainConfig;
    }

    bool newGroup = false;
    if (isContainment) {
        Corona *corona = qobject_cast<Corona*>(q->scene());
        KConfigGroup containmentConfig;
        //kDebug() << "got a corona, baby?" << (QObject*)corona << (QObject*)q;

        if (corona) {
            containmentConfig = KConfigGroup(corona->config(), "Containments");
        } else {
            containmentConfig =  KConfigGroup(KGlobal::config(), "Containments");
        }

        if (package && !containmentConfig.hasGroup(QString::number(appletId))) {
            newGroup = true;
        }

        mainConfig = new KConfigGroup(&containmentConfig, QString::number(appletId));
    } else {
        KConfigGroup appletConfig;

        Containment *c = q->containment();
        Applet *parentApplet = qobject_cast<Applet *>(q->parent());
        if (parentApplet && parentApplet != static_cast<Applet *>(c)) {
            // this applet is nested inside another applet! use it's config
            // as the parent group in the config
            appletConfig = parentApplet->config();
            appletConfig = KConfigGroup(&appletConfig, "Applets");
        } else if (c) {
            // applet directly in a Containment, as usual
            appletConfig = c->config();
            appletConfig = KConfigGroup(&appletConfig, "Applets");
        } else {
            kWarning() << "requesting config for" << q->name() << "without a containment!";
            appletConfig = KConfigGroup(KGlobal::config(), "Applets");
        }

        if (package && !appletConfig.hasGroup(QString::number(appletId))) {
            newGroup = true;
        }

        mainConfig = new KConfigGroup(&appletConfig, QString::number(appletId));
    }

    if (newGroup) {
        //see if we have a default configuration in our package
        const QString defaultConfigFile = q->package()->filePath("defaultconfig");
        if (!defaultConfigFile.isEmpty()) {
            kDebug() << "copying default config: " << q->package()->filePath("defaultconfig");
            KConfigGroup defaultConfig(KSharedConfig::openConfig(defaultConfigFile)->group("Configuration"));
            defaultConfig.copyTo(mainConfig);
        }
    }

    return mainConfig;
}

QString AppletPrivate::visibleFailureText(const QString &reason)
{
    QString text;

    if (reason.isEmpty()) {
        text = i18n("This object could not be created.");
    } else {
        text = i18n("This object could not be created for the following reason:<p><b>%1</b></p>", reason);
    }

    return text;
}

void AppletPrivate::checkImmutability()
{
    const bool systemImmutable = q->globalConfig().isImmutable() || q->config().isImmutable() ||
                                ((!isContainment && q->containment()) &&
                                    q->containment()->immutability() == SystemImmutable) ||
                                (qobject_cast<Corona*>(q->scene()) && static_cast<Corona*>(q->scene())->immutability() == SystemImmutable);

    if (systemImmutable) {
        q->updateConstraints(ImmutableConstraint);
    }
}

void AppletPrivate::themeChanged()
{
    if (background) {
        //do again the translucent background fallback
        q->setBackgroundHints(backgroundHints);

        qreal left;
        qreal right;
        qreal top;
        qreal bottom;
        background->getMargins(left, top, right, bottom);
        q->setContentsMargins(left, right, top, bottom);
    }
    q->update();
}

void AppletPrivate::resetConfigurationObject()
{
    // make sure mainConfigGroup exists in all cases
    mainConfigGroup();

    mainConfig->deleteGroup();
    delete mainConfig;
    mainConfig = 0;

    Corona * corona = qobject_cast<Corona*>(q->scene());
    if (corona) {
        corona->requireConfigSync();
    }
}

uint AppletPrivate::s_maxAppletId = 0;
int AppletPrivate::s_maxZValue = 0;
int AppletPrivate::s_minZValue = 0;
PackageStructure::Ptr AppletPrivate::packageStructure(0);
QSet<QString> AppletPrivate::s_customCategories;

AppletOverlayWidget::AppletOverlayWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      opacity(0.4)
{
    resize(parent->size());
}

void AppletOverlayWidget::destroy()
{
    Animation *anim = Plasma::Animator::create(Plasma::Animator::DisappearAnimation);
    if (anim) {
        connect(anim, SIGNAL(finished()), this, SLOT(overlayAnimationComplete()));
        anim->setTargetWidget(this);
        anim->start();
    } else {
        overlayAnimationComplete();
    }
}

void AppletOverlayWidget::overlayAnimationComplete()
{
    if (scene()) {
        scene()->removeItem(this);
    }
    deleteLater();
}

void AppletOverlayWidget::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (qFuzzyCompare(1, 1+opacity)) {
        return;
    }

    QColor wash = Plasma::Theme::defaultTheme()->color(Theme::BackgroundColor);
    wash.setAlphaF(opacity);

    Applet *applet = qobject_cast<Applet *>(parentWidget());


    QPainterPath backgroundShape;
    if (!applet || applet->backgroundHints() & Applet::StandardBackground) {
        //FIXME: a resize here is nasty, but perhaps still better than an eventfilter just for that..
        if (parentWidget()->contentsRect().size() != size()) {
            resize(parentWidget()->contentsRect().size());
        }
        backgroundShape = PaintUtils::roundedRectangle(contentsRect(), 5);
    } else {
        backgroundShape = shape();
    }

    painter->setRenderHints(QPainter::Antialiasing);
    painter->fillPath(backgroundShape, wash);
}

} // Plasma namespace

#include "applet.moc"
#include "private/applet_p.moc"
