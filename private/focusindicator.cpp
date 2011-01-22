/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "focusindicator_p.h"

#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QStringBuilder>
#include <QStyleOptionGraphicsItem>

#include <plasma/theme.h>
#include <plasma/framesvg.h>

#include <plasma/animator.h>

namespace Plasma
{

FocusIndicator::FocusIndicator(QGraphicsWidget *parent, const QString &widget)
    : QGraphicsWidget(parent),
      m_parent(parent),
      m_background(new Plasma::FrameSvg(this)),
      m_isUnderMouse(false)
{
    m_background->setImagePath(widget);
    init(parent);
}

FocusIndicator::FocusIndicator(QGraphicsWidget *parent, FrameSvg *svg)
    : QGraphicsWidget(parent),
      m_parent(parent),
      m_background(svg),
      m_isUnderMouse(false)
{
    init(parent);
}

void FocusIndicator::init(QGraphicsWidget *parent)
{
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setAcceptsHoverEvents(true);

    m_background->setCacheAllRenderedFrames(true);

    m_fade = Animator::create(Animator::FadeAnimation, this);
    m_fade->setTargetWidget(this);
    m_fade->setProperty("startOpacity", 0.0);
    m_fade->setProperty("targetOpacity", 1.0);

    m_hoverAnimation = Animator::create(Animator::PixmapTransitionAnimation);
    m_hoverAnimation->setProperty("duration", 250);
    m_hoverAnimation->setTargetWidget(this);

    m_testPrefix = "hover";
    if (m_background->hasElementPrefix("shadow") ||
        m_background->hasElement("shadow")) {
        m_prefix = "shadow";
    }

    parent->installEventFilter(this);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncGeometry()));
}

FocusIndicator::~FocusIndicator()
{
    m_parent->removeEventFilter(this);
    delete m_fade;
}

void FocusIndicator::setCustomGeometry(const QRectF &geometry)
{
    if (m_customGeometry == geometry) {
        return;
    }

    m_customGeometry = geometry;
    syncGeometry();
}

void FocusIndicator::setCustomPrefix(const QString &prefix)
{
    QString was = m_prefix;
    if (!m_prefix.isEmpty() && !m_customPrefix.isEmpty()) {
        m_prefix.remove(m_customPrefix);
    }

    m_customPrefix = prefix;

    if (!m_prefix.isEmpty()) {
        m_prefix.prepend(m_customPrefix);
    }

    m_testPrefix = m_customPrefix % "hover";
    if (m_prefix.isEmpty()) {
        m_prefix = m_customPrefix % "shadow";
    }

    if (m_prefix == was) {
        return;
    }

    syncGeometry();
    resizeEvent(0);
}

bool FocusIndicator::eventFilter(QObject *watched, QEvent *event)
{
    if (Theme::defaultTheme()->useNativeWidgetStyle() ||
        static_cast<QGraphicsWidget *>(watched) != m_parent || !m_parent ) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        m_isUnderMouse = true;
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_isUnderMouse = false;
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverEnter:
            if (!m_parent->hasFocus()) {
                m_prefix = m_customPrefix % "hover";
                syncGeometry();
                m_hoverAnimation->stop();
                if (m_background->hasElementPrefix(m_testPrefix)) {
                    m_background->setElementPrefix(m_customPrefix % "shadow");
                    m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
                    m_background->setElementPrefix(m_customPrefix % "hover");
                    m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
                } else if (m_background->hasElement(m_testPrefix)) {
                    m_hoverAnimation->setProperty("startPixmap", m_background->pixmap(m_customPrefix % "shadow"));
                    m_hoverAnimation->setProperty("targetPixmap", m_background->pixmap(m_customPrefix % "hover"));
                }

                m_hoverAnimation->start();
            }
            break;

        case QEvent::GraphicsSceneHoverLeave:
            if (!m_parent->hasFocus()) {
                m_prefix = m_customPrefix % "shadow";
                syncGeometry();
                m_hoverAnimation->stop();

                if (m_background->hasElementPrefix(m_testPrefix)) {
                    m_background->setElementPrefix(m_customPrefix % "hover");
                    m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
                    m_background->setElementPrefix(m_customPrefix % "shadow");
                    m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
                } else if (m_background->hasElement(m_testPrefix)) {
                    m_hoverAnimation->setProperty("startPixmap", m_background->pixmap(m_customPrefix % "hover"));
                    m_hoverAnimation->setProperty("targetPixmap", m_background->pixmap(m_customPrefix % "shadow"));
                }
                m_hoverAnimation->start();
            }
            break;

        case QEvent::GraphicsSceneResize:
            syncGeometry();
        break;

        case QEvent::FocusIn:
            m_prefix = m_customPrefix % "focus";
            syncGeometry();
            m_hoverAnimation->stop();

            if (m_background->hasElementPrefix(m_customPrefix % "focus")) {
                //m_background->setElementPrefix(m_customPrefix % "shadow");
                m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
                m_background->setElementPrefix(m_customPrefix % "focus");
                m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
            } else if (m_background->hasElement(m_customPrefix % "focus")) {
                //m_hoverAnimation->setProperty("startPixmap", m_background->pixmap(m_customPrefix % "shadow"));
                m_hoverAnimation->setProperty("targetPixmap", m_background->pixmap(m_customPrefix % "focus"));
            }

            m_hoverAnimation->start();
            break;

        case QEvent::FocusOut:
            if (!m_isUnderMouse) {
                m_prefix = m_customPrefix % "shadow";
                syncGeometry();
                m_hoverAnimation->stop();

                if (m_background->hasElementPrefix(m_customPrefix % "focus")) {
                    m_background->setElementPrefix("focus");
                    m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
                    m_background->setElementPrefix("shadow");
                    m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
                } else if (m_background->hasElement(m_customPrefix % "focus")) {
                    m_hoverAnimation->setProperty("startPixmap", m_background->pixmap(m_customPrefix % "focus"));
                    m_hoverAnimation->setProperty("targetPixmap", m_background->pixmap(m_customPrefix % "shadow"));
                }

                m_hoverAnimation->start();
            }
            break;

        default:
            break;
    };

    return false;
}

void FocusIndicator::resizeEvent(QGraphicsSceneResizeEvent *)
{
    if (m_background->hasElementPrefix(m_customPrefix % "shadow")) {
        m_background->setElementPrefix(m_customPrefix % "shadow");
        m_background->resizeFrame(size());
    }

    if (m_background->hasElementPrefix(m_customPrefix % "hover")) { 
        m_background->setElementPrefix(m_customPrefix % "hover");
        m_background->resizeFrame(size());
    }

    if (m_background->hasElementPrefix(m_customPrefix % "focus")) { 
        m_background->setElementPrefix(m_customPrefix % "focus");
        m_background->resizeFrame(size());
    }

    if (m_hoverAnimation->state() == QAbstractAnimation::Running) {
        m_hoverAnimation->stop();
    }

    if (m_background->hasElementPrefix(m_testPrefix)) {
        m_background->setElementPrefix(m_prefix);
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
        m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
    } else if (m_background->hasElement(m_testPrefix)) {
        m_hoverAnimation->setProperty("startPixmap", m_background->pixmap(m_prefix));
        m_hoverAnimation->setProperty("targetPixmap", m_background->pixmap(m_prefix));
    }
}

void FocusIndicator::animateVisibility(const bool visible)
{
    m_fade->setProperty("startOpacity", opacity());

    if (visible) {
        m_fade->setProperty("targetOpacity", 1.0);
    } else {
        m_fade->setProperty("targetOpacity", 0);
    }
    m_fade->start();
}

void FocusIndicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->drawPixmap(
                option->rect,
                m_hoverAnimation->property("currentPixmap").value<QPixmap>());
}

void FocusIndicator::syncGeometry()
{
    if (Theme::defaultTheme()->useNativeWidgetStyle()) {
        hide();
        return;
    } else if (!isVisible()) {
        show();
    }

    QRectF geom;
    if (!m_customGeometry.isEmpty()) {
        geom = m_customGeometry;
    } else {
        geom = m_parent->boundingRect();
    }

    if (m_background->hasElementPrefix(m_testPrefix)) {
        //always take borders from hover to make it stable
        m_background->setElementPrefix(m_testPrefix);
        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        m_background->setElementPrefix(m_prefix);
        setGeometry(QRectF(geom.topLeft() + QPointF(-left, -top), geom.size() + QSize(left+right, top+bottom)));
    } else if (m_background->hasElement(m_testPrefix)) {
        QRectF elementRect = m_background->elementRect(m_testPrefix);
        elementRect.moveCenter(geom.center());
        setGeometry(elementRect);
    }
}

}

#include <focusindicator_p.moc>

