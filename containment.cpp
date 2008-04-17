/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#include "containment.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KMimeType>
#include <KRun>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KWindowSystem>

#include "applethandle_p.h"
#include "corona.h"
#include "phase.h"
#include "desktoptoolbox_p.h"
#include "paneltoolbox_p.h"
#include "svg.h"

namespace Plasma
{

static const int INTER_CONTAINMENT_MARGIN = 6;
static const int VERTICAL_STACKING_OFFSET = 10000;

class Containment::Private
{
public:
    Private(Containment* c)
        : q(c),
          formFactor(Planar),
          location(Floating),
          screen(-1), // no screen
          toolbox(0),
          type(Containment::NoContainmentType),
          positioning(false)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
    }

    Toolbox* createToolbox()
    {
        if (!toolbox) {
            switch (type) {
            case PanelContainment:
                toolbox = new PanelToolbox(q);
                break;
            //defaults to DesktopContainment right now
            default:
                toolbox = new DesktopToolbox(q);
                break;
            }
            positionToolbox();
        }

        return toolbox;
    }

    void positionToolbox()
    {
        QRectF r;
        if (screen < 0) {
            r = q->geometry();
        } else {
            QDesktopWidget *desktop = QApplication::desktop();
            r = desktop->availableGeometry(screen);
        }

        toolbox->setPos(QPointF(r.right(), r.y()));
    }

    void setLockToolText();

    Containment *q;
    FormFactor formFactor;
    Location location;
    Applet::List applets;
    QMap<Applet*, AppletHandle*> handles;
    int screen;
    Toolbox *toolbox;
    Containment::Type type;
    bool positioning;
};

void Containment::Private::setLockToolText()
{
    if (toolbox) {
        Icon *icon = dynamic_cast<Plasma::Icon*>(toolbox->tool("lockWidgets"));
        if (icon) {
            // we know it's an icon becase we made it
            icon->setText(q->isImmutable() ? i18n("Unlock Widgets") :
                                             i18n("Lock Widgets"));
            QSizeF iconSize = icon->sizeFromIconSize(22);
            icon->setMinimumSize(iconSize);
            icon->setMaximumSize(iconSize);
            icon->resize(icon->size());
        }
    }
}

Containment::StyleOption::StyleOption()
    : QStyleOptionGraphicsItem(),
      desktop(-1)
{

}

Containment::StyleOption::StyleOption(const Containment::StyleOption & other)
    : QStyleOptionGraphicsItem(other),
      desktop(other.desktop)
{
}

Containment::StyleOption::StyleOption(const QStyleOptionGraphicsItem &other)
    : QStyleOptionGraphicsItem(other),
      desktop(-1)
{
}

Containment::Containment(QGraphicsItem* parent,
                         const QString& serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new Private(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setDrawStandardBackground(false);
    setContainmentType(CustomContainment);
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setDrawStandardBackground(false);
}

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
    setCacheMode(NoCache);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    //TODO: would be nice to not do this on init, as it causes Phase to init
    connect(Phase::self(), SIGNAL(animationComplete(QGraphicsItem*,Plasma::Phase::Animation)),
            this, SLOT(appletAnimationComplete(QGraphicsItem*,Plasma::Phase::Animation)));

    if (d->type == NoContainmentType) {
        setContainmentType(DesktopContainment);
    }

    Applet::init();
}

void Containment::loadConstraints(KConfigGroup* group)
{
    /*kDebug() << "!!!!!!!!!!!!initConstraints" << group->name() << containmentType();
    kDebug() << "    location:" << group->readEntry("location", (int)d->location);
    kDebug() << "    geom:" << group->readEntry("geometry", geometry());
    kDebug() << "    formfactor:" << group->readEntry("formfactor", (int)d->formFactor);
    kDebug() << "    screen:" << group->readEntry("screen", d->screen);*/

    QRectF geo = group->readEntry("geometry", geometry());
    //override max/min
    if (geo.size() != geo.size().boundedTo(maximumSize())) {
        setMaximumSize(maximumSize().expandedTo(geo.size()));
    }
    if (geo.size() != geo.size().expandedTo(minimumSize())) {
        setMinimumSize(minimumSize().boundedTo(geo.size()));
    }
    setGeometry(geo);

    setLocation((Plasma::Location)group->readEntry("location", (int)d->location));
    setFormFactor((Plasma::FormFactor)group->readEntry("formfactor", (int)d->formFactor));
    setScreen(group->readEntry("screen", d->screen));
}

void Containment::saveConstraints(KConfigGroup* group) const
{
    // locking is saved in Applet::save
    group->writeEntry("screen", d->screen);
    group->writeEntry("formfactor", (int)d->formFactor);
    group->writeEntry("location", (int)d->location);
}

void Containment::containmentConstraintsUpdated(Plasma::Constraints constraints)
{
    if (!isContainment()) {
        return;
    }

    //kDebug() << "got containmentConstraintsUpdated" << constraints << (QObject*)d->toolbox;
    if (constraints & Plasma::ImmutableConstraint) {
        d->setLockToolText();

        // tell the applets too
        foreach (Applet *a, d->applets) {
            a->constraintsUpdated(ImmutableConstraint);
        }
    }

    if ((constraints & Plasma::SizeConstraint || constraints & Plasma::ScreenConstraint) &&
         d->toolbox) {
        //The placement assumes that the geometry width/height is no more than the screen
        if (d->type == PanelContainment) {
            if (formFactor() == Vertical) {
                d->toolbox->setPos(geometry().width()/2 - d->toolbox->boundingRect().width()/2, geometry().bottom());
            //defaulting to Horizontal right now
            } else {
                d->toolbox->setPos(geometry().right(), geometry().height()/2 - d->toolbox->boundingRect().height()/2);
            }
        } else {
            d->toolbox->setPos(geometry().right() - qAbs(d->toolbox->boundingRect().width()), 0);
        }
        d->toolbox->enableTool("addwidgets", !isImmutable());
    }

    if (constraints & Plasma::FormFactorConstraint && d->toolbox) {
        if (formFactor() == Vertical) {
            d->toolbox->setOrientation(Qt::Vertical);
        //defaults to horizontal
        } else {
            d->toolbox->setOrientation(Qt::Horizontal);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        switch (containmentType()) {
            case PanelContainment:
                positionPanel();
                break;
            default:
                positionContainment();
                break;
        }
    }
}

Containment::Type Containment::containmentType() const
{
    return d->type;
}

void Containment::setContainmentType(Containment::Type type)
{
    d->type = type;

    if (isContainment() && type == DesktopContainment) {
        if (!d->toolbox) {
            QGraphicsWidget *addWidgetTool = addToolBoxTool("addwidgets", "list-add", i18n("Add Widgets"));
            connect(addWidgetTool, SIGNAL(clicked()), this, SIGNAL(showAddWidgets()));

            QGraphicsWidget *zoomInTool = addToolBoxTool("zoomIn", "zoom-in", i18n("Zoom In"));
            connect(zoomInTool, SIGNAL(clicked()), this, SLOT(zoomIn()));

            QGraphicsWidget *zoomOutTool = addToolBoxTool("zoomOut", "zoom-out", i18n("Zoom Out"));
            connect(zoomOutTool, SIGNAL(clicked()), this, SIGNAL(zoomOut()));

            if (!isKioskImmutable()) {
                QGraphicsWidget *lockTool = addToolBoxTool("lockWidgets", "object-locked",
                                                          isImmutable() ? i18n("Unlock Widgets") :
                                                                          i18n("Lock Widgets"));
                connect(lockTool, SIGNAL(clicked()), this, SLOT(toggleDesktopImmutability()));
            }

            QGraphicsWidget *activityTool = addToolBoxTool("addSiblingContainment", "list-add", i18n("Add Activity"));
            connect(activityTool, SIGNAL(clicked()), this, SLOT(addSiblingContainment()));
        }

    } else if (isContainment() && type == PanelContainment) {
        d->createToolbox();
        d->toolbox->setSize(24);
        d->toolbox->setIconSize(QSize(16, 16));
    } else {
        delete d->toolbox;
        d->toolbox = 0;
    }
}

void Containment::setIsContainment(bool isContainment)
{
    Applet::setIsContainment(isContainment);

    // reset the toolbox!
    delete d->toolbox;
    d->toolbox = 0;

    setContainmentType(d->type);
}

Corona* Containment::corona() const
{
    return dynamic_cast<Corona*>(scene());
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    //kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!isContainment() || !scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        Applet::contextMenuEvent(event);
        return;
    }

    QPointF point = event->scenePos();
    QGraphicsItem* item = scene()->itemAt(point);
    if (item == this) {
        item = 0;
    }

    Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet && !applet->isContainment()) {
            break;
        }

        // applet may have a value due to finding a containment!
        applet = 0;
        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << (QObject*)applet;
    if (applet) {
        bool hasEntries = false;

        QList<QAction*> actions = applet->contextActions();
        if (!actions.isEmpty()) {
            foreach(QAction* action, actions) {
                desktopMenu.addAction(action);
            }
            hasEntries = true;
        }

        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = new QAction(i18n("%1 Settings", applet->name()), &desktopMenu);
            configureApplet->setIcon(KIcon("configure"));
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(showConfigurationInterface()));
            desktopMenu.addAction(configureApplet);
            hasEntries = true;
        }

        QList<QAction*> containmentActions = contextActions();
        if (!containmentActions.isEmpty()) {
            if (hasEntries) {
                desktopMenu.addSeparator();
            }
            hasEntries = true;
            QMenu *containmentActionMenu = &desktopMenu;

            if (!actions.isEmpty() && containmentActions.count() > 2) {
                containmentActionMenu = new KMenu(i18n("%1 Options", name()), &desktopMenu);
                desktopMenu.addMenu(containmentActionMenu);
            }

            foreach(QAction* action, containmentActions) {
                containmentActionMenu->addAction(action);
            }
        }

        if (scene() && !static_cast<Corona*>(scene())->isImmutable()) {
            if (hasEntries) {
                desktopMenu.addSeparator();
            }

            QAction* closeApplet = new QAction(i18n("Remove this %1", applet->name()), &desktopMenu);
            QVariant appletV;
            appletV.setValue((QObject*)applet);
            closeApplet->setData(appletV);
            closeApplet->setIcon(KIcon("edit-delete"));
            connect(closeApplet, SIGNAL(triggered(bool)),
                    this, SLOT(destroyApplet()));
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
        }

        if (!hasEntries) {
            Applet::contextMenuEvent(event);
            kDebug() << "no entries";
            return;
        }
    } else {
        if (!scene() || (static_cast<Corona*>(scene())->isImmutable() && !KAuthorized::authorizeKAction("unlock_desktop"))) {
            //kDebug() << "immutability";
            Applet::contextMenuEvent(event);
            return;
        }

        QList<QAction*> actions = contextActions();

        if (actions.count() < 1) {
            //kDebug() << "no applet, but no actions";
            Applet::contextMenuEvent(event);
            return;
        }

        foreach(QAction* action, actions) {
            desktopMenu.addAction(action);
        }
    }

    event->accept();
    //kDebug() << "executing at" << event->screenPos();
    desktopMenu.exec(event->screenPos());
}

void Containment::destroyApplet()
{
    QAction *action = qobject_cast<QAction*>(sender());

    if (!action) {
        return;
    }

    Applet *applet = qobject_cast<Applet*>(action->data().value<QObject*>());
    Phase::self()->animateItem(applet, Phase::Disappear);
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor && layout()) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    FormFactor was = d->formFactor;

    createLayout(formFactor);

    d->formFactor = formFactor;

    if (isContainment() && containmentType() == PanelContainment && was != formFactor) {
        // we are a panel and we have chaged our orientation
        positionPanel(true);
    }

    QGraphicsLayout *lay = layout();
    QGraphicsLinearLayout * linearLay = dynamic_cast<QGraphicsLinearLayout *>(lay);
    if (linearLay) {
        foreach (Applet* applet, d->applets) {
            linearLay->addItem(applet);
            applet->updateConstraints(Plasma::FormFactorConstraint);
        }
	//linearLay->addStretch();
    }
    updateConstraints(Plasma::FormFactorConstraint);
}

void Containment::createLayout(FormFactor formFactor)
{
    //note: setting a new layout autodeletes the old one
    //and creating a layout calls setLayout on the parent
    switch (formFactor) {
        case Planar:
        case MediaCenter:
            //setLayout(new QGraphicsLinearLayout());
            break;
        case Horizontal: {
            QGraphicsLinearLayout *lay = new QGraphicsLinearLayout();
            lay->setOrientation(Qt::Horizontal);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(4);
            lay->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            setLayout(lay);
            break;
            }
        case Vertical: {
            QGraphicsLinearLayout *lay = new QGraphicsLinearLayout();
            lay->setOrientation(Qt::Vertical);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(4);
            setLayout(lay);
            break;
            }
        default:
            kDebug() << "This can't be happening! Or... can it? ;)" << d->formFactor;
            //setLayout(0); //auto-delete
            break;
    }
}

FormFactor Containment::formFactor() const
{
    if (isContainment()) {
        return d->formFactor;
    }

    return Applet::formFactor();
}

void Containment::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    bool emitGeomChange = false;

    if ((location == TopEdge || location == BottomEdge) &&
        (d->location == TopEdge || d->location == BottomEdge)) {
        emitGeomChange = true;
    }

    if ((location == RightEdge || location == LeftEdge) &&
        (d->location == RightEdge || d->location == LeftEdge)) {
        emitGeomChange = true;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    if (emitGeomChange) {
        // our geometry on the scene will not actually change,
        // but for the purposes of views it has
        emit geometryChanged();
    }

    updateConstraints(Plasma::LocationConstraint);
}

Location Containment::location() const
{
    return d->location;
}

void Containment::toggleDesktopImmutability()
{
    if (corona()) {
        corona()->setImmutable(!corona()->isImmutable());
    } else {
        setImmutable(!isImmutable());
    }

    d->setLockToolText();
}

void Containment::zoomIn()
{
    emit zoomIn(this);
}

void Containment::addSiblingContainment()
{
    emit addSiblingContainment(this);
}

void Containment::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Containment::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& appletGeometry, bool delayInit)
{
    if (!delayInit && isImmutable()) {
        kDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
        return 0;
    }

    QGraphicsView *v = view();
    if (v) {
        v->setCursor(Qt::BusyCursor);
    }

    Applet* applet = Applet::load(name, id, args);
    if (v) {
        v->unsetCursor();
    }

    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet;
    }

    addApplet(applet, appletGeometry.topLeft(), delayInit);

    if (containmentType() != PanelContainment) {
        //kDebug() << "adding applet" << applet->name() << "with a default geometry of" << appletGeometry << appletGeometry.isValid();
        if (appletGeometry.isValid()) {
            //applet->setGeometry(appletGeometry);
        } else if (appletGeometry.x() != -1 && appletGeometry.y() != -1) {
            // yes, this means we can't have items start -1, -1
            //applet->setGeometry(QRectF(appletGeometry.topLeft(),
//                                    applet->sizeHint()));
        } else if (geometry().isValid()) {
            //applet->setGeometry(geometryForApplet(applet));
        }
    }

    //kDebug() << applet->name() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    Corona *c = corona();
    if (c) {
        connect(applet, SIGNAL(configNeedsSaving()), corona(), SLOT(scheduleConfigSync()));
    }

    emit appletAdded(applet);
    return applet;
}

//pos must be relative to the containment already. use mapfromscene.
//what we're trying to do here for panels is make the applet go to the requested position,
//or somewhere close to it, and get integrated properly into the containment as if it were created
//there.
void Containment::addApplet(Applet *applet, const QPointF &pos, bool delayInit)
{
    if (!delayInit && isImmutable()) {
        return;
    }

    if (!applet) {
        kDebug() << "adding null applet!?!";
        return;
    }

    Containment *currentContainment = applet->containment();
    int index = -1;

    if (containmentType() == PanelContainment) {
        //panels don't want backgrounds, which is important when setting geometry
        applet->setDrawStandardBackground(false);

        // Calculate where the user wants the applet to go before adding it
        //so long as this isn't a new applet with a delayed init
        if (! delayInit || (currentContainment && currentContainment != this)) {
            index = indexAt(pos);
        }
    }

    if (currentContainment && currentContainment != this) {
        applet->removeSceneEventFilter(currentContainment);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        applet->setParentItem(this);

        // now move the old config to the new location
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->resetConfigurationObject();
    } else {
	applet->setParentItem(this);
    }

    d->applets << applet;

    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));

    QGraphicsLayout *lay = layout();

    // Reposition the applet after adding has been done
    //FIXME adding position incorrect
    if (index != -1) {
        QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout *>(lay);
        if (l) {
            l->insertItem(index, applet);
            d->applets.removeAll(applet);
            d->applets.insert(index, applet);
        } else {
            index = -1;
        }
    }

    if (index == -1) {
        if (pos != QPointF(-1, -1)) {
            applet->setPos(pos);
        }

        QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout *>(lay);
        if (l) {
            l->addItem(applet);
        }
        //l->addStretch();
    }

    prepareApplet(applet, delayInit); //must at least flush constraints
}

//containment-relative pos... right?
int Containment::indexAt(const QPointF &pos) const
{
/*    if (pos == QPointF(-1, -1)) {
        return -1;
    }
    QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout *>(layout());
    if (l) {
        foreach (Applet *existingApplet, d->applets) {
            if (formFactor() == Horizontal) {
                qreal middle = (existingApplet->geometry().left() +
                        existingApplet->geometry().right()) / 2.0;
                // Applets are checked in order so there is no need to check
                // if the position is equal to or greater than the applet's
                // leftmost point. This also allows for dropping in the gap
                // between applets.
                if (pos.x() < middle) {
                    return l->indexOf(existingApplet);
                } else if (pos.x() <= existingApplet->geometry().right()) {
                    return l->indexOf(existingApplet) + 1;
                }
            } else {
                qreal middle = (existingApplet->geometry().top() +
                        existingApplet->geometry().bottom()) / 2.0;
                if (pos.y() < middle) {
                    return l->indexOf(existingApplet);
                } else if (pos.y() <= existingApplet->geometry().bottom()) {
                    return l->indexOf(existingApplet) + 1;
                }
            }
        }
    }*/
    return -1;
}

void Containment::prepareApplet(Applet *applet, bool delayInit)
{
    if (delayInit) {
        if (containmentType() == DesktopContainment) {
            applet->installSceneEventFilter(this);
        }
    } else {
        applet->init();
        Phase::self()->animateItem(applet, Phase::Appear);
    }

    applet->updateConstraints(Plasma::AllConstraints | Plasma::StartupCompletedConstraint);
    if (!delayInit) {
        applet->flushUpdatedConstraints();
        emit configNeedsSaving();
    }
}

bool Containment::regionIsEmpty(const QRectF &region, Applet *ignoredApplet) const
{
    foreach (Applet *applet, d->applets) {
        if (applet != ignoredApplet && applet->geometry().intersects(region)) {
            return false;
        }
    }
    return true;
}

void Containment::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    d->applets.removeAll(applet);
    emit appletRemoved(applet);
    emit configNeedsSaving();
}

void Containment::appletAnimationComplete(QGraphicsItem *item, Plasma::Phase::Animation anim)
{
    if (anim == Phase::Disappear) {
        QGraphicsItem *parent = item->parentItem();

        while (parent) {
            if (parent == this) {
                Applet *applet = qgraphicsitem_cast<Applet*>(item);

                if (applet) {
                    applet->destroy();
                }

                break;
            }

            parent = parent->parentItem();
        }
    } else if (anim == Phase::Appear) {
        if (containmentType() == DesktopContainment &&
            item->parentItem() == this &&
            qgraphicsitem_cast<Applet*>(item)) {
                item->installSceneEventFilter(this);
        }
    }
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int screen)
{
    // screen of -1 means no associated screen.
    if (containmentType() == DesktopContainment || containmentType() == CustomContainment) {
#ifndef Q_OS_WIN
        // we want to listen to changes in work area if our screen changes
        if (d->screen < 0 && screen > -1) {
            connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(repositionToolbox()));
        } else if (screen < 0) {
            disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(repositionToolbox()));
        }
#endif
        if (screen > -1 && corona()) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment* currently = corona()->containmentForScreen(screen);
            if (currently && currently != this) {
                //kDebug() << "currently is on screen" << currently->screen() << "and is" << currently->name() << (QObject*)currently << (QObject*)this;
                currently->setScreen(-1);
            }
        }
    }

    //kDebug() << "setting screen to" << screen << "and we are a" << containmentType();
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    if (screen < -1) {
        screen = -1;
    }

    //kDebug() << "setting screen to " << screen << "and type is" << containmentType();
    if (screen < numScreens && screen > -1) {
        if (containmentType() == DesktopContainment ||
            containmentType() == CustomContainment) {
            resize(desktop->screenGeometry(screen).size());
        }
    }

    int oldScreen = d->screen;
    d->screen = screen;
    updateConstraints(Plasma::ScreenConstraint);
    if (oldScreen != screen) {
        emit screenChanged(oldScreen, screen, this);
    }
}

void Containment::positionContainment()
{
    Corona *c = corona();
    if (!c) {
        return;
    }

    QList<Containment*> containments = c->containments();
    QMutableListIterator<Containment*> it(containments);

    while (it.hasNext()) {
        Containment *containment = it.next();
        if (containment == this ||
            containment->containmentType() == PanelContainment) {
            // weed out all containments we don't care about at all
            // e.g. Panels and ourself
            it.remove();
            continue;
        }

        if (collidesWithItem(containment)) {
            break;
        }
    }

    if (!it.hasNext()) {
        // we made it all the way through the list, we have no
        // collisions
        return;
    }

    // we need to find how many screens are to our top and left
    // to calculate the proper offsets for the margins.
    int width = 0;
    int height = 0;

    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();

    for (int i = 0; i < numScreens; ++i) {
        QRect otherScreen = desktop->screenGeometry(i);

        if (width < otherScreen.width()) {
            width = otherScreen.width();
        }

        if (height < otherScreen.height()) {
            height = otherScreen.height();
        }
    }

    //this magic number (4) is the number of columns to try before going to the next row
    width = (width + INTER_CONTAINMENT_MARGIN) * 4;
    height += INTER_CONTAINMENT_MARGIN;

    // a mildly naive "find the first slot" approach
    QRectF r = boundingRect();
    QPointF topLeft(0, 0);
    setPos(topLeft);

    d->positioning = true;
    while (true) {
        it.toFront();

        while (it.hasNext()) {
            Containment *containment = it.next();
            if (collidesWithItem(containment)) {
                break;
            }

            QPointF pos = containment->pos();
            if (pos.x() <= topLeft.x() && pos.y() <= topLeft.y()) {
                // we don't colid with this containment, and it's above
                // and to the left of us, so let's not bother checking
                // it again if we go through this loop again
                it.remove();
            }
        }

        if (!it.hasNext()) {
            // success! no collisions!
            break;
        }

        if (topLeft.x() + (r.width() * 2) + INTER_CONTAINMENT_MARGIN > width) {
            // we ran out of width room, try another row
            topLeft = QPoint(0, topLeft.y() + height);
        } else {
            topLeft.setX(topLeft.x() + r.width() + INTER_CONTAINMENT_MARGIN);
        }

        kDebug() << "trying at" << topLeft;
        setPos(topLeft);
        //kDebug() << collidingItems().count() << collidingItems()[0] << (QGraphicsItem*)this;
    }

    d->positioning = false;
}

void Containment::positionPanel(bool force)
{
    if (!scene()) {
        kDebug() << "no scene yet";
        return;
    }

    // we position panels in negative coordinates, and stack all horizontal
    // and all vertical panels with each other.

    const QPointF p = pos();

    if (!force &&
        p.y() + size().height() < -INTER_CONTAINMENT_MARGIN &&
        scene()->collidingItems(this).isEmpty()) {
        // already positioned and not running into any other panels
        return;
    }

    //TODO: research how non-Horizontal, non-Vertical (e.g. Planar) panels behave here
    bool horiz = formFactor() == Plasma::Horizontal;
    qreal bottom = horiz ? 0 : VERTICAL_STACKING_OFFSET;
    qreal lastHeight = 0;

    // this should be ok for small numbers of panels, but we ever end
    // up managing hundreds of them, this simplistic alogrithm will
    // likely be too slow.
    foreach (const Containment* other, corona()->containments()) {
        if (other == this ||
            other->containmentType() != PanelContainment ||
            horiz != (other->formFactor() == Plasma::Horizontal)) {
            // only line up with panels of the same orientation
            continue;
        }

        if (horiz) {
            qreal y = other->pos().y();
            if (y < bottom) {
                lastHeight = other->size().height();
                bottom = y;
            }
        } else {
            qreal width = other->size().width();
            qreal x = other->pos().x() + width;
            if (x > bottom) {
                lastHeight = width;
                bottom = x + lastHeight;
            }
        }
    }

    kDebug() << "positioning" << (horiz ? "" : "non-") << "horizontal panel; forced?" << force;
    // give a space equal to the height again of the last item so there is
    // room to grow.
    QPointF newPos;
    if (horiz) {
        bottom -= lastHeight + INTER_CONTAINMENT_MARGIN;
        //TODO: fix x position for non-flush-left panels
        kDebug() << "moved to" << QPointF(0, bottom - size().height());
        newPos = QPointF(0, bottom - size().height());
    } else {
        bottom += lastHeight + INTER_CONTAINMENT_MARGIN;
        //TODO: fix y position for non-flush-top panels
        kDebug() << "moved to" << QPointF(bottom + size().width(), -INTER_CONTAINMENT_MARGIN - size().height());
        newPos = QPointF(bottom + size().width(), -INTER_CONTAINMENT_MARGIN - size().height());
    }

    d->positioning = true;
    if (p != newPos) {
        setPos(newPos);
        emit geometryChanged();
    }
    d->positioning = false;
}

int Containment::screen() const
{
    return d->screen;
}

QPoint Containment::effectiveScreenPos() const
{
    if (d->screen < 0) {
        return QPoint();
    }

    QRect r = QApplication::desktop()->screenGeometry(d->screen);
    if (containmentType() == PanelContainment) {
        QRectF p = geometry();

        switch (d->location) {
            case TopEdge:
                return QPoint(r.left() + p.x(), r.top());
                break;
            case BottomEdge:
                return QPoint(r.left() + p.x(), r.bottom() - p.height());
                break;
            case LeftEdge:
                return QPoint(r.left(), r.top() + (p.bottom() + INTER_CONTAINMENT_MARGIN));
                break;
            case RightEdge:
                return QPoint(r.right() - p.width(), r.top() + (p.bottom() + INTER_CONTAINMENT_MARGIN));
                break;
            default:
                //FIXME: implement properly for Floating!
                return p.topLeft().toPoint();
                break;
        }
    } else {
        //NOTE: if we ever support non-origin'd desktop containments
        //      this assumption here will have to change
        return r.topLeft();
    }
}

KPluginInfo::List Containment::knownContainments(const QString &category,
                                                 const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    //kDebug() << "constraint was" << constraint << "which got us" << offers.count() << "matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Containment::knownContainmentsForMimetype(const QString &mimetype)
{
    QString constraint = QString("'%1' in MimeTypes").arg(mimetype);
    //kDebug() << "knownContainmentsForMimetype with" << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    return KPluginInfo::fromServices(offers);
}

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << "drop event:" << event->mimeData()->text();

    QString mimetype(static_cast<Corona*>(scene())->appletMimeType());

    if (event->mimeData()->hasFormat(mimetype) && scene()) {
        QString plasmoidName;
        plasmoidName = event->mimeData()->data(mimetype);
        QRectF geom(mapFromScene(event->scenePos()), QSize(0, 0));
        addApplet(plasmoidName, QVariantList(), 0, geom);
        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->scenePos(), QSize(0, 0));
            QVariantList args;
            args << url.url();
            //             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::listAppletInfoForMimetype(mimeName);

            if (appletList.isEmpty()) {
                // no special applet associated with this mimetype, let's
                addApplet("icon", args, 0, geom);
            } else {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //!appletList.isEmpty()
                addApplet(appletList.first().pluginName(), args, 0, geom);
            }
        }
        event->acceptProposedAction();
    }
}

void Containment::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
    if (event->spontaneous()) {
        Applet::hoverEnterEvent(event);
    }
    Q_UNUSED(event)
}

void Containment::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
//    Applet::hoverLeaveEvent(event);
    Q_UNUSED(event)
}

void Containment::keyPressEvent(QKeyEvent *event)
{
    kDebug() << "keyPressEvent with" << event->key() << "and hoping and wishing for a" << Qt::Key_Tab;
    if (event->key() == Qt::Key_Tab) { // && event->modifiers() == 0) {
        kDebug() << "let's give focus to...." << (QObject*)d->applets.first();
        if (!d->applets.isEmpty()) {
            d->applets.first()->setFocus(Qt::TabFocusReason);
        }
    }
}

bool Containment::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Applet *applet = qgraphicsitem_cast<Applet*>(watched);

    // Otherwise we're watching something we shouldn't be...
    //kDebug() << "got sceneEvent";
    Q_ASSERT(applet!=0);
    if (!d->applets.contains(applet)) {
        return false;
    }

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
        //kDebug() << "got hoverenterEvent" << isImmutable() << " " << applet->isImmutable();
        if (!isImmutable() && !applet->isImmutable()) {
            if (d->handles.contains(applet)) {
                d->handles[applet]->startFading(AppletHandle::FadeIn);
            } else {
                //kDebug() << "generated applet handle";
                //TODO: there should be a small delay on showing these. they pop up too quickly/easily
                //      right now
                AppletHandle *handle = new AppletHandle(this, applet);
                d->handles[applet] = handle;
                connect(handle, SIGNAL(disappearDone(AppletHandle*)),
                        this, SLOT(handleDisappeared(AppletHandle*)));
                connect(applet, SIGNAL(geometryChanged()),
                        handle, SLOT(appletResized()));
            }
        }
        break;
    case QEvent::GraphicsSceneHoverLeave:
        //kDebug() << "got hoverLeaveEvent";
        if (d->handles.contains(applet)) {
            QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
            if (!d->handles[applet]->boundingRect().contains(d->handles[applet]->mapFromScene(he->scenePos()))) {
                d->handles[applet]->startFading(AppletHandle::FadeOut);
            }
        }
    default:
        break;
    }

    return false;
}

void Containment::handleDisappeared(AppletHandle *handle)
{
    d->handles.remove(handle->applet());
    handle->deleteLater();
}

QVariant Containment::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_UNUSED(value)

    if (isContainment() &&
        (change == QGraphicsItem::ItemSceneHasChanged || change == QGraphicsItem::ItemPositionHasChanged) &&
        !d->positioning) {
        switch (containmentType()) {
            case PanelContainment:
                positionPanel();
                break;
            default:
                positionContainment();
                break;
        }
    }

    return Applet::itemChange(change, value);
}

void Containment::emitLaunchActivated()
{
    kDebug();
    emit launchActivated();
}

QGraphicsWidget * Containment::addToolBoxTool(const QString& toolName, const QString& iconName, const QString& iconText)
{
    Plasma::Icon *tool = new Plasma::Icon(this);

    tool->setDrawBackground(true);
    tool->setIcon(KIcon(iconName));
    tool->setText(iconText);
    tool->setOrientation(Qt::Horizontal);
    QSizeF iconSize = tool->sizeFromIconSize(22);
    tool->setMinimumSize(iconSize);
    tool->setMaximumSize(iconSize);
    tool->resize(tool->size());

    d->createToolbox()->addTool(tool, toolName);

    return tool;
}

void Containment::enableToolBoxTool(const QString &toolname, bool enable)
{
    if (d->toolbox) {
        d->toolbox->enableTool(toolname, enable);
    }
}

bool Containment::isToolboxToolEnabled(const QString &toolname) const
{
    if (d->toolbox) {
        return d->toolbox->isToolEnabled(toolname);
    }
    return false;
}

void Containment::showToolbox()
{
    if (d->toolbox) {
        d->toolbox->showToolbox();
    }
}

void Containment::hideToolbox()
{
    if (d->toolbox) {
        d->toolbox->hideToolbox();
    }
}

void Containment::repositionToolbox()
{
    //kDebug() << "reposition" << d->screen << (QObject*)d->toolbox;
    if (d->toolbox) {
        d->positionToolbox();
    }
}

} // Plasma namespace

#include "containment.moc"

