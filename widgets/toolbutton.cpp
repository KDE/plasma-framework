/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "toolbutton.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QDir>
#include <QToolButton>
#include <QApplication>

#include <kicon.h>
#include <kiconeffect.h>
#include <kmimetype.h>
#include <kcolorutils.h>

#include "theme.h"
#include "svg.h"
#include "framesvg.h"
#include "animator.h"
#include "paintutils.h"

namespace Plasma
{

class ToolButtonPrivate
{
public:
    ToolButtonPrivate(ToolButton *toolButton)
        : q(toolButton),
          background(0),
          animId(0),
          fadeIn(false),
          svg(0)
    {
    }

    ~ToolButtonPrivate()
    {
        delete svg;
    }

    void setPixmap(ToolButton *q)
    {
        if (imagePath.isEmpty()) {
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml")) {
            svg = new Svg();
            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            pm = QPixmap(absImagePath);
        }

        static_cast<QToolButton*>(q->widget())->setIcon(KIcon(pm));
    }

    void syncActiveRect();
    void syncBorders();
    void animationUpdate(qreal progress);

    ToolButton *q;

    FrameSvg *background;
    int animId;
    bool fadeIn;
    qreal opacity;
    QRectF activeRect;

    QString imagePath;
    QString absImagePath;
    Svg *svg;
};

void ToolButtonPrivate::syncActiveRect()
{
    background->setElementPrefix("normal");

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);

    background->setElementPrefix("active");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    background->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    activeRect = QRectF(QPointF(0, 0), q->size());
    activeRect.adjust(left - activeLeft, top - activeTop,
                      -(right - activeRight), -(bottom - activeBottom));

    background->setElementPrefix("normal");
}

void ToolButtonPrivate::syncBorders()
{
    //set margins from the normal element
    qreal left, top, right, bottom;

    background->setElementPrefix("normal");
    background->getMargins(left, top, right, bottom);
    q->setContentsMargins(left, top, right, bottom);

    //calc the rect for the over effect
    syncActiveRect();
}

void ToolButtonPrivate::animationUpdate(qreal progress)
{
    if (progress == 1) {
        animId = 0;
        fadeIn = true;
    }

    opacity = fadeIn ? progress : 1 - progress;

    // explicit update
    q->update();
}

ToolButton::ToolButton(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ToolButtonPrivate(this))
{
    QToolButton *native = new QToolButton;
    connect(native, SIGNAL(clicked()), this, SIGNAL(clicked()));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setAutoRaise(true);

    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedFrames(true);
    d->background->setElementPrefix("normal");
    d->syncBorders();
    setAcceptHoverEvents(true);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncBorders()));
}

ToolButton::~ToolButton()
{
    delete d;
}

void ToolButton::setAutoRaise(bool raise)
{
    nativeWidget()->setAutoRaise(raise);
}

bool ToolButton::autoRaise() const
{
    return nativeWidget()->autoRaise();
}

void ToolButton::setText(const QString &text)
{
    static_cast<QToolButton*>(widget())->setText(text);
}

QString ToolButton::text() const
{
    return static_cast<QToolButton*>(widget())->text();
}

void ToolButton::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() &&
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            (path[0] == '/' || path.startsWith(":/"))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap(this);
}

QString ToolButton::image() const
{
    return d->imagePath;
}

void ToolButton::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ToolButton::styleSheet()
{
    return widget()->styleSheet();
}

QToolButton *ToolButton::nativeWidget() const
{
    return static_cast<QToolButton*>(widget());
}

void ToolButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap(this);

   if (d->background) {
        //resize all four panels
        d->background->setElementPrefix("pressed");
        d->background->resizeFrame(size());
        d->background->setElementPrefix("focus");
        d->background->resizeFrame(size());

        d->syncActiveRect();

        d->background->setElementPrefix("active");
        d->background->resizeFrame(d->activeRect.size());

        d->background->setElementPrefix("normal");
        d->background->resizeFrame(size());
   }

   QGraphicsProxyWidget::resizeEvent(event);
}

void ToolButton::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    if (!styleSheet().isNull()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QToolButton *button = nativeWidget();

    QStyleOptionToolButton buttonOpt;
    buttonOpt.initFrom(button);
    buttonOpt.icon = button->icon();
    buttonOpt.text = button->text();
    buttonOpt.iconSize = button->iconSize();
    buttonOpt.toolButtonStyle = button->toolButtonStyle();


    if (d->animId || !button->autoRaise() || (buttonOpt.state & QStyle::State_MouseOver) || (buttonOpt.state & QStyle::State_On)) {
        if (button->isDown() || (buttonOpt.state & QStyle::State_On)) {
            d->background->setElementPrefix("pressed");
        } else {
            d->background->setElementPrefix("normal");
        }
        d->background->resizeFrame(size());

        if (d->animId) {
            QPixmap buffer = d->background->framePixmap();

            QPainter bufferPainter(&buffer);
            bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QColor alphaColor(Qt::black);
            alphaColor.setAlphaF(qMin(qreal(0.95), d->opacity));
            bufferPainter.fillRect(buffer.rect(), alphaColor);
            bufferPainter.end();

            painter->drawPixmap(QPoint(0,0), buffer);

            buttonOpt.palette.setColor(QPalette::ButtonText, KColorUtils::mix(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1-d->opacity));
        } else {
            d->background->paintFrame(painter);
            buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
        }

    } else {
        buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }

    button->style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, painter, button);
}

void ToolButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (nativeWidget()->isDown() || !nativeWidget()->autoRaise()) {
        return;
    }

    const int FadeInDuration = 75;

    if (d->animId) {
        Plasma::Animator::self()->stopCustomAnimation(d->animId);
    }
    d->animId = Plasma::Animator::self()->customAnimation(
        40 / (1000 / FadeInDuration), FadeInDuration,
        Plasma::Animator::LinearCurve, this, "animationUpdate");

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ToolButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (nativeWidget()->isDown() || !nativeWidget()->autoRaise()) {
        return;
    }

    const int FadeOutDuration = 150;

    if (d->animId) {
        Plasma::Animator::self()->stopCustomAnimation(d->animId);
    }

    d->fadeIn = false;
    d->animId = Plasma::Animator::self()->customAnimation(
        40 / (1000 / FadeOutDuration), FadeOutDuration,
        Plasma::Animator::LinearCurve, this, "animationUpdate");

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

} // namespace Plasma

#include <toolbutton.moc>

