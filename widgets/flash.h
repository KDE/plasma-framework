/*
 *   Copyright (C) 2007 by André Duffeck <andre@duffeck.de>
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

#ifndef FLASH_H_
#define FLASH_H_

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>
#include <QtGui/QTextOption>

#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * Class that allows to flash text or icons inside plasma
 */
class PLASMA_EXPORT Flash : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        Flash(QGraphicsItem *parent = 0);
        virtual ~Flash();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        QRectF boundingRect() const;
        QSize size() const;
        int height() const;
        int width() const;
        void setSize(const QSize &size);
        void setWidth(int width);
        void setHeight(int height);

        void setFont( const QFont & );
        void setColor( const QColor & );
        void setDuration( int duration );

        void flash( const QString &text, int duration = 0, const QTextOption &option = QTextOption(Qt::AlignCenter) );
        void flash( const QPixmap &pixmap, int duration = 0, Qt::Alignment align = Qt::AlignCenter );

    public Q_SLOTS:
        void kill();

    protected Q_SLOTS:
        void fadeIn();
        void fadeOut();

    protected:
        QPixmap renderPixmap();

    private:
        class Private;
        Private * const d;
};

}

#endif
