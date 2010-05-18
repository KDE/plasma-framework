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
#ifndef PLASMA_ICONWIDGET_P_H
#define PLASMA_ICONWIDGET_P_H

#include <QtCore/QEvent>
#include <QtCore/QWeakPointer>
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QTextLayout>
#include <QtGui/QTextOption>

#include <plasma/plasma_export.h>
#include <plasma/framesvg.h>
#include <plasma/theme.h>
#include <plasma/svg.h>

#include "iconwidget.h"
#include "private/actionwidgetinterface_p.h"

class QAction;
class QPainter;
class QTextLayout;
class QPropertyAnimation;

namespace Plasma
{

class Animation;
class IconHoverAnimation;

class IconHoverAnimation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue)

public:
    IconHoverAnimation(QObject *parent = 0);

    qreal value() const;

    bool fadeIn() const;
    void setFadeIn(bool fadeIn);

    QPropertyAnimation *animation() const;
    void setAnimation(QPropertyAnimation *animation);

protected slots:
    void setValue(qreal value);

private:
    qreal m_value;
    bool m_fadeIn;
    QWeakPointer<QPropertyAnimation> m_animation;
};

class PLASMA_EXPORT IconAction
{
public:
    IconAction(IconWidget *icon, QAction *action);

    void show();
    void hide();
    bool isVisible() const;
    bool isAnimating() const;

    QAction *action() const;

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

    IconWidget *m_icon;
    QAction *m_action;
    QPixmap m_pixmap;
    QRectF m_rect;

    bool m_hovered;
    bool m_pressed;
    bool m_selected;
    bool m_visible;

    QWeakPointer<Animation> m_animation;
};

struct Margin
{
    qreal left, right, top, bottom;
};

class IconWidgetPrivate : public ActionWidgetInterface<IconWidget>
{
public:
    enum MarginType {
        ItemMargin = 0,
        TextMargin,
        IconMargin,
        NMargins
    };

    enum IconWidgetState {
        NoState = 0,
        HoverState = 1,
        PressedState = 2,
        ManualPressedState = 4
    };
    Q_DECLARE_FLAGS(IconWidgetStates, IconWidgetState)

    IconWidgetPrivate(IconWidget *i);
    ~IconWidgetPrivate();

    void changed()
    {
        emit q->changed();
    }

    void drawBackground(QPainter *painter, IconWidgetState state);
    void drawText(QPainter *painter);
    void drawTextItems(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        const QTextLayout &labelLayout, const QTextLayout &infoLayout) const;

    QPixmap decoration(const QStyleOptionGraphicsItem *option, bool useHoverEffect, bool usePressedEffect);
    QPointF iconPosition(const QStyleOptionGraphicsItem *option, const QPixmap &pixmap) const;

    QSizeF displaySizeHint(const QStyleOptionGraphicsItem *option, const qreal width) const;

    QBrush foregroundBrush(const QStyleOptionGraphicsItem *option) const;
    QBrush backgroundBrush(const QStyleOptionGraphicsItem *option) const;

    QString elidedText(QTextLayout &layout,
                       const QStyleOptionGraphicsItem *option,
                       const QSizeF &maxSize) const;

    QSizeF layoutText(QTextLayout &layout,
                      const QStyleOptionGraphicsItem *option,
                      const QString &text, const QSizeF &constraints) const;

    QSizeF layoutText(QTextLayout &layout, const QString &text,
                      qreal maxWidth) const;

    QRectF labelRectangle(const QStyleOptionGraphicsItem *option,
                          const QPixmap &icon, const QString &string) const;

    void layoutTextItems(const QStyleOptionGraphicsItem *option,
                         const QPixmap &icon, QTextLayout *labelLayout,
                         QTextLayout *infoLayout, QRectF *textBoundingRect) const;

    int maxWordWidth(const QString text) const;

    inline void setLayoutOptions(QTextLayout &layout,
                                 const QStyleOptionGraphicsItem *options,
                                 const Qt::Orientation orientation) const;

    inline Qt::LayoutDirection iconDirection(const QStyleOptionGraphicsItem *option) const;

    enum {
        Minibutton = 64,
        MinibuttonHover = 128,
        MinibuttonPressed = 256
    };

    enum ActionPosition {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight,
        LastIconPosition = BottomRight
    };

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
    inline QRectF actionRect(ActionPosition position) const;

    void actionDestroyed(QObject *obj);
    void svgChanged();

    void readColors();
    void colorConfigChanged();
    void iconConfigChanged();
    QFont widgetFont() const;
    void hoverAnimationFinished();
    void init();
    void layoutIcons(const QStyleOptionGraphicsItem *option);
    void animateMainIcon(bool, const IconWidgetStates state);

    IconWidget *q;
    QString text;
    QString infoText;
    Svg *iconSvg;
    FrameSvg *background;
    QString iconSvgElement;
    QPixmap iconSvgPixmap;
    QColor textColor;
    QColor textBgColor;
    QColor shadowColor;
    IconHoverAnimation *hoverAnimation;
    QSizeF iconSize;
    QSizeF preferredIconSize;
    QSizeF minimumIconSize;
    QSizeF maximumIconSize;
    QIcon icon;
    QIcon oldIcon;
    IconWidgetStates states;
    Qt::Orientation orientation;
    int numDisplayLines;
    QSizeF currentSize;
    QPointF clickStartPos;
    mutable QList<QRect> haloRects;
    QTimer *iconChangeTimer;

    QList<IconAction*> cornerActions;

    Margin verticalMargin[NMargins];
    Margin horizontalMargin[NMargins];
    Margin *activeMargins;

    bool iconSvgElementChanged : 1;
    bool invertLayout : 1;
    bool drawBg : 1;
    bool textBgCustomized : 1;
    static const int maxDisplayLines = 5;
    static const int iconActionSize = 26;
    static const int iconActionMargin = 4;
    bool customFont;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IconWidgetPrivate::IconWidgetStates)

// Inline methods
void IconWidgetPrivate::setLayoutOptions(QTextLayout &layout,
                                         const QStyleOptionGraphicsItem *option,
                                         const Qt::Orientation orientation) const
{
    QTextOption textoption;
    textoption.setTextDirection(option->direction);
    if (orientation == Qt::Horizontal) {
        textoption.setAlignment(Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter));
    } else {
        textoption.setAlignment(Qt::AlignCenter);
    }

    textoption.setWrapMode(QTextOption::WordWrap);  // NOTE: assumption as well

    layout.setFont(widgetFont());
    layout.setTextOption(textoption);
}

Qt::LayoutDirection IconWidgetPrivate::iconDirection(const QStyleOptionGraphicsItem *option) const
{
    Qt::LayoutDirection direction;

    if (invertLayout && orientation == Qt::Horizontal) {
        if (option->direction == Qt::LeftToRight) {
            direction = Qt::RightToLeft;
        } else {
            direction = Qt::LeftToRight;
        }
    } else {
        direction = option->direction;
    }

    return direction;
}

void IconWidgetPrivate::setActiveMargins()
{
    //sync here itemmargin with contentsrect, not overly pretty but it's where it's more reliable
    qreal left, top, right, bottom;
    q->getContentsMargins(&left, &top, &right, &bottom);
    if (left || top || right || bottom) {
        verticalMargin[ItemMargin].left = horizontalMargin[ItemMargin].left = left;
        verticalMargin[ItemMargin].top = horizontalMargin[ItemMargin].top = top;
        verticalMargin[ItemMargin].right = horizontalMargin[ItemMargin].right = right;
        verticalMargin[ItemMargin].bottom = horizontalMargin[ItemMargin].bottom = bottom;
    }
    activeMargins = (orientation == Qt::Horizontal ? horizontalMargin : verticalMargin);
}

void IconWidgetPrivate::setVerticalMargin(MarginType type, qreal left, qreal top,
                                          qreal right, qreal bottom)
{
    verticalMargin[type].left   = left;
    verticalMargin[type].right  = right;
    verticalMargin[type].top    = top;
    verticalMargin[type].bottom = bottom;
}

void IconWidgetPrivate::setHorizontalMargin(MarginType type, qreal left, qreal top,
                                            qreal right, qreal bottom)
{
    horizontalMargin[type].left   = left;
    horizontalMargin[type].right  = right;
    horizontalMargin[type].top    = top;
    horizontalMargin[type].bottom = bottom;
}

void IconWidgetPrivate::setVerticalMargin(MarginType type, qreal horizontal, qreal vertical)
{
    setVerticalMargin(type, horizontal, vertical, horizontal, vertical);
}

void IconWidgetPrivate::setHorizontalMargin(MarginType type, qreal horizontal, qreal vertical)
{
    setHorizontalMargin(type, horizontal, vertical, horizontal, vertical);
}

QRectF IconWidgetPrivate::addMargin(const QRectF &rect, MarginType type) const
{
    Q_ASSERT(activeMargins);

    const Margin &m = activeMargins[type];
    return rect.adjusted(-m.left, -m.top, m.right, m.bottom);
}

QRectF IconWidgetPrivate::subtractMargin(const QRectF &rect, MarginType type) const
{
    Q_ASSERT(activeMargins);

    const Margin &m = activeMargins[type];
    return rect.adjusted(m.left, m.top, -m.right, -m.bottom);
}

QSizeF IconWidgetPrivate::addMargin(const QSizeF &size, MarginType type) const
{
    Q_ASSERT(activeMargins);

    const Margin &m = activeMargins[type];
    return QSizeF(size.width() + m.left + m.right, size.height() + m.top + m.bottom);
}

QSizeF IconWidgetPrivate::subtractMargin(const QSizeF &size, MarginType type) const
{
    Q_ASSERT(activeMargins);

    const Margin &m = activeMargins[type];
    return QSizeF(size.width() - m.left - m.right, size.height() - m.top - m.bottom);
}

int IconWidgetPrivate::maxWordWidth(const QString text) const
{
    QFontMetricsF fm = Plasma::Theme::defaultTheme()->fontMetrics();
    QStringList words = text.split(' ');

    qreal maxWidth = 0;
    foreach (const QString &word, words) {
        maxWidth = qMax(maxWidth, fm.width(word));
    }
    return maxWidth;
}

QRectF IconWidgetPrivate::actionRect(ActionPosition position) const
{
    switch (position) {
    case TopLeft:
        return QRectF(iconActionMargin,
                      iconActionMargin,
                      iconActionSize,
                      iconActionSize);
    case TopRight:
        return QRectF(currentSize.width() - iconActionSize - iconActionMargin,
                      iconActionMargin,
                      iconActionSize,
                      iconActionSize);
    case BottomLeft:
        return QRectF(iconActionMargin,
                      currentSize.height() - iconActionSize - iconActionMargin,
                      iconActionSize,
                      iconActionSize);
    //BottomRight
    default:
        return QRectF(currentSize.width() - iconActionSize - iconActionMargin,
                      currentSize.height() - iconActionSize - iconActionMargin,
                      iconActionSize,
                      iconActionSize);
    }
}

} // Namespace

#endif

