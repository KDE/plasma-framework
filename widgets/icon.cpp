/*
 *   Copyright (C) 2007 by Aaron Seigo aseigo@kde.org
 *   Copyright (C) 2007 by Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>

//#define PROVE_IT_CAN_BE_DONE

#ifdef PROVE_IT_CAN_BE_DONE
#include <private/qwindowsurface_p.h>
#endif

#include <KIcon>
#include <KImageEffect>
#include <KUrl>
#include <KRun>
#include <KMimeType>
#include <KDebug>

#include "phase.h"
#include "svg.h"
#include "effects/blur.cpp"
#include "icon.h"

namespace Plasma
{

class PrivateSvgInfo
{
    public:
        PrivateSvgInfo()
            : svg("widgets/iconbutton"),
              svgElements(0)
        {
            svg.setContentType(Plasma::Svg::ImageSet);

            //TODO: recheck when svg changes
            checkSvgElements();
        }

        void checkSvgElements()
        {
            if (svg.elementExists("background")) {
                svgElements |= SvgBackground;
            }

            if (svg.elementExists("background-hover")) {
                svgElements |= SvgBackgroundHover;
            }

            if (svg.elementExists("background-pressed")) {
                svgElements |= SvgBackgroundPressed;
            }

            if (svg.elementExists("foreground")) {
                svgElements |= SvgForeground;
            }

            if (svg.elementExists("foreground-hover")) {
                svgElements |= SvgForegroundHover;
            }

            if (svg.elementExists("foreground-pressed")) {
                svgElements |= SvgForegroundPressed;
            }

            if (svg.elementExists("minibutton")) {
                svgElements |= SvgMinibutton;
            }

            if (svg.elementExists("minibutton-hover")) {
                svgElements |= SvgMinibuttonHover;
            }

            if (svg.elementExists("minibutton-pressed")) {
                svgElements |= SvgMinibuttonPressed;
            }
        }

        enum {
            NoSvg = 0,
            SvgBackground = 1,
            SvgBackgroundHover = 2,
            SvgBackgroundPressed = 4,
            SvgForeground = 8,
            SvgForegroundHover = 16,
            SvgForegroundPressed = 32,
            SvgMinibutton = 64,
            SvgMinibuttonHover = 256,
            SvgMinibuttonPressed = 128
        };

        Svg svg;
        int svgElements;
};


class Icon::Private : public PrivateSvgInfo
{
    public:
        Private()
            : size(128*1.1, 128*1.1),
              iconSize(128, 128),
              state(Private::NoState)
        {
            svg.resize(size);
        }

        enum ButtonState
        {
            NoState,
            HoverState,
            PressedState
        };

        QString text;
        QSizeF size;
        QSizeF iconSize;
        QIcon icon;
        ButtonState state;

        IconAction *testAction;
        QHash<Icon::ActionPosition, IconAction*> cornerActions;
};

class IconAction::Private : public PrivateSvgInfo
{
    public:
        Private() :
            hovered(false),
            pressed(false),
            visible(false),
            animationId(-1)
        {}

        QPixmap pixmap;
        QRectF rect;

        bool hovered;
        bool pressed;
        bool selected;
        bool visible;

        Phase::AnimId animationId;
};

IconAction::IconAction(QObject *parent)
    : QAction(parent),
      d(new Private)
{
}

IconAction::IconAction(const QIcon &icon, QObject *parent)
    : QAction(icon, QString(), parent),
      d(new Private)
{
}

void IconAction::show()
{
    if (d->animationId)
        Phase::self()->stopElementAnimation(d->animationId);

    rebuildPixmap();    // caching needs to be done

    QGraphicsItem *parentItem = dynamic_cast<QGraphicsItem*>(parent()); // dangerous
    d->animationId = Phase::self()->animateElement(parentItem, Phase::ElementAppear);
    Phase::self()->setAnimationPixmap(d->animationId, d->pixmap);
    d->visible = true;
}

void IconAction::hide()
{
    if (d->animationId)
        Phase::self()->stopElementAnimation(d->animationId);

    rebuildPixmap();    // caching needs to be done

    QGraphicsItem *parentItem = dynamic_cast<QGraphicsItem*>(parent()); // dangerous
    d->animationId = Phase::self()->animateElement(parentItem, Phase::ElementDisappear);
    Phase::self()->setAnimationPixmap(d->animationId, d->pixmap);
    d->visible = false;
}

bool IconAction::isVisible() const
{
    return d->visible;
}

bool IconAction::isPressed() const
{
    return d->pressed;
}

bool IconAction::isHovered() const
{
    return d->hovered;
}

void IconAction::setSelected(bool selected)
{
    d->selected = selected;
}

bool IconAction::isSelected() const
{
    return d->selected;
}

void IconAction::setRect(const QRectF &rect)
{
    d->rect = rect;
}

QRectF IconAction::rect() const
{
    return d->rect;
}

void IconAction::rebuildPixmap()
{
    // Determine proper QIcon mode based on selection status
    QIcon::Mode mode = QIcon::Normal;
    if (d->selected)
        mode = QIcon::Selected;

    // Determine proper svg element
    QString element;
    if (d->svgElements & Private::SvgMinibutton)
        element = "minibutton";

    if (d->pressed)
    {
        if (d->svgElements & Private::SvgMinibuttonPressed)
            element = "minibutton-pressed";
        else if (d->svgElements & Private::SvgMinibuttonHover)
            element = "minibutton-hover";
    }
    else if (d->hovered)
    {
        if (d->svgElements & Private::SvgMinibuttonHover)
            element = "minibutton-hover";
    }

    // Draw everything
    d->pixmap = QPixmap(26, 26);
    d->pixmap.fill(Qt::transparent);

    QPainter painter(&d->pixmap);
    // first paint the foreground element
    if (!element.isEmpty()) {
        d->svg.resize(d->pixmap.size());
        d->svg.paint(&painter, 0, 0, element);
    }

    // then paint the icon
    icon().paint(&painter, 2, 2, 22, 22, Qt::AlignCenter, mode);     // more assumptions

}

bool IconAction::event(QEvent::Type type, const QPointF &pos)
{
    switch (type)
    {
        case QEvent::MouseButtonPress:
            {
                setSelected(d->rect.contains(pos));
                return isSelected();
            }
            break;

        case QEvent::MouseMove:
            {
                bool wasSelected = isSelected();
                bool active = d->rect.contains(pos);
                setSelected(wasSelected && active);
                return (wasSelected != isSelected()) || active;
            }
            break;

        case QEvent::MouseButtonRelease:
            {
                bool wasSelected = isSelected();
                setSelected(false);
                if (wasSelected)
                    trigger();

                return wasSelected;
            }
            break;

        case QEvent::GraphicsSceneHoverEnter:
            d->pressed = false;
            d->hovered = true;
            break;

        case QEvent::GraphicsSceneHoverLeave:
            d->pressed = false;
            d->hovered = false;
            break;

        default:
            break;
    }

    return false;
}

Phase::AnimId IconAction::animationId() const
{
    return d->animationId;
}

void IconAction::paint(QPainter *painter) const
{
    painter->drawPixmap(d->rect.toRect(), Phase::self()->animationResult(d->animationId));
}




Icon::Icon(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new Private)
{
    init();
}

Icon::Icon(const QString &text, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new Private)
{
    setText(text);
    init();
}

Icon::Icon(const QIcon &icon, const QString &text, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new Private)
{
    setText(text);
    setIcon(icon);
    init();
}

Icon::~Icon()
{
    delete d;
}

void Icon::init()
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);

    d->testAction = new IconAction(KIcon("exec"), this);
    setCornerAction(TopLeft, d->testAction);
}

void Icon::setCornerAction(ActionPosition pos, IconAction *action)
{
    if (d->cornerActions[pos])
    {
        // TODO: do we delete it, or just clear it (watch out for leaks with the latter!)
    }

    d->cornerActions[pos] = action;
    switch (pos)
    {
        case TopLeft:
            {
                QRectF rect(6, 6, 32, 32);
                action->setRect(rect);
            }
            break;

        case TopRight:
            break;
        case BottomLeft:
            break;
        case BottomRight:
            break;
    }
}

void Icon::calculateSize()
{
    prepareGeometryChange();
    QFontMetrics fm(QApplication::font());      // TODO: get the font somewhere more appropriate
    QSizeF fmSize = fm.size(Qt::AlignHCenter | Qt::AlignTop, d->text);

    int margin = 6;     // hmmm
    qreal height = d->iconSize.height() + (margin*2) + fmSize.height();
    qreal width = margin + qMax(fmSize.width(), d->iconSize.width()) + margin;
    d->size = QSizeF(width, height);
    d->svg.resize(d->size);
    update();
}

QRectF Icon::boundingRect() const
{
    return QRectF(QPointF(0, 0), d->size);
}

void Icon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

#ifdef PROVE_IT_CAN_BE_DONE
     if (d->state == Private::HoverState && scene()) {
         QList<QGraphicsView*> views = scene()->views();
         if (views.count() > 0) {
             QPixmap* pix = static_cast<QPixmap*>(views[0]->windowSurface()->paintDevice());
             QImage image(boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
             {
                 QPainter p(&image);
                 p.drawPixmap(image.rect(), *pix, sceneBoundingRect());
             }
             expblur<16,7>(image, 8);
             painter->drawImage(0, 0, image);
         }
     }
#endif

    QString element;
    if (d->svgElements & Private::SvgBackground) {
        element = "background";
    }

    switch (d->state) {
        case Private::NoState:
            break;
        case Private::HoverState:
            if (d->svgElements & Private::SvgBackgroundHover) {
                element = "background-hover";
            }
            break;
        case Private::PressedState:
            if (d->svgElements & Private::SvgBackgroundPressed) {
                element = "background-pressed";
            } else if (d->svgElements & Private::SvgBackgroundHover) {
                element = "background-hover";
            }
            break;
    }

    if (!element.isEmpty()) {
        d->svg.paint(painter, 0, 0, element);
        element = QString();
    }

    if (!d->icon.isNull()) {
        qreal iw = d->iconSize.width();
        qreal ih = d->iconSize.height();
        qreal deltaX = (d->size.width() - iw) / 2;
        qreal deltaY = (d->size.height() - ih) / 2 ;
        if (d->state == Private::PressedState) {
            painter->save();
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            painter->scale(0.95, 0.95);
            deltaY *= 0.95;
            deltaX *= 0.95;
            painter->drawPixmap((int)(deltaX + (iw * .025)), (int)(deltaY + (ih * .025)),
                                d->icon.pixmap(d->iconSize.toSize()));
            painter->restore();
        } else {
            painter->drawPixmap((int)deltaX, (int)deltaY, d->icon.pixmap(d->iconSize.toSize()));
        }
    }

    // Make it default
    if (d->svgElements & Private::SvgForeground) {
        element = "foreground";
    }

    switch (d->state) {
        case Private::NoState:
            break;
        case Private::HoverState:
            if (d->svgElements & Private::SvgForegroundHover) {
                element = "foreground-hover";
            }
            break;
        case Private::PressedState:
            if (d->svgElements & Private::SvgForegroundPressed) {
                element = "foreground-pressed";
            } else if (d->svgElements & Private::SvgForegroundHover) {
                element = "foreground-hover";
            }
            break;
    }

    if (!element.isEmpty()) {
        //kDebug() << "painting " << element << endl;
        d->svg.paint(painter, 0, 0, element);
        element.clear();
    }

    // Draw top-left button
    foreach (IconAction *action, d->cornerActions)
    {
        if (action->animationId())
            action->paint(painter);
    }

    // Draw text last because its overlayed
    if (!d->text.isEmpty()) {
        qreal offset = (d->iconSize.height() + 12);     // TODO this shouldn't be hardcoded?
        QRectF textRect(0, offset, d->size.width(), d->size.height() - offset);

        QTextOption textOpt;
        textOpt.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        textOpt.setWrapMode(QTextOption::WordWrap);

        painter->setPen(Qt::white);
        painter->drawText(textRect, d->text, textOpt);
    }

}

void Icon::setText(const QString& text)
{
    d->text = text;
    calculateSize();
}

QString Icon::text() const
{
    return d->text;
}

void Icon::setIcon(const QString& icon)
{
    if (icon.isEmpty()) {
        setIcon(QIcon());
        return;
    }

    setIcon(KIcon(icon));
}

void Icon::setIcon(const QIcon& icon)
{
    d->icon = icon;
    calculateSize();
}

QSizeF Icon::iconSize() const
{
    return d->iconSize;
}

void Icon::setIconSize(const QSizeF& s)
{
    d->iconSize = s;
    calculateSize();
}

void Icon::setIconSize(int w, int h)
{
    setIconSize(QSizeF(w, h));
}

bool Icon::isDown()
{
    return d->state == Private::PressedState;
}

void Icon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    foreach (IconAction *action, d->cornerActions)
        action->event(event->type(), event->pos());

    QGraphicsItem::mousePressEvent(event);
}

void Icon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool inside = boundingRect().contains(event->pos());
    bool wasClicked = d->state == Private::PressedState && inside;

    if (inside) {
        d->state = Private::HoverState;

        foreach (IconAction *action, d->cornerActions)
            action->event(event->type(), event->pos());

    }
    else
        d->state = Private::NoState;

    if (wasClicked)
    {
        emit pressed(false);
        emit clicked();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void Icon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    foreach (IconAction *action, d->cornerActions)
    {
        action->show();
        action->event(event->type(), event->pos());
    }

    d->state = Private::HoverState;
    QGraphicsItem::hoverEnterEvent(event);
}

void Icon::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    foreach (IconAction *action, d->cornerActions)
    {
        action->hide();
        action->event(event->type(), event->pos());
    }

    d->state = Private::NoState;
    QGraphicsItem::hoverLeaveEvent(event);
}

void Icon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->state == Private::PressedState) {
        d->state = Private::HoverState;
    }

    QGraphicsItem::mouseMoveEvent(event);
}


QSizeF Icon::sizeHint() const
{
    return d->size;
}

QSizeF Icon::minimumSize() const
{
    return d->size; // probably should be more flexible on this =)
}

QSizeF Icon::maximumSize() const
{
    return d->size;
}

Qt::Orientations Icon::expandingDirections() const
{
    return Qt::Horizontal;
}

bool Icon::hasHeightForWidth() const
{
    return true;
}

qreal Icon::heightForWidth(qreal w) const
{
    return w; //FIXME: we shouldn't assume squareness but actually calculate based on text and what not
}

bool Icon::hasWidthForHeight() const
{
    return true;
}

qreal Icon::widthForHeight(qreal h) const
{
    return h; //FIXME: we shouldn't assume squareness but actually calculate based on text and what not
}

QRectF Icon::geometry() const
{
    return boundingRect();
}

void Icon::setGeometry(const QRectF &r)
{
    // TODO: this is wrong, but we should probably never call setGeometry anyway!
    setIconSize(r.size());
    setPos(r.x(),r.y());
}

} // namespace Plasma

#include "icon.moc"
