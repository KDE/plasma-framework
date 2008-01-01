/*
 *   Copyright (C) 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 by Matt Broadstone <mbroadst@gmail.com>
 *   Copyright (C) 2006-2007 Fredrik Höglund <fredrik@kde.org>
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
#ifndef ICON_P_H
#define ICON_P_H

#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QTextLayout>
#include <QtGui/QTextOption>

#include <plasma/plasma_export.h>
#include <plasma/svg.h>

#include "icon.h"

class QAction;
class QPainter;
class QTextLayout;

namespace Plasma
{

class PLASMA_EXPORT IconAction
{
public:
    IconAction(Icon* icon, QAction* action);

    void show();
    void hide();
    bool isVisible() const;

    Phase::AnimId animationId() const;
    QAction* action() const;

    void paint(QPainter *painter) const;
    bool event(QEvent::Type type, const QPointF &pos);

    void setSelected(bool selected);
    bool isSelected() const;

    bool isHovered() const;
    bool isPressed() const;

    void setRect(const QRectF &rect);
    QRectF rect() const;

private:
    void rebuildPixmap();

    Icon *m_icon;
    QAction *m_action;
    QPixmap m_pixmap;
    QRectF m_rect;

    bool m_hovered;
    bool m_pressed;
    bool m_selected;
    bool m_visible;

    Phase::AnimId m_animationId;
};


struct Margin
{
    qreal left, right, top, bottom;
};

class Icon::Private
{
public:
    enum MarginType { ItemMargin = 0, TextMargin, IconMargin, NMargins };

    enum IconState
    {
        NoState = 0,
        HoverState = 1,
        PressedState = 2,
        ManualPressedState = 4
    };
    Q_DECLARE_FLAGS(IconStates, IconState)

public:
    Private();
    ~Private();

    void drawBackground(QPainter *painter, IconState state);
    void drawForeground(QPainter *painter, IconState state);
    void drawText(QPainter *painter);
    void drawTextItems(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        const QTextLayout &labelLayout, const QTextLayout &infoLayout) const;

    QPixmap decoration(const QStyleOptionGraphicsItem *option, bool useHoverEffect);
    QPointF iconPosition(const QStyleOptionGraphicsItem *option, const QPixmap &pixmap) const;

    QSizeF displaySizeHint(const QStyleOptionGraphicsItem *option, const qreal width) const;

    QBrush foregroundBrush(const QStyleOptionGraphicsItem *option) const;
    QBrush backgroundBrush(const QStyleOptionGraphicsItem *option) const;

    QString elidedText(QTextLayout &layout, const QStyleOptionGraphicsItem *option,
                        const QSizeF &maxSize) const;
    QSizeF layoutText(QTextLayout &layout, const QStyleOptionGraphicsItem *option,
                        const QString &text, const QSizeF &constraints) const;
    QSizeF layoutText(QTextLayout &layout, const QString &text, qreal maxWidth) const;
    QRectF labelRectangle(const QStyleOptionGraphicsItem *option, const QPixmap &icon,
                            const QString &string) const;
    void layoutTextItems(const QStyleOptionGraphicsItem *option, const QPixmap &icon,
                            QTextLayout *labelLayout, QTextLayout *infoLayout, QRectF *textBoundingRect) const;

    inline void setLayoutOptions(QTextLayout &layout, const QStyleOptionGraphicsItem *options) const;


    // Margin functions
    inline void setActiveMargins();
    void setVerticalMargin(MarginType type, qreal left, qreal right, qreal top, qreal bottom);
    void setHorizontalMargin(MarginType type, qreal left, qreal right, qreal top, qreal bottom);
    inline void setVerticalMargin(MarginType type, qreal hor, qreal ver);
    inline void setHorizontalMargin(MarginType type, qreal hor, qreal ver);
    inline QRectF addMargin(const QRectF &rect, MarginType type) const;
    inline QRectF subtractMargin(const QRectF &rect, MarginType type) const;
    inline QSizeF addMargin(const QSizeF &size, MarginType type) const;
    inline QSizeF subtractMargin(const QSizeF &size, MarginType type) const;


    void checkSvgElements();

    enum {
        NoSvg = 0,
        SvgBackground = 1,
        SvgBackgroundHover = 2,
        SvgBackgroundPressed = 4,
        SvgForeground = 8,
        SvgForegroundHover = 16,
        SvgForegroundPressed = 32,
        SvgMinibutton = 64,
        SvgMinibuttonHover = 128,
        SvgMinibuttonPressed = 256
    };

    enum ActionPosition {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight,
        LastIconPosition
    };

    QString text;
    QString infoText;
    Svg svg;
    Svg *iconSvg;
    QString iconSvgElement;
    QPixmap iconSvgPixmap;
    int svgElements;
    QColor textColor;
    QColor shadowColor;
    QSizeF iconSize;
    QIcon icon;
    IconStates states;
    Qt::Orientation orientation;
    int numDisplayLines;
    QSizeF currentSize;

    QList<IconAction*> cornerActions;

    Margin verticalMargin[NMargins];
    Margin horizontalMargin[NMargins];
    Margin *activeMargins;

    static const int maxDisplayLines = 5;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Icon::Private::IconStates)

// Inline methods
void Icon::Private::setLayoutOptions(QTextLayout &layout, const QStyleOptionGraphicsItem *option) const
{
    QTextOption textoption;
    textoption.setTextDirection(option->direction);
    textoption.setAlignment(Qt::AlignCenter);   // NOTE: assumption
    textoption.setWrapMode(QTextOption::WordWrap);  // NOTE: assumption as well

    layout.setFont(QApplication::font());    // NOTE: find better ways to get the font
    layout.setTextOption(textoption);
}

void Icon::Private::setActiveMargins()
{
    activeMargins = (orientation == Qt::Horizontal ?
            horizontalMargin : verticalMargin);
}

void Icon::Private::setVerticalMargin(MarginType type, qreal left, qreal top, qreal right, qreal bottom)
{
    verticalMargin[type].left   = left;
    verticalMargin[type].right  = right;
    verticalMargin[type].top    = top;
    verticalMargin[type].bottom = bottom;
}

void Icon::Private::setHorizontalMargin(MarginType type, qreal left, qreal top, qreal right, qreal bottom)
{
    horizontalMargin[type].left   = left;
    horizontalMargin[type].right  = right;
    horizontalMargin[type].top    = top;
    horizontalMargin[type].bottom = bottom;
}

void Icon::Private::setVerticalMargin(MarginType type, qreal horizontal, qreal vertical)
{
    setVerticalMargin(type, horizontal, vertical, horizontal, vertical);
}

void Icon::Private::setHorizontalMargin(MarginType type, qreal horizontal, qreal vertical)
{
    setHorizontalMargin(type, horizontal, vertical, horizontal, vertical);
}

QRectF Icon::Private::addMargin(const QRectF &rect, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return rect.adjusted(-m.left, -m.top, m.right, m.bottom);
}

QRectF Icon::Private::subtractMargin(const QRectF &rect, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return rect.adjusted(m.left, m.top, -m.right, -m.bottom);
}

QSizeF Icon::Private::addMargin(const QSizeF &size, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return QSizeF(size.width() + m.left + m.right, size.height() + m.top + m.bottom);
}

QSizeF Icon::Private::subtractMargin(const QSizeF &size, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return QSizeF(size.width() - m.left - m.right, size.height() - m.top - m.bottom);
}

} // Namespace

#endif

