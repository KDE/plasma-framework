/*
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLASMA_WIDGETSNAPSHOT_P_H
#define PLASMA_WIDGETSNAPSHOT_P_H

#include <QGraphicsWidget>

namespace Plasma
{

class WidgetSnapShot : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsWidget *target READ target WRITE setTarget)

public:
    explicit WidgetSnapShot(QGraphicsItem *parent = 0);
    virtual ~WidgetSnapShot();

    virtual void setTarget(QGraphicsWidget *target);
    QGraphicsWidget *target() const;

    virtual void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    bool isIconBigger() const;

    QPixmap snapShot() const;

private:
    void paintSubChildren(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QGraphicsItem *target);

    bool m_iconBig;
    int stack;
    QPixmap m_snapShot;
    QGraphicsWidget *m_target;
};

} // namespace Plasma

#endif // PLASMA_WIDGETSNAPSHOT_P_H
