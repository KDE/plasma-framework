/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
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

#include "applet.h"
#include "applet_p.h"

#include <cmath>
#include <limits>

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QPainter>
#include <QSize>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTimer>
#include <QUiLoader>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>

#include <KIcon>
#include <KColorScheme>
#include <KConfigDialog>
#include <KDialog>
#include <KIconLoader>
#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>
#include <KServiceTypeTrader>
#include <KWindowSystem>

#include <Solid/PowerManagement>

#include "plasma/configxml.h"
#include "plasma/containment.h"
#include "plasma/containment_p.h"
#include "plasma/corona.h"
#include "plasma/dataenginemanager.h"
#include "plasma/package.h"
#include "plasma/packages_p.h"
#include "plasma/plasma.h"
#include "plasma/scripting/appletscript.h"
#include "plasma/shadowitem_p.h"
#include "plasma/svg.h"
#include "plasma/panelsvg.h"
#include "plasma/theme.h"
#include "plasma/view.h"

//#define DYNAMIC_SHADOWS
namespace Plasma
{

Applet::Applet(QGraphicsItem *parent,
               const QString& serviceID,
               uint appletId)
    :  QGraphicsWidget(parent),
       d(new Private(KService::serviceByStorageId(serviceID), appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
}

Applet::Applet(QObject* parentObject, const QVariantList& args)
    :  QGraphicsWidget(0),
       d(new Private(KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString()),
                     args.count() > 1 ? args[1].toInt() : 0, this))
{
    setParent(parentObject);
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();

    // the brain damage seen in the initialization list is due to the
    // inflexibility of KService::createInstance
}

Applet::~Applet()
{
    if (d->transient) {
        d->resetConfigurationObject();
    }

    delete d;
}

PackageStructure::Ptr Applet::packageStructure()
{
    if (!Private::packageStructure) {
        Private::packageStructure = new PlasmoidPackage();
    }

    return Private::packageStructure;
}

void Applet::init()
{
    if (d->script && !d->script->init()) {
        setFailedToLaunch(true, i18n("Script initialization failed"));
    }
    //Here is the code for the window frame
    //setWindowFlags(Qt::Window);
}

uint Applet::id() const
{
    return d->appletId;
}

void Applet::save(KConfigGroup* group) const
{
    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group->writeEntry("immutability", (int)d->immutability);
    group->writeEntry("plugin", pluginName());
    //FIXME: for containments, we need to have some special values here w/regards to
    //       screen affinity (e.g. "bottom of screen 0")
    //kDebug() << pluginName() << "geometry is" << geometry() << "pos is" << pos() << "bounding rect is" << boundingRect();
    group->writeEntry("geometry", geometry());
    group->writeEntry("zvalue", zValue());

    if (transform() == QTransform()) {
        group->deleteEntry("transform");
    } else {
        QList<qreal> m;
        QTransform t = transform();
        m << t.m11() << t.m12() << t.m13() << t.m21() << t.m22() << t.m23() << t.m31() << t.m32() << t.m33();
        group->writeEntry("transform", m);
        //group->writeEntry("transform", transformToString(transform()));
    }

    KConfigGroup appletConfigGroup(group, "Configuration");
    //FIXME: we need a global save state too
    saveState(&appletConfigGroup);
}

void Applet::restore(KConfigGroup *c)
{
    QList<qreal> m = c->readEntry("transform", QList<qreal>());
    if (m.count() == 9) {
        QTransform t(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        setTransform(t);
    }

    qreal z = c->readEntry("zvalue", 0);

    if (z >= Private::s_maxZValue) {
        Private::s_maxZValue = z;
    }

    setZValue(z);

    setImmutability((ImmutabilityType)c->readEntry("immutability", (int)NotImmutable));
}

void Applet::setFailedToLaunch(bool failed, const QString& reason)
{
    if (d->failed == failed) {
        return;
    }

    d->failed = failed;
    prepareGeometryChange();

    qDeleteAll(QGraphicsItem::children());
    setLayout(0);

    if (failed) {
        setBackgroundHints(d->backgroundHints|StandardBackground);

        QGraphicsLinearLayout *failureLayout = new QGraphicsLinearLayout();
        failureLayout->setContentsMargins(0, 0, 0, 0);
        QGraphicsProxyWidget * failureWidget = new QGraphicsProxyWidget(this);
        QLabel * label = new QLabel(d->visibleFailureText(reason));
        //FIXME : Get the theme color font
        label->setAttribute(Qt::WA_NoSystemBackground);
        label->setAutoFillBackground(false);
        label->setWordWrap(true);
        failureWidget->setWidget(label);
        failureLayout->addItem(failureWidget);
        setLayout(failureLayout);
        setMinimumSize(label->size());
        resize(300,250);
        d->background->resizePanel(geometry().size());

    }
    update();
}

void Applet::saveState(KConfigGroup* group) const
{
    if (group->config()->name() != config().config()->name()) {
        // we're being saved to a different file!
        // let's just copy the current values in our configuration over
        KConfigGroup c = config();
        d->copyEntries(&c, group);
    }
}

KConfigGroup Applet::config(const QString &group) const
{
    KConfigGroup cg = config();
    return KConfigGroup(&cg, group);
}

KConfigGroup Applet::config() const
{
    if (d->isContainment) {
        return *(d->mainConfigGroup());
    }

    return KConfigGroup(d->mainConfigGroup(), "Configuration");
}

KConfigGroup Applet::globalConfig() const
{
    KConfigGroup globalAppletConfig;
    const Containment *c = containment();
    QString group = isContainment() ? "ContainmentGlobals" : "AppletGlobals";

    if (c && c->corona()) {
        KSharedConfig::Ptr coronaConfig = c->corona()->config();
        globalAppletConfig = KConfigGroup(coronaConfig, group);
    } else {
        globalAppletConfig = KConfigGroup(KGlobal::config(), group);
    }

    return KConfigGroup(&globalAppletConfig, d->globalName());
}

void Applet::destroy()
{
    //kDebug() << "???????????????? DESTROYING APPLET" << name() << " ???????????????????????????";
    if (d->configXml) {
        d->configXml->setDefaults();
    }

    d->transient = true;
    deleteLater();
}

ConfigXml* Applet::configScheme() const
{
    return d->configXml;
}

DataEngine* Applet::dataEngine(const QString& name) const
{
    int index = d->loadedEngines.indexOf(name);
    if (index != -1) {
        return DataEngineManager::self()->engine(name);
    }

    DataEngine* engine = DataEngineManager::self()->loadEngine(name);
    if (engine->isValid()) {
        d->loadedEngines.append(name);
    }

    return engine;
}

const Package* Applet::package() const
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

    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            return view;
        }
    }
    return 0;
}

QRectF Applet::mapFromView(const QGraphicsView *view, const QRect &rect) const
{
    // TODO: Confirm that adjusted() is needed and is not covering for some
    // issue elsewhere
    return mapFromScene(view->mapToScene(rect)).boundingRect().adjusted(0, 0, 1, 1);
}

QRect Applet::mapToView(const QGraphicsView *view, const QRectF &rect) const
{
    // TODO: Confirm that adjusted() is needed and is not covering for some
    // issue elsewhere
    return view->mapFromScene(mapToScene(rect)).boundingRect().adjusted(0, 0, -1, -1);
}

QPoint Applet::popupPosition(const QSize &s) const
{
    QGraphicsView *v = view();
    Q_ASSERT(v);

    QPoint pos = v->mapFromScene(scenePos());
    pos = v->mapToGlobal(pos);
    kDebug() << "==> position is" << scenePos() << v->mapFromScene(scenePos()) << pos;
    Plasma::View *pv = dynamic_cast<Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
        pos = QPoint(pos.x(), pos.y() - s.height());
        break;
    case TopEdge:
        pos = QPoint(pos.x(), pos.y() + (int)size().height());
        break;
    case LeftEdge:
        pos = QPoint(pos.x() + (int)size().width(), pos.y());
        break;
    case RightEdge:
        pos = QPoint(pos.x() - s.width(), pos.y());
        break;
    default:
        if (pos.y() - s.height() > 0) {
             pos = QPoint(pos.x(), pos.y() - s.height());
        } else {
             pos = QPoint(pos.x(), pos.y() + (int)size().height());
        }
    }

    //are we out of screen?

    QRect screenRect = QApplication::desktop()->screenGeometry(pv ? pv->containment()->screen() : -1);
    kDebug() << "==> rect for" << (pv ? pv->containment()->screen() : -1) << "is" << screenRect;

    if (pos.rx() + s.width() > screenRect.right()) {
        pos.rx() -= ((pos.rx() + s.width()) - screenRect.right());
    }

    if (pos.ry() + s.height() > screenRect.bottom()) {
        pos.ry() -= ((pos.ry() + s.height()) - screenRect.bottom());
    }
    pos.rx() = qMax(0, pos.rx());

    return pos;
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
    //kDebug() << constraints << "constraints are FormFactor: " << formFactor() << ", Location: " << location();
    if (d->script) {
        d->script->constraintsEvent(constraints);
    }
}

QString Applet::name() const
{
    if (!d->appletDescription.isValid()) {
        return i18n("Unknown Applet");
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
    return Solid::PowerManagement::appShouldConserveResources();
}

QString Applet::category() const
{
    if (!d->appletDescription.isValid()) {
        return i18n("Miscellaneous");
    }

    return d->appletDescription.category();
}

QString Applet::category(const KPluginInfo& applet)
{
    return applet.property("X-KDE-PluginInfo-Category").toString();
}

QString Applet::category(const QString& appletName)
{
    if (appletName.isEmpty()) {
        return QString();
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        return QString();
    }

    return offers.first()->property("X-KDE-PluginInfo-Category").toString();
}

ImmutabilityType Applet::immutability() const
{
    //Returning the more strict immutability between the applet immutability and Corona one
    ImmutabilityType coronaImmutability = NotImmutable;

    if (dynamic_cast<Corona*>(scene())) {
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
    if (d->immutability == immutable) {
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
    d->backgroundHints = hints;

    //Draw the standard background?
    if (hints & StandardBackground) {
        if (!d->background) {
            d->background = new Plasma::PanelSvg();
            d->background->setImagePath("widgets/background");
            d->background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
            qreal left, top, right, bottom;
            d->background->getMargins(left, top, right, bottom);
            setContentsMargins(left, right, top, bottom);

            QSizeF fitSize(left + right, top + bottom);
            if (minimumSize().expandedTo(fitSize) != minimumSize()) {
                setMinimumSize(minimumSize().expandedTo(fitSize));
            }
            d->background->resizePanel(boundingRect().size());
        }
    } else if (d->background) {
        qreal left, top, right, bottom;
        d->background->getMargins(left, top, right, bottom);
        //Setting a minimum size of 0,0 would result in the panel to be only
        //on the first virtual desktop
        setMinimumSize(qMax(minimumSize().width() - left - right, 1.0),
                       qMax(minimumSize().height() - top - bottom, 1.0));

        delete d->background;
        d->background = 0;
        setContentsMargins(0, 0, 0, 0);
    }

    //Draw the shadow?
    //There are various problems with shadows right now:
    //
    //1) shadows can be seen through translucent areas, which is probably technically correct ubt
    //looks odd
    //2) the shape of the item odesn't conform to the shape of the standard background, e.g. with
    //rounded corners
#ifdef DYNAMIC_SHADOWS
    if (hints & ShadowedBackground) {
        if (d->shadow) {
            d->shadow->setVisible(true);
        } else {
            shadow = new ShadowItem(this);
            if (scene()) {
                scene()->addItem(d->shadow);
                d->shadow->show();
            }
        }
    } else {
        delete d->shadow;
        d->shadow = 0;
    }
#endif
}

bool Applet::hasFailedToLaunch() const
{
    return d->failed;
}

void Applet::paintWindowFrame ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    //Here come the code for the window frame
    //kDebug()<<"ENTER in windowFrame";
    //painter->drawRoundedRect(windowFrameGeometry(),5,5);
}

bool Applet::configurationRequired() const
{
    return d->needsConfigOverlay != 0;
}

void Applet::setConfigurationRequired(bool needsConfig)
{
    if ((d->needsConfigOverlay != 0) == needsConfig) {
        return;
    }

    if (d->needsConfigOverlay) {
        delete d->needsConfigOverlay;
        d->needsConfigOverlay = 0;
        return;
    }

    d->needsConfigOverlay = new AppletOverlayWidget(this);
    d->needsConfigOverlay->resize(boundingRect().size());

    int zValue = 100;
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        if (child->zValue() > zValue) {
            zValue = child->zValue() + 1;
        }
    }
    d->needsConfigOverlay->setZValue(zValue);

    qDeleteAll(d->needsConfigOverlay->QGraphicsItem::children());
    QPushButton* button = new QPushButton();
    button->setText(i18n("Configure..."));
    connect(button, SIGNAL(clicked()), this, SLOT(showConfigurationInterface()));
    QGraphicsLinearLayout *configLayout = new QGraphicsLinearLayout(this);
    configLayout->setContentsMargins(0, 0, 0, 0);
    QGraphicsProxyWidget * configWidget = new QGraphicsProxyWidget(this);
    configWidget->setWidget(button);
    configLayout->addItem(configWidget);
    setLayout(configLayout);
    d->needsConfigOverlay->show();
}

void Applet::flushPendingConstraintsEvents()
{
    if (d->pendingConstraints == NoConstraint) {
        return;
    }

    //kDebug() << "fushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Plasma::Constraints c = d->pendingConstraints;
    d->pendingConstraints = NoConstraint;

    if (c & Plasma::SizeConstraint && d->needsConfigOverlay) {
        d->needsConfigOverlay->setGeometry(QRectF(QPointF(0, 0), boundingRect().size()));
        // FIXME:: rather horrible hack to work around the fact we don't have spacers
        //         for layouts, and with WoC coming i'd rather not expend effort there
        QGraphicsItem * button = d->needsConfigOverlay->QGraphicsItem::children().first();
        if (button) {
            QSizeF s = button->boundingRect().size();
            button->setPos(d->needsConfigOverlay->boundingRect().width() / 2 - s.width() / 2,
                           d->needsConfigOverlay->boundingRect().height() / 2 - s.height() / 2);
        }
    }

    if (c & Plasma::FormFactorConstraint) {
        FormFactor f = formFactor();
        //FIXME: this is a bit of a mess: calling setBackgroundHints twice,
        //       and moving an item to a non-vert/horiz containment that
        //       normally doesn't request a background would then get one!
        if (f == Planar) {
            setBackgroundHints(d->backgroundHints|ShadowedBackground);
        } else if (d->backgroundHints&ShadowedBackground) {
            setBackgroundHints(d->backgroundHints^ShadowedBackground);
        }

        if (!isContainment() && f != Vertical && f != Horizontal) {
            setBackgroundHints(d->backgroundHints|StandardBackground);
        } else if(d->backgroundHints&StandardBackground) {
            setBackgroundHints(d->backgroundHints^StandardBackground);
        }
    }

    Containment* containment = qobject_cast<Plasma::Containment*>(this);
    if (isContainment() && containment) {
        containment->d->containmentConstraintsEvent(c);
    }

    constraintsEvent(c);

    if (layout()) {
        layout()->updateGeometry();
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

void Applet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (d->shadow && d->shadow->shadowedSize() != boundingRect().size()) {
        //kDebug() << "sizes are " << d->shadow->shadowedSize() << boundingRect().size();
        d->shadow->generate();
    }

    painter->save();

    if (transform().isRotating()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->setRenderHint(QPainter::Antialiasing);
    }

    if (d->background &&
        formFactor() != Plasma::Vertical &&
        formFactor() != Plasma::Horizontal) {
        //kDebug() << "option rect is" << option->rect;
        d->background->paintPanel(painter, option->rect, QPointF(0,0));
    }

    if (!d->failed) {
        if (widget && isContainment()) {
            // note that the widget we get is actually the viewport of the view, not the view itself
            View* v = qobject_cast<Plasma::View*>(widget->parent());
            if (!v || v->isWallpaperEnabled()) {
                Containment::StyleOption coption(*option);
                coption.view = v;

                paintInterface(painter, &coption, QRect(QPoint(0,0), boundingRect().size().toSize()));
            }

            painter->restore();
            return;
        }

        //kDebug() << "paint interface of" << (QObject*) this;
        paintInterface(painter, option, QRect(QPoint(0,0),  boundingRect().size().toSize()));
    }
    painter->restore();
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect & contentsRect)
{
    if (d->script) {
        d->script->paintInterface(painter, option, contentsRect);
    } else {
        //kDebug() << "Applet::paintInterface() default impl";
    }
}

FormFactor Applet::formFactor() const
{
    Containment* c = containment();
    return c ? c->d->formFactor : Plasma::Planar;
}

Containment* Applet::containment() const
{
    if (isContainment()) {
        Containment *c = dynamic_cast<Containment*>(const_cast<Applet*>(this));
        if (c) {
            return c;
        }
    }

    QGraphicsItem *parent = parentItem();
    Containment *c = 0;

    while (parent) {
        Containment *possibleC = dynamic_cast<Containment*>(parent);
        if (possibleC && possibleC->isContainment()) {
            c = possibleC;
            break;
        }
        parent = parent->parentItem();
    }

    return c;
}

Location Applet::location() const
{
    Containment* c = containment();
    return c ? c->d->location : Plasma::Desktop;
}

Plasma::AspectRatioMode Applet::aspectRatioMode() const
{
    return d->aspectRatioMode;
}

void Applet::setAspectRatioMode(Plasma::AspectRatioMode mode)
{
    d->aspectRatioMode = mode;
}

void Applet::registerAsDragHandle( QGraphicsItem * item )
{
    if (!item) {
        return;
    }

    int index = d->registeredAsDragHandle.indexOf(item);
    if (index == -1) {
        d->registeredAsDragHandle.append(item);
        item->installSceneEventFilter(this);
    }
}

void Applet::unregisterDragHandle( QGraphicsItem * item )
{
    if (!item) {
        return;
    }

    int index = d->registeredAsDragHandle.indexOf(item);
    if (index != -1) {
        d->registeredAsDragHandle.removeAt(index);
        item->removeSceneEventFilter(this);
    }
}

bool Applet::isRegisteredAsDragHandle( QGraphicsItem * item )
{
    return (d->registeredAsDragHandle.indexOf(item) != -1);
}

bool Applet::hasConfigurationInterface() const
{
    return d->hasConfigurationInterface;
}

void Applet::setHasConfigurationInterface(bool hasInterface)
{
    d->hasConfigurationInterface = hasInterface;
}

bool Applet::eventFilter( QObject *o, QEvent * e )
{
    return QObject::eventFilter(o, e);
}

bool Applet::sceneEventFilter( QGraphicsItem * watched, QEvent * event )
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMouseMove: {
            if (d->registeredAsDragHandle.contains( watched )) {
                mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
                return true;
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
    if (d->immutability == NotImmutable && formFactor() == Plasma::Planar) {
        QGraphicsItem *parent = parentItem();
        Plasma::Applet *applet = qgraphicsitem_cast<Plasma::Applet*>(parent);

        if (applet && applet->isContainment()) {
            // our direct parent is a containment. just move ourselves.
            QPointF curPos = event->pos();
            QPointF lastPos = event->lastPos();
            QPointF delta = curPos-lastPos;

            moveBy(delta.x(),delta.y());
        } else if (parent) {
            //don't move the icon as well because our parent (usually an appletHandle) will do it for us
            //parent->moveBy(delta.x(),delta.y());
            QPointF curPos = parent->transform().map(event->pos());
            QPointF lastPos = parent->transform().map(event->lastPos());
            QPointF delta = curPos-lastPos;

            parent->setPos(parent->pos() + delta);
        }

    }
}

void Applet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setFocus(Qt::MouseFocusReason);
    QGraphicsWidget::mousePressEvent(event);
}

void Applet::focusInEvent(QFocusEvent * event)
{
    kDebug() << "FOCUS! FOCUS! FOCUS!";
    QGraphicsWidget::focusInEvent(event);
}

void Applet::showConfigurationInterface()
{
    if (!hasConfigurationInterface()) {
        return;
    }

    const QString dialogId = QString("%1settings%2").arg(id()).arg(name());
    KConfigDialog * dlg = KConfigDialog::exists(dialogId);

    if (dlg) {
        KWindowSystem::setOnDesktop(dlg->winId(), KWindowSystem::currentDesktop());
        dlg->show();
        KWindowSystem::activateWindow(dlg->winId());
        return;
    }

    const QString windowTitle = i18nc("@title:window", "%1 Settings", name());
    if (d->package && d->configXml) {
        QString uiFile = d->package->filePath("mainconfigui");
        if (uiFile.isEmpty()) {
            return;
        }

        KConfigDialog *dialog = new KConfigDialog(0, dialogId, d->configXml);
        dialog->setWindowTitle(windowTitle);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);

        QUiLoader loader;
        QFile f(uiFile);
        if (!f.open(QIODevice::ReadOnly)) {
            delete dialog;
            return;
        }

        QWidget *w = loader.load(&f);
        f.close();

        dialog->addPage(w, i18n("Settings"), icon(), i18n("%1 Settings", name()));
        dialog->show();
    } else if (d->script) {
        d->script->showConfigurationInterface();
    } else {
        KConfigSkeleton *nullManager = new KConfigSkeleton(0);
        KConfigDialog *dialog = new KConfigDialog(0, dialogId, nullManager);
        dialog->setFaceType(KPageDialog::Auto);
        dialog->setWindowTitle(windowTitle);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        createConfigurationInterface(dialog);
        //TODO: would be nice to not show dialog if there are no pages added?
        connect(dialog, SIGNAL(finished()), nullManager, SLOT(deleteLater()));
        dialog->show();
        dialog->enableButtonApply(true);
    }

    emit releaseVisualFocus();
}

void Applet::createConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
    // virtual method reimplemented by subclasses.
    // do not put anything here ...
}

KPluginInfo::List Applet::listAppletInfo(const QString &category,
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

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    //kDebug() << "Applet::listAppletInfo constraint was '" << constraint << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Applet::listAppletInfoForMimetype(const QString &mimetype)
{
    QString constraint = QString("'%1' in MimeTypes").arg(mimetype);
    //kDebug() << "listAppletInfoForMimetype with" << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    return KPluginInfo::fromServices(offers);
}

QStringList Applet::listCategories(const QString &parentApp, bool visibleOnly)
{
    QString constraint = "exist [X-KDE-PluginInfo-Category]";

    if (parentApp.isEmpty()) {
        constraint.append(" and not exist [X-KDE-ParentApp]");
    } else {
        constraint.append(" and [X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    QStringList categories;
    foreach (const KService::Ptr &applet, offers) {
        QString appletCategory = applet->property("X-KDE-PluginInfo-Category").toString();
        if (visibleOnly && applet->noDisplay()) {
            // we don't want to show the hidden category
            continue;
        }

        //kDebug() << "   and we have " << appletCategory;
        if (appletCategory.isEmpty()) {
            if (!categories.contains(i18n("Miscellaneous"))) {
                categories << i18n("Miscellaneous");
            }
        } else  if (!categories.contains(appletCategory)) {
            categories << appletCategory;
        }
    }

    categories.sort();
    return categories;
}

Applet* Applet::load(const QString& appletName, uint appletId, const QVariantList& args)
{
    if (appletName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    bool isContainment = false;
    if (offers.isEmpty()) {
        //TODO: what would be -really- cool is offer to try and download the applet
        //      from the network at this point
        offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
        isContainment = true;
        if (offers.isEmpty()) {
            kDebug() << "offers is empty for " << appletName;
            return 0;
        }
    } /* else if (offers.count() > 1) {
        kDebug() << "hey! we got more than one! let's blindly take the first one";
    } */

    KService::Ptr offer = offers.first();

    if (appletId == 0) {
        appletId = ++Private::s_maxAppletId;
    }

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
        kDebug() << "we have a script using the" << offer->property("X-Plasma-API").toString() << "API";
        if (isContainment) {
            return new Containment(0, offer->storageId(), appletId);
        }
        return new Applet(0, offer->storageId(), appletId);
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << appletId << args;
    QString error;
    Applet* applet = offer->createInstance<Plasma::Applet>(0, allArgs, &error);

    if (!applet) {
        kDebug() << "Couldn't load applet \"" << appletName << "\"! reason given: " << error;
    }

    return applet;
}

Applet* Applet::load(const KPluginInfo& info, uint appletId, const QVariantList& args)
{
    if (!info.isValid()) {
        return 0;
    }

    return load(info.pluginName(), appletId, args);
}

QVariant Applet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionChange:
        if (d->shadow) {
            d->shadow->adjustPosition();
        }
        break;
    case ItemSceneHasChanged: {
        QGraphicsScene *newScene = qvariant_cast<QGraphicsScene*>(value);
        if (newScene) {
            d->checkImmutability();
        }

        if (d->shadow) {
            if (d->shadow->scene()) {
                d->shadow->scene()->removeItem(d->shadow);
            }

            if (newScene) {
                newScene->addItem(d->shadow);
                d->shadow->generate();
                d->shadow->adjustPosition();
                d->shadow->show();
            }
        }
    }
        break;
    case ItemVisibleChange:
        if (d->shadow) {
            d->shadow->setVisible(isVisible());
        }
        break;
    default:
        break;
    };

    return QGraphicsWidget::itemChange(change, value);
}

QSizeF Applet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);

    if (d->aspectRatioMode == Plasma::Square) {
        if (formFactor() == Horizontal) {
            hint.setWidth(hint.height());
        } else {
            hint.setHeight(hint.width());
        }
    }

    return hint;
}

void Applet::setGeometry(const QRectF& geometry)
{
    QRectF beforeGeom = QGraphicsWidget::geometry();
    QGraphicsWidget::setGeometry(geometry);
    if (geometry.size() != beforeGeom.size()) {
        updateConstraints(Plasma::SizeConstraint);
        if (d->background) {
            d->background->resizePanel(boundingRect().size());
        }
        emit geometryChanged();
    } else  if (geometry.topLeft() != beforeGeom.topLeft()) {
        /*if (d->background) {
            kDebug() << QGraphicsWidget::geometry();
        }*/
        emit geometryChanged();
    }
}

void Applet::raise()
{
    setZValue(++Private::s_maxZValue);
}

void Applet::lower()
{
    setZValue(--Private::s_minZValue);
}

void Applet::setIsContainment(bool isContainment)
{
    if (d->isContainment == isContainment) {
        return;
    }

    d->isContainment = isContainment;

    Containment *c = qobject_cast<Containment*>(this);
    if (c) {
        // set up the toolbox
        c->setContainmentType(c->containmentType());
    }
}

bool Applet::isContainment() const
{
    return d->isContainment;
}


// PRIVATE CLASS IMPLEMENTATION

Applet::Private::Private(KService::Ptr service, int uniqueID, Applet *applet)
        : appletId(uniqueID),
          q(applet),
          backgroundHints(StandardBackground),
          appletDescription(service),
          package(0),
          needsConfigOverlay(0),
          background(0),
          script(0),
          configXml(0),
          shadow(0),
          cachedBackground(0),
          mainConfig(0),
          pendingConstraints(NoConstraint),
          aspectRatioMode(Plasma::KeepAspectRatio),
          immutability(NotImmutable),
          hasConfigurationInterface(false),
          failed(false),
          isContainment(false),
          square(false),
          transient(false)
{
    if (appletId == 0) {
        appletId = ++s_maxAppletId;
    } else if (appletId > s_maxAppletId) {
        s_maxAppletId = appletId;
    }
}

Applet::Private::~Private()
{
    foreach ( const QString& engine, loadedEngines ) {
        DataEngineManager::self()->unloadEngine( engine );
    }
    delete background;
    delete package;
    delete configXml;
    delete shadow;
    delete cachedBackground;
    delete mainConfig;
}

void Applet::Private::init()
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    q->setAcceptsHoverEvents(true);
    q->setFlag(QGraphicsItem::ItemIsFocusable, true);

    if (!appletDescription.isValid()) {
        q->setFailedToLaunch(true, i18n("Invalid applet description"));
        return;
    }

    QString api = appletDescription.property("X-Plasma-API").toString();

    // we have a scripted plasmoid
    if (!api.isEmpty()) {
        // find where the Package is
        QString path = KStandardDirs::locate("data",
                                                "plasma/plasmoids/" + appletDescription.pluginName() +
                                                "/");

        if (path.isEmpty()) {
            q->setFailedToLaunch(true, i18n("Could not locate the %1 package required for the %2 widget.",
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
                    q->setFailedToLaunch(true, i18n("Could not create a %1 ScriptEngine for the %2 widget.",
                                                    api, appletDescription.name()));
                }
            } else {
                q->setFailedToLaunch(true, i18n("Could not open the %1 package required for the %2 widget.",
                                                        appletDescription.pluginName(), appletDescription.name()));
                delete package;
                package = 0;
            }

            if (package) {
                setupScriptSupport();
            }
        }
    }

    q->setBackgroundHints(DefaultBackground);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), q, SLOT(themeChanged()));
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engin
void Applet::Private::setupScriptSupport()
{
    Q_ASSERT(package);
    QString xmlPath = package->filePath("mainconfigxml");
    if (!xmlPath.isEmpty()) {
        QFile file(xmlPath);
        // FIXME: KConfigSkeleton doesn't play well with KConfigGroup =/
        KConfigGroup config = q->config();
        configXml = new ConfigXml(&config, &file);
    }

    if (!package->filePath("mainconfigui").isEmpty()) {
        q->setHasConfigurationInterface(true);
    }
}

QString Applet::Private::globalName() const
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.service()->library();
}

QString Applet::Private::instanceName()
{
    if (!appletDescription.isValid()) {
        return QString();
    }

    return appletDescription.service()->library() + QString::number(appletId);
}

void Applet::Private::scheduleConstraintsUpdate(Plasma::Constraints c)
{
    if (pendingConstraints == NoConstraint) {
        QTimer::singleShot(0, q, SLOT(flushPendingConstraintsEvents()));
    }
    pendingConstraints |= c;
}

KConfigGroup* Applet::Private::mainConfigGroup()
{
    if (mainConfig) {
        return mainConfig;
    }

    if (isContainment) {
        const Containment *asContainment = qobject_cast<Containment*>(const_cast<Applet*>(q));
        Q_ASSERT(asContainment);

        KConfigGroup containmentConfig;
        //kDebug() << "got a corona, baby?" << (QObject*)asContainment->corona();
        if (asContainment->corona()) {
            containmentConfig = KConfigGroup(asContainment->corona()->config(), "Containments");
        } else {
            containmentConfig =  KConfigGroup(KGlobal::config(), "Containments");
        }

        mainConfig = new KConfigGroup(&containmentConfig, QString::number(appletId));
    } else {
        KConfigGroup appletConfig;
        if (q->containment()) {
            appletConfig = q->containment()->config();
            appletConfig = KConfigGroup(&appletConfig, "Applets");
        } else {
            kWarning() << "requesting config for" << q->name() << "without a containment!";
            appletConfig = KConfigGroup(KGlobal::config(), "Applets");
        }

        mainConfig = new KConfigGroup(&appletConfig, QString::number(appletId));
    }

    return mainConfig;
}

void Applet::Private::copyEntries(KConfigGroup *source, KConfigGroup *destination)
{
    foreach (const QString &group, source->groupList()) {
        KConfigGroup subSource(source, group);
        KConfigGroup subDest(destination, group);
        copyEntries(&subSource, &subDest);
    }

    QMap<QString, QString> entries = source->entryMap();
    QMapIterator<QString, QString> it(entries);
    while (it.hasNext()) {
        it.next();
        destination->writeEntry(it.key(), it.value());
    }
}

QString Applet::Private::visibleFailureText(const QString& reason)
{
    QString text;

    if (reason.isEmpty()) {
        text = i18n("This object could not be created.");
    } else {
        text = i18n("This object could not be created for the following reason:<p><b>%1</b></p>", reason);
    }

    return text;
}

void Applet::Private::checkImmutability()
{
    const bool systemImmutable = q->globalConfig().isImmutable() || q->config().isImmutable() ||
                                ((!isContainment && q->containment()) &&
                                    q->containment()->immutability() == SystemImmutable) ||
                                (dynamic_cast<Corona*>(q->scene()) && static_cast<Corona*>(q->scene())->immutability() == SystemImmutable);

    if (systemImmutable) {
        q->updateConstraints(ImmutableConstraint);
    }
}

void Applet::Private::themeChanged()
{
    q->update();
}

void Applet::Private::resetConfigurationObject()
{
    mainConfigGroup()->deleteGroup();
    delete mainConfig;
    mainConfig = 0;
}

uint Applet::Private::s_maxAppletId = 0;
uint Applet::Private::s_maxZValue = 0;
uint Applet::Private::s_minZValue = 0;
PackageStructure::Ptr Applet::Private::packageStructure(0);

AppletOverlayWidget::AppletOverlayWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
{
    resize(parent->size());
}

void AppletOverlayWidget::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QColor wash = Plasma::Theme::defaultTheme()->color(Theme::BackgroundColor);
    wash.setAlphaF(.6);
    painter->fillPath(parentItem()->shape(), wash);
    painter->restore();
}

} // Plasma namespace

#include "applet.moc"
