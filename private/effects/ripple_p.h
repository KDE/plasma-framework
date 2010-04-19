/*
 * Copyright (C) 2010 Bruno Abinader <bruno.abinader@indt.org.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLASMA_EFFECTS_RIPPLE_P_H
#define PLASMA_EFFECTS_RIPPLE_P_H

#include <QtGui/QGraphicsEffect>

class QObject;
class QPainter;

namespace Plasma
{

/**
 * @class RippleEffect plasma/private/effects/ripple_p.h
 * @short Ripple effect.
 *
 * Simulates a ripple effect on the source. This class can be used to simulate a "water" animation.
 */
class RippleEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(qint8 offset READ offset WRITE setOffset)
    Q_PROPERTY(qint8 damping READ damping WRITE setDamping)
    Q_PROPERTY(qint8 heigth READ heigth WRITE setHeigth)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    /**
     * Default constructor
     *
     * @param parent Effect object parent.
     */
    RippleEffect(QObject *parent = 0);

    /** Destructor */
    ~RippleEffect();

    /**
     * Returns the ripple offset. The offset is used to calculate the distande between
     * neighbour pixels.
     * @return ripple offset
     */
    qint8 offset() const;
    /** Returns the ripple damping factor. The damping factor is used to reduce the wave height
     * through each pass.
     * @return ripple damping factor
     */
    qint8 damping() const;
    /** Returns the ripple wave heigth factor. The heigth factor is used to enlarge or reduce the
     * initial wave heigth.
     * @return ripple wave heigth factor
     */
    qint8 heigth() const;
    /** Returns the ripple opacity. The opacity is used to reduce the effect opacity when
     * animating.
     * @return ripple opacity level
     */
    qreal opacity() const;

public slots:
    /**
     * Set ripple offset (e.g. 1).
     */
    void setOffset(qint8 offset);
    /**
     * Set ripple damping factor (e.g. 16).
     */
    void setDamping(qint8 damping);
    /**
     * Set ripple wave heigth factor (e.g. 1).
     */
    void setHeigth(qint8 heigth);
    /**
     * Set ripple opacity level (e.g. 1.0).
     */
    void setOpacity(qreal opacity);

signals:
    /**
     * Emitted when the ripple offset has changed.
     * @param offset the ripple offset
     */
    void offsetChanged(qint8 offset);
    /**
     * Emitted when the ripple damping factor has changed.
     * @param damping the ripple damping factor
     */
    void dampingChanged(qint8 damping);
    /**
     * Emitted when the ripple wave heigth factor has changed.
     * @param heigth the ripple wave heigth factor
     */
    void heigthChanged(qint8 heigth);

protected:
    /**
     * Reimplemented from QGraphicsEffect::draw().
     * @param painter source painter
     */
    void draw(QPainter *painter);

private:
    Q_DISABLE_COPY(RippleEffect)

    qint8 m_offset; /** Ripple offset (default is 1) */
    qint8 m_damping; /** Ripple damping factor (default is 16) */
    qint8 m_heigth; /** Ripple wave heigth factor (default is 1) */
    qreal m_opacity; /** Ripple opacity level (default is 0.0) */
    QSize m_mapSize; /** Ripple matrix wave size */
    int **m_previousMap; /** Last ripple matrix wave */
    int **m_currentMap; /** Current ripple matrix wave */
};

} // namespace Plasma

#endif // PLASMA_EFFECTS_RIPPLE_P_H
