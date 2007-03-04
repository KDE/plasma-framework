/*
 *   Copyright (C) 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QGraphicsTextItem>

#include <kdemacros.h>

#include "datavisualization.h"

namespace Plasma
{

class KDE_EXPORT LineEdit : public QGraphicsTextItem,
                            public DataVisualization
{
    public:
        LineEdit(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
        ~LineEdit();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public Q_SLOTS:
        void data(const DataSource::Data&);

};

} // namespace Plasma

#endif
