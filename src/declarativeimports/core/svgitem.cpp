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

#include <QQuickWindow>
#include <QSGTexture>
#include <QRectF>
#include <QDebug>

#include "plasma/svg.h"

#include <QuickAddons/ManagedTextureNode>

#include <cmath> //floor()

namespace Plasma
{

SvgItem::SvgItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_smooth(false),
      m_textureChanged(false)
{
    setFlag(QQuickItem::ItemHasContents, true);
    connect(&Units::instance(), &Units::devicePixelRatioChanged, this, &SvgItem::updateDevicePixelRatio);
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

    scheduleImageUpdate();
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
        disconnect(m_svg.data(), nullptr, this, nullptr);
    }
    m_svg = svg;
    updateDevicePixelRatio();

    if (svg) {
        connect(svg, &Svg::repaintNeeded, this, &SvgItem::updateNeeded);
        connect(svg, &Svg::repaintNeeded, this, &SvgItem::naturalSizeChanged);
        connect(svg, &Svg::sizeChanged, this, &SvgItem::naturalSizeChanged);
    }

    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }

    scheduleImageUpdate();

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
    Q_UNUSED(updatePaintNodeData);
    if (!window() || !m_svg) {
        delete oldNode;
        return nullptr;
    }

    //this is more than just an optimization, uploading a null image to QSGAtlasTexture causes a crash
    if (width() == 0.0 || height() == 0.0) {
        delete oldNode;
        return nullptr;
    }

    ManagedTextureNode *textureNode = static_cast<ManagedTextureNode *>(oldNode);
    if (!textureNode) {
        textureNode = new ManagedTextureNode;
        textureNode->setFiltering(QSGTexture::Linear);
        m_textureChanged = true;
    }

    //TODO use a heuristic to work out when to redraw
    //if !m_smooth and size is approximate simply change the textureNode.rect without
    //updating the material

    if (m_textureChanged || textureNode->texture()->textureSize() != QSize(width(), height())) {
        //despite having a valid size sometimes we still get a null QImage from Plasma::Svg
        //loading a null texture to an atlas fatals
        //Dave E fixed this in Qt in 5.3.something onwards but we need this for now
        if (m_image.isNull()) {
            delete textureNode;
            return nullptr;
        }

        QSharedPointer<QSGTexture> texture(window()->createTextureFromImage(m_image, QQuickWindow::TextureCanUseAtlas));
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
    scheduleImageUpdate();
}

void SvgItem::updateDevicePixelRatio()
{
    if (m_svg) {
        //devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
        //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
        if (window()) {
            m_svg.data()->setDevicePixelRatio(qMax<qreal>(1.0, floor(window()->devicePixelRatio())));
        } else {
            m_svg.data()->setDevicePixelRatio(qMax<qreal>(1.0, floor(qApp->devicePixelRatio())));
        }
        m_svg.data()->setScaleFactor(qMax<qreal>(1.0, floor(Units::instance().devicePixelRatio())));
    }
}

void SvgItem::scheduleImageUpdate()
{
    polish();
    update();
}

void SvgItem::updatePolish()
{
    QQuickItem::updatePolish();

    if (m_svg) {
        //setContainsMultipleImages has to be done there since m_frameSvg can be shared with somebody else
        m_textureChanged = true;
        m_svg.data()->setContainsMultipleImages(!m_elementID.isEmpty());
        m_image = m_svg.data()->image(QSize(width(), height()), m_elementID);
    }
}

void SvgItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size() && newGeometry.isValid()) {
        scheduleImageUpdate();
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

} // Plasma namespace

