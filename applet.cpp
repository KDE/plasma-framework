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

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QGraphicsGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QLabel>
#include <QList>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QSize>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTimer>
#include <QUiLoader>

#include <KAction>
#include <KIcon>
#include <KColorScheme>
#include <KConfigDialog>
#include <KDialog>
#include <KIconLoader>
#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>
#include <KServiceTypeTrader>
#include <KShortcut>
#include <KWindowSystem>
#include <KActionCollection>

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
#include "plasma/toolbox_p.h"
#include "plasma/view.h"
#include "plasma/widgets/label.h"
#include "plasma/widgets/pushbutton.h"
#include "plasma/tooltipmanager.h"

//#define DYNAMIC_SHADOWS
namespace Plasma
{

Applet::Applet(QGraphicsItem *parent,
               const QString& serviceID,
               uint appletId)
    :  QGraphicsWidget(parent),
       d(new AppletPrivate(KService::serviceByStorageId(serviceID), appletId, this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init();
}

Applet::Applet(QObject* parentObject, const QVariantList& args)
    :  QGraphicsWidget(0),
       d(new AppletPrivate(KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString()),
                     args.count() > 1 ? args[1].toInt() : 0, this))
{
    // now remove those first two items since those are managed by Applet and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList&>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();

        if (!mutableArgs.isEmpty()) {
            mutableArgs.removeFirst();
        }
    }

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
    if (!AppletPrivate::packageStructure) {
        AppletPrivate::packageStructure = new PlasmoidPackage();
    }

    return AppletPrivate::packageStructure;
}

void Applet::init()
{
    if (d->script && !d->script->init()) {
        setFailedToLaunch(true, i18n("Script initialization failed"));
    }
}

uint Applet::id() const
{
    return d->appletId;
}

void Applet::save(KConfigGroup &g) const
{
    KConfigGroup group = g;
    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group.writeEntry("immutability", (int)d->immutability);
    group.writeEntry("plugin", pluginName());
    //FIXME: for containments, we need to have some special values here w/regards to
    //       screen affinity (e.g. "bottom of screen 0")
    //kDebug() << pluginName() << "geometry is" << geometry() << "pos is" << pos() << "bounding rect is" << boundingRect();
    group.writeEntry("geometry", geometry());
    group.writeEntry("zvalue", zValue());

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
    //FIXME: we need a global save state too
    saveState(appletConfigGroup);

    if (d->activationAction) {
        KConfigGroup shortcutConfig(&group, "Shortcuts");
        shortcutConfig.writeEntry("global", d->activationAction->globalShortcut().toString());
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
    QString shortcutText = shortcutConfig.readEntry("global", QString());
    if (!shortcutText.isEmpty()) {
        setGlobalShortcut(KShortcut(shortcutText));
    }

    // local shortcut, if any
    //TODO: implement; the shortcut will need to be registered with the containment
    /*
    shortcutText = shortcutConfig.readEntry("local", QString());
    if (!shortcutText.isEmpty()) {
        //TODO: implement; the shortcut 
    }
    */
}

void AppletPrivate::setFocus()
{
    kDebug() << "setting focus";
    q->setFocus(Qt::ShortcutFocusReason);
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
        Label *failureWidget = new Plasma::Label(this);
        failureWidget->setText(d->visibleFailureText(reason));
        QLabel *label = failureWidget->nativeWidget();
        label->setWordWrap(true);
        failureLayout->addItem(failureWidget);
        setLayout(failureLayout);
        resize(300,250);
        setMinimumSize(failureWidget->size());
        //resize(label->size());
        d->background->resizePanel(geometry().size());
    }
    update();
}

void Applet::saveState(KConfigGroup &group) const
{
    if (group.config()->name() != config().config()->name()) {
        // we're being saved to a different file!
        // let's just copy the current values in our configuration over
        KConfigGroup c = config();
        c.copyTo(&group);
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
    if (immutability() != Mutable || d->transient) {
        return; //don't double delete
    }

    d->transient = true;

    if (isContainment()) {
        d->cleanUpAndDelete();
    } else {
        connect(Animator::self(), SIGNAL(animationFinished(QGraphicsItem*,Plasma::Animator::Animation)),
                this, SLOT(appletAnimationComplete(QGraphicsItem*,Plasma::Animator::Animation)));
        Animator::self()->animateItem(this, Animator::DisappearAnimation);
    }
}

void AppletPrivate::appletAnimationComplete(QGraphicsItem *item, Plasma::Animator::Animation anim)
{
    if (anim != Animator::DisappearAnimation || item != q) {
        return; //it's not our time yet
    }

    cleanUpAndDelete();
}

void AppletPrivate::selectItemToDestroy()
{
    //FIXME: this will not work nicely with multiple screens and being zoomed out!
    if (q->isContainment() &&
        q->view() && q->view()->transform().isScaling() &&
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

    q->destroy();
}

void AppletPrivate::cleanUpAndDelete()
{
    //kDebug() << "???????????????? DESTROYING APPLET" << name() << " ???????????????????????????";
    QGraphicsWidget *parent = dynamic_cast<QGraphicsWidget *>(q->parentItem());
    //it probably won't matter, but right now if there are applethandles, *they* are the parent.
    //not the containment.

    //is the applet in a containment and is the containment have a layout? if yes, we remove the applet in the layout
    if (parent && parent->layout()) {
        QGraphicsLayout *l = parent->layout();
        for (int i = 0; i < l->count(); ++i) {
            if (q == l->itemAt(i)) {
                l->removeAt(i);
                break;
            }
        }
    }

    if (configXml) {
        configXml->setDefaults();
    }

    q->scene()->removeItem(q);
    q->deleteLater();
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

    QGraphicsView *found = 0;
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            if (!found || view->isActiveWindow()) {
                found = view;
            }
        }
    }
    return found;
}

QRectF Applet::mapFromView(const QGraphicsView *view, const QRect &rect) const
{
    // Why is this adjustment needed? Qt calculation error?
    return mapFromScene(view->mapToScene(rect)).boundingRect().adjusted(0, 0, 1, 1);;
}

QRect Applet::mapToView(const QGraphicsView *view, const QRectF &rect) const
{
    // Why is this adjustment needed? Qt calculation error?
    return view->mapFromScene(mapToScene(rect)).boundingRect().adjusted(0, 0, -1, -1);;
}

QPoint Applet::popupPosition(const QSize &s) const
{
    return Plasma::popupPosition(this, s);
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
    ImmutabilityType coronaImmutability = Mutable;

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
    if ((hints & StandardBackground) || (hints & TranslucentBackground)) {
        if (!d->background) {
            d->background = new Plasma::PanelSvg();
        }

        if ((hints & TranslucentBackground) && Plasma::Theme::defaultTheme()->currentThemeHasImage("widgets/translucentbackground")) {
            d->background->setImagePath("widgets/translucentbackground");
        } else {
            d->background->setImagePath("widgets/background");
        }

        d->background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
        qreal left, top, right, bottom;
        d->background->getMargins(left, top, right, bottom);
        setContentsMargins(left, right, top, bottom);
        QSizeF fitSize(left + right, top + bottom);
        if (minimumSize().expandedTo(fitSize) != minimumSize()) {
            setMinimumSize(minimumSize().expandedTo(fitSize));
        }
        d->background->resizePanel(boundingRect().size());
    } else if (d->background) {
        qreal left, top, right, bottom;
        d->background->getMargins(left, top, right, bottom);
        //Setting a minimum size of 0,0 would result in the panel to be only
        //on the first virtual desktop
        setMinimumSize(qMax(minimumSize().width() - left - right, qreal(1.0)),
                       qMax(minimumSize().height() - top - bottom, qreal(1.0)));

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
            d->shadow = new ShadowItem(this);
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

void Applet::paintWindowFrame(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    //Here come the code for the window frame
    //kDebug() << windowFrameGeometry();
    //painter->drawRoundedRect(windowFrameGeometry(), 5, 5);
}

bool Applet::configurationRequired() const
{
    return d->needsConfigOverlay != 0;
}

void Applet::setConfigurationRequired(bool needsConfig, const QString &reason)
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
    d->needsConfigOverlay->resize(contentsRect().size());
    d->needsConfigOverlay->setPos(contentsRect().topLeft());

    int zValue = 100;
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        if (child->zValue() > zValue) {
            zValue = child->zValue() + 1;
        }
    }
    d->needsConfigOverlay->setZValue(zValue);

    qDeleteAll(d->needsConfigOverlay->QGraphicsItem::children());
    QGraphicsGridLayout *configLayout = new QGraphicsGridLayout(d->needsConfigOverlay);
    configLayout->setContentsMargins(0, 0, 0, 0);

  //  configLayout->addStretch();
    configLayout->setColumnStretchFactor(0, 10);
    configLayout->setColumnStretchFactor(2, 10);
    configLayout->setRowStretchFactor(0, 10);
    configLayout->setRowStretchFactor(3, 10);

    int row = 1;
    if (!reason.isEmpty()) {
        Label *explanation = new Label(d->needsConfigOverlay);
        explanation->setText(reason);
        configLayout->addItem(explanation, row, 1);
        configLayout->setColumnStretchFactor(1, 10);
        ++row;
        //configLayout->setAlignment(explanation, Qt::AlignBottom | Qt::AlignCenter);
    }

    PushButton *configWidget = new PushButton(d->needsConfigOverlay);
    configWidget->setText(i18n("Configure..."));
    connect(configWidget, SIGNAL(clicked()), this, SLOT(showConfigurationInterface()));
    configLayout->addItem(configWidget, row, 1);
    //configLayout->setAlignment(configWidget, Qt::AlignTop | Qt::AlignCenter);
    //configLayout->addStretch();

    d->needsConfigOverlay->show();
}

void Applet::flushPendingConstraintsEvents()
{
    if (d->pendingConstraints == NoConstraint) {
        return;
    }

    if (d->constraintsTimerId) {
        killTimer(d->constraintsTimerId);
        d->constraintsTimerId = 0;
    }

    //kDebug() << "fushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Plasma::Constraints c = d->pendingConstraints;
    d->pendingConstraints = NoConstraint;

    if (c & Plasma::StartupCompletedConstraint) {
        //common actions
        bool unlocked = immutability() == Mutable;
        //FIXME desktop containments can't be removed while in use.
        //it's kinda silly to have a keyboard shortcut for something that can only be used when the
        //shortcut isn't active.
        QAction* closeApplet = new QAction(this);
        closeApplet->setIcon(KIcon("edit-delete"));
        closeApplet->setEnabled(unlocked);
        closeApplet->setVisible(unlocked);
        closeApplet->setShortcutContext(Qt::WidgetWithChildrenShortcut); //don't clash with other views
        if (isContainment()) {
            closeApplet->setText(i18n("Remove this %1 Activity", name()));
            closeApplet->setShortcut(QKeySequence("ctrl+shift+r"));
        } else {
            closeApplet->setText(i18n("Remove this %1", name()));
            closeApplet->setShortcut(QKeySequence("ctrl+r"));
        }
        connect(closeApplet, SIGNAL(triggered(bool)), this, SLOT(selectItemToDestroy()));
        d->actions.addAction("remove", closeApplet);
    }

    if (c & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Mutable;
        QAction *action = d->actions.action("remove");
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }
    }

    if (c & Plasma::SizeConstraint && d->needsConfigOverlay) {
        d->needsConfigOverlay->setGeometry(QRectF(QPointF(0, 0), contentsRect().size()));
        QGraphicsItem *button = 0;
        QList<QGraphicsItem*> children = d->needsConfigOverlay->QGraphicsItem::children();

        if (!children.isEmpty()) {
            button = children.first();
        }

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
        } else if(d->backgroundHints&TranslucentBackground) {
            setBackgroundHints(d->backgroundHints^TranslucentBackground);
        }
    }

    //enforce square size in panels
    if ((c & Plasma::SizeConstraint || c & Plasma::FormFactorConstraint) &&
        aspectRatioMode() == Plasma::Square) {
        if (formFactor() == Horizontal) {
            setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding));
        } else if (formFactor() == Vertical) {
            setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
        }

        updateGeometry();
    }

    //enforce a constrained square size in panels
    if ((c & Plasma::SizeConstraint || c & Plasma::FormFactorConstraint) &&
        aspectRatioMode() == Plasma::ConstrainedSquare) {
        if (formFactor() == Horizontal) {
            setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding));
        } else if (formFactor() == Vertical) {
            setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
        }

        updateGeometry();
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

QAction* Applet::action(QString name) const
{
    return d->actions.action(name);
}

void Applet::addAction(QString name, QAction *action)
{
    d->actions.addAction(name, action);
}

void Applet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainter *p;
    //FIXME: we should probably set the pixmap to screenSize(), but that breaks stuff atm.
    QPixmap pixmap(boundingRect().size().toSize());

    QGraphicsView* qgv = qobject_cast<QGraphicsView*>(widget ? widget->parent() : 0);
    bool ghost = (qgv && (qgv == d->ghostView));

    if (ghost) {
        // The applet has to be displayed semi transparent. Create a pixmap and a painter on
        // that pixmap where the applet can draw on so we can draw the result transparently
        // at the end.
        kDebug() << "Painting ghosted...";

        pixmap.fill(Qt::transparent);

        p = new QPainter();
        p->begin(&pixmap);
    } else {
        p = painter;
    }

    if (d->shadow && d->shadow->shadowedSize() != boundingRect().size()) {
        //kDebug() << "sizes are " << d->shadow->shadowedSize() << boundingRect().size();
        d->shadow->generate();
    }

    p->save();

    if (transform().isRotating()) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);
    }

    if (d->background &&
        formFactor() != Plasma::Vertical &&
        formFactor() != Plasma::Horizontal) {
        //kDebug() << "option rect is" << option->rect;
        d->background->paintPanel(p, option->rect, QPointF(0,0));
    }

    if (!d->failed) {
        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        const QRect contentsRect = QRectF(QPointF(0,0), boundingRect().size())
                        .adjusted(left, top, -right, -bottom).toAlignedRect();
        if (widget && isContainment()) {
            // note that the widget we get is actually the viewport of the view, not the view itself
            View* v = qobject_cast<Plasma::View*>(widget->parent());
            if (!v || v->isWallpaperEnabled()) {
                Containment::StyleOption coption(*option);
                coption.view = v;

                paintInterface(p, &coption, contentsRect);
            }

            p->restore();
            return;
        }

        //kDebug() << "paint interface of" << (QObject*) this;
        paintInterface(p, option, contentsRect);
    }
    p->restore();

    if (ghost) {
        // Lets display the pixmap that we've just drawn... transparently.
        p->setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p->fillRect(pixmap.rect(), QColor(0, 0, 0, (0.3 * 255)));
        p->end();

        delete p;

        painter->drawPixmap(0, 0, pixmap);
    }
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

void Applet::setGlobalShortcut(const KShortcut &shortcut)
{
    if (!d->activationAction) {
        d->activationAction = new KAction(this);
        d->activationAction->setText(i18n("Activate %1 Widget", name()));
        d->activationAction->setObjectName(QString("activate widget %1").arg(name())); // NO I18N
        connect(d->activationAction, SIGNAL(triggered()), this, SIGNAL(activate()));
        connect(this, SIGNAL(activate()), this, SLOT(setFocus()));

        QList<QWidget *> widgets = d->actions.associatedWidgets();
        foreach (QWidget *w, widgets) {
            w->addAction(d->activationAction);
        }
    }

    d->activationAction->setGlobalShortcut(shortcut);
}

KShortcut Applet::globalShortcut() const
{
    if (d->activationAction) {
        return d->activationAction->globalShortcut();
    }

    return KShortcut();
}

void Applet::addAssociatedWidget(QWidget *widget)
{
    d->actions.addAssociatedWidget(widget);
}

void Applet::removeAssociatedWidget(QWidget *widget)
{
    d->actions.removeAssociatedWidget(widget);
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

void Applet::unregisterAsDragHandle(QGraphicsItem *item)
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
    if (d->hasConfigurationInterface == hasInterface) {
        return;
    }
    d->hasConfigurationInterface = hasInterface;
    //config action
    //TODO respect security when it's implemented (4.2)
    QAction *configAction = d->actions.action("configure");
    if (hasInterface) {
        if (! configAction) { //should be always true
            configAction = new QAction(i18n("%1 Settings", name()), this);
            configAction->setIcon(KIcon("configure"));
            configAction->setShortcutContext(Qt::WidgetWithChildrenShortcut); //don't clash with other views
            if (isContainment()) {
                //kDebug() << "I am a containment";
                configAction->setShortcut(QKeySequence("ctrl+shift+s"));
            } else {
                configAction->setShortcut(QKeySequence("ctrl+s"));
            }
            //TODO how can we handle configuration of the shortcut in a way that spans all applets?
            connect(configAction, SIGNAL(triggered(bool)),
                    this, SLOT(showConfigurationInterface()));
            d->actions.addAction("configure", configAction);
        }
    } else {
        d->actions.removeAction(configAction);
    }
}

bool Applet::eventFilter( QObject *o, QEvent * e )
{
    return QObject::eventFilter(o, e);
}

bool Applet::sceneEventFilter( QGraphicsItem * watched, QEvent * event )
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMouseMove: {
            //don't move when the containment is not mutable, in the rare case the containment doesn't exists consider it as mutable
            if ((!containment() || containment()->immutability() == Mutable) && d->registeredAsDragHandle.contains( watched )) {
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
    if (immutability() == Mutable && formFactor() == Plasma::Planar) {
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
        d->background->resizePanel(boundingRect().size());
    }

    updateConstraints(Plasma::SizeConstraint);
    emit geometryChanged();
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

            if (d->script) {
                d->script->showConfigurationInterface();
            }
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
        //TODO: Apply button does not correctly work for now, so do not show it
        dialog->showButton( KDialog::Apply, false );
        dialog->show();
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
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
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
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
        kDebug() << "we have a script using the" << offer->property("X-Plasma-API").toString() << "API";
        if (isContainment) {
            return new Containment(0, offer->storageId(), appletId);
        }
        return new Applet(0, offer->storageId(), appletId);
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
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
    //kDebug() << change;
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
    case ItemPositionHasChanged:
        emit geometryChanged();
        break;
    default:
        break;
    };

    return QGraphicsWidget::itemChange(change, value);
}

QPainterPath Applet::shape() const
{
    if (d->script) {
        return d->script->shape();
    }

    return QGraphicsWidget::shape();
}

QSizeF Applet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
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
        if (formFactor() == Horizontal && (which == Qt::MaximumSize || size().height() <= KIconLoader::SizeLarge)) {
            hint.setWidth(size().height());
        //enforce a size not taller than wide
        } else if (formFactor() == Vertical && (which == Qt::MaximumSize || size().width() <= KIconLoader::SizeLarge)) {
            hint.setHeight(size().width());
        }
    }

    return hint;
}

void Applet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->constraintsTimerId) {
        killTimer(d->constraintsTimerId);
        d->constraintsTimerId = 0;
        flushPendingConstraintsEvents();
    }
}

QRect Applet::screenRect() const
{
    QPointF bottomRight = pos();
    bottomRight.setX(bottomRight.x() + size().width());
    bottomRight.setY(bottomRight.y() + size().height());

    Containment *c;
    c = containment();

    if (c) {
        QGraphicsView *v;
        v = c->view();

        if (v) {
            QPoint tL = v->mapToGlobal(v->mapFromScene(pos()));
            QPoint bR = v->mapToGlobal(v->mapFromScene(bottomRight));

            kDebug() << "screenRect = " << QPoint(tL.x(), tL.y()), QSize(bR.x() - tL.x(), bR.y() - tL.y());
            return QRect(QPoint(tL.x(), tL.y()), QSize(bR.x() - tL.x(), bR.y() - tL.y()));
        }
    }

    //The applet isn't in any containment, or in a containment that doesn't have a view on it.
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

void Applet::setIsContainment(bool isContainment)
{
    if (d->isContainment == isContainment) {
        return;
    }

    d->isContainment = isContainment;

    Containment *c = qobject_cast<Containment*>(this);
    if (c) {
        if (isContainment) {
            // set up the toolbox
            c->d->createToolBox();
        } else {
            delete c->d->toolBox;
            c->d->toolBox = 0;
        }
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
          backgroundHints(Applet::StandardBackground),
          appletDescription(service),
          package(0),
          needsConfigOverlay(0),
          background(0),
          script(0),
          configXml(0),
          shadow(0),
          mainConfig(0),
          pendingConstraints(NoConstraint),
          aspectRatioMode(Plasma::KeepAspectRatio),
          ghostView(0),
          immutability(Mutable),
          actions(applet),
          activationAction(0),
          constraintsTimerId(0),
          hasConfigurationInterface(false),
          failed(false),
          isContainment(false),
          transient(false)
{
    if (appletId == 0) {
        appletId = ++s_maxAppletId;
    } else if (appletId > s_maxAppletId) {
        s_maxAppletId = appletId;
    }
}

AppletPrivate::~AppletPrivate()
{
    foreach ( const QString& engine, loadedEngines ) {
        DataEngineManager::self()->unloadEngine( engine );
    }
    delete background;
    delete package;
    delete configXml;
    delete shadow;
    delete mainConfig;
}

void AppletPrivate::init()
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    q->setCacheMode(Applet::DeviceCoordinateCache);
    q->setAcceptsHoverEvents(true);
    q->setFlag(QGraphicsItem::ItemIsFocusable, true);
    // FIXME: adding here because nothing seems to be doing it in QGraphicsView,
    // but it doesn't actually work anyways =/
    q->setLayoutDirection(qApp->layoutDirection());

    if (!appletDescription.isValid()) {
        kDebug() << "Check your constructor! You probably want to be passing in a Service::Ptr or a QVariantList with a valid storageid as arg[0].";
        return;
    }

    QString api = appletDescription.property("X-Plasma-API").toString();

    // we have a scripted plasmoid
    if (!api.isEmpty()) {
        // find where the Package is
        QString path = KStandardDirs::locate("data",
                                             "plasma/plasmoids/" + appletDescription.pluginName() + "/");

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

    q->setBackgroundHints(Applet::DefaultBackground);

    QObject::connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), q, SLOT(themeChanged()));
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engin
void AppletPrivate::setupScriptSupport()
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
    if (!constraintsTimerId) {
        constraintsTimerId = q->startTimer(0);
    }
    pendingConstraints |= c;
}

KConfigGroup* AppletPrivate::mainConfigGroup()
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

QString AppletPrivate::visibleFailureText(const QString& reason)
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
                                (dynamic_cast<Corona*>(q->scene()) && static_cast<Corona*>(q->scene())->immutability() == SystemImmutable);

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
    mainConfigGroup()->deleteGroup();
    delete mainConfig;
    mainConfig = 0;
}

uint AppletPrivate::s_maxAppletId = 0;
uint AppletPrivate::s_maxZValue = 0;
uint AppletPrivate::s_minZValue = 0;
PackageStructure::Ptr AppletPrivate::packageStructure(0);

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
