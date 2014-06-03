/*
 *   Copyright 2010 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
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

#include "svgitem.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QQuickWindow>
#include <QSGTexture>
#include <QRectF>
#include <QDebug>

#include "plasma/svg.h"

#include "svgtexturenode.h"

#include <cmath> //floor()

namespace Plasma
{

SvgItem::SvgItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_smooth(false),
      m_textureChanged(false)
{
    setFlag(QQuickItem::ItemHasContents, true);
    connect(&m_units, &Units::devicePixelRatioChanged, this, &SvgItem::updateDevicePixelRatio);
}

SvgItem::~SvgItem()
{
}

void SvgItem::setElementId(const QString &elementID)
{
    if (elementID == m_elementID) {
        return;
    }

    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }

    m_elementID = elementID;
    emit elementIdChanged();
    emit naturalSizeChanged();

    m_textureChanged = true;
    update();
}

QString SvgItem::elementId() const
{
    return m_elementID;
}

QSizeF SvgItem::naturalSize() const
{
    if (!m_svg) {
        return QSizeF();
    } else if (!m_elementID.isEmpty()) {
        return m_svg.data()->elementSize(m_elementID);
    }

    return m_svg.data()->size();
}

void SvgItem::setSvg(Plasma::Svg *svg)
{
    if (m_svg) {
        disconnect(m_svg.data(), 0, this, 0);
    }
    m_svg = svg;
    updateDevicePixelRatio();

    if (svg) {
        connect(svg, SIGNAL(repaintNeeded()), this, SLOT(updateNeeded()));
        connect(svg, SIGNAL(repaintNeeded()), this, SIGNAL(naturalSizeChanged()));
        connect(svg, SIGNAL(sizeChanged()), this, SIGNAL(naturalSizeChanged()));
    }

    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }

    m_textureChanged = true;

    emit svgChanged();
    emit naturalSizeChanged();
}

Plasma::Svg *SvgItem::svg() const
{
    return m_svg.data();
}

void SvgItem::setSmooth(const bool smooth)
{
    if (smooth == m_smooth) {
        return;
    }
    m_smooth = smooth;
    emit smoothChanged();
}

bool SvgItem::smooth() const
{
    return m_smooth;
}

QSGNode *SvgItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    if (!window() || !m_svg) {
        delete oldNode;
        return Q_NULLPTR;
    }

    SVGTextureNode *textureNode = static_cast<SVGTextureNode *>(oldNode);
    if (!textureNode) {
        textureNode = new SVGTextureNode;
        textureNode->setFiltering(QSGTexture::Linear);
        m_textureChanged = true;
    }

    //TODO use a heuristic to work out when to redraw
    //if !m_smooth and size is approximate simply change the textureNode.rect without
    //updating the material

    if (m_textureChanged || textureNode->texture()->textureSize() != QSize(width(), height())) {
        //setContainsMultipleImages has to be done there since m_frameSvg can be shared with somebody else
        m_svg.data()->setContainsMultipleImages(!m_elementID.isEmpty());
        const QImage image = m_svg.data()->image(QSize(width(), height()), m_elementID);
        QSGTexture *texture = window()->createTextureFromImage(image);
        if (m_smooth) {
            texture->setFiltering(QSGTexture::Linear);
        }
        textureNode->setTexture(texture);
        m_textureChanged = false;

        textureNode->setRect(0, 0, width(), height());
    }

    return textureNode;
}

void SvgItem::updateNeeded()
{
    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }
    m_textureChanged = true;
    update();
}

void SvgItem::setImplicitWidth(qreal width)
{
    if (implicitWidth() == width) {
        return;
    }

    QQuickItem::setImplicitWidth(width);

    emit implicitWidthChanged();
}

qreal SvgItem::implicitWidth() const
{
    return QQuickItem::implicitWidth();
}

void SvgItem::setImplicitHeight(qreal height)
{
    if (implicitHeight() == height) {
        return;
    }

    QQuickItem::setImplicitHeight(height);

    emit implicitHeightChanged();
}

qreal SvgItem::implicitHeight() const
{
    return QQuickItem::implicitHeight();
}

void SvgItem::updateDevicePixelRatio()
{
    if (m_svg) {
        //devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
        //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
        m_svg.data()->setDevicePixelRatio(qMax<qreal>(1.0, floor(m_units.devicePixelRatio())));
    }
}

} // Plasma namespace

#include "svgitem.moc"
