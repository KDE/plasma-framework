/*
 *   Copyright 2010 Marco Martin <mart@kde.org>
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

#include "svgitem_p.h"

#include <QtGui/QPainter>

#include "kdebug.h"
#include "plasma/svg.h"

namespace Plasma
{

SvgItem::SvgItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}


SvgItem::~SvgItem()
{
}

void SvgItem::setElementId(const QString &elementID)
{
    m_elementID = elementID;
    emit sizeChanged();
    update();
}

QString SvgItem::elementId() const
{
    return m_elementID;
}

qreal SvgItem::naturalWidth() const
{
    if (!m_svg) {
        return 0;
    } else if (!m_elementID.isEmpty()) {
        return m_svg.data()->elementSize(m_elementID).height();
    } else {
        return m_svg.data()->size().height();
    }
}

qreal SvgItem::naturalHeight() const
{
    if (!m_svg) {
        return 0;
    } else if (!m_elementID.isEmpty()) {
        return m_svg.data()->elementSize(m_elementID).width();
    } else {
        return m_svg.data()->size().width();
    }
}

void SvgItem::setSvg(Plasma::Svg *svg)
{
    if (m_svg) {
        disconnect(m_svg.data(), 0, this, 0);
    }
    m_svg = svg;
    connect(svg, SIGNAL(repaintNeeded()), this, SLOT(update()));
    connect(svg, SIGNAL(repaintNeeded()), this, SLOT(sizeChanged()));
    emit sizeChanged();
}

Plasma::Svg *SvgItem::svg() const
{
    return m_svg.data();
}

void SvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_svg) {
        return;
    }
    //setContainsMultipleImages has to be done there since m_frameSvg can be shared with somebody else
    m_svg.data()->setContainsMultipleImages(!m_elementID.isEmpty());
    m_svg.data()->paint(painter, boundingRect(), m_elementID);
}

} // Plasma namespace

#include "svgitem_p.moc"
