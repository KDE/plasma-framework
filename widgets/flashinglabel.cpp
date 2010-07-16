/*
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>
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
 *   51 Franklin Stre
 *   et, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "flashinglabel.h"

#include <QtCore/QString>
#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <QtCore/QWeakPointer>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

#include <kdebug.h>

#include <plasma/animator.h>
#include <plasma/animations/animation.h>
#include <plasma/theme.h>

using namespace Plasma;

class Plasma::FlashingLabelPrivate
{
    public:
        enum FlashingLabelType {
            Text,
            Pixmap
        };
        enum State {
            Visible,
            Invisible
        };

        FlashingLabelPrivate(FlashingLabel *flash)
            : q(flash),
              defaultDuration(3000),
              type(FlashingLabelPrivate::Text),
              color(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor)),
              state(FlashingLabelPrivate::Invisible),
              autohide(false)
        {
            fadeOutTimer.setInterval(defaultDuration);
            fadeOutTimer.setSingleShot(true);
            fadeInTimer.setInterval(0);
            fadeInTimer.setSingleShot(true);
            QObject::connect(Theme::defaultTheme(), SIGNAL(themeChanged()), q, SLOT(setPalette()));
        }

        ~FlashingLabelPrivate() { }

        void renderPixmap(const QSize &size);
        void setupFlash(int duration);
        void elementAnimationFinished();
        void setPalette();

        FlashingLabel *q;
        int defaultDuration;
        FlashingLabelType type;
        QTimer fadeInTimer;
        QTimer fadeOutTimer;
        QString text;
        QColor color;
        QFont font;
        QPixmap pixmap;

        QWeakPointer<Plasma::Animation> anim;
        QPixmap renderedPixmap;

        QTextOption textOption;
        Qt::Alignment alignment;

        State state;
        bool autohide;
};

FlashingLabel::FlashingLabel(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new FlashingLabelPrivate(this))
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setCacheMode(NoCache);
    connect(&d->fadeOutTimer, SIGNAL(timeout()), this, SLOT(fadeOut()));
    connect(&d->fadeInTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));
}

FlashingLabel::~FlashingLabel()
{
    delete d;
}

int FlashingLabel::duration() const
{
    return d->defaultDuration;
}

void FlashingLabel::setDuration(int duration)
{
    if (duration < 1) {
        return;
    }

    d->defaultDuration = duration;
}

QColor FlashingLabel::color() const
{
    return d->color;
}

void FlashingLabel::setColor(const QColor &color)
{
    d->color = color;
}

QFont FlashingLabel::font() const
{
    return d->font;
}

void FlashingLabel::setFont(const QFont &font)
{
    d->font = font;
}

void FlashingLabel::flash(const QString &text, int duration, const QTextOption &option)
{
    if (text.isEmpty()) {
        return;
    }

    //kDebug() << duration << text;
    d->type = FlashingLabelPrivate::Text;
    d->text = text;
    d->textOption = option;
    d->setupFlash(duration);
}

void FlashingLabel::flash(const QPixmap &pixmap, int duration, Qt::Alignment align)
{
    if (pixmap.isNull()) {
        return;
    }

    d->type = FlashingLabelPrivate::Pixmap;
    d->pixmap = pixmap;
    d->alignment = align;
    d->setupFlash(duration);
}

void FlashingLabel::setAutohide(bool autohide)
{
    d->autohide = autohide;

    if (autohide) {
        if (d->anim.data()) {
            connect(d->anim.data(), SIGNAL(finished()), this, SLOT(elementAnimationFinished()));
        }
    } else if (d->anim.data()) {
        disconnect(d->anim.data(), SIGNAL(finished()), this, SLOT(elementAnimationFinished()));
    }
}

bool FlashingLabel::autohide() const
{
    return d->autohide;
}

void FlashingLabel::kill()
{
    d->fadeInTimer.stop();
    if (d->state == FlashingLabelPrivate::Visible) {
        fadeOut();
    }
}

void FlashingLabel::fadeIn()
{
    //kDebug();
    if (d->autohide) {
        show();
    }

    d->state = FlashingLabelPrivate::Visible;
    if (!d->anim.data()) {
        d->anim = Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
        Plasma::Animation *animation = d->anim.data();
        animation->setProperty("startPixmap", d->renderedPixmap);
        animation->setTargetWidget(this);
        animation->start();
    } else {
        Plasma::Animation *animation = d->anim.data();
        if (animation->state() == QAbstractAnimation::Running) {
            animation->stop();
            animation->start();
        }
    }
}

void FlashingLabel::fadeOut()
{
    if (d->state == FlashingLabelPrivate::Invisible) {
        return;    // FlashingLabel was already killed - do not animate again
    }

    d->state = FlashingLabelPrivate::Invisible;
    if (d->anim.data()) {
        Plasma::Animation *animation = d->anim.data();
        animation->setProperty("direction", QAbstractAnimation::Backward);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        d->anim = Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
        Plasma::Animation *animation = d->anim.data();
        animation->setProperty("direction", QAbstractAnimation::Backward);
        animation->setProperty("startPixmap", d->renderedPixmap);
        animation->setTargetWidget(this);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void FlashingLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (d->anim.data() && d->anim.data()->state() == QAbstractAnimation::Running) {
        Plasma::Animation *animation = d->anim.data();
        painter->drawPixmap(0, 0, qvariant_cast<QPixmap>(animation->property("currentPixmap")));
    } else if (d->state == FlashingLabelPrivate::Visible) {
        painter->drawPixmap(0, 0, d->renderedPixmap);
    }
}

QSizeF FlashingLabel::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::PreferredSize) {
        QFontMetrics fm(d->font);
        return fm.boundingRect(d->text).size();
    }
    return QGraphicsWidget::sizeHint(which, constraint);
}

void FlashingLabelPrivate::renderPixmap(const QSize &size)
{
    if (renderedPixmap.size() != size) {
        renderedPixmap = QPixmap(size);
    }
    renderedPixmap.fill(Qt::transparent);

    QPainter painter(&renderedPixmap);
    if (type == FlashingLabelPrivate::Text) {
        painter.setPen(color);
        painter.setFont(font);
        painter.drawText(QRect(QPoint(0, 0), size), text, textOption);
    } else if (type == FlashingLabelPrivate::Pixmap) {
        QPoint p;

        if(alignment & Qt::AlignLeft) {
            p.setX(0);
        } else if (alignment & Qt::AlignRight) {
            p.setX(size.width() - pixmap.width());
        } else {
            p.setX((size.width() - pixmap.width()) / 2);
        }

        if (alignment & Qt::AlignTop) {
            p.setY(0);
        } else if (alignment & Qt::AlignRight) {
            p.setY(size.height() - pixmap.height());
        } else {
            p.setY((size.height() - pixmap.height()) / 2);
        }

        painter.drawPixmap(p, pixmap);
    }
    painter.end();

    if (anim.data()) {
        Plasma::Animation *animation = anim.data();
        animation->setProperty("startPixmap", renderedPixmap);
    }
}

void FlashingLabelPrivate::setupFlash(int duration)
{
    fadeOutTimer.stop();
    fadeOutTimer.setInterval(duration > 0 ? duration : defaultDuration);

    renderPixmap(q->size().toSize());
    if (state != FlashingLabelPrivate::Visible) {
        fadeInTimer.start();
    } else {
        q->update();
    }

    if (fadeOutTimer.interval() > 0) {
        fadeOutTimer.start();
    }
}

void FlashingLabelPrivate::elementAnimationFinished()
{
    if (autohide && state == FlashingLabelPrivate::Invisible && anim.data()) {
        q->hide();
    }
}

void FlashingLabelPrivate::setPalette()
{
    color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    q->update();
}

#include "flashinglabel.moc"
