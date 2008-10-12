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

#include "flash.h"

#include <QtCore/QString>
#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

#include <KDebug>

#include <plasma/animator.h>

using namespace Plasma;

class Plasma::FlashPrivate
{
    public:
        enum FlashType {
            Text,
            Pixmap
        };
        enum State {
            Visible,
            Invisible
        };

        FlashPrivate()
            : defaultDuration(3000),
              type(FlashPrivate::Text),
              color(Qt::black),
              animId(0),
              state(FlashPrivate::Invisible)
        {
            //TODO: put this on a diet by using timerEvent instead?
            fadeOutTimer.setInterval(defaultDuration);
            fadeOutTimer.setSingleShot(true);
            fadeInTimer.setInterval(0);
            fadeInTimer.setSingleShot(true);
        }
        ~FlashPrivate() { }

        void renderPixmap(const QSize &size);
        void setupFlash(Flash *flash, int duration);

        int defaultDuration;
        FlashType type;
        QTimer fadeInTimer;
        QTimer fadeOutTimer;
        QString text;
        QColor color;
        QFont font;
        QPixmap pixmap;

        int animId;
        QPixmap renderedPixmap;

        QTextOption textOption;
        Qt::Alignment alignment;

        State state;
};

Flash::Flash(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new FlashPrivate)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setCacheMode(NoCache);
    connect(&d->fadeOutTimer, SIGNAL(timeout()), this, SLOT(fadeOut()));
    connect(&d->fadeInTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));
}

Flash::~Flash()
{
    delete d;
}

void Flash::setDuration(int duration)
{
    if (duration < 1) {
        return;
    }

    d->defaultDuration = duration;
}

void Flash::setColor(const QColor &color)
{
    d->color = color;
}

void Flash::setFont(const QFont &font)
{
    d->font = font;
}

void Flash::flash(const QString &text, int duration, const QTextOption &option)
{
    if (text.isEmpty()) {
        return;
    }

    //kDebug() << duration << text;
    d->type = FlashPrivate::Text;
    d->text = text;
    d->textOption = option;
    d->setupFlash(this, duration);
}

void Flash::flash(const QPixmap &pixmap, int duration, Qt::Alignment align)
{
    if (pixmap.isNull()) {
        return;
    }

    d->type = FlashPrivate::Pixmap;
    d->pixmap = pixmap;
    d->alignment = align;
    d->setupFlash(this, duration);
}

void Flash::kill()
{
    d->fadeInTimer.stop();
    if (d->state == FlashPrivate::Visible) {
        fadeOut();
    }
}

void Flash::fadeIn()
{
    //kDebug();
    d->state = FlashPrivate::Visible;
    d->animId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);
    Plasma::Animator::self()->setInitialPixmap(d->animId, d->renderedPixmap);
}

void Flash::fadeOut()
{
    if (d->state == FlashPrivate::Invisible) {
        return;    // Flash was already killed - do not animate again
    }

    d->state = FlashPrivate::Invisible;
    d->animId = Plasma::Animator::self()->animateElement(
        this, Plasma::Animator::DisappearAnimation);
    Plasma::Animator::self()->setInitialPixmap(d->animId, d->renderedPixmap);
}

void Flash::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (d->animId && !Plasma::Animator::self()->currentPixmap(d->animId).isNull()) {
        painter->drawPixmap(0, 0, Plasma::Animator::self()->currentPixmap(d->animId));
    } else {
        d->animId = 0;

        if (d->state == FlashPrivate::Visible) {
            painter->drawPixmap(0, 0, d->renderedPixmap);
        }
    }
}

void FlashPrivate::renderPixmap(const QSize &size)
{
    if (renderedPixmap.size() != size) {
        renderedPixmap = QPixmap(size);
    }
    renderedPixmap.fill(Qt::transparent);

    QPainter painter(&renderedPixmap);
    if (type == FlashPrivate::Text) {
        painter.setPen(color);
        painter.setFont(font);
        painter.drawText(QRect(QPoint(0, 0), size), text, textOption);
    } else if (type == FlashPrivate::Pixmap) {
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

    if (animId) {
        Plasma::Animator::self()->setInitialPixmap(animId, renderedPixmap);
    }
}

void FlashPrivate::setupFlash(Flash *flash, int duration)
{
    fadeOutTimer.stop();
    fadeOutTimer.setInterval(duration > 0 ? duration : defaultDuration);

    renderPixmap(flash->size().toSize());
    if (state != FlashPrivate::Visible) {
        fadeInTimer.start();
    } else {
        flash->update();
    }

    if (fadeOutTimer.interval() > 0) {
        fadeOutTimer.start();
    }
}

#include "flash.moc"
