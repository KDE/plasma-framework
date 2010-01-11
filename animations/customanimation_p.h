/*********************************************************************/
/*                                                                   */
/* Copyright (C)  2010  Igor Oliveira <igor.oliveira@openbossa.org>  */
/* Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com> */
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

#ifndef PLASMA_ANIMATIONS_CUSTOMANIMATION_H
#define PLASMA_ANIMATIONS_CUSTOMANIMATION_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

class QString;

namespace Plasma
{

class CustomAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(QString callback READ callback WRITE setCallback)
    Q_PROPERTY(QVariant startValue READ startValue WRITE setStartValue)
    Q_PROPERTY(QVariant endValue READ endValue WRITE setEndValue)

public:
    CustomAnimation(QObject *parent = 0);

    QString callback() const;
    void setCallback(const QString &method);

    QVariant startValue() const;
    void setStartValue(const QVariant &value);

    QVariant endValue() const;
    void setEndValue(const QVariant &value);

protected:
    void updateCurrentTime(int currentTime);

private:
    QVariant interpolate(const QVariant &start, const QVariant &end, qreal delta);

private:
    QString m_method;
    QVariant m_startValue;
    QVariant m_endValue;
};
}

#endif
