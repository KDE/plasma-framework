/*
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Riccardo Iaconelli <riccardo@kde.org>
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

#include "corona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsGridLayout>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include <cmath>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcutsdialog.h>

#include "animator.h"
#include "abstracttoolbox.h"
#include "containment.h"
#include "containmentactionspluginsconfig.h"
#include "view.h"
#include "private/animator_p.h"
#include "private/applet_p.h"
#include "private/containment_p.h"
#include "tooltipmanager.h"
#include "abstractdialogmanager.h"

using namespace Plasma;

namespace Plasma
{

// constant controlling how long between requesting a configuration sync
// and one happening should occur. currently 10 seconds
const int CONFIG_SYNC_TIMEOUT = 10000;

class CoronaPrivate
{
public:
    CoronaPrivate(Corona *corona)
        : q(corona),
          immutability(Mutable),
          mimetype("text/x-plasmoidservicename"),
          config(0),
          actions(corona)
    {
        if (KGlobal::hasMainComponent()) {
            configName = KGlobal::mainComponent().componentName() + "-appletsrc";
        } else {
            configName = "plasma-appletsrc";
        }
    }

    ~CoronaPrivate()
    {
        qDeleteAll(containments);
    }

    void init()
    {
        configSyncTimer.setSingleShot(true);
        QObject::connect(&configSyncTimer, SIGNAL(timeout()), q, SLOT(syncConfig()));

        //some common actions
        actions.setConfigGroup("Shortcuts");

        KAction *lockAction = actions.addAction("lock widgets");
        QObject::connect(lockAction, SIGNAL(triggered(bool)), q, SLOT(toggleImmutability()));
        lockAction->setText(i18n("Lock Widgets"));
        lockAction->setAutoRepeat(true);
        lockAction->setIcon(KIcon("object-locked"));
        lockAction->setData(AbstractToolBox::ControlTool);
        lockAction->setShortcut(KShortcut("alt+d, l"));
        lockAction->setShortcutContext(Qt::ApplicationShortcut);

        //FIXME this doesn't really belong here. desktop KCM maybe?
        //but should the shortcuts be per-app or really-global?
        //I don't know how to make kactioncollections use plasmarc
        KAction *action = actions.addAction("configure shortcuts");
        QObject::connect(action, SIGNAL(triggered()), q, SLOT(showShortcutConfig()));
        action->setText(i18n("Shortcut Settings"));
        action->setIcon(KIcon("configure-shortcuts"));
        action->setAutoRepeat(false);
        action->setData(AbstractToolBox::ConfigureTool);
        //action->setShortcut(KShortcut("ctrl+h"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        actions.readSettings();

        //fake containment/applet actions
        KActionCollection *aActions = AppletPrivate::defaultActions(q);
        KActionCollection *cActions = AppletPrivate::defaultActions(q); //containment has to start with applet stuff
        ContainmentPrivate::addDefaultActions(cActions); //now it's really containment
        //grab the current stuff
        cActions->readSettings();
        aActions->readSettings();

        shortcutsDlg.setModal(false);
        shortcutsDlg.addCollection(aActions);
        shortcutsDlg.addCollection(cActions);

        QObject::connect(&shortcutsDlg, SIGNAL(saved()), q, SIGNAL(shortcutsChanged()));
    }

    void showShortcutConfig()
    {
        //show a kshortcutsdialog with the actions
        shortcutsDlg.configure();
    }

    void toggleImmutability()
    {
        if (immutability == Mutable) {
            q->setImmutability(UserImmutable);
        } else {
            q->setImmutability(Mutable);
        }
    }

    void saveLayout(KSharedConfigPtr cg) const
    {
        KConfigGroup containmentsGroup(cg, "Containments");
        foreach (const Containment *containment, containments) {
            QString cid = QString::number(containment->id());
            KConfigGroup containmentConfig(&containmentsGroup, cid);
            containment->save(containmentConfig);
        }
    }

    void updateContainmentImmutability()
    {
        foreach (Containment *c, containments) {
            // we need to tell each containment that immutability has been altered
            c->updateConstraints(ImmutableConstraint);
        }
    }

    void containmentDestroyed(QObject *obj)
    {
        // we do a static_cast here since it really isn't an Containment by this
        // point anymore since we are in the qobject dtor. we don't actually
        // try and do anything with it, we just need the value of the pointer
        // so this unsafe looking code is actually just fine.
        Containment* containment = static_cast<Plasma::Containment*>(obj);
        int index = containments.indexOf(containment);

        if (index > -1) {
            containments.removeAt(index);
            q->requestConfigSync();
        }
    }

    void syncConfig()
    {
        q->config()->sync();
        emit q->configSynced();
    }

    Containment *addContainment(const QString &name, const QVariantList &args,
                                uint id, bool delayedInit)
    {
        QString pluginName = name;
        Containment *containment = 0;
        Applet *applet = 0;

        //kDebug() << "Loading" << name << args << id;

        if (pluginName.isEmpty()) {
            // default to the desktop containment
            pluginName = "desktop";
        }

        bool loadingNull = pluginName == "null";
        if (!loadingNull) {
            applet = Applet::load(pluginName, id, args);
            containment = dynamic_cast<Containment*>(applet);
        }

        if (!containment) {
            if (!loadingNull) {
                kDebug() << "loading of containment" << name << "failed.";
            }

            // in case we got a non-Containment from Applet::loadApplet or
            // a null containment was requested
            if (applet) {
                // the applet probably doesn't know what's hit it, so let's pretend it can be
                // initialized to make assumptions in the applet's dtor safer
                q->addItem(applet);
                applet->init();
                q->removeItem(applet);
                delete applet;
            }
            applet = containment = new Containment(0, 0, id);

            if (loadingNull) {
                containment->setDrawWallpaper(false);
            } else {
                containment->setFailedToLaunch(false);
            }

            // we want to provide something and don't care about the failure to launch
            containment->setFormFactor(Plasma::Planar);
        }

        applet->d->isContainment = true;
        containment->setPos(containment->d->preferredPos(q));
        q->addItem(containment);
        applet->d->setIsContainment(true, true);
        containments.append(containment);

        if (!delayedInit) {
            containment->init();
            KConfigGroup cg = containment->config();
            containment->restore(cg);
            containment->updateConstraints(Plasma::StartupCompletedConstraint);
            containment->save(cg);
            q->requestConfigSync();
            containment->flushPendingConstraintsEvents();
        }

        QObject::connect(containment, SIGNAL(destroyed(QObject*)),
                         q, SLOT(containmentDestroyed(QObject*)));
        QObject::connect(containment, SIGNAL(configNeedsSaving()),
                         q, SLOT(requestConfigSync()));
        QObject::connect(containment, SIGNAL(releaseVisualFocus()),
                         q, SIGNAL(releaseVisualFocus()));
        QObject::connect(containment, SIGNAL(screenChanged(int,int,Plasma::Containment*)),
                         q, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)));

        if (!delayedInit) {
            emit q->containmentAdded(containment);
        }

        return containment;
    }

    void offscreenWidgetDestroyed(QObject *);
    QList<Plasma::Containment *> importLayout(const KConfigBase &conf, bool mergeConfig);

    static bool s_positioningContainments;

    Corona *q;
    ImmutabilityType immutability;
    QString mimetype;
    QString configName;
    KSharedConfigPtr config;
    QTimer configSyncTimer;
    QList<Containment*> containments;
    QHash<uint, QGraphicsWidget*> offscreenWidgets;
    KActionCollection actions;
    KShortcutsDialog shortcutsDlg;
    QMap<Containment::Type, ContainmentActionsPluginsConfig> containmentActionsDefaults;
    QWeakPointer<AbstractDialogManager>dialogManager;
};

bool CoronaPrivate::s_positioningContainments = false;

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new CoronaPrivate(this))
{
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Corona ctor start";
    d->init();
    ToolTipManager::self()->m_corona = this;
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
    KConfigGroup trans(KGlobal::config(), "PlasmaTransientsConfig");
    trans.deleteGroup();

    // FIXME: Same fix as in Plasma::View - make sure that when the focused widget is
    //        destroyed we don't try to transfer it to something that's already been
    //        deleted.
    clearFocus();
    delete d;
}

void Corona::setAppletMimeType(const QString &type)
{
    d->mimetype = type;
}

QString Corona::appletMimeType()
{
    return d->mimetype;
}

void Corona::saveLayout(const QString &configName) const
{
    KSharedConfigPtr c;

    if (configName.isEmpty() || configName == d->configName) {
        c = config();
    } else {
        c = KSharedConfig::openConfig(configName);
    }

    d->saveLayout(c);
}

void Corona::requestConfigSync()
{
    // TODO: should we check into our immutability before doing this?

    //NOTE: this is a pretty simplistic model: we simply save no more than CONFIG_SYNC_TIMEOUT
    //      after the first time this is called. not much of a heuristic for save points, but
    //      it should at least compress these activities a bit and provide a way for applet
    //      authors to ween themselves from the sync() disease. A more interesting/dynamic
    //      algorithm for determining when to actually sync() to disk might be better, though.
    if (!d->configSyncTimer.isActive()) {
        d->configSyncTimer.start(CONFIG_SYNC_TIMEOUT);
    }
}

void Corona::requireConfigSync()
{
    d->syncConfig();
}

void Corona::initializeLayout(const QString &configName)
{
    clearContainments();
    loadLayout(configName);

    if (d->containments.isEmpty()) {
        loadDefaultLayout();
        if (!d->containments.isEmpty()) {
            requestConfigSync();
        }
    }

    if (config()->isImmutable()) {
        d->updateContainmentImmutability();
    }

    KConfigGroup coronaConfig(config(), "General");
    setImmutability((ImmutabilityType)coronaConfig.readEntry("immutability", (int)Mutable));
}

bool containmentSortByPosition(const Containment *c1, const Containment *c2)
{
    return c1->id() < c2->id();
}

void Corona::layoutContainments()
{
    if (CoronaPrivate::s_positioningContainments) {
        return;
    }

    CoronaPrivate::s_positioningContainments = true;

    //TODO: we should avoid running this too often; consider compressing requests
    //      with a timer.
    QList<Containment*> c = containments();
    QMutableListIterator<Containment*> it(c);

    while (it.hasNext()) {
        Containment *containment = it.next();
        if (containment->containmentType() == Containment::PanelContainment ||
            containment->containmentType() == Containment::CustomPanelContainment ||
            offscreenWidgets().contains(containment)) {
            // weed out all containments we don't care about at all
            // e.g. Panels and ourself
            it.remove();
            continue;
        }
    }

    qSort(c.begin(), c.end(), containmentSortByPosition);

    if (c.isEmpty()) {
        CoronaPrivate::s_positioningContainments = false;
        return;
    }

    int column = 0;
    int x = 0;
    int y = 0;
    int rowHeight = 0;

    it.toFront();
    while (it.hasNext()) {
        Containment *containment = it.next();
        containment->setPos(x, y);
        //kDebug() << ++count << "setting to" << x << y;

        int height = containment->size().height();
        if (height > rowHeight) {
            rowHeight = height;
        }

        ++column;

        if (column == CONTAINMENT_COLUMNS) {
            column = 0;
            x = 0;
            y += rowHeight + INTER_CONTAINMENT_MARGIN + TOOLBOX_MARGIN;
            rowHeight = 0;
        } else {
            x += containment->size().width() + INTER_CONTAINMENT_MARGIN;
        }
        //kDebug() << "column: " << column << "; x " << x << "; y" << y << "; width was"
        //         << containment->size().width();
    }

    CoronaPrivate::s_positioningContainments = false;
}


void Corona::loadLayout(const QString &configName)
{
    if (!configName.isEmpty() && configName != d->configName) {
        // if we have a new config name passed in, then use that as the config file for this Corona
        d->config = 0;
        d->configName = configName;
    }

    KSharedConfigPtr conf = config();
    d->importLayout(*conf, conf != config());
}

QList<Plasma::Containment *> Corona::importLayout(const KConfigBase &conf)
{
    if (const KConfigGroup *group = dynamic_cast<const KConfigGroup *>(&conf)) {
        if (!group->isValid()) {
            return QList<Containment *>();
        }
    }

    return d->importLayout(conf, true);
}

QList<Plasma::Containment *> CoronaPrivate::importLayout(const KConfigBase &conf, bool mergeConfig)
{
    QList<Plasma::Containment *> newContainments;
    QSet<uint> containmentsIds;

    foreach (Containment *containment, containments) {
        containmentsIds.insert(containment->id());
    }

    KConfigGroup containmentsGroup(&conf, "Containments");

    foreach (const QString &group, containmentsGroup.groupList()) {
        KConfigGroup containmentConfig(&containmentsGroup, group);

        if (containmentConfig.entryMap().isEmpty()) {
            continue;
        }

        uint cid = group.toUInt();
        if (containmentsIds.contains(cid)) {
            cid = ++AppletPrivate::s_maxAppletId;
        } else if (cid > AppletPrivate::s_maxAppletId) {
            AppletPrivate::s_maxAppletId = cid;
        }

        if (mergeConfig) {
            KConfigGroup realConf(q->config(), "Containments");
            realConf = KConfigGroup(&realConf, QString::number(cid));
            containmentConfig.copyTo(&realConf);
        }

        //kDebug() << "got a containment in the config, trying to make a" << containmentConfig.readEntry("plugin", QString()) << "from" << group;
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Adding Containment" << containmentConfig.readEntry("plugin", QString());
        Containment *c = addContainment(containmentConfig.readEntry("plugin", QString()), QVariantList(), cid, true);
        if (!c) {
            continue;
        }

        newContainments.append(c);
        containmentsIds.insert(c->id());

        c->init();
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Init Containment" << c->pluginName();
        c->restore(containmentConfig);
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Restored Containment" << c->pluginName();
    }

    foreach (Containment *containment, newContainments) {
        containment->updateConstraints(Plasma::StartupCompletedConstraint);
        containment->d->initApplets();
        emit q->containmentAdded(containment);
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Containment" << containment->name();
    }

    return newContainments;
}

Containment *Corona::containmentForScreen(int screen, int desktop) const
{
    foreach (Containment *containment, d->containments) {
        if (containment->screen() == screen &&
            (desktop < 0 || containment->desktop() == desktop) &&
            (containment->containmentType() == Containment::DesktopContainment ||
             containment->containmentType() == Containment::CustomContainment)) {
            return containment;
        }
    }

    return 0;
}

QList<Containment*> Corona::containments() const
{
    return d->containments;
}

void Corona::clearContainments()
{
    foreach (Containment *containment, d->containments) {
        containment->clearApplets();
    }
}

KSharedConfigPtr Corona::config() const
{
    if (!d->config) {
        d->config = KSharedConfig::openConfig(d->configName);
    }

    return d->config;
}

Containment *Corona::addContainment(const QString &name, const QVariantList &args)
{
    return d->addContainment(name, args, 0, false);
}

Containment *Corona::addContainmentDelayed(const QString &name, const QVariantList &args)
{
    return d->addContainment(name, args, 0, true);
}

void Corona::mapAnimation(Animator::Animation from, Animator::Animation to)
{
    AnimatorPrivate::mapAnimation(from, to);
}

void Corona::mapAnimation(Animator::Animation from, const QString &to)
{
    AnimatorPrivate::mapAnimation(from, to);
}

void Corona::addOffscreenWidget(QGraphicsWidget *widget)
{
    foreach (QGraphicsWidget *w, d->offscreenWidgets) {
        if (w == widget) {
            kDebug() << "widget is already an offscreen widget!";
            return;
        }
    }

    //search for an empty spot in the topleft quadrant of the scene. each 'slot' is QWIDGETSIZE_MAX
    //x QWIDGETSIZE_MAX, so we're guaranteed to never have to move widgets once they're placed here.
    int i = 0;
    while (d->offscreenWidgets.contains(i)) {
        i++;
    }

    d->offscreenWidgets[i] = widget;
    widget->setPos((-i - 1) * QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX);

    QGraphicsWidget *pw = widget->parentWidget();
    widget->setParentItem(0);
    widget->setParent(pw);

    //kDebug() << "adding offscreen widget at slot " << i;
    if (!widget->scene()) {
        addItem(widget);
    }

    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(offscreenWidgetDestroyed(QObject*)));
}

void Corona::removeOffscreenWidget(QGraphicsWidget *widget)
{
    QMutableHashIterator<uint, QGraphicsWidget *> it(d->offscreenWidgets);

    while (it.hasNext()) {
        if (it.next().value() == widget) {
            it.remove();
            return;
        }
    }
}

QList <QGraphicsWidget *> Corona::offscreenWidgets() const
{
    return d->offscreenWidgets.values();
}

void CoronaPrivate::offscreenWidgetDestroyed(QObject *o)
{
    // at this point, it's just a QObject, not a QGraphicsWidget, but we still need
    // a pointer of the appropriate type.
    // WARNING: DO NOT USE THE WIDGET POINTER FOR ANYTHING OTHER THAN POINTER COMPARISONS
    QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(o);
    q->removeOffscreenWidget(widget);
}

int Corona::numScreens() const
{
    return 1;
}

QRect Corona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    if (views().isEmpty()) {
        return sceneRect().toRect();
    } else {
        QGraphicsView *v = views()[0];
        QRect r = sceneRect().toRect();
        r.moveTo(v->mapToGlobal(v->pos()));
        return r;
    }
}

QRegion Corona::availableScreenRegion(int id) const
{
    return QRegion(screenGeometry(id));
}

QPoint Corona::popupPosition(const QGraphicsItem *item, const QSize &s)
{
    return popupPosition(item, s, Qt::AlignLeft);
}

QPoint Corona::popupPosition(const QGraphicsItem *item, const QSize &s, Qt::AlignmentFlag alignment)
{
    // TODO: merge both methods (also these in Applet) into one (with optional alignment) when we can break compatibility
    // TODO: add support for more flags in the future?

    QGraphicsView *v = viewFor(item);

    if (!v) {
        return QPoint(0, 0);
    }

    QPoint pos;
    QTransform sceneTransform = item->sceneTransform();

    //swap direction if necessary
    if (QApplication::isRightToLeft() && alignment != Qt::AlignCenter) {
        if (alignment == Qt::AlignRight) {
            alignment = Qt::AlignLeft;
        } else {
            alignment = Qt::AlignRight;
        }
    }

    //if the applet is rotated the popup position has to be un-transformed
    if (sceneTransform.isRotating()) {
        qreal angle = acos(sceneTransform.m11());
        QTransform newTransform;
        QPointF center = item->sceneBoundingRect().center();

        newTransform.translate(center.x(), center.y());
        newTransform.rotateRadians(-angle);
        newTransform.translate(-center.x(), -center.y());
        pos = v->mapFromScene(newTransform.inverted().map(item->scenePos()));
    } else {
        pos = v->mapFromScene(item->scenePos());
    }

    pos = v->mapToGlobal(pos);
    //kDebug() << "==> position is" << item->scenePos() << v->mapFromScene(item->scenePos()) << pos;
    Plasma::View *pv = dynamic_cast<Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv && pv->containment()) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
    case TopEdge: {
        if (alignment == Qt::AlignCenter) {
            pos.setX(pos.x() + item->boundingRect().width()/2 - s.width()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setX(pos.x() + item->boundingRect().width() - s.width());
        }

        if (pos.x() + s.width() > v->geometry().right()) {
            pos.setX(v->geometry().right() - s.width());
        } else {
            pos.setX(qMax(pos.x(), v->geometry().left()));
        }
        break;
    }
    case LeftEdge:
    case RightEdge: {
        if (alignment == Qt::AlignCenter) {
            pos.setY(pos.y() + item->boundingRect().height()/2 - s.height()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setY(pos.y() + item->boundingRect().height() - s.height());
        }

        if (pos.y() + s.height() > v->geometry().bottom()) {
            pos.setY(v->geometry().bottom() - s.height());
        } else {
            pos.setY(qMax(pos.y(), v->geometry().top()));
        }
        break;
    }
    default:
        if (alignment == Qt::AlignCenter) {
            pos.setX(pos.x() + item->boundingRect().width()/2 - s.width()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setX(pos.x() + item->boundingRect().width() - s.width());
        }
        break;
    }

    switch (loc) {
    case BottomEdge:
        pos.setY(v->geometry().y() - s.height());
        break;
    case TopEdge:
        pos.setY(v->geometry().bottom());
        break;
    case LeftEdge:
        pos.setX(v->geometry().right());
        break;
    case RightEdge:
        pos.setX(v->geometry().x() - s.width());
        break;
    default:
        if (pos.y() - s.height() > 0) {
             pos.ry() = pos.y() - s.height();
        } else {
             pos.ry() = pos.y() + (int)item->boundingRect().size().height() + 1;
        }
    }

    //are we out of screen?
    int screen = ((pv && pv->containment()) ? pv->containment()->screen() : -1);
    if (screen == -1) {
        if (pv) {
            screen = pv->screen();
        } else {
            // fall back to asking the actual system what screen the view is on
            // in the case we are dealing with a non-PlasmaView QGraphicsView
            screen = QApplication::desktop()->screenNumber(v);
        }
    }

    QRect screenRect = screenGeometry(screen);
    //kDebug() << "==> rect for" << screen << "is" << screenRect;

    if (loc != LeftEdge && pos.x() + s.width() > screenRect.right()) {
        pos.rx() -= ((pos.x() + s.width()) - screenRect.right());
    }

    if (loc != TopEdge && pos.y() + s.height() > screenRect.bottom()) {
        pos.ry() -= ((pos.y() + s.height()) - screenRect.bottom());
    }

    pos.rx() = qMax(0, pos.x());
    return pos;
}

void Corona::loadDefaultLayout()
{
}

void Corona::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragEnterEvent(event);
}

void Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragLeaveEvent(event);
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);
}

ImmutabilityType Corona::immutability() const
{
    return d->immutability;
}

void Corona::setImmutability(const ImmutabilityType immutable)
{
    if (d->immutability == immutable ||
        d->immutability == SystemImmutable) {
        return;
    }

    kDebug() << "setting immutability to" << immutable;
    d->immutability = immutable;
    d->updateContainmentImmutability();
    //tell non-containments that might care (like plasmaapp or a custom corona)
    emit immutabilityChanged(immutable);

    //update our actions
    QAction *action = d->actions.action("lock widgets");
    if (action) {
        if (d->immutability == SystemImmutable) {
            action->setEnabled(false);
            action->setVisible(false);
        } else {
            bool unlocked = d->immutability == Mutable;
            action->setText(unlocked ? i18n("Lock Widgets") : i18n("Unlock Widgets"));
            action->setIcon(KIcon(unlocked ? "object-locked" : "object-unlocked"));
            action->setEnabled(true);
            action->setVisible(true);
        }
    }

    KConfigGroup cg(config(), "General");

    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    cg.writeEntry("immutability", (int)d->immutability);
    requestConfigSync();
}

QList<Plasma::Location> Corona::freeEdges(int screen) const
{
    QList<Plasma::Location> freeEdges;
    freeEdges << Plasma::TopEdge << Plasma::BottomEdge
              << Plasma::LeftEdge << Plasma::RightEdge;

    foreach (Containment *containment, containments()) {
        if (containment->screen() == screen &&
            freeEdges.contains(containment->location())) {
            freeEdges.removeAll(containment->location());
        }
    }

    return freeEdges;
}

QAction *Corona::action(QString name) const
{
    return d->actions.action(name);
}

void Corona::addAction(QString name, QAction *action)
{
    d->actions.addAction(name, action);
}

KAction* Corona::addAction(QString name)
{
    return d->actions.addAction(name);
}

QList<QAction*> Corona::actions() const
{
    return d->actions.actions();
}

void Corona::enableAction(const QString &name, bool enable)
{
    QAction *action = d->actions.action(name);
    if (action) {
        action->setEnabled(enable);
        action->setVisible(enable);
    }
}

void Corona::updateShortcuts()
{
    d->actions.readSettings();
    d->shortcutsDlg.addCollection(&d->actions);
}

void Corona::addShortcuts(KActionCollection *newShortcuts)
{
    d->shortcutsDlg.addCollection(newShortcuts);
}

void Corona::setContainmentActionsDefaults(Containment::Type containmentType, const ContainmentActionsPluginsConfig &config)
{
    d->containmentActionsDefaults.insert(containmentType, config);
}

ContainmentActionsPluginsConfig Corona::containmentActionsDefaults(Containment::Type containmentType)
{
    return d->containmentActionsDefaults.value(containmentType);
}

void Corona::setDialogManager(AbstractDialogManager *dialogManager)
{
    d->dialogManager = dialogManager;
}

AbstractDialogManager *Corona::dialogManager()
{
    return d->dialogManager.data();
}

} // namespace Plasma

#include "corona.moc"

