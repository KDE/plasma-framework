/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletquickitem.h"
#include "applet.h"
#include "appletcontext_p.h"
#include "configview.h"
#include "containment.h"
#include "debug_p.h"
#include "plasmoid/appletinterface.h"
#include "plasmoid/containmentinterface.h"
#include "plasmoid/wallpaperinterface.h"
#include "private/appletquickitem_p.h"
#include "private/plasmoidattached_p.h"
#include "sharedqmlengine.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickWindow>
#include <QRandomGenerator>

#include <QDebug>

#include <KLocalizedString>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include <packageurlinterceptor.h>
#include <qloggingcategory.h>
#include <qstringliteral.h>

namespace PlasmaQuick
{

QHash<Plasma::Applet *, AppletQuickItem *> AppletQuickItemPrivate::s_itemsForApplet = QHash<Plasma::Applet *, AppletQuickItem *>();
AppletQuickItemPrivate::PreloadPolicy AppletQuickItemPrivate::s_preloadPolicy = AppletQuickItemPrivate::Uninitialized;

AppletQuickItemPrivate::AppletQuickItemPrivate(AppletQuickItem *item)
    : q(item)
    , switchWidth(-1)
    , switchHeight(-1)
    , initComplete(false)
    , compactRepresentationCheckGuard(false)
{
    if (s_preloadPolicy == Uninitialized) {
        // default as Adaptive
        s_preloadPolicy = Adaptive;

        if (qEnvironmentVariableIsSet("PLASMA_PRELOAD_POLICY")) {
            const QString policy = QString::fromUtf8(qgetenv("PLASMA_PRELOAD_POLICY")).toLower();
            if (policy == QLatin1String("aggressive")) {
                s_preloadPolicy = Aggressive;
            } else if (policy == QLatin1String("none")) {
                s_preloadPolicy = None;
            }
        }

        qCInfo(LOG_PLASMAQUICK) << "Applet preload policy set to" << s_preloadPolicy;
    }
}

int AppletQuickItemPrivate::preloadWeight() const
{
    int defaultWeight;
    const QStringList provides = applet->pluginMetaData().value(QStringLiteral("X-Plasma-Provides"), QStringList());

    // some applet types we want a bigger weight
    if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
        defaultWeight = DefaultLauncherPreloadWeight;
    } else {
        defaultWeight = DefaultPreloadWeight;
    }
    // default widgets to be barely preloaded
    return qBound(0,
                  applet->config().readEntry(QStringLiteral("PreloadWeight"),
                                             qMax(defaultWeight, applet->pluginMetaData().value(QStringLiteral("X-Plasma-PreloadWeight"), 0))),
                  100);
}

QObject *AppletQuickItemPrivate::searchLayoutAttached(QObject *parent)
{
    QObject *layout = nullptr;
    // Search a child that has the needed Layout properties
    // HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    const auto lstChildren = parent->children();
    for (QObject *child : lstChildren) {
        // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        /* clang-format off */
        if (child->property("minimumWidth").isValid()
            && child->property("minimumHeight").isValid()
            && child->property("preferredWidth").isValid()
            && child->property("preferredHeight").isValid()
            && child->property("maximumWidth").isValid()
            && child->property("maximumHeight").isValid()
            && child->property("fillWidth").isValid()
            && child->property("fillHeight").isValid()) { /* clang-format on */
            layout = child;
            break;
        }
    }
    return layout;
}

void AppletQuickItemPrivate::connectLayoutAttached(QObject *item)
{
    // Extract the representation's Layout, if any
    if (!item) {
        return;
    }

    QObject *layout = searchLayoutAttached(item);

    // if the compact repr doesn't export a Layout.* attached property,
    // reset our own with default values
    if (!layout) {
        if (ownLayout) {
            ownLayout->setProperty("minimumWidth", 0);
            ownLayout->setProperty("minimumHeight", 0);
            ownLayout->setProperty("preferredWidth", -1);
            ownLayout->setProperty("preferredHeight", -1);
            ownLayout->setProperty("maximumWidth", std::numeric_limits<qreal>::infinity());
            ownLayout->setProperty("maximumHeight", std::numeric_limits<qreal>::infinity());
            ownLayout->setProperty("fillWidth", false);
            ownLayout->setProperty("fillHeight", false);
        }
        return;
    }

    // propagate all the size hints
    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");

    QObject *newOwnLayout = searchLayoutAttached(q);

    // this should never happen, since we ask to create it if doesn't exists
    if (!newOwnLayout) {
        return;
    }

    // if the representation didn't change, don't do anything
    if (representationLayout == layout) {
        return;
    }

    if (representationLayout) {
        QObject::disconnect(representationLayout, nullptr, q, nullptr);
    }

    // Here we can't use the new connect syntax because we can't link against QtQuick layouts
    QObject::connect(layout, SIGNAL(minimumWidthChanged()), q, SLOT(minimumWidthChanged()));
    QObject::connect(layout, SIGNAL(minimumHeightChanged()), q, SLOT(minimumHeightChanged()));

    QObject::connect(layout, SIGNAL(preferredWidthChanged()), q, SLOT(preferredWidthChanged()));
    QObject::connect(layout, SIGNAL(preferredHeightChanged()), q, SLOT(preferredHeightChanged()));

    QObject::connect(layout, SIGNAL(maximumWidthChanged()), q, SLOT(maximumWidthChanged()));
    QObject::connect(layout, SIGNAL(maximumHeightChanged()), q, SLOT(maximumHeightChanged()));

    QObject::connect(layout, SIGNAL(fillWidthChanged()), q, SLOT(fillWidthChanged()));
    QObject::connect(layout, SIGNAL(fillHeightChanged()), q, SLOT(fillHeightChanged()));

    representationLayout = layout;
    ownLayout = newOwnLayout;

    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");
}

void AppletQuickItemPrivate::propagateSizeHint(const QByteArray &layoutProperty)
{
    if (ownLayout && representationLayout) {
        ownLayout->setProperty(layoutProperty.constData(), representationLayout->property(layoutProperty.constData()));
    }
}

QQuickItem *AppletQuickItemPrivate::createCompactRepresentationItem()
{
    if (!compactRepresentation) {
        return nullptr;
    }

    if (compactRepresentationItem) {
        return compactRepresentationItem;
    }

    QVariantHash initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);
    initialProperties[QStringLiteral("plasmoidItem")] = QVariant::fromValue(q);

    compactRepresentationItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(compactRepresentation, qmlContext(q), initialProperties));

    Q_EMIT q->compactRepresentationItemChanged(compactRepresentationItem);

    return compactRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createFullRepresentationItem()
{
    if (fullRepresentationItem) {
        return fullRepresentationItem;
    }

    if (fullRepresentation && fullRepresentation != qmlObject->mainComponent()) {
        QVariantHash initialProperties;
        initialProperties[QStringLiteral("parent")] = QVariant();
        fullRepresentationItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(fullRepresentation, qmlContext(q), initialProperties));
    }

    if (!fullRepresentationItem) {
        return nullptr;
    }

    Q_EMIT q->fullRepresentationItemChanged(fullRepresentationItem);

    return fullRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createCompactRepresentationExpanderItem()
{
    if (!compactRepresentationExpander) {
        return nullptr;
    }

    if (compactRepresentationExpanderItem) {
        return compactRepresentationExpanderItem;
    }

    compactRepresentationExpanderItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(compactRepresentationExpander, qmlContext(q)));

    if (!compactRepresentationExpanderItem) {
        return nullptr;
    }

    compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject *>(createCompactRepresentationItem()));
    compactRepresentationExpanderItem->setProperty("plasmoidItem", QVariant::fromValue(q));

    return compactRepresentationExpanderItem;
}

bool AppletQuickItemPrivate::appletShouldBeExpanded() const
{
    if (applet->isContainment()) {
        return true;

    } else {
        if (!fullRepresentation) {
            // If a full representation wasn't specified, the onle and only representation of the plasmoid are our
            // direct contents, so we consider it always expanded
            return true;
        }
        if (switchWidth > 0 && switchHeight > 0) {
            return q->width() > switchWidth && q->height() > switchHeight;

            // if a size to switch wasn't set, determine what representation to always chose
        } else {
            // preferred representation set?
            if (preferredRepresentation) {
                return preferredRepresentation == fullRepresentation;
                // Otherwise, base on FormFactor
            } else {
                return (applet->formFactor() != Plasma::Types::Horizontal && applet->formFactor() != Plasma::Types::Vertical);
            }
        }
    }
}

void AppletQuickItemPrivate::preloadForExpansion()
{
    qint64 time = 0;
    if (QLoggingCategory::defaultCategory()->isInfoEnabled()) {
        time = QDateTime::currentMSecsSinceEpoch();
    }

    if (!createFullRepresentationItem()) {
        return;
    }

    // When not already expanded, also preload the expander
    if (!appletShouldBeExpanded() && !applet->isContainment() && (!preferredRepresentation || preferredRepresentation != fullRepresentation)) {
        createCompactRepresentationExpanderItem();
    }

    if (!appletShouldBeExpanded() && compactRepresentationExpanderItem) {
        compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant::fromValue<QObject *>(createFullRepresentationItem()));
    } else if (fullRepresentationItem) {
        fullRepresentationItem->setProperty("parent", QVariant::fromValue<QObject *>(q));
    }

    // preallocate nodes
    if (fullRepresentationItem && fullRepresentationItem->window()) {
        fullRepresentationItem->window()->create();
    }

    qCDebug(LOG_PLASMAQUICK) << "Applet" << applet->title() << "loaded after" << (QDateTime::currentMSecsSinceEpoch() - time) << "msec";
}

void AppletQuickItemPrivate::anchorsFillParent(QQuickItem *item, QQuickItem *parent)
{
    if (item->parentItem() != parent) {
        return;
    }
    // just set once, don't bind
    QQmlProperty::write(item, QStringLiteral("anchors.fill"), QVariant::fromValue<QObject *>(parent));
}

void AppletQuickItemPrivate::compactRepresentationCheck()
{
    if (!initComplete) {
        return;
    }

    // ignore 0 sizes;
    if (q->width() <= 0 || q->height() <= 0) {
        return;
    }

    // ignore if this widget is being checked somewhere above
    if (compactRepresentationCheckGuard) {
        return;
    }

    bool full = appletShouldBeExpanded();

    if ((full && fullRepresentationItem && fullRepresentationItem == currentRepresentationItem)
        || (!full && compactRepresentationItem && compactRepresentationItem == currentRepresentationItem)) {
        return;
    }

    compactRepresentationCheckGuard = true;

    // Expanded
    if (full) {
        QQuickItem *item = createFullRepresentationItem();

        if (item) {
            // unwire with the expander
            if (compactRepresentationExpanderItem) {
                compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant());
                compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant());
                compactRepresentationExpanderItem->setVisible(false);
            }

            // the fullrepresentation being the complete AppletItem is actually allowed when the main ui
            // is child of the root item (like many panel applets)
            if (item != q) {
                item->setParentItem(q);
                anchorsFillParent(item, q);
            }

            if (compactRepresentationItem) {
                compactRepresentationItem->setVisible(false);
            }

            currentRepresentationItem = item;
            connectLayoutAttached(item);

            expanded = true;
            Q_EMIT q->expandedChanged(true);
        }

    } else {
        // Icon
        QQuickItem *compactItem = createCompactRepresentationItem();
        QQuickItem *compactExpanderItem = createCompactRepresentationExpanderItem();

        if (compactItem && compactExpanderItem) {
            // set the root item as the main visible item
            compactItem->setVisible(true);
            compactExpanderItem->setParentItem(q);
            compactExpanderItem->setVisible(true);
            anchorsFillParent(compactExpanderItem, q);

            if (fullRepresentationItem) {
                fullRepresentationItem->setProperty("parent", QVariant());
            }

            compactExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject *>(compactItem));
            // The actual full representation will be connected when created
            compactExpanderItem->setProperty("fullRepresentation", QVariant());

            currentRepresentationItem = compactItem;
            connectLayoutAttached(compactItem);

            expanded = false;
            Q_EMIT q->expandedChanged(false);
        }
    }

    compactRepresentationCheckGuard = false;
}

void AppletQuickItemPrivate::minimumWidthChanged()
{
    propagateSizeHint("minimumWidth");
}

void AppletQuickItemPrivate::minimumHeightChanged()
{
    propagateSizeHint("minimumHeight");
}

void AppletQuickItemPrivate::preferredWidthChanged()
{
    propagateSizeHint("preferredWidth");
}

void AppletQuickItemPrivate::preferredHeightChanged()
{
    propagateSizeHint("preferredHeight");
}

void AppletQuickItemPrivate::maximumWidthChanged()
{
    propagateSizeHint("maximumWidth");
}

void AppletQuickItemPrivate::maximumHeightChanged()
{
    propagateSizeHint("maximumHeight");
}

void AppletQuickItemPrivate::fillWidthChanged()
{
    propagateSizeHint("fillWidth");
}

void AppletQuickItemPrivate::fillHeightChanged()
{
    propagateSizeHint("fillHeight");
}

AppletQuickItem::AppletQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
    , d(new AppletQuickItemPrivate(this))
{
}

AppletQuickItem::~AppletQuickItem()
{
    AppletQuickItemPrivate::s_itemsForApplet.remove(d->applet);
    // decrease weight
    if (d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive) {
        d->applet->config().writeEntry(QStringLiteral("PreloadWeight"), qMax(0, d->preloadWeight() - AppletQuickItemPrivate::PreloadWeightDecrement));
    }

    // Here the order is important
    delete d->compactRepresentationItem;
    delete d->fullRepresentationItem;
    delete d->compactRepresentationExpanderItem;
    delete d;
}

bool AppletQuickItem::hasItemForApplet(Plasma::Applet *applet)
{
    return AppletQuickItemPrivate::s_itemsForApplet.contains(applet);
}

AppletQuickItem *AppletQuickItem::itemForApplet(Plasma::Applet *applet)
{
    // Don't try to create applet items when the app is closing
    if (!applet || qApp->closingDown()) {
        return nullptr;
    }

    // TODO: move somewhere else? in plasmacore import?
    if (AppletQuickItemPrivate::s_itemsForApplet.isEmpty()) {
        const char *uri = "org.kde.plasma.plasmoid";
        qmlRegisterUncreatableType<PlasmoidAttached>(uri, 2, 0, "Plasmoid", QStringLiteral("Do not create objects of type Plasmoid"));
        qmlRegisterUncreatableType<ContainmentAttached>(uri, 2, 0, "Containment", QStringLiteral("Do not create objects of type Containment"));
        qmlRegisterUncreatableType<WallpaperInterface>(uri, 2, 0, "Wallpaper", QStringLiteral("Do not create objects of type Wallpaper"));

        qmlRegisterType<AppletInterface>(uri, 2, 0, "PlasmoidItem");
        qmlRegisterType<ContainmentInterface>(uri, 2, 0, "ContainmentItem");
        qmlRegisterAnonymousType<Plasma::Applet>("org.kde.plasma.plasmoid", 1);
        qmlRegisterAnonymousType<Plasma::Containment>("org.kde.plasma.plasmoid", 1);
        qmlRegisterAnonymousType<Plasma::Corona>("org.kde.plasma.plasmoid", 1);
    }
    auto it = AppletQuickItemPrivate::s_itemsForApplet.constFind(applet);
    if (it != AppletQuickItemPrivate::s_itemsForApplet.constEnd()) {
        return it.value();
    }

    Plasma::Containment *pc = qobject_cast<Plasma::Containment *>(applet);

    // TODO: kill packageurlinterceptor
    auto *qmlObject = new PlasmaQuick::SharedQmlEngine(applet);
    qmlObject->engine()->setProperty("_kirigamiTheme", QStringLiteral("KirigamiPlasmaStyle"));
    qmlObject->setInitializationDelayed(true);
    if (qmlObject->engine()->urlInterceptors().isEmpty()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(qmlObject->engine().get(), KPackage::Package());
        interceptor->setForcePlasmaStyle(true);
        qmlObject->engine()->addUrlInterceptor(interceptor);
    }
    if (applet->pluginMetaData().isValid()) {
        const QString rootPath = applet->pluginMetaData().value(QStringLiteral("X-Plasma-RootPath"));
        if (!rootPath.isEmpty()) {
            qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + rootPath);
        } else {
            qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + applet->pluginMetaData().pluginId());
        }
    }

    AppletQuickItem *item = nullptr;
    qmlObject->setSource(applet->kPackage().fileUrl("mainscript"));
    if (pc && pc->isContainment()) {
        item = qobject_cast<ContainmentInterface *>(qmlObject->rootObject());
        if (!item && qmlObject->mainComponent() && !qmlObject->mainComponent()->isError()) {
            applet->setLaunchErrorMessage(i18n("The root item of %1 must be of type ContaimentItem", applet->kPackage().fileUrl("mainscript").toString()));
        }
    } else {
        item = qobject_cast<AppletInterface *>(qmlObject->rootObject());
        if (!item && qmlObject->mainComponent() && !qmlObject->mainComponent()->isError()) {
            applet->setLaunchErrorMessage(i18n("The root item of %1 must be of type PlasmoidItem", applet->kPackage().fileUrl("mainscript").toString()));
        }
    }

    if (!item || !qmlObject->mainComponent() || qmlObject->mainComponent()->isError() || applet->failedToLaunch()) {
        QString reason;
        QJsonObject errorData;
        errorData[QStringLiteral("appletName")] = i18n("Unknown Applet");
        errorData[QStringLiteral("isDebugMode")] = qEnvironmentVariableIntValue("PLASMA_ENABLE_QML_DEBUG") != 0;

        if (applet->failedToLaunch()) {
            reason = applet->launchErrorMessage();
            errorData[QStringLiteral("errors")] = QJsonArray::fromStringList({reason});
        }
        if (applet->kPackage().isValid()) {
            const auto errors = qmlObject->mainComponent()->errors();
            QStringList errorList;
            for (const QQmlError &error : errors) {
                reason += error.toString() + QLatin1Char('\n');
                errorList << error.toString();
            }
            errorData[QStringLiteral("errors")] = QJsonArray::fromStringList(errorList);
            errorData[QStringLiteral("appletName")] = applet->pluginMetaData().name();
            reason = i18n("Error loading QML file: %1 %2", qmlObject->mainComponent()->url().toString(), reason);
        } else {
            reason = i18n("Error loading Applet: package inexistent. %1", applet->launchErrorMessage());
            errorData[QStringLiteral("errors")] = QJsonArray::fromStringList({reason});
        }

        qCWarning(LOG_PLASMAQUICK) << errorData[QStringLiteral("appletName")];
        qCWarning(LOG_PLASMAQUICK) << errorData[QStringLiteral("errors")];

        qmlObject->setSource(applet->containment()->corona()->kPackage().fileUrl("appleterror"));

        // even the error message QML may fail
        if (qmlObject->mainComponent()->isError()) {
            return nullptr;
        }

        item = qobject_cast<AppletInterface *>(qmlObject->rootObject());
        item->setProperty("errorInformation", errorData);
        applet->setLaunchErrorMessage(reason);
    }

    AppletQuickItemPrivate::s_itemsForApplet[applet] = item;
    qmlObject->setInitializationDelayed(false);
    qmlObject->completeInitialization();

    // A normal applet has UI ready as soon as is loaded, a containment, only when also the wallpaper is loaded
    if (!pc || !pc->isContainment()) {
        applet->updateConstraints(Plasma::Types::UiReadyConstraint);
        applet->flushPendingConstraintsEvents();
    }

    item->setProperty("_plasma_applet", QVariant::fromValue(applet));
    qmlObject->setParent(item);
    item->d->applet = applet;
    item->d->qmlObject = qmlObject;

    if (!qEnvironmentVariableIntValue("PLASMA_NO_CONTEXTPROPERTIES")) {
        qmlObject->rootContext()->setContextProperty(QStringLiteral("plasmoid"), applet);
    }

    applet->connect(applet, &Plasma::Applet::appletDeleted, applet, [](Plasma::Applet *applet) {
        delete AppletQuickItemPrivate::s_itemsForApplet[applet];
        AppletQuickItemPrivate::s_itemsForApplet.remove(applet);
    });

    applet->setProperty("_plasmoid", QVariant::fromValue(item));
    return item;
}

Plasma::Applet *AppletQuickItem::applet() const
{
    return d->applet;
}

void AppletQuickItem::init()
{
    Q_ASSERT(d->applet);

    d->appletPackage = d->applet->kPackage();

    if (d->applet->containment()) {
        if (d->applet->containment()->corona()) {
            d->coronaPackage = d->applet->containment()->corona()->kPackage();
        }

        d->containmentPackage = d->applet->containment()->kPackage();
    }

    // Initialize the main QML file
    QQmlEngine *engine = d->qmlObject->engine().get();

    // if the engine of the qmlObject is different from the static one, then we
    // are using an old version of the api in which every applet had one engine
    // so initialize a private url interceptor
    if (d->applet->kPackage().isValid()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine, d->applet->kPackage());
        interceptor->addAllowedPath(d->coronaPackage.path());
        engine->addUrlInterceptor(interceptor);
    }

    // If no fullRepresentation was defined, we won't create compact and expander either.
    // The only representation available are whatever items defined directly inside PlasmoidItem {}
    // default compactRepresentation is a simple icon provided by the shell package
    if (!d->compactRepresentation && d->fullRepresentation) {
        d->compactRepresentation = new QQmlComponent(engine, this);
        d->compactRepresentation->loadUrl(d->coronaPackage.fileUrl("defaultcompactrepresentation"));
        Q_EMIT compactRepresentationChanged(d->compactRepresentation);
    }

    // default compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!d->compactRepresentationExpander && d->fullRepresentation) {
        d->compactRepresentationExpander = new QQmlComponent(engine, this);
        QUrl compactExpanderUrl = d->containmentPackage.fileUrl("compactapplet");
        if (compactExpanderUrl.isEmpty()) {
            compactExpanderUrl = d->coronaPackage.fileUrl("compactapplet");
        }

        d->compactRepresentationExpander->loadUrl(compactExpanderUrl);
    }

    d->initComplete = true;
    d->compactRepresentationCheck();
    qmlObject()->engine()->rootContext()->setBaseUrl(qmlObject()->source());

    // if we're expanded we don't care about preloading because it will already be the case
    // as well as for containments
    if (d->applet->isContainment() || d->expanded || d->preferredRepresentation == d->fullRepresentation) {
        return;
    }

    if (!d->applet->isContainment() && d->applet->containment()) {
        connect(d->applet->containment(), &Plasma::Containment::uiReadyChanged, this, [this](bool uiReady) {
            if (uiReady && d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive) {
                const int preloadWeight = d->preloadWeight();
                qCDebug(LOG_PLASMAQUICK) << "New Applet " << d->applet->title() << "with a weight of" << preloadWeight;

                // don't preload applets less then a certain weight
                if (d->s_preloadPolicy >= AppletQuickItemPrivate::Aggressive || preloadWeight >= AppletQuickItemPrivate::DelayedPreloadWeight) {
                    // spread the creation over a random delay to make it look
                    // plasma started already, and load the popup in the background
                    // without big noticeable freezes, the bigger the weight the smaller is likely
                    // to be the delay, smaller minimum walue, smaller spread
                    const int min = (100 - preloadWeight) * 20;
                    const int max = (100 - preloadWeight) * 100;
                    const int delay = QRandomGenerator::global()->bounded((max + 1) - min) + min;
                    QTimer::singleShot(delay, this, [this, delay]() {
                        qCDebug(LOG_PLASMAQUICK) << "Delayed preload of " << d->applet->title() << "after" << (qreal)delay / 1000 << "seconds";
                        d->preloadForExpansion();
                    });
                }
            }
        });
    }
}

void AppletQuickItem::classBegin()
{
    QQuickItem::classBegin();
    AppletContext *ac = qobject_cast<AppletContext *>(QQmlEngine::contextForObject(this)->parentContext());
    Q_ASSERT(ac);
    d->applet = ac->applet();
    d->qmlObject = ac->sharedQmlEngine();
}

int AppletQuickItem::switchWidth() const
{
    return d->switchWidth;
}

void AppletQuickItem::setSwitchWidth(int width)
{
    if (d->switchWidth == width) {
        return;
    }

    d->switchWidth = width;
    d->compactRepresentationCheck();
    Q_EMIT switchWidthChanged(width);
}

int AppletQuickItem::switchHeight() const
{
    return d->switchHeight;
}

void AppletQuickItem::setSwitchHeight(int height)
{
    if (d->switchHeight == height) {
        return;
    }

    d->switchHeight = height;
    d->compactRepresentationCheck();
    Q_EMIT switchHeightChanged(height);
}

QQmlComponent *AppletQuickItem::compactRepresentation()
{
    return d->compactRepresentation;
}

void AppletQuickItem::setCompactRepresentation(QQmlComponent *component)
{
    if (d->compactRepresentation == component) {
        return;
    }

    d->compactRepresentation = component;
    Q_EMIT compactRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::fullRepresentation()
{
    return d->fullRepresentation;
}

QObject *AppletQuickItem::testItem()
{
    if (!d->testItem) {
        const QUrl url(d->appletPackage.fileUrl("test"));
        if (url.isEmpty()) {
            return nullptr;
        }

        d->testItem = d->qmlObject->createObjectFromSource(url, qmlContext(this));
        if (d->testItem) {
            d->testItem->setProperty("plasmoidItem", QVariant::fromValue<QObject *>(this));
        }
    }

    return d->testItem;
}

void AppletQuickItem::setFullRepresentation(QQmlComponent *component)
{
    if (d->fullRepresentation == component) {
        return;
    }

    d->fullRepresentation = component;
    Q_EMIT fullRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::preferredRepresentation()
{
    return d->preferredRepresentation;
}

void AppletQuickItem::setPreferredRepresentation(QQmlComponent *component)
{
    if (d->preferredRepresentation == component) {
        return;
    }

    d->preferredRepresentation = component;
    Q_EMIT preferredRepresentationChanged(component);
    d->compactRepresentationCheck();
}

bool AppletQuickItem::isExpanded() const
{
    return d->applet->isContainment() || !d->fullRepresentation || d->expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (d->expanded == expanded) {
        return;
    }

    if (expanded) {
        d->preloadForExpansion();
        // increase on open, ignore containments
        if (d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive && !d->applet->isContainment()) {
            const int newWeight = qMin(d->preloadWeight() + AppletQuickItemPrivate::PreloadWeightIncrement, 100);
            d->applet->config().writeEntry(QStringLiteral("PreloadWeight"), newWeight);
            qCDebug(LOG_PLASMAQUICK) << "Increasing score for" << d->applet->title() << "to" << newWeight;
        }
    }

    d->expanded = expanded;

    Q_EMIT expandedChanged(expanded);
}

bool AppletQuickItem::isActivationTogglesExpanded() const
{
    return d->activationTogglesExpanded;
}

void AppletQuickItem::setActivationTogglesExpanded(bool activationTogglesExpanded)
{
    if (d->activationTogglesExpanded == activationTogglesExpanded) {
        return;
    }
    d->activationTogglesExpanded = activationTogglesExpanded;
    Q_EMIT activationTogglesExpandedChanged(activationTogglesExpanded);
}

bool AppletQuickItem::hideOnWindowDeactivate() const
{
    return d->hideOnWindowDeactivate;
}

void AppletQuickItem::setHideOnWindowDeactivate(bool hide)
{
    if (d->hideOnWindowDeactivate == hide) {
        return;
    }
    d->hideOnWindowDeactivate = hide;
    Q_EMIT hideOnWindowDeactivateChanged(hide);
}

bool AppletQuickItem::preloadFullRepresentation() const
{
    return d->preloadFullRepresentation;
}

void AppletQuickItem::setPreloadFullRepresentation(bool preload)
{
    if (d->preloadFullRepresentation == preload) {
        return;
    }

    d->preloadFullRepresentation = preload;
    d->createFullRepresentationItem();

    Q_EMIT preloadFullRepresentationChanged(preload);
}

////////////Internals

PlasmaQuick::SharedQmlEngine *AppletQuickItem::qmlObject()
{
    return d->qmlObject;
}

QQuickItem *AppletQuickItem::compactRepresentationItem()
{
    return d->compactRepresentationItem;
}

QQuickItem *AppletQuickItem::fullRepresentationItem()
{
    return d->fullRepresentationItem;
}

void AppletQuickItem::childEvent(QChildEvent *event)
{
    // Added child may be QQuickLayoutAttached
    if (event->added() && !d->ownLayout && d->currentRepresentationItem) {
        // Child has not yet finished initialization at this point
        QTimer::singleShot(0, this, [this]() {
            if (!d->ownLayout) {
                d->connectLayoutAttached(d->currentRepresentationItem);
            }
        });
    }

    QQuickItem::childEvent(event);
}

void AppletQuickItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    d->compactRepresentationCheck();
}

void AppletQuickItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        // we have a window: create the representations if needed
        if (value.window) {
            init();
        }
    }

    QQuickItem::itemChange(change, value);
}

}

#include "moc_appletquickitem.cpp"
