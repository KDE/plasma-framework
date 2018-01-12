/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
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

#include "appletquickitem.h"
#include "private/appletquickitem_p.h"

#include <QQmlComponent>
#include <QQmlExpression>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQmlContext>

#include <QDebug>

#include <klocalizedstring.h>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <kdeclarative/qmlobjectsharedengine.h>

#include <packageurlinterceptor.h>
#include <private/package_p.h>

namespace PlasmaQuick
{

QHash<QObject *, AppletQuickItem *> AppletQuickItemPrivate::s_rootObjects = QHash<QObject *, AppletQuickItem *>();


AppletQuickItemPrivate::AppletQuickItemPrivate(Plasma::Applet *a, AppletQuickItem *item)
    : q(item),
      switchWidth(-1),
      switchHeight(-1),
      applet(a),
      expanded(false),
      activationTogglesExpanded(false)
{
}

void AppletQuickItemPrivate::init()
{
    if (!applet->pluginMetaData().isValid()) {
        // This `qmlObject` is used in other parts of the code
        qmlObject = new KDeclarative::QmlObject(q);
        return;
    }

    qmlObject = new KDeclarative::QmlObjectSharedEngine(q);
    if (!qmlObject->engine()->urlInterceptor()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(qmlObject->engine(), KPackage::Package());
        interceptor->setForcePlasmaStyle(true);
        qmlObject->engine()->setUrlInterceptor(interceptor);
    }
}

void AppletQuickItemPrivate::connectLayoutAttached(QObject *item)
{
    QObject *layout = 0;

    //Extract the representation's Layout, if any
    //No Item?
    if (!item) {
        return;
    }

    //Search a child that has the needed Layout properties
    //HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    foreach (QObject *child, item->children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
                child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
                child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
                child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
           ) {
            layout = child;
            break;
        }
    }

    //if the compact repr doesn't export a Layout.* attached property,
    //reset our own with default values
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

    //propagate all the size hints
    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");


    QObject *ownLayout = 0;

    foreach (QObject *child, q->children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
                child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
                child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
                child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
           ) {
            ownLayout = child;
            break;
        }
    }

    //this should never happen, since we ask to create it if doesn't exists
    if (!ownLayout) {
        return;
    }

    //if the representation didn't change, don't do anything
    if (representationLayout == layout) {
        return;
    }

    if (representationLayout) {
        QObject::disconnect(representationLayout, 0, q, 0);
    }

    //Here we can't use the new connect syntax because we can't link against QtQuick layouts
    QObject::connect(layout, SIGNAL(minimumWidthChanged()),
                     q, SLOT(minimumWidthChanged()));
    QObject::connect(layout, SIGNAL(minimumHeightChanged()),
                     q, SLOT(minimumHeightChanged()));

    QObject::connect(layout, SIGNAL(preferredWidthChanged()),
                     q, SLOT(preferredWidthChanged()));
    QObject::connect(layout, SIGNAL(preferredHeightChanged()),
                     q, SLOT(preferredHeightChanged()));

    QObject::connect(layout, SIGNAL(maximumWidthChanged()),
                     q, SLOT(maximumWidthChanged()));
    QObject::connect(layout, SIGNAL(maximumHeightChanged()),
                     q, SLOT(maximumHeightChanged()));

    QObject::connect(layout, SIGNAL(fillWidthChanged()),
                     q, SLOT(fillWidthChanged()));
    QObject::connect(layout, SIGNAL(fillHeightChanged()),
                     q, SLOT(fillHeightChanged()));

    representationLayout = layout;
    AppletQuickItemPrivate::ownLayout = ownLayout;

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
        return 0;
    }

    if (compactRepresentationItem) {
        return compactRepresentationItem;
    }

    QVariantHash initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);

    compactRepresentationItem = qobject_cast<QQuickItem*>(qmlObject->createObjectFromComponent(compactRepresentation, QtQml::qmlContext(qmlObject->rootObject()), initialProperties));

    emit q->compactRepresentationItemChanged(compactRepresentationItem);

    return compactRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createFullRepresentationItem()
{
    if (fullRepresentationItem) {
        return fullRepresentationItem;
    }

    if (fullRepresentation && fullRepresentation != qmlObject->mainComponent()) {
        QVariantHash initialProperties;
        initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);
        fullRepresentationItem = qobject_cast<QQuickItem*>(qmlObject->createObjectFromComponent(fullRepresentation, QtQml::qmlContext(qmlObject->rootObject()), initialProperties));
    } else {
        fullRepresentation = qmlObject->mainComponent();
        fullRepresentationItem = qobject_cast<QQuickItem*>(qmlObject->rootObject());
        emit q->fullRepresentationChanged(fullRepresentation);
    }

    if (!fullRepresentationItem) {
        return 0;
    }

    emit q->fullRepresentationItemChanged(fullRepresentationItem);

    return fullRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createCompactRepresentationExpanderItem()
{
    if (!compactRepresentationExpander) {
        return 0;
    }

    if (compactRepresentationExpanderItem) {
        return compactRepresentationExpanderItem;
    }

    compactRepresentationExpanderItem = qobject_cast<QQuickItem*>(qmlObject->createObjectFromComponent(compactRepresentationExpander, QtQml::qmlContext(qmlObject->rootObject())));

    if (!compactRepresentationExpanderItem) {
        return 0;
    }

    compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject*>(createCompactRepresentationItem()));

    return compactRepresentationExpanderItem;
}

void AppletQuickItemPrivate::compactRepresentationCheck()
{
    if (!qmlObject->rootObject()) {
        return;
    }

    //ignore 0 sizes;
    if (q->width() <= 0 || q->height() <= 0) {
        return;
    }

    bool full = false;

    if (applet->isContainment()) {
        full = true;

    } else {
        if (switchWidth > 0 && switchHeight > 0) {
            full = q->width() > switchWidth && q->height() > switchHeight;
            //if a size to switch wasn't set, determine what representation to always chose
        } else {
            //preferred representation set?
            if (preferredRepresentation) {
                full = preferredRepresentation == fullRepresentation;
                //Otherwise, base on FormFactor
            } else {
                full = (applet->formFactor() != Plasma::Types::Horizontal && applet->formFactor() != Plasma::Types::Vertical);
            }
        }

        if ((full && fullRepresentationItem && fullRepresentationItem == currentRepresentationItem) ||
                (!full && compactRepresentationItem && compactRepresentationItem == currentRepresentationItem)
           ) {
            return;
        }
    }

    //Expanded
    if (full) {
        QQuickItem *item = createFullRepresentationItem();

        if (item) {
            //unwire with the expander
            if (compactRepresentationExpanderItem) {
                compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant());
                compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant());
                compactRepresentationExpanderItem->setVisible(false);
            }

            item->setParentItem(q);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(qmlObject->rootObject()), item, QStringLiteral("parent"));
                QQmlProperty prop(item, QStringLiteral("anchors.fill"));
                prop.write(expr.evaluate());
            }

            if (compactRepresentationItem) {
                compactRepresentationItem->setVisible(false);
            }

            currentRepresentationItem = item;
            connectLayoutAttached(item);
            expanded = true;
            emit q->expandedChanged(true);
        }

        //Icon
    } else {
        QQuickItem *compactItem = createCompactRepresentationItem();
        QQuickItem *compactExpanderItem = createCompactRepresentationExpanderItem();

        if (compactItem && compactExpanderItem) {
            //set the root item as the main visible item
            compactItem->setVisible(true);
            compactExpanderItem->setParentItem(q);
            compactExpanderItem->setVisible(true);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(qmlObject->rootObject()), compactExpanderItem, QStringLiteral("parent"));
                QQmlProperty prop(compactExpanderItem, QStringLiteral("anchors.fill"));
                prop.write(expr.evaluate());
            }

            if (fullRepresentationItem) {
                fullRepresentationItem->setProperty("parent", QVariant());
            }

            compactExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject*>(compactItem));
            //The actual full representation will be connected when created
            compactExpanderItem->setProperty("fullRepresentation", QVariant());

            currentRepresentationItem = compactItem;
            connectLayoutAttached(compactItem);

            expanded = false;
            emit q->expandedChanged(false);
        }
    }
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




AppletQuickItem::AppletQuickItem(Plasma::Applet *applet, QQuickItem *parent)
    : QQuickItem(parent),
      d(new AppletQuickItemPrivate(applet, this))
{
    d->init();

    if (d->applet) {
        d->appletPackage = d->applet->kPackage();

        if (d->applet->containment()) {
            if (d->applet->containment()->corona()) {
                d->coronaPackage = d->applet->containment()->corona()->kPackage();
            }

            d->containmentPackage = d->applet->containment()->kPackage();
        }

        if (d->applet->pluginMetaData().isValid()) {
            const QString rootPath = d->applet->pluginMetaData().value(QStringLiteral("X-Plasma-RootPath"));
            if (!rootPath.isEmpty()) {
                d->qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + rootPath);
            } else {
                d->qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + d->applet->pluginMetaData().pluginId());
            }
        }

        // set the graphicObject dynamic property on applet
        d->applet->setProperty("_plasma_graphicObject", QVariant::fromValue(this));
    
        setVisible(d->applet->shownInInputModes().contains(d->applet->inputMode()));
        connect(d->applet, &Plasma::Applet::inputModeChanged, this, [this]() {
    qWarning()<<"AAAAA"<<d->applet->inputMode()<<d->applet->shownInInputModes().contains(d->applet->inputMode());
            setVisible(d->applet->shownInInputModes().contains(d->applet->inputMode()));
        });
    }

    d->qmlObject->setInitializationDelayed(true);

    setProperty("_plasma_applet", QVariant::fromValue(d->applet));
}

AppletQuickItem::~AppletQuickItem()
{
    //Here the order is important
    delete d->compactRepresentationItem;
    delete d->fullRepresentationItem;
    delete d->compactRepresentationExpanderItem;

    AppletQuickItemPrivate::s_rootObjects.remove(d->qmlObject->rootContext());
}

AppletQuickItem *AppletQuickItem::qmlAttachedProperties(QObject *object)
{
    QQmlContext *context;
    //is it using shared engine mode?
    if (!QtQml::qmlEngine(object)->parent()) {
        context = QtQml::qmlContext(object);
        //search the root context of the applet in which the object is in
        while (context) {
            //the rootcontext of an applet is a child of the engine root context
            if (context->parentContext() == QtQml::qmlEngine(object)->rootContext()) {
                break;
            }

            context = context->parentContext();
        }
    //otherwise index by root context
    } else {
        context = QtQml::qmlEngine(object)->rootContext();
    }
    //at the moment of the attached object creation, the root item is the only one that hasn't a parent
    //only way to avoid creation of this attached for everybody but the root item
    if (!object->parent() && AppletQuickItemPrivate::s_rootObjects.contains(context)) {
        return AppletQuickItemPrivate::s_rootObjects.value(context);
    } else {
        return 0;
    }
}

Plasma::Applet *AppletQuickItem::applet() const
{
    return d->applet;
}

void AppletQuickItem::init()
{
    //FIXME: Plasmoid attached property should be fixed since can't be indexed by engine anymore
    if (AppletQuickItemPrivate::s_rootObjects.contains(d->qmlObject->rootContext())) {
        return;
    }

    AppletQuickItemPrivate::s_rootObjects[d->qmlObject->rootContext()] = this;

    Q_ASSERT(d->applet);

    //Initialize the main QML file
    QQmlEngine *engine = d->qmlObject->engine();

    //if the engine of the qmlObject is different from the static one, then we
    //are using an old version of the api in which every applet had one engine
    //so initialize a private url interceptor
    if (d->applet->kPackage().isValid() && !qobject_cast<KDeclarative::QmlObjectSharedEngine *>(d->qmlObject)) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine, d->applet->kPackage());
        interceptor->addAllowedPath(d->coronaPackage.path());
        engine->setUrlInterceptor(interceptor);
    }

    //Force QtQuickControls to use the "Plasma" style for this engine.
    //this way is possible to mix QtQuickControls and plasma components in applets
    //while still having the desktop style in configuration dialogs
    if (!engine->property("_plasma_qqc_style_set").toBool()) {
        QQmlComponent c(engine);
        c.setData(QByteArrayLiteral("import QtQuick 2.1\n\
            import QtQuick.Controls 1.0\n\
            import QtQuick.Controls.Private 1.0\n \
            QtObject {\
              Component.onCompleted: {\
                Settings.styleName = \"Plasma\";\
              }\
            }"), QUrl());
        QObject *o = c.create();
        o->deleteLater();
        engine->setProperty(("_plasma_qqc_style_set"), true);
    }

    d->qmlObject->setSource(d->applet->kPackage().fileUrl("mainscript"));

    if (!engine || !engine->rootContext() || !engine->rootContext()->isValid() || !d->qmlObject->mainComponent() || d->qmlObject->mainComponent()->isError() || d->applet->failedToLaunch()) {
        QString reason;
        if (d->applet->failedToLaunch()) {
            reason = d->applet->launchErrorMessage();
        } else if (d->applet->kPackage().isValid()) {
            foreach (QQmlError error, d->qmlObject->mainComponent()->errors()) {
                reason += error.toString() + QLatin1Char('\n');
            }
            reason = i18n("Error loading QML file: %1", reason);
        } else {
            reason = i18n("Error loading Applet: package inexistent. %1", applet()->launchErrorMessage());
        }

        d->qmlObject->setSource(d->coronaPackage.fileUrl("appleterror"));
        d->qmlObject->completeInitialization();

        //even the error message QML may fail
        if (d->qmlObject->mainComponent()->isError()) {
            return;
        } else {
            d->qmlObject->rootObject()->setProperty("reason", reason);
        }

        d->applet->setLaunchErrorMessage(reason);
    }

    d->qmlObject->rootContext()->setContextProperty(QStringLiteral("plasmoid"), this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    //initialize with our size only if valid
    if (width() > 0 && height() > 0) {
        const qreal w = parentItem() ? std::min(parentItem()->width(), width()) : width();
        const qreal h = parentItem() ? std::min(parentItem()->height(), height()) : height();
        initialProperties[QStringLiteral("width")] = w;
        initialProperties[QStringLiteral("height")] = h;
    }
    d->qmlObject->setInitializationDelayed(false);
    d->qmlObject->completeInitialization(initialProperties);

    //otherwise, initialize our size to root object's size
    if (d->qmlObject->rootObject() && (width() <= 0 || height() <= 0)) {
        const qreal w = d->qmlObject->rootObject()->property("width").value<qreal>();
        const qreal h = d->qmlObject->rootObject()->property("height").value<qreal>();
        setSize(parentItem() ? QSizeF(std::min(parentItem()->width(), w), std::min(parentItem()->height(), h)) : QSizeF(w, h));
    }

    //default fullrepresentation is our root main component, if none specified
    if (!d->fullRepresentation) {
        d->fullRepresentation = d->qmlObject->mainComponent();
        d->fullRepresentationItem = qobject_cast<QQuickItem*>(d->qmlObject->rootObject());

        emit fullRepresentationChanged(d->fullRepresentation);
    }

    //default compactRepresentation is a simple icon provided by the shell package
    if (!d->compactRepresentation) {
        d->compactRepresentation = new QQmlComponent(engine, this);
        d->compactRepresentation->loadUrl(d->coronaPackage.fileUrl("defaultcompactrepresentation"));
        emit compactRepresentationChanged(d->compactRepresentation);
    }

    //default compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!d->compactRepresentationExpander) {
        d->compactRepresentationExpander = new QQmlComponent(engine, this);
        QUrl compactExpanderUrl = d->containmentPackage.fileUrl("compactapplet");

        if (compactExpanderUrl.isEmpty()) {
            compactExpanderUrl = d->coronaPackage.fileUrl("compactapplet");
        }

        d->compactRepresentationExpander->loadUrl(compactExpanderUrl);
    }

    d->compactRepresentationCheck();
    qmlObject()->engine()->rootContext()->setBaseUrl(qmlObject()->source());
    qmlObject()->engine()->setContextForObject(this, qmlObject()->engine()->rootContext());
}


Plasma::Package AppletQuickItem::appletPackage() const
{
    return Plasma::Package(d->appletPackage);
}

void AppletQuickItem::setAppletPackage(const Plasma::Package &package)
{
    d->appletPackage = package.kPackage();
}

Plasma::Package AppletQuickItem::coronaPackage() const
{
    return Plasma::Package(d->coronaPackage);
}

void AppletQuickItem::setCoronaPackage(const Plasma::Package &package)
{
    d->coronaPackage = package.kPackage();
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
    emit switchWidthChanged(width);
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
    emit switchHeightChanged(height);
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
    emit compactRepresentationChanged(component);
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

        d->testItem = d->qmlObject->createObjectFromSource(url, QtQml::qmlContext(rootItem()));
        if (d->testItem) {
            d->testItem->setProperty("plasmoidItem", QVariant::fromValue<QObject*>(this));
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
    emit fullRepresentationChanged(component);
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
    emit preferredRepresentationChanged(component);
    d->compactRepresentationCheck();
}

bool AppletQuickItem::isExpanded() const
{
    return d->expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (d->expanded == expanded) {
        return;
    }

    if (expanded) {
        d->createFullRepresentationItem();
        if (!d->applet->isContainment() &&
                (!d->preferredRepresentation ||
                 d->preferredRepresentation != d->fullRepresentation)) {
            d->createCompactRepresentationExpanderItem();
        }

        if (d->compactRepresentationExpanderItem) {
            d->compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant::fromValue<QObject*>(d->createFullRepresentationItem()));
        } else {
            d->fullRepresentationItem->setProperty("parent", QVariant::fromValue<QObject*>(this));
        }
    }

    d->expanded = expanded;
    emit expandedChanged(expanded);
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
    emit activationTogglesExpandedChanged(activationTogglesExpanded);
}

////////////Internals

KDeclarative::QmlObject *AppletQuickItem::qmlObject()
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

QObject *AppletQuickItem::rootItem()
{
    return d->qmlObject->rootObject();
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

void AppletQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->compactRepresentationCheck();
}

void AppletQuickItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the representations if needed
        if (value.window) {
            init();
        }
    }

    QQuickItem::itemChange(change, value);
}

}

#include "moc_appletquickitem.cpp"

