/*********************************************************************/
/*                                                                   */
/* Copyright(C)2009 Adenilson Cavalcanti adenilson.silva@idnt.org.br */
/*                                                                   */
/* This program is free software; you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License       */
/* as published by the Free Software Foundation; either version 2    */
/* of the License, or (at your option) any later version.            */
/*                                                                   */
/* This program is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     */
/* GNU General Public License for more details.                      */
/*                                                                   */
/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software       */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA     */
/* 02110-1301, USA.                                                  */
/*********************************************************************/

#ifndef PULSERSHADOW_P_H
#define PULSERSHADOW_P_H

#include <QGraphicsWidget>

namespace Plasma
{

class ShadowFake: public QGraphicsWidget
{
    Q_OBJECT

public:
    ShadowFake(QGraphicsItem *parent = 0);

    ~ShadowFake();

    void copyTarget(QGraphicsWidget *target);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

private:
    QPixmap *photo;
};


}

#endif
