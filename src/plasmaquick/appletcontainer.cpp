/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletcontainer.h"
#include "debug_p.h"
#include "private/appletcontainer_p.h"
#include "appletrepresentation.h"

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
#include <kdeclarative/qmlobjectsharedengine.h>

#include <packageurlinterceptor.h>
#include <qloggingcategory.h>

namespace PlasmaQuick
{
QHash<QObject *, AppletContainer *> AppletContainerPrivate::s_rootObjects = QHash<QObject *, AppletContainer *>();

AppletContainerPrivate::PreloadPolicy AppletContainerPrivate::s_preloadPolicy = AppletContainerPrivate::Uninitialized;

AppletContainerPrivate::AppletContainerPrivate(AppletContainer *item)
    : q(item)
    , switchWidth(-1)
    , switchHeight(-1)
    , expanded(false)
    , activationTogglesExpanded(true)
    , initComplete(false)
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

void AppletContainerPrivate::init()
{
    
}

int AppletContainerPrivate::preloadWeight() const
{
    int defaultWeight;
    const QStringList provides(KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides")));

    // some applet types we want a bigger weight
    if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
        defaultWeight = DefaultLauncherPreloadWeight;
    } else {
        defaultWeight = DefaultPreloadWeight;
    }
    // default widgets to be barely preloaded
    return qBound(0,
                  applet->config().readEntry(QStringLiteral("PreloadWeight"),
                                             qMax(defaultWeight, applet->pluginMetaData().rawData().value(QStringLiteral("X-Plasma-PreloadWeight")).toInt())),
                  100);
}

bool AppletContainerPrivate::appletShouldBeExpanded() const
{
    if (!applet || applet->isContainment()) {
        return true;

    } else {
        if (switchWidth > 0 && switchHeight > 0) {
            return q->width() > switchWidth && q->height() > switchHeight;

            // if a size to switch wasn't set, determine what representation to always chose
        } else {
            // preferred representation set?
            if (appletRepresentation->preferredRepresentation()) {
                return appletRepresentation->preferredRepresentation() == appletRepresentation->fullRepresentation();
                // Otherwise, base on FormFactor
            } else {
                return (applet->formFactor() != Plasma::Types::Horizontal && applet->formFactor() != Plasma::Types::Vertical);
            }
        }
    }
}

void AppletContainerPrivate::createExpanderItem()
{
    // default expander is the popup in which fullRepresentation goes
    if (!expander) {
        
        QUrl compactExpanderUrl = containmentPackage.fileUrl("compactapplet");

        if (compactExpanderUrl.isEmpty()) {
            compactExpanderUrl = coronaPackage.fileUrl("compactapplet");
        }

        expander = new QQmlComponent(qmlEngine(q), compactExpanderUrl, q);
    }

    if (expanderItem) {
        return;
    }

    QVariantMap initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);

    expanderItem = qobject_cast<QQuickItem *>(expander->createWithInitialProperties(initialProperties, qmlContext(q)));
}

void AppletContainerPrivate::preloadForExpansion()
{
    qint64 time = 0;
    if (QLoggingCategory::defaultCategory()->isInfoEnabled()) {
        time = QDateTime::currentMSecsSinceEpoch();
    }

    // It will create it if not existent
    QQuickItem * fullRepresentationItem = appletRepresentation->fullRepresentationItem();

    // When not already expanded, also preload the expander
    if (!expanded && !applet->isContainment() && (!appletRepresentation->preferredRepresentation() || appletRepresentation->preferredRepresentation() != appletRepresentation->fullRepresentation())) {
        createExpanderItem();
    }

    if (!appletShouldBeExpanded() && expanderItem) {
        expanderItem->setProperty("contentItem", QVariant::fromValue<QObject *>(fullRepresentationItem));
    } else if (fullRepresentationItem) {
        fullRepresentationItem->setProperty("parent", QVariant::fromValue<QObject *>(q));
    }

    // preallocate scene graph nodes
    if (fullRepresentationItem && fullRepresentationItem->window()) {
        fullRepresentationItem->window()->create();
    }

    qCDebug(LOG_PLASMAQUICK) << "Applet" << applet->title() << "loaded after" << (QDateTime::currentMSecsSinceEpoch() - time) << "msec";
}

void AppletContainerPrivate::compactRepresentationCheck()
{
    if (!appletRepresentation) {
        return;
    }

    if (appletShouldBeExpanded()) {
        appletRepresentation->fullRepresentationItem()->setParentItem(q);
        appletRepresentation->fullRepresentationItem()->setVisible(true);
        if (appletRepresentation->compactRepresentationItem()) {
            appletRepresentation->compactRepresentationItem()->setVisible(false);
        }
    } else {
        appletRepresentation->compactRepresentationItem()->setParentItem(q);
        appletRepresentation->compactRepresentationItem()->setVisible(true);
        appletRepresentation->fullRepresentationItem()->setVisible(false);
    }
}

AppletContainer::AppletContainer(QQuickItem *parent)
    : QQuickItem(parent)
    , d(new AppletContainerPrivate(this))
{
   // d->init();

    if (d->applet) {
        d->appletPackage = d->applet->kPackage();

        if (d->applet->containment()) {
            if (d->applet->containment()->corona()) {
                d->coronaPackage = d->applet->containment()->corona()->kPackage();
            }

            d->containmentPackage = d->applet->containment()->kPackage();
        }
    }
}

AppletContainer::~AppletContainer()
{
    // decrease weight
    if (d->s_preloadPolicy >= AppletContainerPrivate::Adaptive) {
        d->applet->config().writeEntry(QStringLiteral("PreloadWeight"), qMax(0, d->preloadWeight() - AppletContainerPrivate::PreloadWeightDecrement));
    }

    delete d;
}


void AppletContainer::setApplet(Plasma::Applet *applet)
{
    if (!applet || applet == d->applet) {
        return;
    }

    d->applet = applet;

    // TODO: remove duplication
    if (d->applet) {
        d->appletPackage = d->applet->kPackage();

        if (d->applet->containment()) {
            if (d->applet->containment()->corona()) {
                d->coronaPackage = d->applet->containment()->corona()->kPackage();
            }

            d->containmentPackage = d->applet->containment()->kPackage();
        }
    }

    if (window()) {
        d->appletRepresentation = AppletRepresentation::representationForApplet(applet, qmlEngine(this));
    }

    if (d->appletRepresentation) {
        Q_EMIT appletRepresentationChanged(d->appletRepresentation);
    } else {
        qWarning() << "unable to load Applet" << applet->title();
    }

    Q_EMIT appletChanged(applet);

    d->compactRepresentationCheck();
}

Plasma::Applet *AppletContainer::applet() const
{
    return d->applet;
}

AppletRepresentation *AppletContainer::appletRepresentation()
{
    return d->appletRepresentation;
}

QQuickItem *AppletContainer::contentItem()
{
    return d->contentItem;
}

QQuickItem *AppletContainer::temporaryExpansionItem()
{
    return d->temporaryExpansionItem;
}

void AppletContainer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->compactRepresentationCheck();
}

void AppletContainer::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        // we have a window: create the representations if needed
        if (value.window) {
            d->init();
            //FIXME
            d->appletRepresentation = AppletRepresentation::representationForApplet(d->applet, qmlEngine(this));
            d->compactRepresentationCheck();
        }
    }

    QQuickItem::itemChange(change, value);
}

}

#include "moc_appletcontainer.cpp"
