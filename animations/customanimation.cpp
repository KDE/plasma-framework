/////////////////////////////////////////////////////////////////////////
// customanimation.cpp                                                 //
//                                                                     //
// Copyright (C)  2010  Igor Oliveira <igor.oliveira@openbossa.org>    //
// Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com    //
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "customanimation_p.h"

#include <QString>

namespace Plasma
{

CustomAnimation::CustomAnimation(QObject *parent)
    : Animation(parent)
{
}

QString CustomAnimation::callback() const
{
    return m_method;
}

void CustomAnimation::setCallback(const QString &slot)
{
    m_method = slot;
}

QVariant CustomAnimation::startValue() const
{
    return m_startValue;
}

void CustomAnimation::setStartValue(const QVariant &value)
{
    m_startValue = value;
}

QVariant CustomAnimation::endValue() const
{
    return m_endValue;
}

void CustomAnimation::setEndValue(const QVariant &value)
{
    m_endValue = value;
}

void CustomAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *obj = targetWidget();
    if (obj) {
        qreal delta = currentTime / qreal(duration());
        delta = Animation::easingCurve().valueForProgress(delta);

        const QVariant retValue = interpolate(startValue(), endValue(), delta);
        QMetaObject::invokeMethod(obj, m_method.toUtf8().data(), Q_ARG(QVariant, retValue));
    }

    Animation::updateCurrentTime(currentTime);
}

QVariant CustomAnimation::interpolate(const QVariant &start, const QVariant &end, qreal delta)
{
    QVariant retValue;

    if (start.type() != end.type()) {
        return retValue;
    }

    if (start.type() == QVariant::Double) {
        qreal realStartValue = start.toReal();
        qreal realEndValue = end.toReal();

        qreal retRealValue = (realStartValue - realEndValue) * delta;

        retValue = QVariant(retRealValue);
    } else if (start.type() == QVariant::Int) {
        int intStartValue = start.toInt();
        int intEndValue = end.toInt();

        int retIntValue = (intStartValue - intEndValue) * delta;
        retValue = QVariant(retIntValue);
    } else if (start.type() == QVariant::RectF) {
        QRectF rectfStartValue = start.toRectF();
        QRectF rectfEndValue = end.toRectF();

        QRectF retRectfValue = QRectF((rectfStartValue.x() - rectfEndValue.x()) * delta,
                                      (rectfStartValue.y() - rectfEndValue.x()) * delta,
                                      (rectfStartValue.width() - rectfEndValue.width()) * delta,
                                      (rectfStartValue.height() - rectfEndValue.height()) * delta);
        retValue = QVariant(retRectfValue);
    }

    return retValue;
}

}
