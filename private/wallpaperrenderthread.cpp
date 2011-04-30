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

#include <QCoreApplication>
#include <QPainter>
#include <QFile>
#include <QSvgRenderer>

#ifndef PLASMA_NO_SOLID
#include <solid/device.h>
#include <solid/deviceinterface.h>
#endif

#include <kdebug.h>

namespace Plasma
{

int WallpaperRenderRequest::s_token = 0;
int WallpaperRenderThread::s_rendererCount = 0;
QQueue<WallpaperRenderRequest> WallpaperRenderThread::s_renderQueue;

WallpaperRenderThread::WallpaperRenderThread(const WallpaperRenderRequest &request, QObject *parent)
    : QThread(parent),
      m_request(request),
      m_abort(false)
{
    qRegisterMetaType<WallpaperRenderRequest>("WallpaperRenderRequest");
    if (!request.requester) {
        deleteLater();
        return;
    }

    ++s_rendererCount;
    connect(this, SIGNAL(done(WallpaperRenderRequest,QImage)),
            request.requester.data(), SLOT(newRenderCompleted(WallpaperRenderRequest,QImage)));
}

WallpaperRenderThread::~WallpaperRenderThread()
{
    kDebug() << "rendering done";
    m_abort = true;
    wait();
    --s_rendererCount;
    checkQueue();
}

void WallpaperRenderThread::render(const WallpaperRenderRequest &request)
{
    QObject *requester = request.requester.data();
    if (!requester) {
        return;
    }

    // remove all dead requests and requests previously made for the same parent
    QMutableListIterator<WallpaperRenderRequest> it(s_renderQueue);
    while (it.hasNext()) {
        const WallpaperRenderRequest &request = it.next();

        if (!request.requester || request.requester.data() == requester) {
            it.remove();
        }
    }

    s_renderQueue.append(request);
    checkQueue();
}

void WallpaperRenderThread::checkQueue()
{
    if (s_renderQueue.isEmpty()) {
        return;
    }

    if (QCoreApplication::closingDown()) {
        s_renderQueue.clear();
        return;
    }

#ifndef PLASMA_NO_SOLID
    const int numProcs = qMax(1, Solid::Device::listFromType(Solid::DeviceInterface::Processor).count());
#else
    const int numProcs = 1;
#endif
    kDebug() << "checking rendering against" << s_rendererCount << numProcs;
    if (s_rendererCount < numProcs) {
        WallpaperRenderThread *renderThread = new WallpaperRenderThread(s_renderQueue.dequeue());
        renderThread->start();
    }
}

void WallpaperRenderThread::run()
{
    kDebug() << "rendering wallpaper" << m_request.file;
    QImage result(m_request.size, QImage::Format_ARGB32_Premultiplied);
    result.fill(m_request.color.rgba());

    if (m_request.file.isEmpty() || !QFile::exists(m_request.file)) {
        if (!m_abort) {
            emit done(m_request, result);
        }

        kDebug() << "oh, fuck it";
        deleteLater();
        return;
    }

    QPoint pos(0, 0);
    //const float ratio = qMax(float(1), m_request.size.width() / float(m_request.size.height()));
    const bool scalable = m_request.file.endsWith(QLatin1String("svg")) || m_request.file.endsWith(QLatin1String("svgz"));
    bool tiled = false;
    QSize scaledSize;
    QImage img;

    // set image size
    QSize imgSize(1, 1);
    if (scalable) {
        // scalable: image can be of any size
        imgSize = imgSize.expandedTo(m_request.size);
    } else {
        // otherwise, use the natural size of the loaded image
        img = QImage(m_request.file);
        imgSize = imgSize.expandedTo(img.size());
        //kDebug() << "loaded with" << imgSize << ratio;
    }

    // set render parameters according to resize mode
    switch (m_request.resizeMethod)
    {
        case Wallpaper::ScaledResize:
            scaledSize = m_request.size;
            break;
        case Wallpaper::CenteredResize:
            scaledSize = imgSize;
            pos = QPoint((m_request.size.width() - scaledSize.width()) / 2,
                         (m_request.size.height() - scaledSize.height()) / 2);

            //If the picture is bigger than the screen, shrink it
            if (m_request.size.width() < imgSize.width() && imgSize.width() > imgSize.height()) {
                int width = m_request.size.width();
                int height = width * scaledSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
                pos = QPoint((m_request.size.width() - scaledSize.width()) / 2,
                        (m_request.size.height() - scaledSize.height()) / 2);
            } else if (m_request.size.height() < imgSize.height()) {
                int height = m_request.size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
                pos = QPoint((m_request.size.width() - scaledSize.width()) / 2,
                             (m_request.size.height() - scaledSize.height()) / 2);
            }

            break;
        case Wallpaper::MaxpectResize: {
            float xratio = (float) m_request.size.width() / imgSize.width();
            float yratio = (float) m_request.size.height() / imgSize.height();
            if (xratio > yratio) {
                int height = m_request.size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            } else {
                int width = m_request.size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            }

            pos = QPoint((m_request.size.width() - scaledSize.width()) / 2,
                         (m_request.size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::ScaledAndCroppedResize: {
            float xratio = (float) m_request.size.width() / imgSize.width();
            float yratio = (float) m_request.size.height() / imgSize.height();
            if (xratio > yratio) {
                int width = m_request.size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            } else {
                int height = m_request.size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            }
            pos = QPoint((m_request.size.width() - scaledSize.width()) / 2,
                         (m_request.size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::TiledResize:
            scaledSize = imgSize;
            tiled = true;
            break;
        case Wallpaper::CenterTiledResize:
            scaledSize = imgSize;
            pos = QPoint(-scaledSize.width() + ((m_request.size.width() - scaledSize.width()) / 2) % scaledSize.width(),
                         -scaledSize.height() + ((m_request.size.height() - scaledSize.height()) / 2) % scaledSize.height());
            tiled = true;
            break;
    }

    QPainter p(&result);
    //kDebug() << token << scalable << scaledSize << imgSize;
    if (scalable) {
        // tiling is ignored for scalable wallpapers
        QSvgRenderer svg(m_request.file);
        if (m_abort) {
            deleteLater();
        kDebug() << "oh, fuck it 2";
            return;
        }
        svg.render(&p);
    } else {
        if (scaledSize != imgSize) {
            img = img.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        if (m_abort) {
            deleteLater();
        kDebug() << "oh, fuck it 3";
            return;
        }

        if (tiled) {
            for (int x = pos.x(); x < m_request.size.width(); x += scaledSize.width()) {
                for (int y = pos.y(); y < m_request.size.height(); y += scaledSize.height()) {
                    p.drawImage(QPoint(x, y), img);
                    if (m_abort) {
        kDebug() << "oh, fuck it 4";
                        deleteLater();
                        return;
                    }
                }
            }
        } else {
            p.drawImage(pos, img);
        }
    }

    // signal we're done
    if (!m_abort) {
        kDebug() << "*****************************************************";
        emit done(m_request, result);
    }

    deleteLater();
}

} // namespace Plasma

#include "wallpaperrenderthread_p.moc"

