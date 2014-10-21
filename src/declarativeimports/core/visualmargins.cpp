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

#include "visualmargins.h"

VisualMargins::VisualMargins(QObject *parent)
    : QObject(parent),
      m_left(0),
      m_top(0),
      m_right(0),
      m_bottom(0)
{
}

VisualMargins::~VisualMargins()
{
}


qreal VisualMargins::left() const
{
    return m_left;
}

void VisualMargins::setLeft(qreal left)
{
    if (left == m_left) {
        return;
    }

    m_left = left;
    emit marginsChanged();
}

qreal VisualMargins::top() const
{
    return m_top;
}

void VisualMargins::setTop(qreal top)
{
    if (top == m_top) {
        return;
    }

    m_top = top;
    emit marginsChanged();
}

qreal VisualMargins::right() const
{
    return m_right;
}

void VisualMargins::setRight(qreal right)
{
    if (right == m_right) {
        return;
    }

    m_right = right;
    emit marginsChanged();
}

qreal VisualMargins::bottom() const
{
    return m_bottom;
}

void VisualMargins::setBottom(qreal bottom)
{
    if (bottom == m_bottom) {
        return;
    }

    m_bottom = bottom;
    emit marginsChanged();
}

void VisualMargins::bindTo(QObject *o)
{
    VisualMargins *other = 0;
    for (auto child : o->children()) {
        other = qobject_cast<VisualMargins *>(child);
        if (other) {
            break;
        }
    }

    if (other == m_other) {
        return;
    }

    if (m_other) {
        disconnect(m_other.data(), &VisualMargins::marginsChanged,
                this, &VisualMargins::updateMarginsFromOther);
    }

    m_other = other;

    if (!other) {
        return;
    }

    connect(m_other.data(), &VisualMargins::marginsChanged,
            this, &VisualMargins::updateMarginsFromOther);
    updateMarginsFromOther();
}

void VisualMargins::updateMarginsFromOther()
{
    if (!m_other) {
        return;
    }

    m_left = m_other->left();
    m_top = m_other->top();
    m_right = m_other->right();
    m_bottom = m_other->bottom();

    emit marginsChanged();
}

VisualMargins *VisualMargins::qmlAttachedProperties(QObject *object)
{
    return new VisualMargins(object);
}

#include "moc_visualmargins.cpp"
