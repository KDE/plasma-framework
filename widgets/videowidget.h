/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_VIDEOWIDGET_H
#define PLASMA_VIDEOWIDGET_H

#include <QtGui/QGraphicsProxyWidget>

namespace Phonon {
    class VideoWidget;
    class MediaObject;
    class AudioOutput;
}

namespace Plasma
{

class VideoWidgetPrivate;

/**
 * @class VideoWidget plasma/widgets/videowidget.h <Plasma/Widgets/VideoWidget>
 *
 * @short Provides a video player widget
 */
class VideoWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QString file READ file WRITE setFile)
    Q_PROPERTY(QString currentTime READ currentTime)
    Q_PROPERTY(QString totalTime READ totalTime)
    Q_PROPERTY(QString remainingTime READ remainingTime)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)

public:
    explicit VideoWidget(QGraphicsWidget *parent = 0);
    ~VideoWidget();

    void setFile(const QString &path);

    //TODO: decide ifsupporting just file from the api or even just make use just MediaObject wtith no api here
    QString file() const;

    Q_INVOKABLE Phonon::MediaObject *mediaObject() const;

    Q_INVOKABLE Phonon::AudioOutput *audioOutput() const;

    //FIXME: bunch of stuff wrapped from MediaObject: makes sense for scripting or just use MediaObject also for scripts?
    qint64 currentTime() const;

    qint64 totalTime() const;

    qint64 remainingTime() const;

    /**
     * Sets the stylesheet used to control the visual display of this VideoWidget
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * @return the native widget wrapped by this VideoWidget
     */
    Phonon::VideoWidget *nativeWidget() const;

public Q_SLOTS:
    void play();

    void pause();

    void stop();

    void seek(qint64 time);

Q_SIGNALS:
    void tick(qint64 time);
    void aboutToFinish();

private:
    VideoWidgetPrivate * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
