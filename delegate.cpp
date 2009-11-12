/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>
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
#include "delegate.h"

#include <cmath>
#include <math.h>

// Qt
#include <QApplication>
#include <QFontMetrics>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

// KDE
#include <kcolorutils.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>

// plasma
#include <plasma/paintutils.h>
#include <plasma/framesvg.h>

namespace Plasma
{

class DelegatePrivate
{
    public:
        DelegatePrivate() { }

        ~DelegatePrivate() { }

        QFont fontForSubTitle(const QFont &titleFont) const;
        QRect titleRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QRect subTitleRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;

        QMap<int, int> roles;

        static const int ICON_TEXT_MARGIN = 10;
        static const int TEXT_RIGHT_MARGIN = 5;
        static const int ACTION_ICON_SIZE = 22;

        static const int ITEM_LEFT_MARGIN = 5;
        static const int ITEM_RIGHT_MARGIN = 5;
        static const int ITEM_TOP_MARGIN = 5;
        static const int ITEM_BOTTOM_MARGIN = 5;

        bool m_showToolTip;
        FrameSvg *svg;
};

QFont DelegatePrivate::fontForSubTitle(const QFont &titleFont) const
{
    QFont subTitleFont = titleFont;
    subTitleFont.setPointSize(qMax(subTitleFont.pointSize() - 2,
                              KGlobalSettings::smallestReadableFont().pointSize()));
    return subTitleFont;
}

QRect DelegatePrivate::titleRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font(option.font);
    font.setBold(true);
    QFontMetrics fm(font);

    Qt::Alignment textAlignment =
        option.decorationAlignment & Qt::AlignRight ? Qt::AlignRight : Qt::AlignLeft;

    QRect emptyRect;
    if (option.direction == Qt::LeftToRight) {
        emptyRect = option.rect.adjusted(
            option.decorationSize.width() + ICON_TEXT_MARGIN + ITEM_LEFT_MARGIN,
            ITEM_TOP_MARGIN, -ITEM_RIGHT_MARGIN, -ITEM_BOTTOM_MARGIN);
    } else {
        emptyRect = option.rect.adjusted(
            ITEM_LEFT_MARGIN, ITEM_TOP_MARGIN,
            -ITEM_RIGHT_MARGIN - option.decorationSize.width() - ICON_TEXT_MARGIN, -ITEM_BOTTOM_MARGIN);
    }

    if (emptyRect.width() < 0) {
        emptyRect.setWidth(0);
        return emptyRect;
    }

    QRect textRect = QStyle::alignedRect(
        option.direction,
        textAlignment,
        fm.boundingRect(index.data(Qt::DisplayRole).toString()).size(),
        emptyRect);

    textRect.setWidth(textRect.width() + TEXT_RIGHT_MARGIN);
    textRect.setHeight(emptyRect.height() / 2);
    return textRect;
}

QRect DelegatePrivate::subTitleRect(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QString subTitle = index.data(roles[Delegate::SubTitleRole]).toString();

    QFontMetrics fm(fontForSubTitle(option.font));

    QRect textRect = titleRect(option, index);
    int right = textRect.right();

    //if title=subtitle subtitle won't be displayed
    if (subTitle != index.data(Qt::DisplayRole).toString()) {
        textRect.setWidth(fm.width("  " + subTitle) + TEXT_RIGHT_MARGIN);
    } else {
        textRect.setWidth(0);
    }
    textRect.translate(0, textRect.height());

    if (option.direction == Qt::RightToLeft) {
        textRect.moveRight(right);
    }

    return textRect;
}

Delegate::Delegate(QObject *parent)
    : QAbstractItemDelegate(parent),
      d(new DelegatePrivate)
{
    d->svg = new FrameSvg(this);
    d->svg->setImagePath("widgets/viewitem");
    d->svg->setElementPrefix("hover");
}

Delegate::~Delegate()
{
    delete d;
}

void Delegate::setRoleMapping(SpecificRoles role, int actual)
{
    d->roles[role] = actual;
}

int Delegate::roleMapping(SpecificRoles role) const
{
    return d->roles[role];
}

QRect Delegate::rectAfterTitle(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect textRect = d->titleRect(option, index);

    QRect emptyRect(0, textRect.top(), option.rect.width() - textRect.width() - DelegatePrivate::ITEM_LEFT_MARGIN - DelegatePrivate::ITEM_RIGHT_MARGIN - option.decorationSize.width() - DelegatePrivate::ICON_TEXT_MARGIN, textRect.height());

    if (option.direction == Qt::LeftToRight) {
        emptyRect.moveLeft(textRect.right());
    } else {
        emptyRect.moveRight(textRect.left());
    }

    if (emptyRect.width() < 0) {
        emptyRect.setWidth(0);
    }

    return emptyRect;
}

QRect Delegate::rectAfterSubTitle(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect textRect = d->subTitleRect(option, index);

    QRect emptyRect(0, textRect.top(), option.rect.width() - textRect.width() - DelegatePrivate::ITEM_LEFT_MARGIN - DelegatePrivate::ITEM_RIGHT_MARGIN - option.decorationSize.width() - DelegatePrivate::ICON_TEXT_MARGIN, textRect.height());

    if (option.direction == Qt::LeftToRight) {
        emptyRect.moveLeft(textRect.right());
    } else {
        emptyRect.moveRight(textRect.left());
    }

    if (emptyRect.width() < 0) {
        emptyRect.setWidth(0);
    }

    return emptyRect;
}

QRect Delegate::emptyRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect afterTitleRect = rectAfterTitle(option, index);
    QRect afterSubTitleRect = rectAfterSubTitle(option, index);

    afterTitleRect.setHeight(afterTitleRect.height() * 2);
    afterSubTitleRect.setTop(afterTitleRect.top());

    return afterTitleRect.intersected(afterSubTitleRect);
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    const bool hover = option.state & (QStyle::State_MouseOver | QStyle::State_Selected);

    QRect contentRect = option.rect;
    contentRect.setBottom(contentRect.bottom() - 1);

    QRect decorationRect =
        QStyle::alignedRect(option.direction,
                            option.decorationPosition == QStyleOptionViewItem::Left ?
                            Qt::AlignLeft : Qt::AlignRight,
                            option.decorationSize,
                            contentRect.adjusted(DelegatePrivate::ITEM_LEFT_MARGIN, DelegatePrivate::ITEM_TOP_MARGIN, -DelegatePrivate::ITEM_RIGHT_MARGIN, -DelegatePrivate::ITEM_BOTTOM_MARGIN));
    decorationRect.moveTop(contentRect.top() + qMax(0, (contentRect.height() - decorationRect.height())) / 2);

    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QString subTitleText = index.data(d->roles[SubTitleRole]).value<QString>();
    //kDebug() << subTitleText;

    QRect titleRect = d->titleRect(option, index);
    titleRect.moveTopLeft(titleRect.topLeft()-option.rect.topLeft());
    QRect subTitleRect = d->subTitleRect(option, index);
    subTitleRect.moveTopLeft(subTitleRect.topLeft()-option.rect.topLeft());

    if (subTitleText == titleText) {
        subTitleText.clear();
    }

    QFont titleFont(option.font);

    // draw icon
    QIcon decorationIcon = index.data(Qt::DecorationRole).value<QIcon>();

    if (index.data(d->roles[ColumnTypeRole]).toInt() == SecondaryActionColumn) {
        if (hover) {
            // Only draw on hover
            const int delta = floor((qreal)(option.decorationSize.width() - DelegatePrivate::ACTION_ICON_SIZE) / 2.0);
            decorationRect.adjust(delta, delta-1, -delta-1, -delta);
            decorationIcon.paint(painter, decorationRect, option.decorationAlignment);
        }
    } else {
        // as default always draw as main column
        decorationIcon.paint(painter, decorationRect, option.decorationAlignment);
    }

    QPixmap buffer(option.rect.size());
    buffer.fill(Qt::transparent);
    QPainter p(&buffer);
    // draw title
    p.setFont(titleFont);
    if (option.palette.color(QPalette::Base).alpha() > 0) {
        p.setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText), 1));
    } else {
        p.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }
    p.drawText(titleRect, Qt::AlignLeft|Qt::AlignVCenter, titleText);

    // draw sub-title, BUT only if:
    //   * SubTitleMandatoryRole is defined and model returns 'true'
    //   * SubTitleMandatoryRole is not defined and the adjasent model indexes
    //     have the same contents of the Qt::DisplayRole
    //   * when model doesn't provide a valid data for SubTitleMandatory role
    //     we also show title on mouse hover
    //
    // the rationale for this is that subtitle text should in most cases not be
    // required to understand the item itself and that showing all the subtexts in a
    // listing makes the information density very high, impacting both the speed at
    // which one can scan the list visually and the aesthetic qualities of the listing.
    bool drawSubTitle = !subTitleText.isEmpty();

    if (drawSubTitle && !hover) {
        // If the model wants to have exact control for subtitles showing
        // it is expected to return a valid data for SubTitleMandatoryRole.
        // If it doesn't return a valid data for this role
        // then by default we well be showing a subtitles for
        // adjasent items with the same content (see comments below too)
        QVariant mandatoryRoleData = index.data(d->roles[SubTitleMandatoryRole]);
        if (mandatoryRoleData.isValid()) {
            drawSubTitle = mandatoryRoleData.value<bool>();
        } else {
            bool uniqueTitle = true;
            QModelIndex sib = index.sibling(index.row() + 1, index.column());
            if (sib.isValid()) {
                uniqueTitle = sib.data(Qt::DisplayRole).value<QString>() != titleText;
            }

            if (uniqueTitle) {
                sib = index.sibling(index.row() + -1, index.column());
                if (sib.isValid()) {
                    uniqueTitle = sib.data(Qt::DisplayRole).value<QString>() != titleText;
                }
            }

            drawSubTitle = !uniqueTitle;
        }
    }


    if (drawSubTitle) {
        if (option.palette.color(QPalette::Base).alpha() > 0) {
            p.setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText), 1));
        } else {
            QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
            textColor.setAlphaF(0.6);
            p.setPen(textColor);
        }

        const QFont subTitleFont = d->fontForSubTitle(option.font);
        p.setFont(subTitleFont);
        p.drawText(subTitleRect, Qt::AlignLeft|Qt::AlignVCenter, subTitleText);
    }
    p.end();


    d->m_showToolTip = false;

    const QColor gradientColor = KColorScheme(QPalette::Active).background(KColorScheme::NormalBackground).color();

    if (option.direction == Qt::LeftToRight) {
        if (((titleRect.width() + decorationRect.width() + 10) > option.rect.width() ||
            (subTitleRect.width() + decorationRect.width() + 15) > option.rect.width()) &&
	    (titleRect.width() > 120 || subTitleRect.width() > 120)) {
            QPainter p(&buffer);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setPen(Qt::NoPen);
            QLinearGradient gr;
            QRect gradientRect(option.rect.width() - 60, titleRect.y(),
                  80, titleRect.height() + subTitleRect.height());
            // draw it on the right side
            gr.setStart(gradientRect.topLeft());
            gr.setFinalStop(gradientRect.topRight());
            gr.setColorAt(0.0, Qt::transparent);
            gr.setColorAt(0.7, gradientColor);
            p.setBrush(QBrush(gr));
            p.drawRect(gradientRect);
            d->m_showToolTip = true;
            p.end();
        }

    } else {
        if (((titleRect.width() + decorationRect.width() + 10) > option.rect.width() ||
            (subTitleRect.width() + decorationRect.width() + 15 )> option.rect.width()) &&
	    (titleRect.width() > 120 || subTitleRect.width() > 120)) {
            buffer.fill(Qt::transparent);
            QPainter p(&buffer);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setPen(Qt::NoPen);
            QLinearGradient gr;
            QRect gradientRect(option.rect.x() - 55, titleRect.y(),
                                60, titleRect.height() + subTitleRect.height());
            gr.setStart(gradientRect.topRight());
            gr.setFinalStop(gradientRect.topLeft());
            gr.setColorAt(0.0, Qt::transparent);
            gr.setColorAt(0.6, gradientColor);
            p.setBrush(QBrush(gr));
            p.drawRect(gradientRect);

            d->m_showToolTip = true;
            p.end();
        }
    }

    painter->drawPixmap(option.rect, buffer, buffer.rect());

    if (hover) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const int column = index.column();
        const int columns = index.model()->columnCount();
        int roundedRadius = 5;
        const bool useSvg = option.palette.color(QPalette::Base).alpha() == 0;

        // use a slightly translucent version of the palette's highlight color
        // for the background
        QColor backgroundColor = option.palette.color(QPalette::Highlight);
        backgroundColor.setAlphaF(0.2);

        QColor backgroundColor2 = option.palette.color(QPalette::Highlight);
        backgroundColor2.setAlphaF(0.5);

        QRect highlightRect = option.rect;
        if (!useSvg) {
            highlightRect.adjust(2, 2, -2, -2);
        }

        QPen outlinePen(backgroundColor, 2);

        if (column == 0) {
            //clip right (or left for rtl languages) to make the connection with the next column
            if (columns > 1) {
                if (useSvg) {
                    roundedRadius = d->svg->marginSize(Plasma::RightMargin);
                }
                painter->setClipRect(option.rect);
                highlightRect.adjust(0, 0, roundedRadius, 0);
            }

            QLinearGradient gradient(highlightRect.topLeft(), highlightRect.topRight());

            //reverse the gradient
            if (option.direction == Qt::RightToLeft) {
                gradient.setStart(highlightRect.topRight());
                gradient.setFinalStop(highlightRect.topLeft());
            }

            gradient.setColorAt(0, backgroundColor);
            gradient.setColorAt(((qreal)titleRect.width()/3.0) / (qreal)highlightRect.width(), backgroundColor2);
            gradient.setColorAt(0.7, backgroundColor);
            outlinePen.setBrush(gradient);
        //last column, clip left (right for rtl)
        } else if (column == columns-1) {
            if (useSvg) {
                roundedRadius = d->svg->marginSize(Plasma::LeftMargin);
            }
            painter->setClipRect(option.rect);
            highlightRect.adjust(-roundedRadius, 0, 0, 0);

        //column < columns-1; clip both ways
        } else {
            if (useSvg) {
                roundedRadius = d->svg->marginSize(Plasma::LeftMargin);
            }
            painter->setClipRect(option.rect);
            highlightRect.adjust(-roundedRadius, 0, +roundedRadius, 0);
        }

        //if the view is transparent paint as plasma, otherwise paint with kde colors
        if (useSvg) {
            d->svg->resizeFrame(highlightRect.size());
            d->svg->paintFrame(painter, highlightRect.topLeft());
        } else {
            painter->setPen(outlinePen);
            painter->drawPath(PaintUtils::roundedRectangle(highlightRect, roundedRadius));
        }

        painter->restore();
    }


}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    QSize size = option.rect.size();

    QFontMetrics metrics(option.font);

    QFontMetrics subMetrics(d->fontForSubTitle(option.font));
    size.setHeight(qMax(option.decorationSize.height(), qMax(size.height(), metrics.height() + subMetrics.ascent()) + 3) + 4);
//    kDebug() << "size hint is" << size << (metrics.height() + subMetrics.ascent());

    const bool useSvg = option.palette.color(QPalette::Base).alpha() == 0;

    if (useSvg) {
        qreal left, top, right, bottom;
        d->svg->getMargins(left, top, right, bottom);
        size += QSize(left+right, top+bottom);
    } else {
        size *= 1.1;
    }

    return size;
}

bool Delegate::showToolTip() const
{
    return d->m_showToolTip;
}

}

#include "delegate.moc"

