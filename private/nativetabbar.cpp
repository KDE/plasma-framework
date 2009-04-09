/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008 Marco Martin <notmart@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "nativetabbar_p.h"

// Qt
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QStyleOption>
#include <QToolButton>

#include <QGradient>
#include <QLinearGradient>

// KDE
#include <kdebug.h>
#include <kcolorutils.h>
#include <kicon.h>
#include <kiconloader.h>

#include "plasma/plasma.h"
#include "plasma/theme.h"
#include "plasma/animator.h"
#include "plasma/framesvg.h"
#include "plasma/paintutils.h"

//#include "private/style_p.h"

namespace Plasma
{

class NativeTabBarPrivate
{
public:
    NativeTabBarPrivate(NativeTabBar *parent)
        : q(parent),
          shape(NativeTabBar::RoundedNorth),
          backgroundSvg(0),
          buttonSvg(0),
          closeIcon("window-close"),
          animationId(-1),
          mousePressOffset(0)
    {
    }

    ~NativeTabBarPrivate()
    {
        delete backgroundSvg;
        delete buttonSvg;
    }

    void syncBorders();
    void storeLastIndex();

    NativeTabBar *q;
    QTabBar::Shape shape; //used to keep track of shape() changes
    FrameSvg *backgroundSvg;
    qreal left, top, right, bottom;
    FrameSvg *buttonSvg;
    qreal buttonLeft, buttonTop, buttonRight, buttonBottom;
    KIcon closeIcon;

    int animationId;

    QRect currentAnimRect;
    QRect startAnimRect;
    int mousePressOffset;
    int lastIndex[2];
    qreal animProgress;
};

void NativeTabBarPrivate::syncBorders()
{
    backgroundSvg->getMargins(left, top, right, bottom);
    buttonSvg->getMargins(buttonLeft, buttonTop, buttonRight, buttonBottom);
}

void NativeTabBarPrivate::storeLastIndex()
{
    // if first run
    if (lastIndex[0] == -1) {
        lastIndex[1] = q->currentIndex();
    }
    lastIndex[0] = lastIndex[1];
    lastIndex[1] = q->currentIndex();
}

NativeTabBar::NativeTabBar(QWidget *parent)
        : KTabBar(parent),
          d(new NativeTabBarPrivate(this))
{
    d->backgroundSvg = new Plasma::FrameSvg();
    d->backgroundSvg->setImagePath("widgets/frame");
    d->backgroundSvg->setElementPrefix("sunken");

    d->buttonSvg = new Plasma::FrameSvg();
    d->buttonSvg->setImagePath("widgets/button");
    d->buttonSvg->setElementPrefix("normal");

    d->syncBorders();

    d->lastIndex[0] = -1;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

NativeTabBar::~NativeTabBar()
{
    delete d;
}

QRect NativeTabBar::tabRect(int index) const
{
    QRect rect = KTabBar::tabRect(index).translated(d->left, d->top);

    if (isVertical()) {
        rect.setWidth(width() - d->left - d->right);

        if (index == count() - 1) {
            rect.adjust(0, 0, 0, -d->bottom);
        }
    } else {
        rect.setHeight(height() - d->top- d->bottom);

        if (index == count() - 1) {
            rect.adjust(0, 0, -d->right, 0);
        }
    }

    return rect;
}

int NativeTabBar::lastIndex() const
{
    return d->lastIndex[0];
}

QSize NativeTabBar::tabSizeHint(int index) const
{
    //return KTabBar::tabSizeHint(index);
    QSize hint = tabSize(index);
    int minwidth = 0;
    int minheight = 0;
    int maxwidth = 0;

    Shape s = shape();
    switch (s) {
        case RoundedSouth:
        case TriangularSouth:
        case RoundedNorth:
        case TriangularNorth:
            if (count() > 0) {
                for (int i = count() - 1; i >= 0; i--) {
                    minwidth += tabSize(i).width();
                }

                if (minwidth < width() - d->left - d->right) {
                    hint.rwidth() += (width() - d->left - d->right - minwidth) / count();
                }
            }
            break;
        case RoundedWest:
        case TriangularWest:
        case RoundedEast:
        case TriangularEast:
            if (count() > 0) {
                for (int i = count() - 1; i >= 0; i--) {
                    minheight += tabSize(i).height();
                    if (tabSize(i).width() > maxwidth) {
                        maxwidth = tabSize(i).width();
                    }
                }

                if (minheight < height()) {
                    hint.rheight() += (height() - minheight) / count();
                }
            }
            break;
    }
    return hint;
}


QSize NativeTabBar::sizeHint() const
{
    return KTabBar::sizeHint();
}

void NativeTabBar::paintEvent(QPaintEvent *event)
{
    if (!styleSheet().isNull()) {
        KTabBar::paintEvent(event);
        return;
    }

    QPainter painter(this);
    //int numTabs = count();
    //bool ltr = painter.layoutDirection() == Qt::LeftToRight; // Not yet used

    if (drawBase()) {
        d->backgroundSvg->paintFrame(&painter);
    }

    // Drawing Tabborders
    QRect movingRect;

    if (d->currentAnimRect.isNull()) {
        movingRect = tabRect(currentIndex());
    } else {
        movingRect = d->currentAnimRect;
    }

    //resizing here because in resizeevent the first time is invalid (still no tabs)
    d->buttonSvg->resizeFrame(movingRect.size());
    d->buttonSvg->paintFrame(&painter, movingRect.topLeft());

    QFontMetrics metrics(painter.font());

    for (int i = 0; i < count(); ++i) {
        QRect rect = tabRect(i).adjusted(d->buttonLeft, d->buttonTop,
                                         -d->buttonRight, -d->buttonBottom);
        // draw tab icon
        QRect iconRect = QRect(rect.x(), rect.y(), iconSize().width(), iconSize().height());

        iconRect.moveCenter(QPoint(iconRect.center().x(), rect.center().y()));
        tabIcon(i).paint(&painter, iconRect);

        // draw tab text
        if (i == currentIndex() && d->animProgress == 1) {
            painter.setPen(Plasma::Theme::defaultTheme()->color(Theme::ButtonTextColor));
        } else {
            QColor color(Plasma::Theme::defaultTheme()->color(Theme::TextColor));
            if (!isTabEnabled(i)) {
                color.setAlpha(140);
            }

            painter.setPen(color);
        }
        QRect textRect = rect;

        if (!tabIcon(i).isNull()) {
            textRect.setLeft(iconRect.right());
        }

        painter.setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextHideMnemonic, tabText(i));

        if (isCloseButtonEnabled()) {
            d->closeIcon.paint(&painter, QRect(closeButtonPos(i), QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall)) );
        }
    }


    QRect scrollButtonsRect;
    foreach (QObject *child, children()) {
        QToolButton *childWidget = qobject_cast<QToolButton *>(child);
        if (childWidget) {
            if (!childWidget->isVisible()) {
                continue;
            }

            if (scrollButtonsRect.isValid()) {
                scrollButtonsRect = scrollButtonsRect.united(childWidget->geometry());
            } else {
                scrollButtonsRect = childWidget->geometry();
            }
        }
    }

    if (scrollButtonsRect.isValid()) {
        scrollButtonsRect.adjust(2, 4, -2, -4);
        painter.save();

        QColor background(Plasma::Theme::defaultTheme()->color(Theme::BackgroundColor));
        background.setAlphaF(0.75);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillPath(PaintUtils::roundedRectangle(scrollButtonsRect, 5), background);
        painter.restore();

        QStyleOption so;
        so.initFrom(this);
        so.palette.setColor(QPalette::ButtonText,
                            Plasma::Theme::defaultTheme()->color(Theme::TextColor));

        so.rect = scrollButtonsRect.adjusted(0, 0, -scrollButtonsRect.width() / 2, 0);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &so, &painter, this);

        so.rect = scrollButtonsRect.adjusted(scrollButtonsRect.width() / 2, 0, 0, 0);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &so, &painter, this);
    }
}

void NativeTabBar::resizeEvent(QResizeEvent *event)
{
    KTabBar::resizeEvent(event);
    d->currentAnimRect = tabRect(currentIndex());
    d->backgroundSvg->resizeFrame(size());
    d->syncBorders();

    update();
}

void NativeTabBar::tabInserted(int index)
{
    KTabBar::tabInserted(index);
    emit sizeHintChanged();
}

void NativeTabBar::tabRemoved(int index)
{
    KTabBar::tabRemoved(index);
    emit sizeHintChanged();
}

void NativeTabBar::tabLayoutChange()
{
    KTabBar::tabLayoutChange();

    if (shape() != d->shape) {
        d->shape = shape();
        emit shapeChanged(d->shape);
    }
}

void NativeTabBar::startAnimation()
{
    d->storeLastIndex();
    Plasma::Animator::self()->customAnimation(
        10, 150, Plasma::Animator::EaseInOutCurve, this, "onValueChanged");
}

void NativeTabBar::onValueChanged(qreal value)
{
    if ((d->animProgress = value) == 1.0) {
        animationFinished();
        return;
    }

    // animation rect
    QRect rect = tabRect(currentIndex());
    QRect lastRect = d->startAnimRect.isNull() ? tabRect(lastIndex())
                                               : d->startAnimRect;
    int x = isHorizontal() ? (int)(lastRect.x() - value * (lastRect.x() - rect.x())) : rect.x();
    int y = isHorizontal() ? rect.y() : (int)(lastRect.y() - value * (lastRect.y() - rect.y()));
    QSizeF sz = lastRect.size() - value * (lastRect.size() - rect.size());
    d->currentAnimRect = QRect(x, y, (int)(sz.width()), (int)(sz.height()));
    update();
}

void NativeTabBar::animationFinished()
{
    d->startAnimRect = QRect();
    d->currentAnimRect = QRect();
    update();
}

bool NativeTabBar::isVertical() const
{
    Shape s = shape();
    if(s == RoundedWest ||
       s == RoundedEast ||
       s == TriangularWest ||
       s == TriangularEast) {
        return true;
    }
    return false;
}

bool NativeTabBar::isHorizontal() const
{
    return !isVertical();
}

QSize NativeTabBar::tabSize(int index) const
{
    QSize hint;
    const QFontMetrics metrics(QApplication::font());
    const QSize textSize = metrics.size(Qt::TextHideMnemonic, tabText(index));
    hint.rwidth() = textSize.width() + iconSize().width();
    hint.rheight() = qMax(iconSize().height(), textSize.height());
    hint.rwidth() += d->buttonLeft + d->buttonRight;
    hint.rheight() += d->buttonTop + d->buttonBottom;

    if (isVertical()) {
        hint.rwidth() = qMax(hint.width(), int(minimumWidth() - d->left - d->right));
    } else {
        hint.rheight() = qMax(hint.height(), int(minimumHeight() - d->top - d->bottom));
    }

    return hint;
}

//Unfortunately copied from KTabBar
QPoint NativeTabBar::closeButtonPos( int tabIndex ) const
{
  QPoint buttonPos;
  if ( tabIndex < 0 ) {
    return buttonPos;
  }

  int availableHeight = height();
  if ( tabIndex == currentIndex() ) {
    QStyleOption option;
    option.initFrom(this);
    availableHeight -= style()->pixelMetric( QStyle::PM_TabBarTabShiftVertical, &option, this );
  }

  const QRect tabBounds = tabRect( tabIndex );
  const int xInc = (height() - KIconLoader::SizeSmall) / 2;

  if ( layoutDirection() == Qt::RightToLeft ) {
    buttonPos = tabBounds.topLeft();
    buttonPos.rx() += xInc;
  } else {
    buttonPos = tabBounds.topRight();
    buttonPos.rx() -= KIconLoader::SizeSmall + xInc;
  }
  buttonPos.ry() += (availableHeight - KIconLoader::SizeSmall) / 2;

  return buttonPos;
}

void NativeTabBar::mousePressEvent(QMouseEvent *event)
{
    if (d->currentAnimRect.isNull()) {
        QRect rect = tabRect(currentIndex());

        if (rect.contains(event->pos())) {
            d->mousePressOffset = event->pos().x();
            event->accept();
            return;
        }
    }

    KTabBar::mousePressEvent(event);
}

void NativeTabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (d->mousePressOffset) {
        d->currentAnimRect = tabRect(currentIndex());

        int pos = qBound(0, d->currentAnimRect.left() + (event->pos().x() - d->mousePressOffset),
                         width() - d->currentAnimRect.width());
        d->currentAnimRect.moveLeft(pos);
        update();
    } else {
        KTabBar::mouseMoveEvent(event);
    }
}

void NativeTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->mousePressOffset) {
        bool left = event->pos().x() - d->mousePressOffset < 0;
        int index = tabAt(QPoint(left ? d->currentAnimRect.left() : d->currentAnimRect.right(), 1));
        d->mousePressOffset = 0;

        if (index != currentIndex() && isTabEnabled(index)) {
            d->startAnimRect = d->currentAnimRect;
            setCurrentIndex(index);
        } else {
            d->currentAnimRect = QRect();
        }

        update();
    } else {
        KTabBar::mouseReleaseEvent(event);
    }
}

void NativeTabBar::wheelEvent(QWheelEvent *event)
{
    if (underMouse()) {
        //Cycle tabs with the circular array tecnique
        if (event->delta() < 0) {
            int index = currentIndex();
            //search for an enabled tab
            for (int i = 0; i < count()-1; ++i) {
                index = (index + 1) % count();
                if (isTabEnabled(index)) {
                    break;
                }
            }

            setCurrentIndex(index);
        } else {
            int index = currentIndex();
            for (int i = 0; i < count()-1; ++i) {
                index = (count() + index -1) % count();
                if (isTabEnabled(index)) {
                    break;
                }
            }

            setCurrentIndex(index);
        }
    } else {
        QTabBar::wheelEvent(event);
    }
}

} // namespace Plasma

#include "nativetabbar_p.moc"

