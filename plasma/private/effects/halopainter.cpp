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

#include "halopainter_p.h"
#include <QPainter>

#include <cmath>

namespace Plasma
{

static inline qreal gaussian(qreal x, qreal sigma)
{
    return (1.0 / std::sqrt(2.0 * M_PI) * sigma)
            * std::exp(-((x * x) / (2.0 * sigma * sigma)));
}

static void gaussianBlur(QImage &image, int radius)
{
    // The gaussian curve is effectively zero after 3 standard deviations.
    qreal sigma = radius / 3.;
    int size = radius * 2 + 1;
    int center = size / 2;
    qreal *kernel = new qreal[size];
    qreal total = 0;

    // Generate the gaussian kernel
    for (int i = 0; i < size; i++) {
        kernel[i] = gaussian(i - center, sigma);
        total += kernel[i];
    }

    // Normalize the kernel
    for (int i = 0; i < size; i++)
        kernel[i] = kernel[i] / total;

    quint32 *buf = new quint32[image.width() * image.height()];
    memset(buf, 0, image.width() * image.height() * sizeof(quint32));

    // Blur the image horizontally
    for (int y = 0; y < image.height(); y++)
    {
        quint32 *src = (quint32*)image.scanLine(y);
        quint32 *dst = buf + image.width() * y;

        for (int x = 0, start = center; x < center; x++, start--) {
            double a = 0;
            for (int i = start; i < size; i++)
                a += qAlpha(src[x - center + i]) * kernel[i];
            dst[x] = qRound(a) << 24;
        }

        for (int x = center; x < image.width() - center; x++) {
            double a = 0;
            for (int i = 0; i < size; i++)
                a += qAlpha(src[x - center + i]) * kernel[i];
            dst[x] = qRound(a) << 24;
        }

        for (int x = image.width() - center, stop = size - 1; x < image.width(); x++, stop--) {
            double a = 0;
            for (int i = 0; i < stop; i++)
                a += qAlpha(src[x - center + i]) * kernel[i];
            dst[x] = qRound(a) << 24;
        }
    }

    // Blur the image vertically
    quint32 *src = buf;
    quint32 *dst = (quint32*)image.bits();

    for (int x = 0; x < image.width(); x++)
    {
        int di = x;

        for (int y = 0, start = center; y < center; y++, start--) {
            double a = 0;
            int si = (y - center + start) * image.width() + x;
            for (int i = start; i < size; i++) {
                a += qAlpha(src[si]) * kernel[i];
                si += image.width();
            }
            dst[di] = qRound(a) << 24;
            di += image.width();
        }

        for (int y = center; y < image.height() - center; y++) {
            double a = 0;
            int si = (y - center) * image.width() + x;
            for (int i = 0; i < size; i++) {
                a += qAlpha(src[si]) * kernel[i];
                si += image.width();
            }
            dst[di] = qRound(a) << 24;
            di += image.width();
        }

        for (int y = image.height() - center, stop = size - 1; y < image.height(); y++, stop--) {
            double a = 0;
            int si = (y - center) * image.width() + x;
            for (int i = 0; i < stop; i++) {
                a += qAlpha(src[si]) * kernel[i];
                si += image.width();
            }
            dst[di] = qRound(a) << 24;
            di += image.width();
        }
    }

    delete [] buf;
    delete [] kernel;
}



// -----------------------------------------------------------------------



TileSet::TileSet(const QPixmap &pixmap)
{
    int tw = pixmap.width() / 3;
    int th = pixmap.height();

    for (int x = 0; x < 3; x++)
        tiles[x] = pixmap.copy(x * tw, 0, tw, th);
}

void TileSet::paint(QPainter *p, const QRect &r)
{
    int tw = tiles[Left].width();
    int th = tiles[Left].height();
    int tw2 = tw * 2;

    if (r.width() < tw2) {
        int sw = r.width() / 2;
        p->drawPixmap(r.x(), r.y(), tiles[Left], 0, 0, sw, tiles[Left].height());
        p->drawPixmap(r.x() + sw, r.y(), tiles[Right], tw - sw, 0, sw, tiles[Right].height());
    } else {
        p->drawPixmap(r.topLeft(), tiles[Left]);
        if (r.width() - tw2 > 0)
            p->drawTiledPixmap(r.x() + tw, r.y(), r.width() - tw2, th, tiles[Center]);
        p->drawPixmap(r.right() - tw + 1, r.y(), tiles[Right]);
    }
}



// -----------------------------------------------------------------------



HaloPainter *HaloPainter::s_instance = 0;


HaloPainter::HaloPainter()
    : m_tileCache(16), m_haloCache(30)
{
}

HaloPainter::~HaloPainter()
{
}

TileSet *HaloPainter::tiles(int height) const
{
    TileSet *tiles = m_tileCache.object(height);

    if (!tiles) {
        QImage image(64 * 3, height + 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);

        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawRoundedRect(image.rect().adjusted(8, 8, -8, -8), height, height / 2);
        p.end();

        gaussianBlur(image, 8);

        p.begin(&image);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(image.rect(), QColor(255, 255, 255, 255));
        p.end();

        tiles = new TileSet(QPixmap::fromImage(image));
        m_tileCache.insert(height, tiles);
    }

    return tiles;
}

void HaloPainter::paint(QPainter *painter, const QRect &textRect) const
{
    int radius = textRect.height() / 2;
    const QRect hr = textRect.adjusted(-8 - radius, -9, 8 + radius, 9);

    int key = hr.width() << 16 | hr.height();
    QPixmap *pixmap = m_haloCache.object(key);

    if (!pixmap) {
        TileSet *halo = tiles(hr.height() - 16);

        pixmap = new QPixmap(hr.size());
        pixmap->fill(Qt::transparent);
        QPainter p(pixmap);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        halo->paint(&p, pixmap->rect());
        QLinearGradient g(0, 0, pixmap->width(), 0);
        if (hr.width() < 80) {
            for (int i = 0; i <= 16; i++) {
                g.setColorAt(i / 16., QColor(0, 0, 0, 164 * (1 - std::pow((i - 8) / 8., 2))));
            }
        } else {
            const qreal pixel = 1. / hr.width();
            for (int i = 0; i <= 8; i++) {
                const QColor color(0, 0, 0, 164 * (1 - std::pow((i - 8) / 8., 2)));
                g.setColorAt(i * (pixel * 40) / 8, color);
                g.setColorAt(1 - i * (pixel * 40) / 8, color);
            }
        }
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pixmap->rect(), g);
        p.end();

        m_haloCache.insert(key, pixmap);
    }

    painter->drawPixmap(hr.topLeft(), *pixmap);
}

} // namespace Plasma

