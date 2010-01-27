/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#ifndef PLASMA_METER_PRIVATE_H
#define PLASMA_METER_PRIVATE_H

#include <QObject>

class QRectF;
class QString;
class QPainter;
class QPropertyAnimation;

namespace Plasma {

class Meter;
class FrameSvg;

class MeterPrivate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int meterValue READ meterValue WRITE setMeterValue)

public:
    MeterPrivate(Meter *m);
    void progressChanged(int progress);
    void paint(QPainter *p, const QString &elementID);
    void text(QPainter *p, int index);
    QRectF barRect();
    void paintBackground(QPainter *p);
    void paintBar(QPainter *p, const QString &prefix);
    void paintForeground(QPainter *p);
    void setSizePolicyAndPreferredSize();
    void setMeterValue(int value);
    int meterValue() const;

    int minimum;
    int maximum;
    int value;
    int targetValue;
    QStringList labels;
    QList<Qt::Alignment> alignments;
    QList<QColor> colors;
    QList<QFont> fonts;
    QString svg;
    Meter::MeterType meterType;
    Plasma::FrameSvg *image;
    int minrotate;
    int maxrotate;
    Meter *meter;
    int movementId;
    QPropertyAnimation *animation;
};

}

#endif

