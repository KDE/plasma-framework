/***********************************************************************/
/* rotation.h                                                          */
/*                                                                     */
/* Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>*/
/* Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*/
/*                                                                     */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public	       */
/* License as published by the Free Software Foundation; either	       */
/* version 2.1 of the License, or (at your option) any later version.  */
/*                                                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.                     */
/*                                                                     */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA                                                     */
/***********************************************************************/

#include <plasma/plasma_export.h>
#include <plasma/animations/animation.h>

#include <QVector3D>

namespace Plasma {

class RotationAnimationPrivate;

class RotationAnimation : public Animation
{

    Q_OBJECT
    Q_PROPERTY(Qt::Axis axis READ axis WRITE setAxis)
    Q_PROPERTY(qint8 reference READ reference WRITE setReference)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)

public:
    RotationAnimation(const qint8 &reference = Center,
		      const Qt::Axis &axis = Qt::ZAxis,
		      const qreal &angle = 180);

    ~RotationAnimation();

    QPropertyAnimation* render(QObject* parent = 0);

    /**
     * get animation rotation axis (e.g. YAxis, ZAxis, XAxis)
     */
    Qt::Axis axis() const;

    /**
     * set animation rotation axis
     * @arg axis Rotation (e.g. YAxis, ZAxis, XAxis)
     */
    void setAxis(const Qt::Axis &axis);

    /**
     * Rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     */
    qint8 reference() const;

    /**
     * Set rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     * @arg reference The reference
     */
    void setReference(const qint8 &reference);

    /**
     * Animation rotation angle (e.g. 45, 180, etc)
     */
    qreal angle() const;

    /**
     * Set animation rotation angle (e.g. 45, 180, etc)
     * @arg angle The angle
     */
    void setAngle(const qreal &angle);



    private:
        RotationAnimationPrivate *const d;
};
} // Plasma
