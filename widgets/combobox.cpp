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

#include "combobox.h"

#include <QPainter>
#include <QApplication>

#include <kcombobox.h>
#include <kmimetype.h>
#include <kiconeffect.h>
#include <kiconloader.h>

#include <plasma/private/style_p.h>
#include "theme.h"
#include "framesvg.h"
#include "animator.h"
#include "paintutils.h"

namespace Plasma
{

class ComboBoxPrivate
{
public:
    ComboBoxPrivate(ComboBox *comboBox)
         : q(comboBox),
           background(0),
           customFont(false),
           underMouse(false)
    {
    }

    ~ComboBoxPrivate()
    {
    }

    void syncActiveRect();
    void syncBorders();
    void animationUpdate(qreal progress);
    void animationFinished(int id);

    ComboBox *q;

    FrameSvg *background;
    FrameSvg *lineEditBackground;
    int animId;
    bool fadeIn;
    qreal opacity;
    QRectF activeRect;
    Style::Ptr style;
    bool customFont;
    bool underMouse;
};

void ComboBoxPrivate::syncActiveRect()
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

void ComboBoxPrivate::syncBorders()
{
    //set margins from the normal element
    qreal left, top, right, bottom;

    background->setElementPrefix("normal");
    background->getMargins(left, top, right, bottom);
    q->setContentsMargins(left, top, right, bottom);

    //calc the rect for the over effect
    syncActiveRect();

    KComboBox *native = q->nativeWidget();
    if (customFont) {
        native->setFont(q->font());
    } else {
        native->setFont(Theme::defaultTheme()->font(Theme::DefaultFont));
    }
}

void ComboBoxPrivate::animationUpdate(qreal progress)
{
    if (progress == 1) {
        animId = -1;
        fadeIn = true;
    }

    opacity = fadeIn ? progress : 1 - progress;

    // explicit update
    q->update();
}


void ComboBoxPrivate::animationFinished(int id)
{
    if (id == animId) {
        animId = -1;
    }
}

ComboBox::ComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ComboBoxPrivate(this))
{
    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedFrames(true);
    d->background->setElementPrefix("normal");
    d->lineEditBackground = new FrameSvg(this);
    d->lineEditBackground->setImagePath("widgets/lineedit");
    d->lineEditBackground->setCacheAllRenderedFrames(true);

    setAcceptHoverEvents(true);

    d->style = Style::sharedStyle();

    setNativeWidget(new KComboBox);

    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncBorders()));
    connect(Plasma::Animator::self(), SIGNAL(customAnimationFinished(int)), this, SLOT(animationFinished(int)));
}

ComboBox::~ComboBox()
{
    delete d;
    Style::doneWithSharedStyle();
}

QString ComboBox::text() const
{
    return static_cast<KComboBox*>(widget())->currentText();
}

void ComboBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ComboBox::styleSheet()
{
    return widget()->styleSheet();
}

void ComboBox::setNativeWidget(KComboBox *nativeWidget)
{
    if (widget()) {
        widget()->deleteLater();
    }

    connect(nativeWidget, SIGNAL(activated(const QString &)), this, SIGNAL(activated(const QString &)));
    connect(nativeWidget, SIGNAL(currentIndexChanged(const QString &)),
            this, SIGNAL(textChanged(const QString &)));

    setWidget(nativeWidget);

    nativeWidget->setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget->setStyle(d->style.data());

    d->syncBorders();
}

KComboBox *ComboBox::nativeWidget() const
{
    return static_cast<KComboBox*>(widget());
}

void ComboBox::addItem(const QString &text)
{
    static_cast<KComboBox*>(widget())->addItem(text);
}

void ComboBox::clear()
{
    static_cast<KComboBox*>(widget())->clear();
}

void ComboBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   if (d->background) {
        //resize needed panels
        d->syncActiveRect();

        d->background->setElementPrefix("focus");
        d->background->resizeFrame(size());

        d->background->setElementPrefix("active");
        d->background->resizeFrame(d->activeRect.size());

        d->background->setElementPrefix("normal");
        d->background->resizeFrame(size());
   }

   QGraphicsProxyWidget::resizeEvent(event);
}

void ComboBox::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    if (!styleSheet().isNull() ||
        Theme::defaultTheme()->useNativeWidgetStyle()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    if (nativeWidget()->isEditable()) {
        if (d->animId != -1 || hasFocus() || d->underMouse) {
            if (hasFocus()) {
                d->lineEditBackground->setElementPrefix("focus");
            } else {
                d->lineEditBackground->setElementPrefix("hover");
            }
            qreal left, top, right, bottom;
            d->lineEditBackground->getMargins(left, top, right, bottom);
            d->lineEditBackground->resizeFrame(size()+QSizeF(left+right, top+bottom));
            if (qFuzzyCompare(d->opacity, (qreal)1.0)) {
                d->lineEditBackground->paintFrame(painter, QPoint(-left, -top));
            } else {
                QPixmap bufferPixmap = d->lineEditBackground->framePixmap();
                QPainter buffPainter(&bufferPixmap);
                buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                buffPainter.fillRect(bufferPixmap.rect(), QColor(0, 0, 0, 256*d->opacity));
                buffPainter.end();
                painter->drawPixmap(bufferPixmap.rect().translated(QPoint(-left, -top)), bufferPixmap, bufferPixmap.rect());
            }
        }
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QPixmap bufferPixmap;

    //normal button
    if (isEnabled()) {
        d->background->setElementPrefix("normal");

        if (d->animId == -1) {
            d->background->paintFrame(painter);
        }
    //disabled widget
    } else {
        bufferPixmap = QPixmap(rect().size().toSize());
        bufferPixmap.fill(Qt::transparent);

        QPainter buffPainter(&bufferPixmap);
        d->background->paintFrame(&buffPainter);
        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        buffPainter.fillRect(bufferPixmap.rect(), QColor(0, 0, 0, 128));

        painter->drawPixmap(0, 0, bufferPixmap);
    }

    //if is under mouse draw the animated glow overlay
    if (isEnabled() && acceptHoverEvents()) {
        if (d->animId != -1) {
            d->background->setElementPrefix("normal");
            QPixmap normalPix = d->background->framePixmap();
            d->background->setElementPrefix("active");
            painter->drawPixmap(
                d->activeRect.topLeft(),
                PaintUtils::transition(d->background->framePixmap(), normalPix, 1 - d->opacity));
        } else if (isUnderMouse()) {
            d->background->setElementPrefix("active");
            d->background->paintFrame(painter, d->activeRect.topLeft());
        }
    }

    if (nativeWidget()->hasFocus()) {
        d->background->setElementPrefix("focus");
        d->background->paintFrame(painter);
    }

    painter->setPen(Theme::defaultTheme()->color(Theme::ButtonTextColor));

    QStyleOptionComboBox comboOpt;

    comboOpt.initFrom(nativeWidget());

    comboOpt.palette.setColor(
        QPalette::ButtonText, Theme::defaultTheme()->color(Theme::ButtonTextColor));
    comboOpt.currentIcon = nativeWidget()->itemIcon(
        nativeWidget()->currentIndex());
    comboOpt.currentText = nativeWidget()->itemText(
        nativeWidget()->currentIndex());
    comboOpt.editable = false;

    nativeWidget()->style()->drawControl(
        QStyle::CE_ComboBoxLabel, &comboOpt, painter, nativeWidget());
    comboOpt.rect = nativeWidget()->style()->subControlRect(
        QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxArrow, nativeWidget());
    nativeWidget()->style()->drawPrimitive(
        QStyle::PE_IndicatorArrowDown, &comboOpt, painter, nativeWidget());
}

void ComboBox::focusInEvent(QFocusEvent *event)
{
    if (nativeWidget()->isEditable() && !d->underMouse) {
        const int FadeInDuration = 75;

        if (d->animId != -1) {
            Animator::self()->stopCustomAnimation(d->animId);
        }
        d->animId = Animator::self()->customAnimation(
            40 / (1000 / FadeInDuration), FadeInDuration,
            Animator::LinearCurve, this, "animationUpdate");
    }

    QGraphicsProxyWidget::focusInEvent(event);
}

void ComboBox::focusOutEvent(QFocusEvent *event)
{
    if (nativeWidget()->isEditable() && !d->underMouse) {
        const int FadeOutDuration = 150;

        if (d->animId != -1) {
            Animator::self()->stopCustomAnimation(d->animId != -1);
        }

        d->fadeIn = false;
        d->animId = Animator::self()->customAnimation(
            40 / (1000 / FadeOutDuration),
            FadeOutDuration, Animator::LinearCurve, this, "animationUpdate");
    }

    QGraphicsProxyWidget::focusInEvent(event);
}

void ComboBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = true;
    if (nativeWidget()->isEditable() && hasFocus()) {
        return;
    }

    const int FadeInDuration = 75;

    if (d->animId != -1) {
        Animator::self()->stopCustomAnimation(d->animId);
    }
    d->animId = Animator::self()->customAnimation(
        40 / (1000 / FadeInDuration), FadeInDuration,
        Animator::LinearCurve, this, "animationUpdate");

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ComboBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = false;
    if (nativeWidget()->isEditable() && hasFocus()) {
        return;
    }

    const int FadeOutDuration = 150;

    if (d->animId != -1) {
        Animator::self()->stopCustomAnimation(d->animId != -1);
    }

    d->fadeIn = false;
    d->animId = Animator::self()->customAnimation(
        40 / (1000 / FadeOutDuration),
        FadeOutDuration, Animator::LinearCurve, this, "animationUpdate");

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

void ComboBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma

#include <combobox.moc>

