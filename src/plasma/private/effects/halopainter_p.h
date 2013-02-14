/*
 *   Copyright © 2009 Fredrik Höglund <fredrik@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#ifndef HALOPAINTER_H
#define HALOPAINTER_H

#include <QCache>
#include <QPixmap>

class QRect;
class QPainter;

namespace Plasma {

class TileSet
{
public:
    enum Tile {
        Left, Center, Right, NTiles
    };

    TileSet(const QPixmap &pixmap);
    ~TileSet() {}

    void paint(QPainter *painter, const QRect &rect);

protected:
    QPixmap tiles[NTiles];
};


// -----------------------------------------------------------------------


class HaloPainter : public QObject
{
public:
    ~HaloPainter();
    static HaloPainter *instance() {
        if (!s_instance) {
            s_instance = new HaloPainter;
        }
        return s_instance;
    }

    static void drawHalo(QPainter *painter, const QRect &textRect) {
        instance()->paint(painter, textRect);
    }

private:
    HaloPainter();
    TileSet *tiles(int height) const;
    void paint(QPainter *painter, const QRect &textRect) const;

private:
    static HaloPainter *s_instance;
    mutable QCache<int, TileSet> m_tileCache;
    mutable QCache<int, QPixmap> m_haloCache;
};

} // namespace Plasma

#endif

