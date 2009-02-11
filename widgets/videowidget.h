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

#include <plasma/plasma_export.h>

namespace Phonon {
    class VideoWidget;
    class MediaObject;
    class AudioOutput;
}

class KUrl;

namespace Plasma
{

class VideoWidgetPrivate;

/**
 * @class VideoWidget plasma/widgets/videowidget.h <Plasma/Widgets/VideoWidget>
 * a Video playing widget via Phonon, it encloses the
 * Phonon::MediaObject and Phonon::AudioOutput too
 *
 * @short Provides a video player widget
 * @since KDE4.3
 */
class PLASMA_EXPORT VideoWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QString currentTime READ currentTime)
    Q_PROPERTY(QString totalTime READ totalTime)
    Q_PROPERTY(QString remainingTime READ remainingTime)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)

public:
    explicit VideoWidget(QGraphicsWidget *parent = 0);
    ~VideoWidget();

    /**
     * Load a certain url that can be a local file or a remote one
     * @arg path resource to play
     */
    void setUrl(const QString &url);

    /**
     * @return the url (local or remote) we are playing
     */
    QString url() const;

    /**
     * @return the Phonon::MediaObject being used
     * @see Phonon::MediaObject
     */
    Q_INVOKABLE Phonon::MediaObject *mediaObject() const;

    /**
     * @return the Phonon::AudioOutput being used
     * @see Phonon::AudioOutput
     */
    Q_INVOKABLE Phonon::AudioOutput *audioOutput() const;

    /**
     * @return the current time of the current media file
     */
    qint64 currentTime() const;

    /**
     * @return the total playing time of the current media file
     */
    qint64 totalTime() const;

    /**
     * @return the time remaining to the current media file
     */
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
    /**
     * Play the current file
     */
    void play();

    /**
     * Pause the current file
     */
    void pause();

    /**
     * Stop the current file
     */
    void stop();

    /**
     * Jump at a given millisecond in the current file
     * @arg time where we want to jump
     */
    void seek(qint64 time);

Q_SIGNALS:
    /**
     * Emitted regularly when the playing is progressing
     * @arg time where we are
     */
    void tick(qint64 time);

    /**
     * Emitted an instant before the playback is finished
     */
    void aboutToFinish();

private:
    VideoWidgetPrivate * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
