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
#include <QTimer>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>

#include <kicon.h>
#include <kfiledialog.h>

#include <phonon/videowidget.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>

#include <plasma/animations/animation.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/slider.h>
#include <plasma/widgets/frame.h>

namespace Plasma
{

class VideoWidgetPrivate
{
public:
    VideoWidgetPrivate(VideoWidget *video)
         : q(video),
           ticking(false),
           forceControlsVisible(false),
           animation(0),
           hideTimer(0),
           shownControls(VideoWidget::NoControls),
           controlsWidget(0),
           previousButton(0),
           playButton(0),
           pauseButton(0),
           stopButton(0),
           playPauseButton(0),
           nextButton(0),
           progress(0),
           volume(0),
           openFileButton(0)
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
    void animateControlWidget(bool show);
    void hideControlWidget();
    void slidingCompleted();
    bool spaceForControlsAvailable();


    VideoWidget *q;

    Phonon::VideoWidget *videoWidget;
    Phonon::AudioOutput *audioOutput;
    Phonon::MediaObject *media;

    bool ticking;
    bool forceControlsVisible;

    //control widgets
    Plasma::Animation *animation;
    QTimer *hideTimer;
    VideoWidget::Controls shownControls;
    Plasma::Frame *controlsWidget;
    IconWidget *previousButton;
    IconWidget *playButton;
    IconWidget *pauseButton;
    IconWidget *stopButton;
    IconWidget *playPauseButton;
    IconWidget *nextButton;
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

    if (playPauseButton) {
        if (newState == Phonon::PlayingState) {
            playPauseButton->setIcon("media-playback-pause");
        } else {
            playPauseButton->setIcon("media-playback-start");
        }
    }
}

void VideoWidgetPrivate::animateControlWidget(bool show)
{
    if (!controlsWidget || controlsWidget->isVisible() == show) {
        return;
    }

    const int distance = controlsWidget->size().height();
    if (!controlsWidget->isVisible()) {
        controlsWidget->setPos(0, -distance);
        controlsWidget->show();
    }

    //clip only when animating
    q->setFlags(q->flags()|QGraphicsItem::ItemClipsChildrenToShape);

    if (!animation) {
        animation = Plasma::Animator::create(Plasma::Animator::SlideAnimation, q);
        animation->setTargetWidget(controlsWidget);
        animation->setProperty("movementDirection", Animation::MoveDown);
        q->connect(animation, SIGNAL(finished()), q, SLOT(slidingCompleted()));
    }

    animation->setProperty("distance", distance);
    animation->setProperty("direction", show? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation->start();
}

void VideoWidgetPrivate::hideControlWidget()
{
    animateControlWidget(false);
}

void VideoWidgetPrivate::slidingCompleted()
{
    if (!controlsWidget) {
        return;
    }

    //usually don't clip
    q->setFlags(q->flags()^QGraphicsItem::ItemClipsChildrenToShape);

    if (controlsWidget->pos().y() < 0) {
        controlsWidget->hide();
    } else if (!forceControlsVisible) {
        hideTimer->start(3000);
    }
}

bool VideoWidgetPrivate::spaceForControlsAvailable()
{
    if (controlsWidget) {
        QSize hint = controlsWidget->effectiveSizeHint(Qt::MinimumSize).toSize();
        return (q->size().width() >= hint.width()) &&
               (q->size().height() >= hint.height());
    } else {
        return true;
    }
}



VideoWidget::VideoWidget(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new VideoWidgetPrivate(this))
{
    d->videoWidget = new Phonon::VideoWidget;
    d->audioOutput = new Phonon::AudioOutput(this);
    d->media = new Phonon::MediaObject(this);
    //it appears that the path has to be created BEFORE setting the proxy
    Phonon::createPath(d->media, d->videoWidget);
    Phonon::createPath(d->media, d->audioOutput);


    setWidget(d->videoWidget);
    d->videoWidget->setWindowIcon(QIcon());
    setAcceptHoverEvents(true);

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

void VideoWidget::setUsedControls(const Controls controls)
{
    d->shownControls = controls;

    //kDebug()<<"Setting used controls"<<controls;

    QGraphicsLinearLayout *controlsLayout = 0;
    if (controls != NoControls && d->controlsWidget == 0) {
        d->controlsWidget = new Plasma::Frame(this);
        d->controlsWidget->setFrameShadow(Plasma::Frame::Raised);
        controlsLayout = new QGraphicsLinearLayout(Qt::Horizontal, d->controlsWidget);
        d->hideTimer = new QTimer(this);
        connect(d->hideTimer, SIGNAL(timeout()), this, SLOT(hideControlWidget()));
    //controls == NoControls
    } else if (d->controlsWidget != 0) {
        d->controlsWidget->deleteLater();
        d->hideTimer->deleteLater();
        d->controlsWidget = 0;

        //disconnect all the stuff that wasn't automatically disconnected 'cause widget deaths
        disconnect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
        disconnect(d->media, SIGNAL(tick(qint64)), this, SLOT(ticked(qint64)));
        disconnect(d->media, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));
        disconnect(d->audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
        return;
    }

    Q_ASSERT(controlsLayout);

    //empty the layout
    while (controlsLayout->count() > 0) {
        controlsLayout->removeAt(0);
    }

    if (controls&Previous) {
        if (!d->previousButton) {
            d->previousButton = new IconWidget(d->controlsWidget);
            d->previousButton->setIcon("media-playback-start");
            connect(d->playButton, SIGNAL(clicked()), this, SLOT(PreviousRequested()));
        }
        controlsLayout->addItem(d->previousButton);
    } else {
        d->previousButton->deleteLater();
        d->previousButton = 0;
    }

    if (controls&Play) {
        if (!d->playButton) {
            d->playButton = new IconWidget(d->controlsWidget);
            d->playButton->setIcon("media-playback-start");
            connect(d->playButton, SIGNAL(clicked()), this, SLOT(play()));
        }
        controlsLayout->addItem(d->playButton);
    } else {
        d->playButton->deleteLater();
        d->playButton = 0;
    }

    if (controls&Pause) {
        if (!d->pauseButton) {
            d->pauseButton = new IconWidget(d->controlsWidget);
            d->pauseButton->setIcon("media-playback-pause");
            connect(d->pauseButton, SIGNAL(clicked()), this, SLOT(pause()));
        }
        controlsLayout->addItem(d->pauseButton);
    } else {
        d->pauseButton->deleteLater();
        d->pauseButton = 0;
    }

    if (controls&Stop) {
        if (!d->stopButton) {
            d->stopButton = new IconWidget(d->controlsWidget);
            d->stopButton->setIcon("media-playback-stop");
            connect(d->stopButton, SIGNAL(clicked()), this, SLOT(stop()));
        }
        controlsLayout->addItem(d->stopButton);
    } else {
        d->stopButton->deleteLater();
        d->stopButton = 0;
    }

    if (controls&PlayPause) {
        if (!d->playPauseButton) {
            d->playPauseButton = new IconWidget(d->controlsWidget);
            d->playPauseButton->setIcon("media-playback-start");
            connect(d->playPauseButton, SIGNAL(clicked()), this, SLOT(playPause()));
        }
        controlsLayout->addItem(d->playPauseButton);
    } else {
        d->playPauseButton->deleteLater();
        d->playPauseButton = 0;
    }

    if (controls&Next) {
        if (!d->nextButton) {
            d->nextButton = new IconWidget(d->nextButton);
            d->nextButton->setIcon("media-playback-start");
            connect(d->nextButton, SIGNAL(clicked()), this, SIGNAL(nextRequested()));
        }
        controlsLayout->addItem(d->nextButton);
    } else {
        d->nextButton->deleteLater();
        d->nextButton = 0;
    }

    connect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));




    if (controls&Progress) {
        if (!d->progress) {
            d->progress = new Slider(d->controlsWidget);
            d->progress->setMinimum(0);
            d->progress->setMaximum(100);
            d->progress->setOrientation(Qt::Horizontal);
            controlsLayout->setStretchFactor(d->progress, 4);
            d->progress->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

            connect(d->media, SIGNAL(tick(qint64)), this, SLOT(ticked(qint64)));
            connect(d->media, SIGNAL(totalTimeChanged(qint64)), SLOT(totalTimeChanged(qint64)));
            connect(d->progress, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
        }
        controlsLayout->addItem(d->progress);
    } else {
        d->progress->deleteLater();
        d->progress = 0;
    }


    if (controls&Volume) {
        if (!d->volume) {
            d->volume = new Slider(d->controlsWidget);
            d->volume->setMinimum(0);
            d->volume->setMaximum(100);
            d->volume->setValue(100);
            d->volume->setOrientation(Qt::Horizontal);
            d->volume->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

            connect(d->volume, SIGNAL(valueChanged(int)), SLOT(setVolume(int)));
            connect(d->audioOutput, SIGNAL(volumeChanged(qreal)), SLOT(volumeChanged(qreal)));
        }
        controlsLayout->addItem(d->volume);
    } else {
        d->volume->deleteLater();
        d->volume = 0;
    }


    if (controls&OpenFile) {
        if (!d->openFileButton) {
            d->openFileButton = new IconWidget(d->controlsWidget);
            d->openFileButton->setIcon(KIcon("document-open"));
            connect(d->openFileButton, SIGNAL(clicked()), this, SLOT(showOpenFileDialog()));
        }
        controlsLayout->addItem(d->openFileButton);
    } else {
        d->openFileButton->deleteLater();
        d->openFileButton = 0;
    }

    controlsLayout->activate();
    d->controlsWidget->setPos(0,-d->controlsWidget->size().height());
    d->controlsWidget->resize(size().width(), d->controlsWidget->size().height());
    d->controlsWidget->hide();
}

VideoWidget::Controls VideoWidget::usedControls() const
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

void VideoWidget::setControlsVisible(bool visible)
{
    if (d->controlsWidget) {
        d->forceControlsVisible = visible;
        d->animateControlWidget(visible);
    }
}

bool VideoWidget::controlsVisible() const
{
    return d->controlsWidget != 0 && d->controlsWidget->isVisible();
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
        QSize newControlsSize(event->newSize().width(), d->controlsWidget->size().height());
        int newHeight = event->newSize().height();
        qreal leftMargin, topMargin, rightMargin, bottomMargin;
        d->controlsWidget->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

        if (newHeight/5 >= KIconLoader::SizeEnormous) {
            newControlsSize.setHeight(KIconLoader::SizeEnormous+topMargin+bottomMargin);
        } else if (newHeight/5 >= KIconLoader::SizeHuge) {
            newControlsSize.setHeight(KIconLoader::SizeHuge+topMargin+bottomMargin);
        } else if (newHeight/5 >= KIconLoader::SizeLarge) {
            newControlsSize.setHeight(KIconLoader::SizeLarge+topMargin+bottomMargin);
        } else if (newHeight/5 >= KIconLoader::SizeMedium) {
            newControlsSize.setHeight(KIconLoader::SizeMedium+topMargin+bottomMargin);
        } else {
            newControlsSize.setHeight(KIconLoader::SizeSmallMedium+topMargin+bottomMargin);
        }
        d->controlsWidget->resize(newControlsSize);

        if (d->spaceForControlsAvailable()) {
            d->animateControlWidget(false);
        }
    }
}

void VideoWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (d->controlsWidget &&
        !d->forceControlsVisible &&
        d->spaceForControlsAvailable()) {
        d->animateControlWidget(true);
    }
}

void VideoWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (d->controlsWidget && !d->forceControlsVisible) {
        d->hideTimer->start(1000);
    }
}

void VideoWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (d->forceControlsVisible || !d->controlsWidget) {
        return;
    }

    d->hideTimer->start(3000);

    if (!d->controlsWidget->isVisible() &&
        d->spaceForControlsAvailable()) {
        d->animateControlWidget(true);
    }
}

} // namespace Plasma

#include <videowidget.moc>

