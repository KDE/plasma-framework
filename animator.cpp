/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

namespace Plasma
{

Animator::Animator(QObject *parent, const QStringList& list)
    : QObject(parent)
{
    Q_UNUSED(list)
}

Animator::~Animator()
{
}

int Animator::appearFrames()
{
    return 0;
}

void Animator::appear(int frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

int Animator::disappearFrames()
{
    return 0;
}

void Animator::disappear(int frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

int Animator::activateFrames()
{
    return 0;
}

void Animator::activate(int frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

int Animator::frameAppearFrames()
{
    return 0;
}

void Animator::frameAppear(int frame, QGraphicsItem* item, const QRegion& drawable)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
    Q_UNUSED(drawable)
}

void Animator::renderBackground(QImage& background)
{
    Q_UNUSED(background)
}

}; // Plasma namespace

#include "animator.moc"
