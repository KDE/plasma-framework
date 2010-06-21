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

#include <QtCore/QObject>
#include <QtGui/QPainter>

#include "kdebug.h"

#include "ripple_p.h"

/**
 * Allocates an integer matrix with the given size.
 * @param size Matrix size
 * @return integer matrix
 */
int **allocateWaveMap(const QSize &size)
{
    int **waveMap = new int *[size.width()];
    if (!waveMap) {
        kDebug() << "could not allocate wave map";
        return 0;
    }

    for (int x = 0; x < size.width(); ++x) {
        waveMap[x] = new int[size.height()];
        if (!waveMap[x]) {
            kDebug() << "could not allocate wave map";
            return 0;
        }
    }

    return waveMap;
}

/**
 * Deallocates an integer matrix
 * @param waveMap integer matrix
 */
void deallocateWaveMap(int **waveMap)
{
    if (waveMap) {
        delete [] *waveMap;
        delete [] waveMap;
    }
}

namespace Plasma
{

RippleEffect::RippleEffect(QObject *parent)
    : QGraphicsEffect(parent),
      m_offset(1),
      m_damping(16),
      m_heigth(1),
      m_opacity(0.0),
      m_mapSize(boundingRect().size().toSize()),
      m_previousMap(0),
      m_currentMap(0)
{
}

RippleEffect::~RippleEffect()
{
    deallocateWaveMap(m_previousMap);
    deallocateWaveMap(m_currentMap);
}

qint8 RippleEffect::offset() const
{
    return m_offset;
}

qint8 RippleEffect::damping() const
{
    return m_damping;
}

qint8 RippleEffect::heigth() const
{
    return m_heigth;
}

qreal RippleEffect::opacity() const
{
    return m_opacity;
}

void RippleEffect::setOffset(qint8 offset)
{
    m_offset = offset;
    emit offsetChanged(m_offset);
}

void RippleEffect::setDamping(qint8 damping)
{
    m_damping = damping;
    emit dampingChanged(m_damping);
}

void RippleEffect::setHeigth(qint8 heigth)
{
    m_heigth = heigth;
    emit heigthChanged(m_heigth);
}

void RippleEffect::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void RippleEffect::draw(QPainter *painter)
{
    QPoint offset;
    const QImage currentImage = sourcePixmap(Qt::LogicalCoordinates, &offset).toImage();
    QImage modifiedImage = currentImage;
    if (!m_previousMap && !m_currentMap) {
        m_previousMap = allocateWaveMap(currentImage.size());
        m_currentMap = allocateWaveMap(currentImage.size());
    }

    int x, y;
    if (qFuzzyCompare(m_opacity, qreal(0.0))) {
        for (x = 0; x < currentImage.width(); ++x) {
            memset(m_currentMap[x], 0, sizeof(int) * currentImage.height());
            memset(m_previousMap[x], 0, sizeof(int) * currentImage.height());
        }
        m_mapSize = currentImage.size();
        int waveLength = m_mapSize.width() > m_mapSize.height() ? m_mapSize.width() : m_mapSize.height();
        m_currentMap[m_mapSize.width() >> 1][m_mapSize.height() >> 1] = waveLength << m_heigth;
    } else if (m_mapSize != currentImage.size()) {
        const qreal scaleFactorX = qreal(currentImage.width()) / qreal(m_mapSize.width());
        const qreal scaleFactorY = qreal(currentImage.height()) / qreal(m_mapSize.height());
        int **newPreviousMap = allocateWaveMap(currentImage.size());
        int **newCurrentMap = allocateWaveMap(currentImage.size());
        int i, j;
        for (y = 0; y < currentImage.height(); ++y) {
            for (x = 0; x < currentImage.width(); ++x) {
                i = x / scaleFactorX;
                j = y / scaleFactorY;
                newPreviousMap[x][y] = m_previousMap[i][j];
                newCurrentMap[x][y] = m_currentMap[i][j];
            }
        }
        deallocateWaveMap(m_previousMap);
        deallocateWaveMap(m_currentMap);
        m_mapSize = currentImage.size();
        m_previousMap = newPreviousMap;
        m_currentMap = newCurrentMap;
    }

    const int width = m_mapSize.width();
    const int height = m_mapSize.height();
    int neighbours;
    int wave;
    int xOffset, yOffset;
    for (y = m_offset; y < height - m_offset - 1; ++y) {
        for (x = m_offset; x < width - m_offset - 1; ++x) {
            neighbours = m_previousMap[x+m_offset][y] +
                         m_previousMap[x-m_offset][y] +
                         m_previousMap[x][y+m_offset] +
                         m_previousMap[x][y-m_offset];
            if (!neighbours && !m_currentMap[x][y]) {
                continue;
            }

            wave = (neighbours >> 1) - m_currentMap[x][y];
            wave -= wave >> m_damping;
            m_currentMap[x][y] = wave;

            xOffset = x + m_currentMap[x+m_offset][y] - wave;
            yOffset = y + m_currentMap[x][y+m_offset] - wave;

            modifiedImage.setPixel(x, y, currentImage.pixel(
                        qBound(0, xOffset, width - 1),
                        qBound(0, yOffset, height - 1)));
        }
    }

    // Swap wave maps
    int **pointer = m_previousMap;
    m_previousMap = m_currentMap;
    m_currentMap = pointer;

    // Restart wave if image center has no wave
    if (m_currentMap[width >> 1][height >> 1] == 0) {
        int waveLength = width > height ? width : height;
        m_currentMap[width >> 1][height >> 1] = waveLength << m_heigth;
    }

    painter->drawImage(offset, currentImage);
    painter->setOpacity(1 - m_opacity);
    painter->drawImage(offset, modifiedImage);
}

} // namespace Plasma
