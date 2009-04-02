/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "renderthread.h"

#include <QPainter>
#include <QFile>
#include <KDebug>
#include <KSvgRenderer>

RenderThread::RenderThread()
: m_current_token(-1)
, m_size(0, 0)
{
    m_abort = false;
    m_restart = false;
}

RenderThread::~RenderThread()
{
    {
        // abort computation
        QMutexLocker lock(&m_mutex);
        m_abort = true;
        m_condition.wakeOne();
    }

    wait();
}

void RenderThread::setSize(const QSize& size)
{
    QMutexLocker lock(&m_mutex);
    m_size = size;
}

int RenderThread::render(const QString &file,
                          const QColor &color,
                          Background::ResizeMethod method)
{
    int token;
    {
        QMutexLocker lock(&m_mutex);
        m_file = file;
        m_color = color;
        m_method = method;
        m_restart = true;
        token = ++m_current_token;
    }

    if (!isRunning()) {
        start();
    } else {
        m_condition.wakeOne();
    }

    return token;
}

void RenderThread::run()
{
    QString file;
    QColor color;
    QSize size;
    qreal ratio;
    Background::ResizeMethod method;
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
            token = m_current_token;
            file = m_file;
            color = m_color;
            size = m_size;
            ratio = m_size.width() / qreal(m_size.height());
            method = m_method;
        }

        QImage result(size, QImage::Format_ARGB32_Premultiplied);
        result.fill(color.rgba());

        if (file.isEmpty() || !QFile::exists(file)) {
            emit done(token, result);
            continue;
        }

        QPoint pos(0, 0);
        bool tiled = false;
        bool scalable = file.endsWith("svg") || file.endsWith("svgz");
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
            kDebug() << "loaded with" << imgSize << ratio;
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
        case Background::Scale:
            imgSize *= ratio;
            scaledSize = size;
            break;
        case Background::Center:
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
        case Background::Maxpect: {
            imgSize *= ratio;
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
        case Background::ScaleCrop: {
            imgSize *= ratio;
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
        case Background::Tiled:
            scaledSize = imgSize;
            tiled = true;
            break;
        case Background::CenterTiled:
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
        kDebug() << token << scalable << scaledSize << imgSize;
        if (scalable) {
            // tiling is ignored for scalable wallpapers
            KSvgRenderer svg(file);
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
                            goto endLoop;
                        }
                    }
                }
            } else {
                p.drawImage(pos, img);
            }
        }

        // signal we're done
        emit done(token, result);
        endLoop: continue;
    }
}
