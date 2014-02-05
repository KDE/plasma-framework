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

QHash<QObject *, AppletQuickItem *> AppletQuickItem::s_rootObjects = QHash<QObject *, AppletQuickItem *>();

AppletQuickItem::AppletQuickItem(Plasma::Applet *applet, QQuickItem *parent)
    : QQuickItem(parent),
      m_switchWidth(-1),
      m_switchHeight(-1),
      m_applet(applet),
      m_expanded(false)
{
    m_appletPackage = m_applet->package();
    if (m_applet && m_applet->containment() && m_applet->containment()->corona()) {
        m_coronaPackage = m_applet->containment()->corona()->package();
    }

    m_compactRepresentationCheckTimer.setSingleShot(true);
    m_compactRepresentationCheckTimer.setInterval(250);
    connect (&m_compactRepresentationCheckTimer, SIGNAL(timeout()),
             this, SLOT(compactRepresentationCheck()));
    m_compactRepresentationCheckTimer.start();

    m_fullRepresentationResizeTimer.setSingleShot(true);
    m_fullRepresentationResizeTimer.setInterval(250);
    connect (&m_fullRepresentationResizeTimer, &QTimer::timeout,
             [=]() {
                KConfigGroup cg = m_applet->config();
                cg = KConfigGroup(&cg, "PopupApplet");
                cg.writeEntry("DialogWidth", m_fullRepresentationItem.data()->property("width").toInt());
                cg.writeEntry("DialogHeight", m_fullRepresentationItem.data()->property("height").toInt());
            }
    );



    m_qmlObject = new KDeclarative::QmlObject(this);
    m_qmlObject->setInitializationDelayed(true);

    // set the graphicObject dynamic property on applet
    m_applet->setProperty("_plasma_graphicObject", QVariant::fromValue(this));
    setProperty("_plasma_applet", QVariant::fromValue(applet));
}

AppletQuickItem::~AppletQuickItem()
{
    //Here the order is important
    delete m_compactRepresentationItem.data();
    delete m_fullRepresentationItem.data();
    delete m_compactRepresentationExpanderItem.data();

    s_rootObjects.remove(m_qmlObject->engine());
}

Plasma::Applet *AppletQuickItem::applet() const
{
    return m_applet;
}

void AppletQuickItem::init()
{
    if (s_rootObjects.contains(this)) {
        return;
    }

    s_rootObjects[m_qmlObject->engine()] = this;

    Q_ASSERT(m_applet);

    //Initialize the main QML file
    QQmlEngine *engine = m_qmlObject->engine();

    PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine, m_applet->package());
    interceptor->addAllowedPath(m_coronaPackage.path());
    engine->setUrlInterceptor(interceptor);

    m_qmlObject->setSource(QUrl::fromLocalFile(m_applet->package().filePath("mainscript")));

    if (!engine || !engine->rootContext() || !engine->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
        }
        reason = i18n("Error loading QML file: %1", reason);

        m_qmlObject->setSource(QUrl::fromLocalFile(m_coronaPackage.filePath("appleterror")));
        m_qmlObject->completeInitialization();


        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return;
        } else {
            m_qmlObject->rootObject()->setProperty("reason", reason);
        }

        m_applet->setLaunchErrorMessage(reason);
    }

    engine->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    initialProperties["width"] = width();
    initialProperties["height"] = height();
    m_qmlObject->completeInitialization(initialProperties);

    //default fullrepresentation is our root main component, if none specified
    if (!m_fullRepresentation) {
        m_fullRepresentation = m_qmlObject->mainComponent();
        m_fullRepresentationItem = m_qmlObject->rootObject();
        emit fullRepresentationChanged(m_fullRepresentation.data());
    }

    //default m_compactRepresentation is a simple icon provided by the shell package
    if (!m_compactRepresentation) {
        m_compactRepresentation = new QQmlComponent(engine, this);
        m_compactRepresentation.data()->loadUrl(QUrl::fromLocalFile(m_coronaPackage.filePath("defaultcompactrepresentation")));
        emit compactRepresentationChanged(m_compactRepresentation.data());
    }

    //default m_compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!m_compactRepresentationExpander) {
        m_compactRepresentationExpander = new QQmlComponent(engine, this);
        m_compactRepresentationExpander.data()->loadUrl(QUrl::fromLocalFile(m_coronaPackage.filePath("compactapplet")));
        emit compactRepresentationExpanderItemChanged(m_compactRepresentationExpander.data());
    }

}

Plasma::Package AppletQuickItem::appletPackage() const
{
    return m_appletPackage;
}

void AppletQuickItem::setAppletPackage(const Plasma::Package &package)
{
    m_appletPackage = package;
}

Plasma::Package AppletQuickItem::coronaPackage() const
{
    return m_coronaPackage;
}

void AppletQuickItem::setCoronaPackage(const Plasma::Package &package)
{
    m_coronaPackage = package;
}

int AppletQuickItem::switchWidth() const
{
    return m_switchWidth;
}

void AppletQuickItem::setSwitchWidth(int width)
{
    if (m_switchWidth == width) {
        return;
    }

    m_switchWidth = width;
    emit switchWidthChanged(width);
}

int AppletQuickItem::switchHeight() const
{
    return m_switchHeight;
}

void AppletQuickItem::setSwitchHeight(int width)
{
    if (m_switchHeight == width) {
        return;
    }

    m_switchHeight = width;
    emit switchHeightChanged(width);
}

QQmlComponent *AppletQuickItem::compactRepresentation()
{
    return m_compactRepresentation.data();
}

void AppletQuickItem::setCompactRepresentation(QQmlComponent *component)
{
    if (m_compactRepresentation.data() == component) {
        return;
    }

    m_compactRepresentation = component;
    emit compactRepresentationChanged(component);
}


QQmlComponent *AppletQuickItem::fullRepresentation()
{
    return m_fullRepresentation.data();
}

void AppletQuickItem::setFullRepresentation(QQmlComponent *component)
{
    if (m_fullRepresentation.data() == component) {
        return;
    }

    m_fullRepresentation = component;
    emit fullRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::preferredRepresentation()
{
    return m_preferredRepresentation.data();
}

void AppletQuickItem::setPreferredRepresentation(QQmlComponent *component)
{
    if (m_preferredRepresentation.data() == component) {
        return;
    }

    m_preferredRepresentation = component;
    emit preferredRepresentationChanged(component);
}

bool AppletQuickItem::isExpanded() const
{
    return m_expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (m_applet->isContainment()) {
        expanded = true;
    }

    //if there is no compact representation it means it's always expanded
    //Containnments are always expanded
    if (m_expanded == expanded) {
        return;
    }

    createFullRepresentationItem();
    createCompactRepresentationExpanderItem();

    m_expanded = expanded;
    emit expandedChanged(expanded);
}

////////////Internals


KDeclarative::QmlObject *AppletQuickItem::qmlObject()
{
    return m_qmlObject;
}

QObject *AppletQuickItem::compactRepresentationItem()
{
    return m_compactRepresentationItem.data();
}

QObject *AppletQuickItem::fullRepresentationItem()
{
    return m_fullRepresentationItem.data();
}

QObject *AppletQuickItem::compactRepresentationExpanderItem()
{
    return m_compactRepresentationExpanderItem.data();
}



QObject *AppletQuickItem::createCompactRepresentationItem()
{
    if (!m_compactRepresentation) {
        return 0;
    }

    if (m_compactRepresentationItem) {
        return m_compactRepresentationItem.data();
    }

    m_compactRepresentationItem = m_qmlObject->createObjectFromComponent(m_compactRepresentation.data(), QtQml::qmlContext(m_qmlObject->rootObject()));

    emit compactRepresentationItemChanged(m_compactRepresentationItem.data());

    return m_compactRepresentationItem.data();
}

QObject *AppletQuickItem::createFullRepresentationItem()
{
    if (m_fullRepresentationItem) {
        return m_fullRepresentationItem.data();
    }

    if (m_fullRepresentation) {
        m_fullRepresentationItem = m_qmlObject->createObjectFromComponent(m_fullRepresentation.data(), QtQml::qmlContext(m_qmlObject->rootObject()));
    } else {
        m_fullRepresentation = m_qmlObject->mainComponent();
        m_fullRepresentationItem = m_qmlObject->rootObject();
        emit fullRepresentationChanged(m_fullRepresentation.data());
    }

    QQuickItem *graphicsObj = qobject_cast<QQuickItem *>(m_fullRepresentationItem.data());
    connect (graphicsObj, &QQuickItem::widthChanged, [=]() {
        m_fullRepresentationResizeTimer.start();
    });
    connect (graphicsObj, &QQuickItem::heightChanged, [=]() {
        m_fullRepresentationResizeTimer.start();
    });

    emit fullRepresentationItemChanged(m_fullRepresentationItem.data());

    return m_fullRepresentationItem.data();
}

QObject *AppletQuickItem::createCompactRepresentationExpanderItem()
{
    if (!m_compactRepresentationExpander) {
        return 0;
    }

    if (m_compactRepresentationExpanderItem) {
        return m_compactRepresentationExpanderItem.data();
    }

    m_compactRepresentationExpanderItem = m_qmlObject->createObjectFromComponent(m_compactRepresentationExpander.data(), QtQml::qmlContext(m_qmlObject->rootObject()));


    m_compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant::fromValue(createCompactRepresentationItem()));
    m_compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant::fromValue(createFullRepresentationItem()));

    emit compactRepresentationExpanderItemChanged(m_compactRepresentationExpanderItem.data());

    return m_compactRepresentationExpanderItem.data();
}

void AppletQuickItem::connectLayoutAttached(QObject *item)
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
    QQmlProperty p(this, "Layout.minimumWidth", QtQml::qmlContext(m_qmlObject->rootObject()));

    QObject *ownLayout = 0;

    foreach (QObject *child, children()) {
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
    if (m_representationLayout.data() == layout) {
        return;
    }

    if (m_representationLayout) {
        disconnect(m_representationLayout.data(), 0, this, 0);
    }

    //Here we can't use the new connect syntax because we can't link against QtQuick layouts
    connect(layout, SIGNAL(minimumWidthChanged()),
            this, SLOT(minimumWidthChanged()));
    connect(layout, SIGNAL(minimumHeightChanged()),
            this, SLOT(minimumHeightChanged()));

    connect(layout, SIGNAL(preferredWidthChanged()),
            this, SLOT(preferredWidthChanged()));
    connect(layout, SIGNAL(preferredHeightChanged()),
            this, SLOT(preferredHeightChanged()));

    connect(layout, SIGNAL(maximumWidthChanged()),
            this, SLOT(maximumWidthChanged()));
    connect(layout, SIGNAL(maximumHeightChanged()),
            this, SLOT(maximumHeightChanged()));

    connect(layout, SIGNAL(fillWidthChanged()),
            this, SLOT(fillWidthChanged()));
    connect(layout, SIGNAL(fillHeightChanged()),
            this, SLOT(fillHeightChanged()));

    m_representationLayout = layout;
    m_ownLayout = ownLayout;

    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");
}

void AppletQuickItem::propagateSizeHint(const QByteArray &layoutProperty)
{
    if (m_ownLayout && m_representationLayout) {
        m_ownLayout.data()->setProperty(layoutProperty, m_representationLayout.data()->property(layoutProperty));
    }
}

void AppletQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    m_compactRepresentationCheckTimer.start();
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


//// Slots

void AppletQuickItem::compactRepresentationCheck()
{
    //ignore 0,0 sizes;
    if (width() <= 0 && height() <= 0) {
        return;
    }

    bool full = false;

    if (m_applet->isContainment()) {
        full = true;

    } else {
        if (m_switchWidth > 0 && m_switchHeight > 0) {
            full = width() > m_switchWidth && height() > m_switchHeight;
        //if a size to switch wasn't set, determine what representation to always chose
        } else {
            //preferred representation set?
            if (m_preferredRepresentation) {
                full = m_preferredRepresentation.data() == m_fullRepresentation.data();
            //Otherwise, base on FormFactor
            } else {
                full = (m_applet->formFactor() != Plasma::Types::Horizontal && m_applet->formFactor() != Plasma::Types::Vertical);
            }
        }

        if ((full && m_fullRepresentationItem && m_fullRepresentationItem.data() == m_currentRepresentationItem.data()) ||
            (!full && m_compactRepresentationItem && m_compactRepresentationItem.data() == m_currentRepresentationItem.data())
        ) {
            return;
        }
    }

    //Expanded
    if (full) {
        QQuickItem *item = qobject_cast<QQuickItem *>(createFullRepresentationItem());

        if (item) {
            item->setParentItem(this);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(m_qmlObject->rootObject()), item, "parent");
                QQmlProperty prop(item, "anchors.fill");
                prop.write(expr.evaluate());
            }
            if (m_compactRepresentationItem) {
                m_compactRepresentationItem.data()->setProperty("visible", false);
            }
            if (m_compactRepresentationExpanderItem) {
                m_compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant());
                m_compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant());
            }

            m_currentRepresentationItem = item;
            connectLayoutAttached(item);
        }

    //Icon
    } else {
        QQuickItem *compactItem = qobject_cast<QQuickItem *>(createCompactRepresentationItem());

        if (compactItem) {
            //set the root item as the main visible item
            compactItem->setParentItem(this);
            compactItem->setVisible(true);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(m_qmlObject->rootObject()), compactItem, "parent");
                QQmlProperty prop(compactItem, "anchors.fill");
                prop.write(expr.evaluate());
            }

            if (m_fullRepresentationItem) {
                m_fullRepresentationItem.data()->setProperty("parent", QVariant());
            }

            if (m_compactRepresentationExpanderItem) {
                m_compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant::fromValue(compactItem));
                m_compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant::fromValue(createFullRepresentationItem()));
            }

            m_currentRepresentationItem = compactItem;
            connectLayoutAttached(compactItem);
        }
    }
}

void AppletQuickItem::minimumWidthChanged()
{
    propagateSizeHint("minimumWidth");
}

void AppletQuickItem::minimumHeightChanged()
{
    propagateSizeHint("minimumHeight");
}

void AppletQuickItem::preferredWidthChanged()
{
    propagateSizeHint("preferredWidth");
}

void AppletQuickItem::preferredHeightChanged()
{
    propagateSizeHint("preferredHeight");
}

void AppletQuickItem::maximumWidthChanged()
{
    propagateSizeHint("maximumWidth");
}

void AppletQuickItem::maximumHeightChanged()
{
    propagateSizeHint("maximumHeight");
}

void AppletQuickItem::fillWidthChanged()
{
    propagateSizeHint("fillWidth");
}

void AppletQuickItem::fillHeightChanged()
{
    propagateSizeHint("fillHeight");
}


#include "moc_appletquickitem.cpp"

