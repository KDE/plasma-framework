/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "popupplasmawindow.h"

#include <kwindoweffects.h>
#include <kwindowsystem.h>

#include "debug_p.h"
#include <QGuiApplication>
#include <QScreen>
#include <qnamespace.h>
#include <qtmetamacros.h>

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
    updateSlideEffect();

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

bool PopupPlasmaWindow::animated() const
{
    return m_animated;
}

void PopupPlasmaWindow::setAnimated(bool animated)
{
    m_animated = animated;
    updateSlideEffect();
    Q_EMIT animatedChanged();
}

PopupPlasmaWindow::RemoveBorders PopupPlasmaWindow::removeBorderStrategy() const
{
    return m_removeBorderStrategy;
}

void PopupPlasmaWindow::setRemoveBorderStrategy(PopupPlasmaWindow::RemoveBorders strategy)
{
    if (m_removeBorderStrategy == strategy) {
        return;
    }

    m_removeBorderStrategy = strategy;
    queuePositionUpdate(); // This will update borders as well
    Q_EMIT removeBorderStrategyChanged();
}

int PopupPlasmaWindow::margin() const
{
    return m_margin;
}

void PopupPlasmaWindow::setMargin(int margin)
{
    if (m_margin == margin) {
        return;
    }

    m_margin = margin;
    queuePositionUpdate();
    Q_EMIT marginChanged();
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

void PopupPlasmaWindow::updateSlideEffect()
{
    KWindowEffects::SlideFromLocation slideLocation = KWindowEffects::NoEdge;
    if (m_animated) {
        switch (m_popupDirection) {
        case Qt::TopEdge:
            slideLocation = KWindowEffects::BottomEdge;
            break;
        case Qt::BottomEdge:
            slideLocation = KWindowEffects::TopEdge;
            break;
        case Qt::LeftEdge:
            slideLocation = KWindowEffects::RightEdge;
            break;
        case Qt::RightEdge:
            slideLocation = KWindowEffects::LeftEdge;
            break;
        }
    }
    KWindowEffects::slideWindow(this, slideLocation, -1);
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
    placementHint.setMargin(m_margin);

    const QRect popupPosition = TransientPlacementHelper::popupRect(this, placementHint);

    if (KWindowSystem::isPlatformX11()) {
        updatePositionX11(popupPosition.topLeft());
    } else if (KWindowSystem::isPlatformWayland()) {
        updatePositionWayland(popupPosition.topLeft());
    }
    updateBorders(popupPosition);
}

void PlasmaQuick::PopupPlasmaWindow::updatePositionX11(const QPoint &position)
{
    setPosition(position);
}

void PopupPlasmaWindow::updatePositionWayland(const QPoint &position)
{
    // still update's Qt internal reference as it's used by the next dialog
    // this can be dropped when we're using true semantic positioning in the backend
    setPosition(position);

    PlasmaShellWaylandIntegration::get(this)->setPosition(position);
}

void PopupPlasmaWindow::updateBorders(const QRect &globalPosition)
{
    // disables borders for the edges that are touching the screen edge

    QScreen *screen = QGuiApplication::screenAt(globalPosition.center());
    if (!screen) {
        return;
    }
    const QRect screenGeometry = screen->geometry();

    Qt::Edges enabledBorders = Qt::LeftEdge | Qt::RightEdge | Qt::TopEdge | Qt::BottomEdge;

    if (m_margin) {
        setBorders(enabledBorders);
        return;
    }

    if (m_removeBorderStrategy & AtScreenEdges) {
        if (globalPosition.top() <= screenGeometry.top()) {
            enabledBorders.setFlag(Qt::TopEdge, false);
        }
        if (globalPosition.bottom() >= screenGeometry.bottom()) {
            enabledBorders.setFlag(Qt::BottomEdge, false);
        }
        if (globalPosition.left() <= screenGeometry.left()) {
            enabledBorders.setFlag(Qt::LeftEdge, false);
        }
        if (globalPosition.right() >= screenGeometry.right()) {
            enabledBorders.setFlag(Qt::RightEdge, false);
        }
    }
    if (m_removeBorderStrategy & AtPanelEdges) {
        switch (m_popupDirection) {
        case Qt::LeftEdge:
            enabledBorders.setFlag(Qt::RightEdge, false);
            break;
        case Qt::RightEdge:
            enabledBorders.setFlag(Qt::LeftEdge, false);
            break;
        case Qt::BottomEdge:
            enabledBorders.setFlag(Qt::TopEdge, false);
            break;
        case Qt::TopEdge:
        default:
            enabledBorders.setFlag(Qt::BottomEdge, false);
            break;
        }
    }
    setBorders(enabledBorders);
}
