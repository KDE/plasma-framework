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

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>

#include <plasma/plasma_export.h>

#include <plasma/dataengine.h>
#include <plasma/widgets/layoutitem.h>

namespace Plasma
{

/**
 * Class that emulates a QLineEdit inside plasma
 */
class PLASMA_EXPORT LineEdit : public QGraphicsTextItem, public LayoutItem
{
    Q_OBJECT

    public:
        explicit LineEdit(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
        ~LineEdit();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        Qt::Orientations expandingDirections() const;

        QSizeF minimumSize() const;
        QSizeF maximumSize() const;

        bool hasHeightForWidth() const;
        qreal heightForWidth(qreal w) const;

        bool hasWidthForHeight() const;
        qreal widthForHeight(qreal h) const;

        QRectF geometry() const;
        void setGeometry(const QRectF& geometry);
        QSizeF sizeHint() const;


    public Q_SLOTS:
        void updated(const QString&, const Plasma::DataEngine::Data&);

    private:
        class Private;
        Private* const d;
};

} // namespace Plasma

#endif
