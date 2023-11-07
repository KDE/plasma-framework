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

namespace PlasmaQuick
{

class PopupPlasmaWindowPrivate
{
public:
    PopupPlasmaWindowPrivate(PopupPlasmaWindow *_q);

    void updateSlideEffect();
    void updatePosition();
    void updatePositionX11(const QPoint &position);
    void updatePositionWayland(const QPoint &position);
    void updateBorders(const QRect &globalPosition);
    static Qt::Edge oppositeEdge(Qt::Edge edge);

    PopupPlasmaWindow *q;
    QPointer<QQuickItem> m_visualParent;
    PopupPlasmaWindow::RemoveBorders m_removeBorderStrategy = PopupPlasmaWindow::Never;
    bool m_needsReposition = false;
    bool m_floating = false;
    bool m_animated = false;
    int m_margin = 0;
    Qt::Edge m_popupDirection = Qt::TopEdge;
};

PopupPlasmaWindowPrivate::PopupPlasmaWindowPrivate(PopupPlasmaWindow *_q)
    : q(_q)
{
}

void PopupPlasmaWindowPrivate::updateSlideEffect()
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
    KWindowEffects::slideWindow(q, slideLocation, -1);
}

void PopupPlasmaWindowPrivate::updatePosition()
{
    m_needsReposition = false;

    if (!m_visualParent || !m_visualParent->window()) {
        qCWarning(LOG_PLASMAQUICK) << "Exposed with no visual parent. Window positioning broken.";
        return;
    }

    if (m_visualParent->size().isEmpty()) {
        qCWarning(LOG_PLASMAQUICK) << "Exposed with a visual parent that has no size. Window positioning broken.";
        return;
    }

    q->setTransientParent(m_visualParent->window());
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

    const QRect popupPosition = TransientPlacementHelper::popupRect(q, placementHint);

    if (KWindowSystem::isPlatformX11()) {
        updatePositionX11(popupPosition.topLeft());
    } else if (KWindowSystem::isPlatformWayland()) {
        updatePositionWayland(popupPosition.topLeft());
    }
    updateBorders(popupPosition);
}

void PopupPlasmaWindowPrivate::updatePositionX11(const QPoint &position)
{
    q->setPosition(position);
}

void PopupPlasmaWindowPrivate::updatePositionWayland(const QPoint &position)
{
    // still update's Qt internal reference as it's used by the next dialog
    // this can be dropped when we're using true semantic positioning in the backend
    q->setPosition(position);

    PlasmaShellWaylandIntegration::get(q)->setPosition(position);
}

void PopupPlasmaWindowPrivate::updateBorders(const QRect &globalPosition)
{
    // disables borders for the edges that are touching the screen edge

    QScreen *screen = QGuiApplication::screenAt(globalPosition.center());
    if (!screen) {
        return;
    }
    const QRect screenGeometry = screen->geometry();

    Qt::Edges enabledBorders = Qt::LeftEdge | Qt::RightEdge | Qt::TopEdge | Qt::BottomEdge;

    if (m_margin) {
        q->setBorders(enabledBorders);
        return;
    }

    if (m_removeBorderStrategy & PopupPlasmaWindow::AtScreenEdges) {
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
    if (m_removeBorderStrategy & PopupPlasmaWindow::AtPanelEdges) {
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
    q->setBorders(enabledBorders);
}

Qt::Edge PopupPlasmaWindowPrivate::oppositeEdge(Qt::Edge edge)
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

PopupPlasmaWindow::PopupPlasmaWindow(QWindow *parent)
    : PlasmaWindow(parent)
    , d(new PopupPlasmaWindowPrivate(this))
{
}

PopupPlasmaWindow::~PopupPlasmaWindow()
{
}

void PopupPlasmaWindow::setVisualParent(QQuickItem *item)
{
    if (item == d->m_visualParent) {
        return;
    }

    d->m_visualParent = item;
    Q_EMIT visualParentChanged();
    queuePositionUpdate();
}

QQuickItem *PopupPlasmaWindow::visualParent() const
{
    return d->m_visualParent;
}

Qt::Edge PopupPlasmaWindow::popupDirection() const
{
    return d->m_popupDirection;
}

void PopupPlasmaWindow::setPopupDirection(Qt::Edge popupDirection)
{
    if (popupDirection == d->m_popupDirection) {
        return;
    }
    d->m_popupDirection = popupDirection;

    if (isExposed()) {
        qCWarning(LOG_PLASMAQUICK) << "location should be set before showing popup window";
    }
    queuePositionUpdate();
    d->updateSlideEffect();

    Q_EMIT popupDirectionChanged();
}

bool PopupPlasmaWindow::floating() const
{
    return d->m_floating;
}

void PopupPlasmaWindow::setFloating(bool floating)
{
    if (floating == d->m_floating) {
        return;
    }
    d->m_floating = floating;
    queuePositionUpdate();
    Q_EMIT floatingChanged();
}

bool PopupPlasmaWindow::animated() const
{
    return d->m_animated;
}

void PopupPlasmaWindow::setAnimated(bool animated)
{
    d->m_animated = animated;
    d->updateSlideEffect();
    Q_EMIT animatedChanged();
}

PopupPlasmaWindow::RemoveBorders PopupPlasmaWindow::removeBorderStrategy() const
{
    return d->m_removeBorderStrategy;
}

void PopupPlasmaWindow::setRemoveBorderStrategy(PopupPlasmaWindow::RemoveBorders strategy)
{
    if (d->m_removeBorderStrategy == strategy) {
        return;
    }

    d->m_removeBorderStrategy = strategy;
    queuePositionUpdate(); // This will update borders as well
    Q_EMIT removeBorderStrategyChanged();
}

int PopupPlasmaWindow::margin() const
{
    return d->m_margin;
}

void PopupPlasmaWindow::setMargin(int margin)
{
    if (d->m_margin == margin) {
        return;
    }

    d->m_margin = margin;
    queuePositionUpdate();
    Q_EMIT marginChanged();
}

bool PopupPlasmaWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        if (d->m_needsReposition) {
            d->updatePosition();
        }
        break;
    case QEvent::Show:
        d->updatePosition();
        break;
    case QEvent::Resize:
        d->updatePosition();
        break;
    default:
        break;
    }
    return PlasmaQuick::PlasmaWindow::event(event);
}

void PlasmaQuick::PopupPlasmaWindow::queuePositionUpdate()
{
    d->m_needsReposition = true;
}

}

#include "moc_popupplasmawindow.cpp"
