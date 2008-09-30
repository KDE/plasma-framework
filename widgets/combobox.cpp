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

#include <KComboBox>
#include <QPainter>
#include <QApplication>

#include <KMimeType>
#include <KIconEffect>
#include <KIconLoader>

#include "theme.h"
#include "panelsvg.h"
#include "animator.h"

namespace Plasma
{

class ComboBoxPrivate
{
public:
    ComboBoxPrivate(ComboBox *comboBox)
         : q(comboBox),
           background(0),
           activeBackgroundPixmap(0)
    {
    }

    ~ComboBoxPrivate()
    {
    }

    void renderActiveBackgroundPixmap();
    void syncActiveRect();
    void syncBorders();
    void elementAnimationFinished(int id);

    ComboBox *q;

    PanelSvg *background;
    QPixmap *activeBackgroundPixmap;
    int animId;
    QRectF activeRect;
};

void ComboBoxPrivate::renderActiveBackgroundPixmap()
{
    background->setElementPrefix("active");

    activeBackgroundPixmap = new QPixmap(activeRect.size().toSize());
    activeBackgroundPixmap->fill(Qt::transparent);

    QPainter painter(activeBackgroundPixmap);
    background->paintPanel(&painter);
}

void ComboBoxPrivate::syncActiveRect()
{
    background->setElementPrefix("normal");

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);

    background->setElementPrefix("active");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    background->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    activeRect = QRectF(QPointF(0,0), q->size());
    activeRect.adjust(left - activeLeft, top - activeTop, -(right - activeRight), -(bottom - activeBottom));

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
}

void ComboBoxPrivate::elementAnimationFinished(int id)
{
    if (id == animId) {
        animId = -1;
    }
}


ComboBox::ComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ComboBoxPrivate(this))
{
    KComboBox* native = new KComboBox;
    connect(native, SIGNAL(activated(const QString &)), this, SIGNAL(activated(const QString &)));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);

    d->background = new PanelSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedPanels(true);
    d->background->setElementPrefix("normal");

    d->syncBorders();
    setAcceptHoverEvents(true);
    connect(Plasma::Animator::self(), SIGNAL(elementAnimationFinished(int)), this, SLOT(elementAnimationFinished(int)));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncBorders()));
}

ComboBox::~ComboBox()
{
    delete d;
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

KComboBox* ComboBox::nativeWidget() const
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
        d->background->resizePanel(size());

        d->background->setElementPrefix("active");
        d->background->resizePanel(d->activeRect.size());

        d->background->setElementPrefix("normal");
        d->background->resizePanel(size());

        if (d->activeBackgroundPixmap) {
            delete d->activeBackgroundPixmap;
            d->activeBackgroundPixmap = 0;
        }
   }

    QGraphicsProxyWidget::resizeEvent(event);
}

void ComboBox::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    if (!styleSheet().isNull() || nativeWidget()->isEditable()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QPixmap bufferPixmap;

    //Normal button, pressed or not
    if (isEnabled()) {
        d->background->setElementPrefix("normal");
        d->background->paintPanel(painter);
    //flat or disabled
    } else if (!isEnabled()) {
        bufferPixmap = QPixmap(rect().size().toSize());
        bufferPixmap.fill(Qt::transparent);

        QPainter buffPainter(&bufferPixmap);
        d->background->paintPanel(&buffPainter);
        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        buffPainter.fillRect(bufferPixmap.rect(), QColor(0,0,0,128));

        painter->drawPixmap( 0, 0, bufferPixmap);
    }

    //if is under mouse draw the animated glow overlay
    if (isEnabled() && acceptHoverEvents()) {
        if (d->animId != -1) {
            painter->drawPixmap(d->activeRect.topLeft(), Plasma::Animator::self()->currentPixmap(d->animId) );
        } else if (isUnderMouse()) {
            if (d->activeBackgroundPixmap == 0) {
                d->renderActiveBackgroundPixmap();
            }
            painter->drawPixmap( d->activeRect.topLeft(), *d->activeBackgroundPixmap );
        }
    }

    if (nativeWidget()->hasFocus()) {
        d->background->setElementPrefix("focus");
        d->background->paintPanel(painter);
    }


    painter->setPen(Plasma::Theme::defaultTheme()->color(Theme::ButtonTextColor));

    QStyleOptionComboBox comboOpt;

    comboOpt.initFrom(nativeWidget());

    comboOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    comboOpt.currentIcon = nativeWidget()->itemIcon(nativeWidget()->currentIndex());
    comboOpt.currentText = nativeWidget()->itemText(nativeWidget()->currentIndex());
    comboOpt.editable = false;

    nativeWidget()->style()->drawControl(QStyle::CE_ComboBoxLabel, &comboOpt, painter, nativeWidget());
    comboOpt.rect = nativeWidget()->style()->subControlRect(QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxArrow, nativeWidget());
    nativeWidget()->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &comboOpt, painter, nativeWidget());
}

void ComboBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->animId != -1) {
        Plasma::Animator::self()->stopElementAnimation(d->animId);
    }
    d->animId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);

    d->background->setElementPrefix("active");

    if (!d->activeBackgroundPixmap) {
        d->renderActiveBackgroundPixmap();
    }
    Plasma::Animator::self()->setInitialPixmap( d->animId, *d->activeBackgroundPixmap );

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ComboBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->animId != -1) {
        Plasma::Animator::self()->stopElementAnimation(d->animId);
    }
    d->animId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::DisappearAnimation);

    d->background->setElementPrefix("active");

    if (!d->activeBackgroundPixmap) {
        d->renderActiveBackgroundPixmap();
    }
    Plasma::Animator::self()->setInitialPixmap( d->animId, *d->activeBackgroundPixmap );

    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

} // namespace Plasma

#include <combobox.moc>

