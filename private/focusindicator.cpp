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
#include <QStyleOptionGraphicsItem>

#include <plasma/theme.h>
#include <plasma/framesvg.h>

#include <plasma/animator.h>

namespace Plasma
{

FocusIndicator::FocusIndicator(QGraphicsWidget *parent, QString widget)
    : QGraphicsWidget(parent),
      m_parent(parent),
      m_background(0),
      m_isUnderMouse(false)
{
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setAcceptsHoverEvents(true);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath(widget);
    m_background->setElementPrefix("hover");
    m_background->setCacheAllRenderedFrames(true);

    m_fade = Animator::create(Animator::FadeAnimation, this);
    m_fade->setTargetWidget(this);
    m_fade->setProperty("startOpacity", 0.0);
    m_fade->setProperty("targetOpacity", 1.0);

    m_hoverAnimation = Animator::create(Animator::PixmapTransitionAnimation);
    m_hoverAnimation->setProperty("duration", 250);
    m_hoverAnimation->setTargetWidget(this);
    if (m_background->hasElementPrefix("shadow")) {
        m_background->setElementPrefix("shadow");
        m_prefix = "shadow";
        syncGeometry();
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
    }

    parent->installEventFilter(this);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncGeometry()));
    syncGeometry();
}

FocusIndicator::~FocusIndicator()
{
    m_parent->removeEventFilter(this);
    delete m_fade;
}

void FocusIndicator::setCustomGeometry(const QRectF &geometry)
{
    m_customGeometry = geometry;
    syncGeometry();
}

void FocusIndicator::setCustomPrefix(const QString &prefix)
{
    m_customPrefix = prefix;
}

bool FocusIndicator::eventFilter(QObject *watched, QEvent *event)
{
    if (static_cast<QGraphicsWidget *>(watched) != m_parent || !m_parent) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        m_isUnderMouse = true;
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_isUnderMouse = false;
    }

    if (!m_parent->hasFocus() && event->type() == QEvent::GraphicsSceneHoverEnter) {
        m_prefix = m_customPrefix + "hover";
        syncGeometry();
        m_hoverAnimation->stop();
        m_background->setElementPrefix("shadow");
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
        m_background->setElementPrefix("hover");
        m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
        m_hoverAnimation->start();
    } else if (!m_parent->hasFocus() && event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_prefix = m_customPrefix + "shadow";
        syncGeometry();
        m_hoverAnimation->stop();
        m_background->setElementPrefix("hover");
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
        m_background->setElementPrefix("shadow");
        m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
        m_hoverAnimation->start();
    } else if (event->type() == QEvent::GraphicsSceneResize) {
        syncGeometry();
    } else if (event->type() == QEvent::FocusIn) {
        m_prefix = m_customPrefix + "focus";
        syncGeometry();
        m_hoverAnimation->stop();
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
        m_background->setElementPrefix("focus");
        m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
        m_hoverAnimation->start();
    } else if (!m_isUnderMouse && event->type() == QEvent::FocusOut) {
        m_prefix = m_customPrefix + "shadow";
        syncGeometry();
        m_hoverAnimation->stop();

        m_background->setElementPrefix("focus");
        m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
        m_background->setElementPrefix("shadow");
        m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
        m_hoverAnimation->start();
    }

    return false;
}

void FocusIndicator::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    m_background->setElementPrefix("shadow");
    m_background->resizeFrame(event->newSize());
    m_background->setElementPrefix("hover");
    m_background->resizeFrame(event->newSize());
    m_background->setElementPrefix("focus");
    m_background->resizeFrame(event->newSize());

    if (m_hoverAnimation->state() == QAbstractAnimation::Running) {
        m_hoverAnimation->stop();
    }

    m_background->setElementPrefix(m_prefix);
    m_hoverAnimation->setProperty("startPixmap", m_background->framePixmap());
    m_hoverAnimation->setProperty("targetPixmap", m_background->framePixmap());
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
    QRectF geom;
    if (!m_customGeometry.isEmpty()) {
        geom = m_customGeometry;
    } else {
        geom = m_parent->boundingRect();
    }

    if (m_background->hasElementPrefix(m_prefix)) {
        //always take borders from hover to make it stable
        m_background->setElementPrefix("hover");
        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        m_background->setElementPrefix(m_prefix);
        setGeometry(QRectF(geom.topLeft() + QPointF(-left, -top), geom.size() + QSize(left+right, top+bottom)));
    } else if (m_background->hasElement(m_prefix)) {
        QRectF elementRect = m_background->elementRect(m_prefix);
        elementRect.moveCenter(geom.center());
        setGeometry(elementRect);
    }
}

}

#include <focusindicator_p.moc>

