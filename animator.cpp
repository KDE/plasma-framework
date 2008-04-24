/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *             2007 Alexis Ménard <darktears31@gmail.com>
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

#include "animator.h"

#include <QPainter>
#include <QGraphicsItem>

namespace Plasma
{

Animator::Animator(QObject *parent)
    : QObject(parent),
      d(0)
{
}

Animator::~Animator()
{
}

int Animator::animationFPS(Plasma::AnimationDriver::Animation animation) const
{
    Q_UNUSED(animation)
    return 0;
}

int Animator::movementAnimationFPS(Plasma::AnimationDriver::Movement movement) const
{
    Q_UNUSED(movement)
    return 20;
}

int Animator::elementAnimationFPS(Plasma::AnimationDriver::Animation animation) const
{
    Q_UNUSED(animation)
    return 0;
}

int Animator::animationDuration(Plasma::AnimationDriver::Animation) const
{
    return 200;
}

int Animator::movementAnimationDuration(Plasma::AnimationDriver::Movement movement) const
{
    switch (movement) {
        case AnimationDriver::FastSlideInMovement:
        case AnimationDriver::FastSlideOutMovement:
            return 100;
            break;
        default:
            break;
    }

    return 270;
}

int Animator::elementAnimationDuration(Plasma::AnimationDriver::Animation) const
{
    return 333;
}

AnimationDriver::CurveShape Animator::animationCurve(Plasma::AnimationDriver::Animation) const
{
    return AnimationDriver::EaseInOutCurve;
}

AnimationDriver::CurveShape Animator::movementAnimationCurve(Plasma::AnimationDriver::Movement) const
{
    return AnimationDriver::EaseInOutCurve;
}

AnimationDriver::CurveShape Animator::elementAnimationCurve(Plasma::AnimationDriver::Animation) const
{
    return AnimationDriver::EaseInOutCurve;
}

QPixmap Animator::elementAppear(qreal progress, const QPixmap& pixmap)
{
    Q_UNUSED(progress)
    return pixmap;
}

QPixmap Animator::elementDisappear(qreal progress, const QPixmap& pixmap)
{
    Q_UNUSED(progress)
    QPixmap pix(pixmap.size());
    pix.fill(Qt::transparent);

    return pix;
}

void Animator::itemAppear(qreal frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void Animator::itemDisappear(qreal frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void Animator::itemActivated(qreal frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void Animator::itemSlideIn(qreal progress, QGraphicsItem *item, const QPoint &start, const QPoint &destination)
{
    double x = start.x() + (destination.x() - start.x()) * progress;
    double y = start.y() + (destination.y() - start.y()) * progress;
    item->setPos(x, y);
}

void Animator::itemSlideOut(qreal progress, QGraphicsItem *item, const QPoint &start, const QPoint &destination)
{
    //kDebug();
    double x = start.x() + (destination.x() - start.x()) * progress;
    double y = start.y() + (destination.y() - start.y()) * progress;
    item->setPos(x, y);
}

} // Plasma namespace

#include "animator.moc"
