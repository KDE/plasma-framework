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
#include <plasma/private/focusindicator_p.h>
#include "applet.h"
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

    ComboBox *q;

    FrameSvg *background;
    FrameSvg *lineEditBackground;
    int animId;
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
    setZValue(900);

    setAcceptHoverEvents(true);

    d->style = Style::sharedStyle();

    new FocusIndicator(this, "widgets/button");
    setNativeWidget(new KComboBox);
    connect(d->background, SIGNAL(repaintNeeded()), SLOT(syncBorders()));
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
    nativeWidget->setWindowIcon(QIcon());

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
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QPixmap bufferPixmap;

    //normal button
    if (isEnabled()) {
        d->background->setElementPrefix("normal");

        d->background->paintFrame(painter);
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
    QGraphicsProxyWidget::focusInEvent(event);
}

void ComboBox::focusOutEvent(QFocusEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::UnknownStatus);
    }
    QGraphicsProxyWidget::focusOutEvent(event);
}

void ComboBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = true;
    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ComboBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = false;
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

void ComboBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::AcceptingInputStatus);
    }
    QGraphicsProxyWidget::mousePressEvent(event);
}

} // namespace Plasma

#include <combobox.moc>

