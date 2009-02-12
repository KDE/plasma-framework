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

#include "videowidget.h"

#include <QUrl>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>

#include <kicon.h>
#include <kfiledialog.h>

#include <phonon/videowidget.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>

#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/slider.h>

namespace Plasma
{

class VideoWidgetPrivate
{
public:
    VideoWidgetPrivate()
         : ticking(false),
           shownControls(VideoWidget::NoControls),
           controlsWidget(0)
    {
    }

    ~VideoWidgetPrivate()
    {
    }

    void playPause();
    void ticked(qint64 progress);
    void totalTimeChanged(qint64 time);
    void setPosition(int newProgress);
    void setVolume(int value);
    void volumeChanged(qreal value);
    void showOpenFileDialog();
    void openFile(const QString &path);
    void stateChanged(Phonon::State newState, Phonon::State oldState);


    Phonon::VideoWidget *videoWidget;
    Phonon::AudioOutput *audioOutput;
    Phonon::MediaObject *media;

    bool ticking;

    //control widgets
    VideoWidget::Controls shownControls;
    QGraphicsWidget *controlsWidget;
    IconWidget *playPauseButton;
    Slider *progress;
    Slider *volume;
    IconWidget *openFileButton;
};

void VideoWidgetPrivate::playPause()
{
    if (media->state() == Phonon::PlayingState) {
        media->pause();
    } else {
        media->play();
    }
}

void VideoWidgetPrivate::ticked(qint64 newProgress)
{
    ticking = true;
    progress->setValue(newProgress);
    ticking = false;
}

void VideoWidgetPrivate::totalTimeChanged(qint64 time)
{
    ticking = true;
    //FIXME: this will break for veeery long stuff, butPhonon::SeekSlider seems to have the same problem
    progress->setRange(0, time);
    ticking = false;
}

void VideoWidgetPrivate::setPosition(int progress)
{
    if (!ticking) {
        media->seek(progress);
    }
}

void VideoWidgetPrivate::setVolume(int value)
{
     audioOutput->setVolume(qreal(value)/100.0);
}

void VideoWidgetPrivate::volumeChanged(qreal value)
{
     volume->setValue(value*100);
}

void VideoWidgetPrivate::showOpenFileDialog()
{
    openFile(KFileDialog::getOpenFileName());
}

void VideoWidgetPrivate::openFile(const QString &path)
{
    media->setCurrentSource(Phonon::MediaSource(path));
    media->play();
}

void VideoWidgetPrivate::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState)

    if (newState == Phonon::PlayingState) {
        playPauseButton->setIcon("media-playback-pause");
    } else {
        playPauseButton->setIcon("media-playback-start");
    }
}



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

void VideoWidget::setShownControls(Controls controls)
{
    d->shownControls = controls;

    QGraphicsLinearLayout *controlsLayout;
    if (controls != NoControls && d->controlsWidget == 0) {
        d->controlsWidget = new QGraphicsWidget(this);
        controlsLayout = new QGraphicsLinearLayout(Qt::Horizontal, d->controlsWidget);
        d->controlsWidget->setPos(0,0);
        d->controlsWidget->show();
        d->controlsWidget->resize(size().width(), d->controlsWidget->size().height());
    //controls == NoControls
    } else if (d->controlsWidget != 0) {
        d->controlsWidget->deleteLater();
        d->controlsWidget = 0;

        //disconnect all the stuff that wasn't automatically disconnected 'cause widget deaths
        disconnect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
        disconnect(d->media, SIGNAL(tick(qint64)), this, SLOT(ticked(qint64)));
        disconnect(d->media, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));
        disconnect(d->audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
        return;
    }


    d->playPauseButton = new IconWidget(d->controlsWidget);
    d->playPauseButton->setIcon("media-playback-start");
    controlsLayout->addItem(d->playPauseButton);
    connect(d->playPauseButton, SIGNAL(clicked()), this, SLOT(playPause()));
    connect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));

    d->playPauseButton->setVisible(controls&PlayPause);



    d->progress = new Slider(d->controlsWidget);
    d->progress->setMinimum(0);
    d->progress->setMaximum(100);
    d->progress->setOrientation(Qt::Horizontal);
    controlsLayout->addItem(d->progress);
    controlsLayout->setStretchFactor(d->progress, 4);

    connect(d->media, SIGNAL(tick(qint64)), this, SLOT(ticked(qint64)));
    connect(d->media, SIGNAL(totalTimeChanged(qint64)), SLOT(totalTimeChanged(qint64)));
    connect(d->progress, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));

    d->progress->setVisible(controls&Progress);


    d->volume = new Slider(d->controlsWidget);
    d->volume->setMinimum(0);
    d->volume->setMaximum(100);
    d->volume->setValue(100);
    d->volume->setOrientation(Qt::Horizontal);
    controlsLayout->addItem(d->volume);

    connect(d->volume, SIGNAL(valueChanged(int)), SLOT(setVolume(int)));
    connect(d->audioOutput, SIGNAL(volumeChanged(qreal)), SLOT(volumeChanged(qreal)));

    d->volume->setVisible(controls&Volume);


    d->openFileButton = new IconWidget(d->controlsWidget);
    d->openFileButton->setIcon(KIcon("document-open"));
    connect(d->openFileButton, SIGNAL(clicked()), this, SLOT(showOpenFileDialog()));
    controlsLayout->addItem(d->openFileButton);

    d->openFileButton->setVisible(controls&OpenFile);
}

VideoWidget::Controls VideoWidget::shownControls() const
{
    return d->shownControls;
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


void VideoWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);

    if (d->controlsWidget) {
        d->controlsWidget->resize(event->newSize().width(), d->controlsWidget->size().height());
    }
}

} // namespace Plasma

#include <videowidget.moc>

