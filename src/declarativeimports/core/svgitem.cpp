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

#include "svgitem.h"

#include <QPainter>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGSimpleTextureNode>


#include "QDebug"
#include "plasma/svg.h"

namespace Plasma
{

SvgItem::SvgItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_smooth(false),
      m_dirty(false),
      m_texture(0)
{
    setFlag(QQuickItem::ItemHasContents, true);
}


SvgItem::~SvgItem()
{
    delete m_texture;
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
    updateNeeded();
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

QSGNode* SvgItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData);
    if (!m_dirty) {
        return oldNode;
    }

    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!textureNode) {
        textureNode = new QSGSimpleTextureNode;
    }

    if (window() && m_svg) {
        m_svg.data()->resize(width(), height());
        m_svg.data()->setContainsMultipleImages(!m_elementID.isEmpty());
        
        const QImage image = m_svg.data()->image(m_elementID);
        textureNode->setRect(0,0, image.width(), image.height());

        delete m_texture;
        m_texture = window()->createTextureFromImage(image);
        textureNode->setTexture(m_texture);
        m_dirty = false;
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

    m_dirty = true;
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

} // Plasma namespace

#include "svgitem.moc"
