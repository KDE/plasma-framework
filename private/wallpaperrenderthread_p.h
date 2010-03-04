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

#ifndef PLASMA_WALLPAPERRENDERTHREAD_P_H
#define PLASMA_WALLPAPERRENDERTHREAD_P_H

#include <QColor>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "plasma/wallpaper.h"

namespace Plasma
{

class WallpaperRenderThread : public QThread
{
    Q_OBJECT

public:
    WallpaperRenderThread(QObject *parent = 0);
    virtual ~WallpaperRenderThread();

    int render(const QString &file, const QSize &size,
               Wallpaper::ResizeMethod, const QColor &color);

    void setSize(const QSize &size);
    void setRatio(float ratio);
    int currentToken();

Q_SIGNALS:
    void done(WallpaperRenderThread *renderer,int token, const QImage &pixmap,
              const QString &sourceImagePath, const QSize &size,
              int resizeMethod, const QColor &color);
protected:
    virtual void run();

private:
    QMutex m_mutex; // to protect parameters
    QWaitCondition m_condition;

    // protected by mutex
    int m_currentToken;
    QString m_file;
    QColor m_color;
    QSize m_size;
    float m_ratio;
    Wallpaper::ResizeMethod m_method;

    bool m_abort;
    bool m_restart;
};

} // namespace Plasma

#endif // PLASMA_WALLPAPERRENDERTHREAD_P_H
