/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "videowidget.h"

#include <kurl.h>

#include <phonon/videowidget.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>


namespace Plasma
{

class VideoWidgetPrivate
{
public:
    VideoWidgetPrivate()
    {
    }

    ~VideoWidgetPrivate()
    {
    }

    Phonon::VideoWidget *videoWidget;
    Phonon::AudioOutput *audioOutput;
    Phonon::MediaObject *media;
};

VideoWidget::VideoWidget(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new VideoWidgetPrivate)
{
    d->videoWidget = new Phonon::VideoWidget;
    d->audioOutput = new Phonon::AudioOutput(this);
    d->media = new Phonon::MediaObject(this);
    //it appears that the path has to be created BEFORE setting the proxy
    Phonon::createPath(d->media, d->videoWidget);
    Phonon::createPath(d->media, d->audioOutput);


    setWidget(d->videoWidget);

    connect(d->media, SIGNAL(tick(qint64)), this, SIGNAL(tick(qint64)));
    connect(d->media, SIGNAL(aboutToFinish()), this, SIGNAL(aboutToFinish()));
}

VideoWidget::~VideoWidget()
{
    delete d;
}

Phonon::MediaObject *VideoWidget::mediaObject() const
{
    return d->media;
}

Phonon::AudioOutput *VideoWidget::audioOutput() const
{
    return d->audioOutput;
}

void VideoWidget::setUrl(const QString &url)
{
    d->media->setCurrentSource(Phonon::MediaSource(url));
}

QString VideoWidget::url() const
{
    return d->media->currentSource().url().toString();
}

void VideoWidget::play()
{
    d->media->play();
}

void VideoWidget::pause()
{
    d->media->pause();
}

void VideoWidget::stop()
{
    d->media->stop();
}


void VideoWidget::seek(qint64 time)
{
    d->media->seek(time);
}

qint64 VideoWidget::currentTime() const
{
    return d->media->currentTime();
}

qint64 VideoWidget::totalTime() const
{
    return d->media->totalTime();
}

qint64 VideoWidget::remainingTime() const
{
    return d->media->remainingTime();
}

void VideoWidget::setStyleSheet(const QString &stylesheet)
{
    d->videoWidget->setStyleSheet(stylesheet);
}

QString VideoWidget::styleSheet()
{
    return d->videoWidget->styleSheet();
}

Phonon::VideoWidget *VideoWidget::nativeWidget() const
{
    return d->videoWidget;
}

} // namespace Plasma

#include <videowidget.moc>

