/*
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>
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

#ifndef PLASMA_SEPARATOR_H
#define PLASMA_SEPARATOR_H

#include <QtGui/QGraphicsWidget>

#include <plasma/plasma_export.h>

#include <plasma/plasma.h>

namespace Plasma
{

class SeparatorPrivate;

/**
 * @since 4.4
 */
class PLASMA_EXPORT Separator : public QGraphicsWidget
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

    public:
        explicit Separator(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~Separator();

        void setOrientation(Qt::Orientation orientation);
        Qt::Orientation orientation();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;

    private:
        SeparatorPrivate * const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
