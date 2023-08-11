/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "popupplasmawindow.h"
#include <kwindowsystem.h>

#include "debug_p.h"
#include "transientplacementhint_p.h"
#include "waylandintegration_p.h"

using namespace PlasmaQuick;

static Qt::Edge oppositeEdge(Qt::Edge edge)
{
    switch (edge) {
    case Qt::TopEdge:
        return Qt::BottomEdge;
    case Qt::BottomEdge:
        return Qt::TopEdge;
    case Qt::LeftEdge:
        return Qt::RightEdge;
    case Qt::RightEdge:
        return Qt::LeftEdge;
    }
    Q_UNREACHABLE();
}

PopupPlasmaWindow::PopupPlasmaWindow()
{
}

void PopupPlasmaWindow::setVisualParent(QQuickItem *item)
{
    if (item == m_visualParent) {
        return;
    }

    m_visualParent = item;
    Q_EMIT visualParentChanged();
    queuePositionUpdate();
}

QQuickItem *PopupPlasmaWindow::visualParent() const
{
    return m_visualParent;
}

Qt::Edge PopupPlasmaWindow::popupDirection() const
{
    return m_popupDirection;
}

void PopupPlasmaWindow::setPopupDirection(Qt::Edge popupDirection)
{
    if (popupDirection == m_popupDirection) {
        return;
    }
    m_popupDirection = popupDirection;

    if (isExposed()) {
        qCWarning(LOG_PLASMAQUICK) << "location should be set before showing popup window";
    }
    queuePositionUpdate();

    Q_EMIT popupDirectionChanged();
}

bool PopupPlasmaWindow::floating() const
{
    return m_floating;
}

void PopupPlasmaWindow::setFloating(bool floating)
{
    if (floating == m_floating) {
        return;
    }
    m_floating = floating;
    queuePositionUpdate();
    Q_EMIT floatingChanged();
}

bool PopupPlasmaWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        if (m_needsReposition) {
            updatePosition();
        }
        break;
    case QEvent::Show:
        updatePosition();
        break;
    case QEvent::Resize:
        updatePosition();
        break;
    default:
        break;
    }
    return PlasmaQuick::PlasmaWindow::event(event);
}

void PlasmaQuick::PopupPlasmaWindow::queuePositionUpdate()
{
    m_needsReposition = true;
}

void PlasmaQuick::PopupPlasmaWindow::updatePosition()
{
    m_needsReposition = false;

    if (!m_visualParent) {
        qCWarning(LOG_PLASMAQUICK) << "Exposed with no visual parent. Window positioning broken.";
        return;
    }
    setTransientParent(m_visualParent->window());
    TransientPlacementHint placementHint;
    QRectF parentAnchorRect = QRectF(m_visualParent->mapToScene(QPointF(0, 0)), m_visualParent->size());

    if (!m_floating) {
        // pad parentAnchorRect to the window it's in, so that the popup appears outside the panel
        // even if the tooltip area does not fill it
        if (m_popupDirection == Qt::TopEdge || m_popupDirection == Qt::BottomEdge) {
            parentAnchorRect.setTop(0);
            parentAnchorRect.setBottom(m_visualParent->window()->height());
        }
        if (m_popupDirection == Qt::LeftEdge || m_popupDirection == Qt::RightEdge) {
            parentAnchorRect.setLeft(0);
            parentAnchorRect.setRight(m_visualParent->window()->width());
        }
    }

    placementHint.setParentAnchorArea(parentAnchorRect.toRect());
    placementHint.setParentAnchor(m_popupDirection);
    placementHint.setPopupAnchor(oppositeEdge(m_popupDirection));
    placementHint.setFlipConstraintAdjustments(m_floating ? Qt::Vertical : Qt::Orientations());

    const QRect popupPosition = TransientPlacementHelper::popupRect(this, placementHint);
    resize(popupPosition.size());

    if (KWindowSystem::isPlatformX11()) {
        updatePositionX11(popupPosition.topLeft());
    } else if (KWindowSystem::isPlatformWayland()) {
        updatePositionWayland(popupPosition.topLeft());
    }
}

void PlasmaQuick::PopupPlasmaWindow::updatePositionX11(const QPoint &position)
{
    setPosition(position);
}

void PopupPlasmaWindow::updatePositionWayland(const QPoint &position)
{
    PlasmaShellWaylandIntegration::get(this)->setPosition(position);
}
