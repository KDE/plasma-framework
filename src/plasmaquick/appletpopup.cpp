/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletpopup.h"

#include <QGuiApplication>

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
    updateSize();
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

int AppletPopup::implicitWidth() const
{
    return m_implicitWidth;
}

void AppletPopup::setImplicitWidth(int implicitWidth)
{
    if (implicitWidth == m_implicitWidth) {
        return;
    }
    m_implicitWidth = implicitWidth;
    updateSize();
    Q_EMIT implicitWidthChanged();
}

int AppletPopup::implicitHeight() const
{
    return m_implicitHeight;
}

void AppletPopup::setImplicitHeight(int implicitHeight)
{
    if (implicitHeight == m_implicitHeight) {
        return;
    }
    m_implicitHeight = implicitHeight;
    updateSize();
    Q_EMIT implicitHeightChanged();
}

void AppletPopup::updateSize()
{
    if (m_appletInterface) {
        KConfigGroup config = m_appletInterface->applet()->config();
        QSize size;
        size.rwidth() = config.readEntry("popupWidth", 0);
        size.rheight() = config.readEntry("popupHeight", 0);
        if (size.isValid()) {
            resize(size.grownBy(margins()));
            return;
        }
    }
    resize(implicitWidth(), implicitHeight());
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
