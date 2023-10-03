/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transientplacementhint_p.h"
#include <QSharedData>

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

// This class is proposed for Qt6.something, but it's not there yet.
// keep as an implementation detail, and then drop eventually (famous last words)

class TransientPlacementHintPrivate : public QSharedData
{
public:
    QRect parentAnchorRect;
    Qt::Edges parentAnchor = Qt::BottomEdge | Qt::RightEdge;
    Qt::Edges popupAnchor = Qt::TopEdge | Qt::LeftEdge;
    Qt::Orientations slideConstraintAdjustments = Qt::Horizontal | Qt::Vertical;
    Qt::Orientations flipConstraintAdjustments;
    int margin = 0;
};
/*!
 * Constructs a new QTransientPlacementHint
 */
TransientPlacementHint::TransientPlacementHint()
    : d(new TransientPlacementHintPrivate)
{
}

TransientPlacementHint::~TransientPlacementHint()
{
}

TransientPlacementHint::TransientPlacementHint(const TransientPlacementHint &other)
{
    d = other.d;
}
TransientPlacementHint &TransientPlacementHint::operator=(const TransientPlacementHint &other)
{
    d = other.d;
    return *this;
}

bool TransientPlacementHint::isValid() const
{
    return d->parentAnchorRect.isValid();
}

void TransientPlacementHint::setParentAnchorArea(const QRect &parentAnchorRect)
{
    d->parentAnchorRect = parentAnchorRect;
}

QRect TransientPlacementHint::parentAnchorArea() const
{
    return d->parentAnchorRect;
}

void TransientPlacementHint::setParentAnchor(Qt::Edges parentAnchor)
{
    d->parentAnchor = parentAnchor;
}

Qt::Edges TransientPlacementHint::parentAnchor() const
{
    return d->parentAnchor;
}

void TransientPlacementHint::setPopupAnchor(Qt::Edges popupAnchor)
{
    d->popupAnchor = popupAnchor;
}

Qt::Edges TransientPlacementHint::popupAnchor() const
{
    return d->popupAnchor;
}

void TransientPlacementHint::setSlideConstraintAdjustments(Qt::Orientations slideConstraintAdjustments)
{
    d->slideConstraintAdjustments = slideConstraintAdjustments;
}

Qt::Orientations TransientPlacementHint::slideConstraintAdjustments() const
{
    return d->slideConstraintAdjustments;
}

void TransientPlacementHint::setFlipConstraintAdjustments(Qt::Orientations flipConstraintAdjustments)
{
    d->flipConstraintAdjustments = flipConstraintAdjustments;
}

Qt::Orientations TransientPlacementHint::flipConstraintAdjustments() const
{
    return d->flipConstraintAdjustments;
}

int TransientPlacementHint::margin() const
{
    return d->margin;
}

void TransientPlacementHint::setMargin(int margin)
{
    d->margin = margin;
}

static QPoint popupPosition(const QRect &anchorRect, const Qt::Edges parentAnchor, const Qt::Edges popupAnchor, const QSize &popupSize)
{
    QPoint anchorPoint;
    switch (parentAnchor & (Qt::LeftEdge | Qt::RightEdge)) {
    case Qt::LeftEdge:
        anchorPoint.setX(anchorRect.x());
        break;
    case Qt::RightEdge:
        anchorPoint.setX(anchorRect.x() + anchorRect.width());
        break;
    default:
        anchorPoint.setX(qRound(anchorRect.x() + anchorRect.width() / 2.0));
    }
    switch (parentAnchor & (Qt::TopEdge | Qt::BottomEdge)) {
    case Qt::TopEdge:
        anchorPoint.setY(anchorRect.y());
        break;
    case Qt::BottomEdge:
        anchorPoint.setY(anchorRect.y() + anchorRect.height());
        break;
    default:
        anchorPoint.setY(qRound(anchorRect.y() + anchorRect.height() / 2.0));
    }
    // calculate where the top left point of the popup will end up with the applied popup anchor
    QPoint popupPosAdjust;
    switch (popupAnchor & (Qt::LeftEdge | Qt::RightEdge)) {
    case Qt::LeftEdge:
        popupPosAdjust.setX(0);
        break;
    case Qt::RightEdge:
        popupPosAdjust.setX(-popupSize.width());
        break;
    default:
        popupPosAdjust.setX(qRound(-popupSize.width() / 2.0));
    }
    switch (popupAnchor & (Qt::TopEdge | Qt::BottomEdge)) {
    case Qt::TopEdge:
        popupPosAdjust.setY(0);
        break;
    case Qt::BottomEdge:
        popupPosAdjust.setY(-popupSize.height());
        break;
    default:
        popupPosAdjust.setY(qRound(-popupSize.height() / 2.0));
    }
    return anchorPoint + popupPosAdjust;
}

QRect TransientPlacementHelper::popupRect(QWindow *w, const TransientPlacementHint &placement)
{
    Q_ASSERT(placement.isValid());
    QScreen *screen = nullptr;
    QRect globalParentAnchorRect = placement.parentAnchorArea();
    if (w->transientParent()) {
        globalParentAnchorRect = globalParentAnchorRect.translated(w->transientParent()->position());
        screen = w->transientParent()->screen();
    }

    const QMargins margin(placement.margin(), placement.margin(), placement.margin(), placement.margin());
    QSize paddedWindowSize = w->size().grownBy(margin);
    QRect popupRect = QRect(popupPosition(globalParentAnchorRect, placement.parentAnchor(), placement.popupAnchor(), paddedWindowSize), paddedWindowSize);

    if (!screen)
        screen = qApp->screenAt(globalParentAnchorRect.center());
    if (!screen)
        screen = qApp->primaryScreen();

    const QRect screenArea = screen->geometry();

    auto inScreenArea = [screenArea](const QRect &target, Qt::Edges edges = Qt::LeftEdge | Qt::RightEdge | Qt::TopEdge | Qt::BottomEdge) -> bool {
        if (edges & Qt::LeftEdge && target.left() < screenArea.left()) {
            return false;
        }
        if (edges & Qt::TopEdge && target.top() < screenArea.top()) {
            return false;
        }
        if (edges & Qt::RightEdge && target.right() > screenArea.right()) {
            return false;
        }
        if (edges & Qt::BottomEdge && target.bottom() > screenArea.bottom()) {
            return false;
        }
        return true;
    };

    // if that fits, we don't need to do anything
    if (inScreenArea(popupRect)) {
        return popupRect;
    }
    // Otherwise,
    if (placement.flipConstraintAdjustments() & Qt::Horizontal) {
        if (!inScreenArea(popupRect, Qt::LeftEdge | Qt::RightEdge)) {
            // flip both edges (if either bit is set, XOR both)
            auto flippedParentAnchor = placement.parentAnchor();
            if (flippedParentAnchor & (Qt::LeftEdge | Qt::RightEdge)) {
                flippedParentAnchor ^= (Qt::LeftEdge | Qt::RightEdge);
            }
            auto flippedPopupAnchor = placement.popupAnchor();
            if (flippedPopupAnchor & (Qt::LeftEdge | Qt::RightEdge)) {
                flippedPopupAnchor ^= (Qt::LeftEdge | Qt::RightEdge);
            }
            QRect flippedPopupRect = QRect(popupPosition(globalParentAnchorRect, flippedParentAnchor, flippedPopupAnchor, w->size()), w->size());
            // if it still doesn't fit we should continue with the unflipped version
            if (inScreenArea(flippedPopupRect, Qt::LeftEdge | Qt::RightEdge)) {
                popupRect.moveLeft(flippedPopupRect.left());
            }
        }
    }
    if (placement.slideConstraintAdjustments() & Qt::Horizontal) {
        if (!inScreenArea(popupRect, Qt::LeftEdge)) {
            popupRect.moveLeft(screenArea.left());
        }
        if (!inScreenArea(popupRect, Qt::RightEdge)) {
            popupRect.moveRight(screenArea.right());
        }
    }
    if (placement.flipConstraintAdjustments() & Qt::Vertical) {
        if (!inScreenArea(popupRect, Qt::TopEdge | Qt::BottomEdge)) {
            // flip both edges (if either bit is set, XOR both)
            auto flippedParentAnchor = placement.parentAnchor();
            if (flippedParentAnchor & (Qt::TopEdge | Qt::BottomEdge)) {
                flippedParentAnchor ^= (Qt::TopEdge | Qt::BottomEdge);
            }
            auto flippedPopupAnchor = placement.popupAnchor();
            if (flippedPopupAnchor & (Qt::TopEdge | Qt::BottomEdge)) {
                flippedPopupAnchor ^= (Qt::TopEdge | Qt::BottomEdge);
            }
            QRect flippedPopupRect = QRect(popupPosition(globalParentAnchorRect, flippedParentAnchor, flippedPopupAnchor, w->size()), w->size());
            // if it still doesn't fit we should continue with the unflipped version
            if (inScreenArea(flippedPopupRect, Qt::TopEdge | Qt::BottomEdge)) {
                popupRect.moveTop(flippedPopupRect.top());
            }
        }
    }
    if (placement.slideConstraintAdjustments() & Qt::Vertical) {
        if (!inScreenArea(popupRect, Qt::TopEdge)) {
            popupRect.moveTop(screenArea.top());
        }
        if (!inScreenArea(popupRect, Qt::BottomEdge)) {
            popupRect.moveBottom(screenArea.bottom());
        }
    }
    return popupRect.marginsRemoved(margin);
    ;
}
