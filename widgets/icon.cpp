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

//#define BACKINGSTORE_BLUR_HACK

#ifdef BACKINGSTORE_BLUR_HACK
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

        Icon* m_icon;
        QAction* m_action;
        QPixmap m_pixmap;
        QRectF m_rect;

        bool m_hovered;
        bool m_pressed;
        bool m_selected;
        bool m_visible;

        Phase::AnimId m_animationId;
};

class Icon::Private
{
    public:
        Private()
            : svg("widgets/iconbutton"),
              svgElements(0),
              size(128*1.1, 128*1.1),
              iconSize(128, 128),
              state(Private::NoState)
        {
            svg.setContentType(Plasma::Svg::ImageSet);
            svg.resize(size);

            //TODO: recheck when svg changes
            checkSvgElements();
        }

        ~Private()
        {
            qDeleteAll(cornerActions);
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

        enum ButtonState
        {
            NoState,
            HoverState,
            PressedState
        };

        QString text;
        Svg svg;
        int svgElements;
        QSizeF size;
        QSizeF iconSize;
        QIcon icon;
        ButtonState state;

        QAction *testAction;
        QList<IconAction*> cornerActions;
};

IconAction::IconAction(Icon* icon, QAction *action)
    : m_icon(icon),
      m_action(action),
      m_hovered(false),
      m_pressed(false),
      m_visible(false),
      m_animationId(-1)
{
}

void IconAction::show()
{
    if (m_animationId) {
        Phase::self()->stopElementAnimation(m_animationId);
    }

    rebuildPixmap();

    m_animationId = Phase::self()->animateElement(m_icon, Phase::ElementAppear);
    Phase::self()->setAnimationPixmap(m_animationId, m_pixmap);
    m_visible = true;
}

void IconAction::hide()
{
    if (m_animationId) {
        Phase::self()->stopElementAnimation(m_animationId);
    }

    rebuildPixmap();

    m_animationId = Phase::self()->animateElement(m_icon, Phase::ElementDisappear);
    Phase::self()->setAnimationPixmap(m_animationId, m_pixmap);
    m_visible = false;
}

bool IconAction::isVisible() const
{
    return m_visible;
}

bool IconAction::isPressed() const
{
    return m_pressed;
}

bool IconAction::isHovered() const
{
    return m_hovered;
}

void IconAction::setSelected(bool selected)
{
    m_selected = selected;
}

bool IconAction::isSelected() const
{
    return m_selected;
}

void IconAction::setRect(const QRectF &rect)
{
    m_rect = rect;
}

QRectF IconAction::rect() const
{
    return m_rect;
}

void IconAction::rebuildPixmap()
{
    // Determine proper QIcon mode based on selection status
    QIcon::Mode mode = QIcon::Normal;
    if (m_selected) {
        mode = QIcon::Selected;
    }

    // Draw everything
    m_pixmap = QPixmap(26, 26);
    m_pixmap.fill(Qt::transparent);

    int element = Icon::Private::SvgMinibutton;
    if (m_pressed) {
        element = Icon::Private::SvgMinibuttonPressed;
    } else if (m_hovered) {
        element = Icon::Private::SvgMinibuttonHover;
    }

    QPainter painter(&m_pixmap);
    m_icon->drawActionButtonBase(&painter, m_pixmap.size(), element);
    m_action->icon().paint(&painter, 2, 2, 22, 22, Qt::AlignCenter, mode);
}

bool IconAction::event(QEvent::Type type, const QPointF &pos)
{
    switch (type) {
    case QEvent::MouseButtonPress: {
        setSelected(m_rect.contains(pos));
        return isSelected();
        }
        break;

    case QEvent::MouseMove: {
        bool wasSelected = isSelected();
        bool active = m_rect.contains(pos);
        setSelected(wasSelected && active);
        return (wasSelected != isSelected()) || active;
        }
        break;

    case QEvent::MouseButtonRelease: {
        bool wasSelected = isSelected();
        setSelected(false);
        if (wasSelected) {
            m_action->trigger();
        }

        return wasSelected;
        }
        break;

    case QEvent::GraphicsSceneHoverEnter:
        m_pressed = false;
        m_hovered = true;
        break;

    case QEvent::GraphicsSceneHoverLeave:
        m_pressed = false;
        m_hovered = false;
        break;

    default:
            break;
    }

    return false;
}

Phase::AnimId IconAction::animationId() const
{
    return m_animationId;
}

QAction* IconAction::action() const
{
    return m_action;
}

void IconAction::paint(QPainter *painter) const
{
    painter->drawPixmap(m_rect.toRect(), Phase::self()->animationResult(m_animationId));
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

    d->testAction = new QAction(KIcon("exec"), i18n("Open"), this);
    addAction(d->testAction);
}

void Icon::addAction(QAction *action)
{
    int count = d->cornerActions.count();
    if (count > 3) {
        kDebug() << "Icon::addAction(QAction*) no more room for more actions!" << endl;
    }

    IconAction* iconAction = new IconAction(this, action);
    d->cornerActions.append(iconAction);
    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));

    //FIXME: set other icon rects properly, and shift them when we change our size
    switch (count) {
    case Private::TopLeft:
        iconAction->setRect(QRectF(6, 6, 32, 32));
        break;
    case Private::TopRight:
        break;
    case Private::BottomLeft:
        break;
    case Private::BottomRight:
        break;
    }
}

void Icon::actionDestroyed(QObject* action)
{
    QList<IconAction*>::iterator it = d->cornerActions.begin();

    while (it != d->cornerActions.end()) {
        if ((*it)->action() == action) {
            d->cornerActions.erase(it);
            break;
        }
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

#ifdef BACKINGSTORE_BLUR_HACK
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

    d->svg.resize(d->size);

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
    foreach (IconAction *action, d->cornerActions) {
        if (action->animationId()) {
            action->paint(painter);
        }
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

void Icon::drawActionButtonBase(QPainter* painter, const QSize &size, int element)
{
    // Determine proper svg element
    QString id;
    if (d->svgElements & Private::SvgMinibutton) {
        id = "minibutton";
    }

    if (element == Private::SvgMinibuttonPressed) {
        if (d->svgElements & Private::SvgMinibuttonPressed) {
            id = "minibutton-pressed";
        } else if (d->svgElements & Private::SvgMinibuttonHover) {
            id = "minibutton-hover";
        }
    } else if (element == Icon::Private::SvgMinibuttonHover &&
               d->svgElements & Private::SvgMinibuttonHover) {
        id = "minibutton-hover";
    }

    if (!id.isEmpty()) {
        d->svg.resize(size);
        d->svg.paint(painter, 0, 0, id);
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
    foreach (IconAction *action, d->cornerActions) {
        action->event(event->type(), event->pos());
    }

    QGraphicsItem::mousePressEvent(event);
}

void Icon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool inside = boundingRect().contains(event->pos());
    bool wasClicked = d->state == Private::PressedState && inside;

    if (inside) {
        d->state = Private::HoverState;

        foreach (IconAction *action, d->cornerActions) {
            action->event(event->type(), event->pos());
        }

    } else {
        d->state = Private::NoState;
    }

    if (wasClicked) {
        emit pressed(false);
        emit clicked();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void Icon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    foreach (IconAction *action, d->cornerActions) {
        action->show();
        action->event(event->type(), event->pos());
    }

    d->state = Private::HoverState;
    QGraphicsItem::hoverEnterEvent(event);
}

void Icon::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    foreach (IconAction *action, d->cornerActions) {
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
