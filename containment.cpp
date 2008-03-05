/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KMimeType>
#include <KRun>
#include <KServiceTypeTrader>
#include <KStandardDirs>

#include "applethandle_p.h"
#include "corona.h"
#include "phase.h"
#include "desktoptoolbox_p.h"
#include "svg.h"

#include "layouts/freelayout.h"
#include "layouts/boxlayout.h"

namespace Plasma
{

static const int INTER_CONTAINMENT_MARGIN = 6;

class Containment::Private
{
public:
    Private(Containment* c)
        : q(c),
          formFactor(Planar),
          location(Floating),
          screen(-1), // no screen
          toolbox(0),
          type(Containment::NoContainmentType)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
    }

    DesktopToolbox* createToolbox()
    {
        if (!toolbox) {
            toolbox = new DesktopToolbox(q);
            toolbox->setPos(q->geometry().width() - toolbox->boundingRect().width(), 0);
        }

        return toolbox;
    }

    void setLockToolText();

    Containment *q;
    FormFactor formFactor;
    Location location;
    Applet::List applets;
    QMap<Applet*, AppletHandle*> handles;
    int screen;
    DesktopToolbox *toolbox;
    Containment::Type type;
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
            icon->resize(icon->sizeHint());
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
    setDrawStandardBackground(false);
    setContainmentType(CustomContainment);
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
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
}

void Containment::loadConstraints(KConfigGroup* group)
{
    /*kDebug() << "!!!!!!!!!!!!initConstraints" << group->name() << containmentType();
    kDebug() << "    location:" << group->readEntry("location", (int)d->location);
    kDebug() << "    geom:" << group->readEntry("geometry", geometry());
    kDebug() << "    formfactor:" << group->readEntry("formfactor", (int)d->formFactor);
    kDebug() << "    screen:" << group->readEntry("screen", d->screen);*/
    setGeometry(group->readEntry("geometry", geometry()));
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
    //kDebug() << "got containmentConstraintsUpdated" << constraints << (QObject*)d->toolbox;
    if (constraints & Plasma::ImmutableConstraint) {
        d->setLockToolText();

        // tell the applets too
        foreach (Applet *a, d->applets) {
            a->constraintsUpdated(ImmutableConstraint);
        }
    }

    if (constraints & Plasma::ScreenConstraint && d->toolbox) {
        d->toolbox->setPos(geometry().width() - d->toolbox->boundingRect().width(), 0);
        d->toolbox->enableTool("addwidgets", !isImmutable());
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
            Plasma::Widget *addWidgetTool = addToolBoxTool("addwidgets", "list-add", i18n("Add Widgets"));
            connect(addWidgetTool, SIGNAL(clicked()), this, SIGNAL(showAddWidgets()));

            Plasma::Widget *zoomInTool = addToolBoxTool("zoomIn", "zoom-in", i18n("Zoom In"));
            connect(zoomInTool, SIGNAL(clicked()), this, SIGNAL(zoomIn()));

            Plasma::Widget *zoomOutTool = addToolBoxTool("zoomOut", "zoom-out", i18n("Zoom Out"));
            connect(zoomOutTool, SIGNAL(clicked()), this, SIGNAL(zoomOut()));

            if (!isKioskImmutable()) {
                Plasma::Widget *lockTool = addToolBoxTool("lockWidgets", "object-locked",
                                                          isImmutable() ? i18n("Unlock Widgets") :
                                                                          i18n("Lock Widgets"));
                connect(lockTool, SIGNAL(clicked()), this, SLOT(toggleDesktopImmutability()));
            }
        }
    } else {
        delete d->toolbox;
        d->toolbox = 0;
    }
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
        if (containmentActions.count() > 0) {
            hasEntries = true;
            QMenu *containmentActionMenu = &desktopMenu;

            if (actions.count() > 0 && containmentActions.count() > 2) {
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
    d->formFactor = formFactor;
    Layout *lay = 0;
    //note: setting a new layout autodeletes the old one
    //and creating a layout calls setLayout on the parent

    switch (d->formFactor) {
        case Planar:
        case MediaCenter:
            lay = new FreeLayout(this);
            break;
        case Horizontal:
            lay = new BoxLayout(BoxLayout::LeftToRight, this);
            lay->setMargins(0, 0, 0, 0);
            lay->setSpacing(4);
            break;
        case Vertical:
            lay = new BoxLayout(BoxLayout::TopToBottom, this);
            lay->setMargins(0, 0, 0, 0);
            lay->setSpacing(4);
            break;
        default:
            kDebug() << "This can't be happening! Or... can it? ;)" << d->formFactor;
            setLayout(0); //auto-delete
            break;
    }

    if (lay) {
        foreach (Applet* applet, d->applets) {
            lay->addItem(applet);
            applet->updateConstraints(Plasma::FormFactorConstraint);
        }
    }

    updateConstraints(Plasma::FormFactorConstraint);
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
            applet->setGeometry(appletGeometry);
        } else if (appletGeometry.x() != -1 && appletGeometry.y() != -1) {
            // yes, this means we can't have items start -1, -1
            applet->setGeometry(QRectF(appletGeometry.topLeft(),
                                    applet->sizeHint()));
        } else if (geometry().isValid()) {
            applet->setGeometry(geometryForApplet(applet));
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
        applet->resetConfigurationObject();
        currentContainment->d->applets.removeAll(applet);
        addChild(applet);

        // now move the old config to the new location
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
    } else {
        addChild(applet);
    }

    d->applets << applet;

    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));

    if (containmentType() == PanelContainment) {
        // Reposition the applet after adding has been done
        if (index != -1) {
            BoxLayout *l = dynamic_cast<BoxLayout *>(layout());
            l->insertItem(index, l->takeAt(l->indexOf(applet)));
            d->applets.removeAll(applet);
            d->applets.insert(index, applet);
        }
    } else {
        //FIXME if it came from a panel its bg was disabled
        //maybe we should expect the applet to handle that on a constraint update?

        //should we really do this? hell, do we have any business playing with the geometry of
        //non-panel applets at all?
        if (pos != QPointF(-1, -1)) {
            applet->setPos(pos);
        }
    }

    prepareApplet(applet, delayInit); //must at least flush constraints
}

//containment-relative pos... right?
int Containment::indexAt(const QPointF &pos) const
{
    if (pos == QPointF(-1, -1)) {
        return -1;
    }
    BoxLayout *l = dynamic_cast<BoxLayout *>(layout());
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
    }
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

QRectF Containment::geometryForApplet(Applet *applet) const
{
    // The value part of these maps isn't used. Only sorted keys are needed.
    QMap<qreal, bool> xPositions;
    QMap<qreal, bool> yPositions;

    // Add the top-left corner offset by the applet's border
    QPointF offset = applet->boundingRect().topLeft();
    xPositions[-offset.x()] = true;
    yPositions[-offset.y()] = true;

    QRectF placement(QPointF(0, 0), applet->sizeHint());
    foreach (Applet *existingApplet, d->applets) {
        QPointF bottomRight = existingApplet->geometry().bottomRight();
        if (bottomRight.x() + placement.width() < geometry().width()) {
            xPositions[bottomRight.x() + 1] = true;
        }
        if (bottomRight.y() + placement.height() < geometry().height()) {
            yPositions[bottomRight.y() + 1] = true;
        }
    }

    // Try to fit it in an empty space
    foreach (qreal x, xPositions.keys()) {
        foreach (qreal y, yPositions.keys()) {
            placement.moveTo(x, y);
            if (regionIsEmpty(placement, applet)) {
                return placement;
            }
        }
    }

    // Otherwise place it in the centre of the screen
    placement.moveLeft(geometry().width() / 2 - placement.width() / 2);
    placement.moveTop(geometry().height() / 2 - placement.height() / 2);
    return placement;
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
    // sanity check to make sure someone else doesn't have this screen already!
    if (screen > -1 && containmentType() == DesktopContainment && corona()) {
        Containment* currently = corona()->containmentForScreen(screen);
        if (currently && currently != this) {
            //kDebug() << "currently is on screen" << currently->screen() << "and is" << currently->name() << (QObject*)currently << (QObject*)this;
            currently->setScreen(-1);
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
        QRect r = desktop->screenGeometry(screen);

        if (containmentType() == DesktopContainment) {
            // we need to find how many screens are to our top and left
            // to calculate the proper offsets for the margins.
            int x = r.x();
            int y = r.y();
            int screensLeft = 0;
            int screensAbove = 0;
            for (int i = 0; i < numScreens; ++i) {
                QRect otherScreen = desktop->screenGeometry(screen);
                if (x > otherScreen.x()) {
                    ++screensLeft;
                }

                if (y > otherScreen.y()) {
                    ++screensAbove;
                }
            }

            r.moveLeft(r.x() + INTER_CONTAINMENT_MARGIN * screensLeft);
            r.moveTop(r.y() + INTER_CONTAINMENT_MARGIN * screensAbove);

            // FIXME: positioning at this x,y will break if we switch between containments for a
            //        given screen! we should change the pos() on new containment setup.
            setGeometry(r);
            //kDebug() << "setting geometry to" << desktop->screenGeometry(screen) << r << geometry();
        } else if (containmentType() == PanelContainment) {
            //kDebug() << "we are a panel on" << r << ", let's move ourselves to a negative coordinate system" << -(r.y() * 2) - r.height() - INTER_CONTAINMENT_MARGIN;
            // panels are moved into negative coords; we double the x() so that each screen get's
            // it's own area for panels
            int vertOffset = (r.y() * 2) + r.height() + INTER_CONTAINMENT_MARGIN;
            translate(0, -vertOffset);
        }
    }

    d->screen = screen;
    updateConstraints(Plasma::ScreenConstraint);
}

int Containment::screen() const
{
    return d->screen;
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
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->scenePos(), QSize(0, 0));
            QVariantList args;
            args << url.url();
            //             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::knownAppletsForMimetype(mimeName);

            if (appletList.isEmpty()) {
                // no special applet associated with this mimetype, let's
                addApplet("icon", args, 0, geom);
            } else {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //appletList.count() > 0?
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

void Containment::emitLaunchActivated()
{
    kDebug();
    emit launchActivated();
}

Plasma::Widget * Containment::addToolBoxTool(const QString& toolName, const QString& iconName, const QString& iconText)
{
    Plasma::Icon *tool = new Plasma::Icon(this);

    tool->setDrawBackground(true);
    tool->setIcon(KIcon(iconName));
    tool->setText(iconText);
    tool->setOrientation(Qt::Horizontal);
    QSizeF iconSize = tool->sizeFromIconSize(22);
    tool->setMinimumSize(iconSize);
    tool->setMaximumSize(iconSize);
    tool->resize(tool->sizeHint());

    d->createToolbox()->addTool(tool, toolName);

    return tool;
}

void Containment::enableToolBoxTool(const QString &toolname, bool enable)
{
    d->createToolbox()->enableTool(toolname, enable);
}

bool Containment::isToolboxToolEnabled(const QString &toolname) const
{
    return d->createToolbox()->isToolEnabled(toolname);
}

void Containment::showToolbox()
{
    d->createToolbox()->showToolbox();
}

void Containment::hideToolbox()
{
    d->createToolbox()->hideToolbox();
}

} // Plasma namespace

#include "containment.moc"

