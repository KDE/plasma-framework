/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svgitem.h"

#include <QDebug>
#include <QQuickWindow>
#include <QRectF>
#include <QSGTexture>

#include "plasma/svg.h"

#include <QuickAddons/ManagedTextureNode>

#include <cmath> //floor()

namespace Plasma
{
SvgItem::SvgItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_textureChanged(false)
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
    Q_EMIT elementIdChanged();
    Q_EMIT naturalSizeChanged();

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

    Q_EMIT svgChanged();
    Q_EMIT naturalSizeChanged();
}

Plasma::Svg *SvgItem::svg() const
{
    return m_svg.data();
}

QSGNode *SvgItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData);
    if (!window() || !m_svg) {
        delete oldNode;
        return nullptr;
    }

    // this is more than just an optimization, uploading a null image to QSGAtlasTexture causes a crash
    if (width() == 0.0 || height() == 0.0) {
        delete oldNode;
        return nullptr;
    }

    ManagedTextureNode *textureNode = static_cast<ManagedTextureNode *>(oldNode);
    if (!textureNode) {
        textureNode = new ManagedTextureNode;
        m_textureChanged = true;
    }

    // TODO use a heuristic to work out when to redraw
    // if !m_smooth and size is approximate simply change the textureNode.rect without
    // updating the material

    if (m_textureChanged || textureNode->texture()->textureSize() != QSize(width(), height())) {
        // despite having a valid size sometimes we still get a null QImage from Plasma::Svg
        // loading a null texture to an atlas fatals
        // Dave E fixed this in Qt in 5.3.something onwards but we need this for now
        if (m_image.isNull()) {
            delete textureNode;
            return nullptr;
        }

        QSharedPointer<QSGTexture> texture(window()->createTextureFromImage(m_image, QQuickWindow::TextureCanUseAtlas));
        textureNode->setTexture(texture);
        m_textureChanged = false;

        textureNode->setRect(0, 0, width(), height());
    }

    textureNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);

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
        // devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
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
        // setContainsMultipleImages has to be done there since m_frameSvg can be shared with somebody else
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
