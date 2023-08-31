/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2022 Luca Carlon<carlon.luca@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "windowresizehandler.h"

#include <QEvent>
#include <QMouseEvent>
#include <QWindow>

using namespace PlasmaQuick;

class WindowResizeHandlerPrivate
{
public:
    QWindow *window;
    QMargins margins;
    Qt::Edges activeEdges;

    bool updateMouseCursor(const QPointF &globalMousePos);
    Qt::Edges hitTest(const QPointF &pos);

private:
    bool overridingCursor = false;
    bool hitTestLeft(const QPointF &pos);
    bool hitTestRight(const QPointF &pos);
    bool hitTestTop(const QPointF &pos);
    bool hitTestBottom(const QPointF &pos);
};

PlasmaQuick::WindowResizeHandler::WindowResizeHandler(QWindow *parent)
    : QObject(parent)
    , d(new WindowResizeHandlerPrivate)
{
    d->window = parent;
    d->window->installEventFilter(this);
}

WindowResizeHandler::~WindowResizeHandler()
{
    d->window->removeEventFilter(this);
}

void WindowResizeHandler::setMargins(const QMargins &margins)
{
    d->margins = margins;
}

QMargins WindowResizeHandler::margins() const
{
    return d->margins;
}

void WindowResizeHandler::setActiveEdges(Qt::Edges edges)
{
    d->activeEdges = edges;
}

Qt::Edges WindowResizeHandler::activeEdges() const
{
    return d->activeEdges;
}

bool WindowResizeHandler::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (event->type() == QEvent::MouseMove && d->updateMouseCursor(me->globalPosition())) {
            return false;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            const QPointF globalMousePos = me->globalPosition();
            const Qt::Edges sides = d->hitTest(globalMousePos) & d->activeEdges;
            if (sides) {
                d->window->startSystemResize(sides);
                return true;
            }
        }
    } break;
    default:
        break;
    }
    return false;
}

bool WindowResizeHandlerPrivate::updateMouseCursor(const QPointF &globalMousePos)
{
    Qt::Edges sides = hitTest(globalMousePos) & activeEdges;
    if (!sides) {
        if (overridingCursor) {
            window->unsetCursor();
            overridingCursor = false;
        }
        return false;
    }

    if (sides == Qt::Edges(Qt::LeftEdge | Qt::TopEdge)) {
        window->setCursor(Qt::SizeFDiagCursor);
    } else if (sides == Qt::Edges(Qt::RightEdge | Qt::TopEdge)) {
        window->setCursor(Qt::SizeBDiagCursor);
    } else if (sides == Qt::Edges(Qt::LeftEdge | Qt::BottomEdge)) {
        window->setCursor(Qt::SizeBDiagCursor);
    } else if (sides == Qt::Edges(Qt::RightEdge | Qt::BottomEdge)) {
        window->setCursor(Qt::SizeFDiagCursor);
    } else if (sides.testFlag(Qt::TopEdge)) {
        window->setCursor(Qt::SizeVerCursor);
    } else if (sides.testFlag(Qt::LeftEdge)) {
        window->setCursor(Qt::SizeHorCursor);
    } else if (sides.testFlag(Qt::RightEdge)) {
        window->setCursor(Qt::SizeHorCursor);
    } else {
        window->setCursor(Qt::SizeVerCursor);
    }

    overridingCursor = true;
    return true;
}

Qt::Edges WindowResizeHandlerPrivate::hitTest(const QPointF &pos)
{
    bool left = hitTestLeft(pos);
    bool right = hitTestRight(pos);
    bool top = hitTestTop(pos);
    bool bottom = hitTestBottom(pos);
    Qt::Edges edges;
    if (left) {
        edges.setFlag(Qt::LeftEdge);
    }
    if (right) {
        edges.setFlag(Qt::RightEdge);
    }
    if (bottom) {
        edges.setFlag(Qt::BottomEdge);
    }
    if (top) {
        edges.setFlag(Qt::TopEdge);
    }

    return edges;
}

bool WindowResizeHandlerPrivate::hitTestLeft(const QPointF &pos)
{
    const QRect geometry = window->geometry();
    const QRectF rect(geometry.x(), geometry.y(), margins.left(), geometry.height());
    return rect.contains(pos);
}

bool WindowResizeHandlerPrivate::hitTestRight(const QPointF &pos)
{
    const QRect geometry = window->geometry();
    const QRectF rect(geometry.x() + geometry.width() - margins.right(), geometry.y(), margins.right(), geometry.height());
    return rect.contains(pos);
}

bool WindowResizeHandlerPrivate::hitTestTop(const QPointF &pos)
{
    const QRect geometry = window->geometry();
    const QRectF rect(geometry.x(), geometry.y(), geometry.width(), margins.top());
    return rect.contains(pos);
}

bool WindowResizeHandlerPrivate::hitTestBottom(const QPointF &pos)
{
    const QRect geometry = window->geometry();
    const QRectF rect(geometry.x(), geometry.y() + geometry.height() - margins.bottom(), geometry.width(), margins.bottom());
    return rect.contains(pos);
}
