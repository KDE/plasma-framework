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
#include <kdeclarative/qmlobject.h>
#include <plasma/scripting/appletscript.h>

#include <packageurlinterceptor.h>

namespace PlasmaQuick
{

QHash<QObject *, AppletQuickItem *> AppletQuickItemPrivate::s_rootObjects = QHash<QObject *, AppletQuickItem *>();

AppletQuickItemPrivate::AppletQuickItemPrivate(Plasma::Applet *a, AppletQuickItem *item)
    : q(item),
      switchWidth(-1),
      switchHeight(-1),
      applet(a),
      expanded(false)
{
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
        }
    }

    if (!layout) {
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

    //HACK: check the Layout properties we wrote
    QQmlProperty p(q, "Layout.minimumWidth", QtQml::qmlContext(qmlObject->rootObject()));

    QObject *ownLayout = 0;

    foreach (QObject *child, q->children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
                child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
                child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
                child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
           ) {
            ownLayout = child;
        }
    }

    //this should never happen, since we ask to create it if doesn't exists
    if (!ownLayout) {
        return;
    }

    //if the representation didn't change, don't do anything
    if (representationLayout.data() == layout) {
        return;
    }

    if (representationLayout) {
        QObject::disconnect(representationLayout.data(), 0, q, 0);
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
        ownLayout.data()->setProperty(layoutProperty, representationLayout.data()->property(layoutProperty));
    }
}

QObject *AppletQuickItemPrivate::createCompactRepresentationItem()
{
    if (!compactRepresentation) {
        return 0;
    }

    if (compactRepresentationItem) {
        return compactRepresentationItem.data();
    }

    compactRepresentationItem = qmlObject->createObjectFromComponent(compactRepresentation.data(), QtQml::qmlContext(qmlObject->rootObject()));

    emit q->compactRepresentationItemChanged(compactRepresentationItem.data());

    return compactRepresentationItem.data();
}

QObject *AppletQuickItemPrivate::createFullRepresentationItem()
{
    if (fullRepresentationItem) {
        return fullRepresentationItem.data();
    }

    if (fullRepresentation && fullRepresentation.data() != qmlObject->mainComponent()) {
        fullRepresentationItem = qmlObject->createObjectFromComponent(fullRepresentation.data(), QtQml::qmlContext(qmlObject->rootObject()));
    } else {
        fullRepresentation = qmlObject->mainComponent();
        fullRepresentationItem = qmlObject->rootObject();
        emit q->fullRepresentationChanged(fullRepresentation.data());
    }

    if (!fullRepresentationItem) {
        return 0;
    }

    QQuickItem *graphicsObj = qobject_cast<QQuickItem *>(fullRepresentationItem.data());

    QObject::connect(graphicsObj, &QQuickItem::widthChanged, [ = ]() {
        fullRepresentationResizeTimer.start();
    });
    QObject::connect(graphicsObj, &QQuickItem::heightChanged, [ = ]() {
        fullRepresentationResizeTimer.start();
    });

    emit q->fullRepresentationItemChanged(fullRepresentationItem.data());

    return fullRepresentationItem.data();
}

QObject *AppletQuickItemPrivate::createCompactRepresentationExpanderItem()
{
    if (!compactRepresentationExpander) {
        return 0;
    }

    if (compactRepresentationExpanderItem) {
        return compactRepresentationExpanderItem.data();
    }

    compactRepresentationExpanderItem = qmlObject->createObjectFromComponent(compactRepresentationExpander.data(), QtQml::qmlContext(qmlObject->rootObject()));

    if (!compactRepresentationExpanderItem) {
        return 0;
    }

    compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant::fromValue(createCompactRepresentationItem()));

    return compactRepresentationExpanderItem.data();
}

void AppletQuickItemPrivate::compactRepresentationCheck()
{
    //ignore 0,0 sizes;
    if (q->width() <= 0 && q->height() <= 0) {
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
                full = preferredRepresentation.data() == fullRepresentation.data();
                //Otherwise, base on FormFactor
            } else {
                full = (applet->formFactor() != Plasma::Types::Horizontal && applet->formFactor() != Plasma::Types::Vertical);
            }
        }

        if ((full && fullRepresentationItem && fullRepresentationItem.data() == currentRepresentationItem.data()) ||
                (!full && compactRepresentationItem && compactRepresentationItem.data() == currentRepresentationItem.data())
           ) {
            return;
        }
    }

    //Expanded
    if (full) {
        QQuickItem *item = qobject_cast<QQuickItem *>(createFullRepresentationItem());

        if (item) {
            //unwire with the expander
            if (compactRepresentationExpanderItem) {
                compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant());
                compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant());
                compactRepresentationExpanderItem.data()->setProperty("visible", false);
            }

            item->setParentItem(q);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(qmlObject->rootObject()), item, "parent");
                QQmlProperty prop(item, "anchors.fill");
                prop.write(expr.evaluate());
            }

            if (compactRepresentationItem) {
                compactRepresentationItem.data()->setProperty("visible", false);
            }

            currentRepresentationItem = item;
            connectLayoutAttached(item);
            expanded = true;
            emit q->expandedChanged(true);
        }

        //Icon
    } else {
        QQuickItem *compactItem = qobject_cast<QQuickItem *>(createCompactRepresentationItem());
        QQuickItem *compactExpanderItem = qobject_cast<QQuickItem *>(createCompactRepresentationExpanderItem());

        if (compactItem && compactExpanderItem) {
            //set the root item as the main visible item
            compactItem->setVisible(true);
            compactExpanderItem->setParentItem(q);
            compactExpanderItem->setVisible(true);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(qmlObject->rootObject()), compactExpanderItem, "parent");
                QQmlProperty prop(compactExpanderItem, "anchors.fill");
                prop.write(expr.evaluate());
            }

            if (fullRepresentationItem) {
                fullRepresentationItem.data()->setProperty("parent", QVariant());
            }

            compactExpanderItem->setProperty("compactRepresentation", QVariant::fromValue(compactItem));
            //The actual full representation will be connected when created
            compactExpanderItem->setProperty("fullRepresentation", QVariant::fromValue(QVariant()));

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
    if (d->applet) {
        d->appletPackage = d->applet->package();
    }
    if (d->applet && d->applet->containment() && d->applet->containment()->corona()) {
        d->coronaPackage = d->applet->containment()->corona()->package();
    }

    d->compactRepresentationCheckTimer.setSingleShot(true);
    d->compactRepresentationCheckTimer.setInterval(250);
    connect(&d->compactRepresentationCheckTimer, SIGNAL(timeout()),
            this, SLOT(compactRepresentationCheck()));
    d->compactRepresentationCheckTimer.start();

    d->fullRepresentationResizeTimer.setSingleShot(true);
    d->fullRepresentationResizeTimer.setInterval(250);
    connect(&d->fullRepresentationResizeTimer, &QTimer::timeout,
    [ = ]() {
        if (!d->applet->isContainment()) {
            KConfigGroup cg = d->applet->config();
            cg = KConfigGroup(&cg, "PopupApplet");
            cg.writeEntry("DialogWidth", d->fullRepresentationItem.data()->property("width").toInt());
            cg.writeEntry("DialogHeight", d->fullRepresentationItem.data()->property("height").toInt());
        }
    });

    d->qmlObject = new KDeclarative::QmlObject(this);
    d->qmlObject->setInitializationDelayed(true);

    // set the graphicObject dynamic property on applet
    d->applet->setProperty("_plasma_graphicObject", QVariant::fromValue(this));
    setProperty("_plasma_applet", QVariant::fromValue(applet));
}

AppletQuickItem::~AppletQuickItem()
{
    //Here the order is important
    delete d->compactRepresentationItem.data();
    delete d->fullRepresentationItem.data();
    delete d->compactRepresentationExpanderItem.data();

    AppletQuickItemPrivate::s_rootObjects.remove(d->qmlObject->engine());
}

AppletQuickItem *AppletQuickItem::qmlAttachedProperties(QObject *object)
{
    //at the moment of the attached object creation, the root item is the only one that hasn't a parent
    //only way to avoid creation of this attached for everybody but the root item
    if (!object->parent() && AppletQuickItemPrivate::s_rootObjects.contains(QtQml::qmlEngine(object))) {
        return AppletQuickItemPrivate::s_rootObjects.value(QtQml::qmlEngine(object));
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
    if (AppletQuickItemPrivate::s_rootObjects.contains(d->qmlObject->engine())) {
        return;
    }

    AppletQuickItemPrivate::s_rootObjects[d->qmlObject->engine()] = this;

    Q_ASSERT(d->applet);

    //Initialize the main QML file
    QQmlEngine *engine = d->qmlObject->engine();

    if (d->applet->package().isValid()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine, d->applet->package());
        interceptor->addAllowedPath(d->coronaPackage.path());
        engine->setUrlInterceptor(interceptor);
    }

    d->qmlObject->setSource(QUrl::fromLocalFile(d->applet->package().filePath("mainscript")));

    if (!engine || !engine->rootContext() || !engine->rootContext()->isValid() || d->qmlObject->mainComponent()->isError()) {
        QString reason;
        if (d->applet->package().isValid()) {
            foreach (QQmlError error, d->qmlObject->mainComponent()->errors()) {
                reason += error.toString() + '\n';
            }
            reason = i18n("Error loading QML file: %1", reason);
        } else {
            reason = i18n("Error loading Applet: package inexistent. %1", applet()->launchErrorMessage());
        }

        d->qmlObject->setSource(QUrl::fromLocalFile(d->coronaPackage.filePath("appleterror")));
        d->qmlObject->completeInitialization();

        //even the error message QML may fail
        if (d->qmlObject->mainComponent()->isError()) {
            return;
        } else {
            d->qmlObject->rootObject()->setProperty("reason", reason);
        }

        d->applet->setLaunchErrorMessage(reason);
    }

    engine->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    initialProperties["width"] = width();
    initialProperties["height"] = height();
    d->qmlObject->completeInitialization(initialProperties);

    //default fullrepresentation is our root main component, if none specified
    if (!d->fullRepresentation) {
        d->fullRepresentation = d->qmlObject->mainComponent();
        d->fullRepresentationItem = d->qmlObject->rootObject();

        if (d->qmlObject->rootObject()) {
            QQuickItem *graphicsObj = qobject_cast<QQuickItem *>(d->fullRepresentationItem.data());
            QObject::connect(graphicsObj, &QQuickItem::widthChanged, [ = ]() {
                d->fullRepresentationResizeTimer.start();
            });
            QObject::connect(graphicsObj, &QQuickItem::heightChanged, [ = ]() {
                d->fullRepresentationResizeTimer.start();
            });
        }
        emit fullRepresentationChanged(d->fullRepresentation.data());
    }

    //default d->compactRepresentation is a simple icon provided by the shell package
    if (!d->compactRepresentation) {
        d->compactRepresentation = new QQmlComponent(engine, this);
        d->compactRepresentation.data()->loadUrl(QUrl::fromLocalFile(d->coronaPackage.filePath("defaultcompactrepresentation")));
        emit compactRepresentationChanged(d->compactRepresentation.data());
    }

    //default d->compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!d->compactRepresentationExpander) {
        d->compactRepresentationExpander = new QQmlComponent(engine, this);
        d->compactRepresentationExpander.data()->loadUrl(QUrl::fromLocalFile(d->coronaPackage.filePath("compactapplet")));
    }

}

Plasma::Package AppletQuickItem::appletPackage() const
{
    return d->appletPackage;
}

void AppletQuickItem::setAppletPackage(const Plasma::Package &package)
{
    d->appletPackage = package;
}

Plasma::Package AppletQuickItem::coronaPackage() const
{
    return d->coronaPackage;
}

void AppletQuickItem::setCoronaPackage(const Plasma::Package &package)
{
    d->coronaPackage = package;
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

void AppletQuickItem::setSwitchHeight(int width)
{
    if (d->switchHeight == width) {
        return;
    }

    d->switchHeight = width;
    emit switchHeightChanged(width);
}

QQmlComponent *AppletQuickItem::compactRepresentation()
{
    return d->compactRepresentation.data();
}

void AppletQuickItem::setCompactRepresentation(QQmlComponent *component)
{
    if (d->compactRepresentation.data() == component) {
        return;
    }

    d->compactRepresentation = component;
    emit compactRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::fullRepresentation()
{
    return d->fullRepresentation.data();
}

void AppletQuickItem::setFullRepresentation(QQmlComponent *component)
{
    if (d->fullRepresentation.data() == component) {
        return;
    }

    d->fullRepresentation = component;
    emit fullRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::preferredRepresentation()
{
    return d->preferredRepresentation.data();
}

void AppletQuickItem::setPreferredRepresentation(QQmlComponent *component)
{
    if (d->preferredRepresentation.data() == component) {
        return;
    }

    d->preferredRepresentation = component;
    emit preferredRepresentationChanged(component);
}

bool AppletQuickItem::isExpanded() const
{
    return d->expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (d->applet->isContainment()) {
        expanded = true;
    }

    //if there is no compact representation it means it's always expanded
    //Containnments are always expanded
    if (d->expanded == expanded) {
        return;
    }

    if (expanded) {
        d->createFullRepresentationItem();
        if (!d->applet->isContainment() &&
                (!d->preferredRepresentation ||
                 d->preferredRepresentation.data() != d->fullRepresentation.data())) {
            d->createCompactRepresentationExpanderItem();
        }

        KConfigGroup cg = d->applet->config();
        cg = KConfigGroup(&cg, "PopupApplet");
        const int width = cg.readEntry("DialogWidth", -1);
        const int height = cg.readEntry("DialogHeight", -1);
        if (width > 0  && height > 0) {
            d->fullRepresentationItem.data()->setProperty("width", width);
            d->fullRepresentationItem.data()->setProperty("height", height);
        }

        if (d->compactRepresentationExpanderItem) {
            d->compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant::fromValue(d->createFullRepresentationItem()));
        } else {
            d->fullRepresentationItem.data()->setProperty("parent", QVariant::fromValue(this));
        }
    }

    d->expanded = expanded;
    emit expandedChanged(expanded);
}

////////////Internals

KDeclarative::QmlObject *AppletQuickItem::qmlObject()
{
    return d->qmlObject;
}

QObject *AppletQuickItem::compactRepresentationItem()
{
    return d->compactRepresentationItem.data();
}

QObject *AppletQuickItem::fullRepresentationItem()
{
    return d->fullRepresentationItem.data();
}

void AppletQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->compactRepresentationCheckTimer.start();
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

