/*
 *   Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 *   Copyright (c) 2009 Aaron Seigo <aseigo@kde.org>
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

#include "plasma/private/wallpaperrenderthread_p.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>

#include <kdebug.h>

namespace Plasma
{

WallpaperRenderThread::WallpaperRenderThread(QObject *parent)
    : QThread(parent),
      m_currentToken(-1)
{
    m_abort = false;
    m_restart = false;
}

WallpaperRenderThread::~WallpaperRenderThread()
{
    {
        // abort computation
        QMutexLocker lock(&m_mutex);
        m_abort = true;
        m_condition.wakeOne();
    }

    wait();
}

void WallpaperRenderThread::setSize(const QSize& size)
{
    QMutexLocker lock(&m_mutex);
    m_size = size;
}

int WallpaperRenderThread::render(const QString &file,
                                  const QSize &size,
                                  Wallpaper::ResizeMethod method,
                                  const QColor &color)
{
    int token;
    {
        QMutexLocker lock(&m_mutex);
        m_file = file;
        m_color = color;
        m_method = method;
        m_size = size;
        m_restart = true;
        token = ++m_currentToken;
    }

    if (!isRunning()) {
        start(QThread::LowPriority);
    } else {
        m_condition.wakeOne();
    }

    return token;
}

void WallpaperRenderThread::run()
{
    QString file;
    QColor color;
    QSize size;
    qreal ratio;
    Wallpaper::ResizeMethod method;
    int token;

    forever {
        {
            QMutexLocker lock(&m_mutex);

            while (!m_restart && !m_abort) {
                m_condition.wait(&m_mutex);
            }

            if (m_abort) {
                return;
            }

            m_restart = false;

            // load all parameters in nonshared variables
            token = m_currentToken;
            file = m_file;
            color = m_color;
            size = m_size;
            ratio = m_size.width() / qreal(m_size.height());
            method = m_method;
        }

        QImage result(size, QImage::Format_ARGB32_Premultiplied);
        result.fill(color.rgba());

        if (file.isEmpty() || !QFile::exists(file)) {
            emit done(this, token, result, file, size, method, color);
            break;
        }

        QPoint pos(0, 0);
        bool tiled = false;
        bool scalable = file.endsWith(QLatin1String("svg")) || file.endsWith(QLatin1String("svgz"));
        QSize scaledSize;
        QImage img;

        // set image size
        QSize imgSize;
        if (scalable) {
            // scalable: image can be of any size
            imgSize = size;
        } else {
            // otherwise, use the natural size of the loaded image
            img = QImage(file);
            imgSize = img.size();
            //kDebug() << "loaded with" << imgSize << ratio;
        }

        // if any of them is zero we may run into a div-by-zero below.
        if (imgSize.width() < 1) {
            imgSize.setWidth(1);
        }

        if (imgSize.height() < 1) {
            imgSize.setHeight(1);
        }

        if (ratio < 1) {
            ratio = 1;
        }

        // set render parameters according to resize mode
        switch (method)
        {
        case Wallpaper::ScaledResize:
            scaledSize = size;
            break;
        case Wallpaper::CenteredResize:
            scaledSize = imgSize;
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);

            //If the picture is bigger than the screen, shrink it
            if (size.width() < imgSize.width() && imgSize.width() > imgSize.height()) {
                int width = size.width();
                int height = width * scaledSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
                pos = QPoint((size.width() - scaledSize.width()) / 2,
                             (size.height() - scaledSize.height()) / 2);
            } else if (size.height() < imgSize.height()) {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
                pos = QPoint((size.width() - scaledSize.width()) / 2,
                             (size.height() - scaledSize.height()) / 2);
            }

            break;
        case Wallpaper::MaxpectResize: {
            float xratio = (float) size.width() / imgSize.width();
            float yratio = (float) size.height() / imgSize.height();
            if (xratio > yratio) {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            } else {
                int width = size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            }
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::ScaledAndCroppedResize: {
            float xratio = (float) size.width() / imgSize.width();
            float yratio = (float) size.height() / imgSize.height();
            if (xratio > yratio) {
                int width = size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            } else {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            }
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::TiledResize:
            scaledSize = imgSize;
            tiled = true;
            break;
        case Wallpaper::CenterTiledResize:
            scaledSize = imgSize;
            pos = QPoint(
                -scaledSize.width() +
                    ((size.width() - scaledSize.width()) / 2) % scaledSize.width(),
                -scaledSize.height() +
                    ((size.height() - scaledSize.height()) / 2) % scaledSize.height());
            tiled = true;
            break;
        }

        QPainter p(&result);
        //kDebug() << token << scalable << scaledSize << imgSize;
        if (scalable) {
            // tiling is ignored for scalable wallpapers
            QSvgRenderer svg(file);
            if (m_restart) {
                continue;
            }
            svg.render(&p);
        } else {
            if (scaledSize != imgSize) {
                img = img.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }

            if (m_restart) {
                continue;
            }

            if (tiled) {
                for (int x = pos.x(); x < size.width(); x += scaledSize.width()) {
                    for (int y = pos.y(); y < size.height(); y += scaledSize.height()) {
                        p.drawImage(QPoint(x, y), img);
                        if (m_restart) {
                            continue;
                        }
                    }
                }
            } else {
                p.drawImage(pos, img);
            }
        }

        // signal we're done
        emit done(this, token, result, file, size, method, color);
        break;
    }
}

int WallpaperRenderThread::currentToken()
{
    return m_currentToken;
}

} // namespace Plasma

#include "wallpaperrenderthread_p.moc"

