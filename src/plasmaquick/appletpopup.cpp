/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletpopup.h"

#include <QGuiApplication>
#include <QQmlProperty>
#include <qpa/qplatformwindow.h> // for QWINDOWSIZE_MAX

#include <KConfigGroup>
#include <KWindowSystem>

#include "applet.h"
#include "appletquickitem.h"
#include "edgeeventforwarder.h"
#include "waylandintegration_p.h"
#include "windowresizehandler.h"

// used in detecting if focus passes to config UI
#include "configview.h"
#include "sharedqmlengine.h"

// This is a proxy object that connects to the Layout attached property of an item
// it also handles turning properties to proper defaults
// we need a wrapper as QQmlProperty can't disconnect

namespace PlasmaQuick
{

class LayoutChangedProxy : public QObject
{
    Q_OBJECT
public:
    LayoutChangedProxy(QQuickItem *item);
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize implicitSize() const;
Q_SIGNALS:
    void implicitSizeChanged();
    void minimumSizeChanged();
    void maximumSizeChanged();

private:
    QQmlProperty m_minimumWidth;
    QQmlProperty m_maximumWidth;
    QQmlProperty m_minimumHeight;
    QQmlProperty m_maximumHeight;
    QQmlProperty m_preferredWidth;
    QQmlProperty m_preferredHeight;
    QPointer<QQuickItem> m_item;
};
}

using namespace PlasmaQuick;

AppletPopup::AppletPopup()
{
    setAnimated(true);
    setFlags(flags() | Qt::Dialog);

    PlasmaShellWaylandIntegration::get(this)->setRole(QtWayland::org_kde_plasma_surface::role::role_appletpopup);

    auto edgeForwarder = new EdgeEventForwarder(this);
    edgeForwarder->setMargins(margins());
    connect(this, &PlasmaWindow::marginsChanged, this, [edgeForwarder, this]() {
        edgeForwarder->setMargins(margins());
    });
    // edges that have a border are not on a screen edge
    // we want to forward on sides touching screen edges
    edgeForwarder->setActiveEdges(~borders());
    connect(this, &PlasmaWindow::bordersChanged, this, [edgeForwarder, this]() {
        edgeForwarder->setActiveEdges(~borders());
    });

    auto windowResizer = new WindowResizeHandler(this);
    windowResizer->setMargins(margins());
    connect(this, &PlasmaWindow::marginsChanged, this, [windowResizer, this]() {
        windowResizer->setMargins(margins());
    });
    windowResizer->setActiveEdges(borders());
    connect(this, &PlasmaWindow::bordersChanged, this, [windowResizer, this]() {
        windowResizer->setActiveEdges(borders());
    });

    connect(this, &PlasmaWindow::mainItemChanged, this, &AppletPopup::onMainItemChanged);
    connect(this, &PlasmaWindow::marginsChanged, this, &AppletPopup::updateMaxSize);
    connect(this, &PlasmaWindow::marginsChanged, this, &AppletPopup::updateSize);
    connect(this, &PlasmaWindow::marginsChanged, this, &AppletPopup::updateMinSize);
}

AppletPopup::~AppletPopup()
{
}

QQuickItem *AppletPopup::appletInterface() const
{
    return m_appletInterface.data();
}

void AppletPopup::setAppletInterface(QQuickItem *appletInterface)
{
    if (appletInterface == m_appletInterface) {
        return;
    }

    m_appletInterface = qobject_cast<AppletQuickItem *>(appletInterface);
    m_sizeExplicitlySetFromConfig = false;

    if (m_appletInterface) {
        KConfigGroup config = m_appletInterface->applet()->config();
        QSize size;
        size.rwidth() = config.readEntry("popupWidth", 0);
        size.rheight() = config.readEntry("popupHeight", 0);
        if (size.isValid()) {
            m_sizeExplicitlySetFromConfig = true;
            resize(size.grownBy(margins()));
            return;
        }
    }

    Q_EMIT appletInterfaceChanged();
}

bool AppletPopup::hideOnWindowDeactivate() const
{
    return m_hideOnWindowDeactivate;
}

void AppletPopup::setHideOnWindowDeactivate(bool hideOnWindowDeactivate)
{
    if (hideOnWindowDeactivate == m_hideOnWindowDeactivate) {
        return;
    }
    m_hideOnWindowDeactivate = hideOnWindowDeactivate;
    Q_EMIT hideOnWindowDeactivateChanged();
}

void AppletPopup::hideEvent(QHideEvent *event)
{
    // Persist the size if this contains an applet
    if (m_appletInterface) {
        KConfigGroup config = m_appletInterface->applet()->config();
        // save size without margins, so we're robust against theme changes
        const QSize popupSize = size().shrunkBy(margins());
        config.writeEntry("popupWidth", popupSize.width());
        config.writeEntry("popupHeight", popupSize.height());
        config.sync();
    }

    PopupPlasmaWindow::hideEvent(event);
}

void AppletPopup::focusOutEvent(QFocusEvent *ev)
{
    if (m_hideOnWindowDeactivate) {
        bool parentHasFocus = false;

        QWindow *parentWindow = transientParent();

        while (parentWindow) {
            if (parentWindow->isActive() && !(parentWindow->flags() & Qt::WindowDoesNotAcceptFocus)) {
                parentHasFocus = true;
                break;
            }

            parentWindow = parentWindow->transientParent();
        }

        const QWindow *focusWindow = QGuiApplication::focusWindow();
        bool childHasFocus = focusWindow && ((focusWindow->isActive() && isAncestorOf(focusWindow)) || (focusWindow->type() & Qt::Popup) == Qt::Popup);

        const bool viewClicked = qobject_cast<const PlasmaQuick::SharedQmlEngine *>(focusWindow) || qobject_cast<const ConfigView *>(focusWindow);

        if (viewClicked || (!parentHasFocus && !childHasFocus)) {
            setVisible(false);
        }
    }

    PopupPlasmaWindow::focusOutEvent(ev);
}

void AppletPopup::onMainItemChanged()
{
    QQuickItem *mainItem = PlasmaWindow::mainItem();
    if (!mainItem) {
        m_layoutChangedProxy.reset();
        return;
    }

    // update window to mainItem size hints
    m_layoutChangedProxy.reset(new LayoutChangedProxy(mainItem));
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::maximumSizeChanged, this, &AppletPopup::updateMaxSize);
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::minimumSizeChanged, this, &AppletPopup::updateMinSize);
    connect(m_layoutChangedProxy.data(), &LayoutChangedProxy::implicitSizeChanged, this, &AppletPopup::updateSize);

    updateMinSize();
    updateMaxSize();
    updateSize();
}

void AppletPopup::updateMinSize()
{
    if (!m_layoutChangedProxy) {
        return;
    }
    setMinimumSize(m_layoutChangedProxy->minimumSize().grownBy(margins()));
}

void AppletPopup::updateMaxSize()
{
    if (!m_layoutChangedProxy) {
        return;
    }
    setMaximumSize(m_layoutChangedProxy->maximumSize().grownBy(margins()));
}

void AppletPopup::updateSize()
{
    if (m_sizeExplicitlySetFromConfig) {
        return;
    }
    if (!m_layoutChangedProxy) {
        return;
    }
    resize(m_layoutChangedProxy->implicitSize().grownBy(margins()));
}

LayoutChangedProxy::LayoutChangedProxy(QQuickItem *item)
    : m_item(item)
{
    m_minimumWidth = QQmlProperty(item, QStringLiteral("Layout.minimumWidth"), qmlContext(item));
    m_minimumHeight = QQmlProperty(item, QStringLiteral("Layout.minimumHeight"), qmlContext(item));
    m_maximumWidth = QQmlProperty(item, QStringLiteral("Layout.maximumWidth"), qmlContext(item));
    m_maximumHeight = QQmlProperty(item, QStringLiteral("Layout.maximumHeight"), qmlContext(item));
    m_preferredWidth = QQmlProperty(item, QStringLiteral("Layout.preferredWidth"), qmlContext(item));
    m_preferredHeight = QQmlProperty(item, QStringLiteral("Layout.preferredHeight"), qmlContext(item));

    m_minimumWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::minimumSizeChanged).methodIndex());
    m_minimumHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::minimumSizeChanged).methodIndex());
    m_maximumWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::maximumSizeChanged).methodIndex());
    m_maximumHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::maximumSizeChanged).methodIndex());
    m_preferredWidth.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::implicitSizeChanged).methodIndex());
    m_preferredHeight.connectNotifySignal(this, QMetaMethod::fromSignal(&LayoutChangedProxy::implicitSizeChanged).methodIndex());
    connect(item, &QQuickItem::implicitWidthChanged, this, &LayoutChangedProxy::implicitSizeChanged);
    connect(item, &QQuickItem::implicitHeightChanged, this, &LayoutChangedProxy::implicitSizeChanged);
}

QSize LayoutChangedProxy::maximumSize() const
{
    QSize size(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX);
    qreal width = m_maximumWidth.read().toReal();
    if (qIsFinite(width) && width > 0) {
        size.setWidth(width);
    }
    qreal height = m_maximumHeight.read().toReal();
    if (qIsFinite(height) && height > 0) {
        size.setHeight(height);
    }

    return size;
}

QSize LayoutChangedProxy::implicitSize() const
{
    QSize size(200, 200);

    // Layout.preferredSize has precedent over implicit in layouts
    // so mimic that behaviour here
    if (m_item) {
        size = QSize(m_item->implicitWidth(), m_item->implicitHeight());
    }
    qreal width = m_preferredWidth.read().toReal();
    if (qIsFinite(width) && width > 0) {
        size.setWidth(width);
    }
    qreal height = m_preferredHeight.read().toReal();
    if (qIsFinite(height) && height > 0) {
        size.setHeight(height);
    }
    return size;
}

QSize LayoutChangedProxy::minimumSize() const
{
    QSize size(0, 0);
    qreal width = m_minimumWidth.read().toReal();
    if (qIsFinite(width) && width > 0) {
        size.setWidth(width);
    }
    qreal height = m_minimumHeight.read().toReal();
    if (qIsFinite(height) && height > 0) {
        size.setHeight(height);
    }

    return size;
}

#include "appletpopup.moc"
