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

#include "framesvgitem.h"

#include <QPainter>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>

#include "QDebug"

enum FramePart {
    TopLeft,
    Top,
    TopRight,
    Left,
    Centre,
    Right,
    BotttomLeft,
    Bottom,
    BottomRight
};

namespace Plasma
{

FrameSvgItemMargins::FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent)
    : QObject(parent),
      m_frameSvg(frameSvg)
{
    //qDebug() << "margins at: " << left() << top() << right() << bottom();
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

qreal FrameSvgItemMargins::left() const
{
    return m_frameSvg->marginSize(Types::LeftMargin);
}

qreal FrameSvgItemMargins::top() const
{
    return m_frameSvg->marginSize(Types::TopMargin);
}

qreal FrameSvgItemMargins::right() const
{
    return m_frameSvg->marginSize(Types::RightMargin);
}

qreal FrameSvgItemMargins::bottom() const
{
    return m_frameSvg->marginSize(Types::BottomMargin);
}

void FrameSvgItemMargins::update()
{
    emit marginsChanged();
}

FrameSvgItem::FrameSvgItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_frameSvg = new Plasma::FrameSvg(this);
    m_frameSvg->setCacheAllRenderedFrames(false);
    m_margins = new FrameSvgItemMargins(m_frameSvg, this);
    setFlag(ItemHasContents, true);
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(doUpdate()));
    m_textures.fill(0, 9);
}


FrameSvgItem::~FrameSvgItem()
{
    qDeleteAll(m_textures);
}

void FrameSvgItem::setImagePath(const QString &path)
{
    if (m_frameSvg->imagePath() == path) {
        return;
    }

    m_frameSvg->setContainsMultipleImages(true);
    m_frameSvg->setImagePath(path);
    m_frameSvg->setElementPrefix(m_prefix);

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    emit imagePathChanged();
    m_margins->update();

    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        update();
    }
}

QString FrameSvgItem::imagePath() const
{
    return m_frameSvg->imagePath();
}


void FrameSvgItem::setPrefix(const QString &prefix)
{
    if (m_prefix == prefix) {
        return;
    }

    m_frameSvg->setElementPrefix(prefix);
    m_prefix = prefix;

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    emit prefixChanged();
    m_margins->update();

    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        update();
    }
}

QString FrameSvgItem::prefix() const
{
    return m_prefix;
}

FrameSvgItemMargins *FrameSvgItem::margins() const
{
    return m_margins;
}

void FrameSvgItem::setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders)
{
    if (m_frameSvg->enabledBorders() == borders)
        return;

    m_frameSvg->setEnabledBorders(borders);
    emit enabledBordersChanged();
    update();
}

Plasma::FrameSvg::EnabledBorders FrameSvgItem::enabledBorders() const
{
    return m_frameSvg->enabledBorders();
}

QSGNode* FrameSvgItem::updatePaintNode(QSGNode *centralNode, QQuickItem::UpdatePaintNodeData*)
{
    if (!window() || !m_frameSvg) {
        delete centralNode;
        return 0;
    }

    if (!centralNode) {
        centralNode = new QSGNode;
    }

    if (centralNode->childCount() != 9) {
        for (int i=0; i < 9 ; i++) {
            QSGSimpleTextureNode *textureNode = new QSGSimpleTextureNode;
            centralNode->appendChildNode(textureNode);
        }
    }


    const int topWidth = m_frameSvg->elementSize(m_prefix % "top").width();
    const int topHeight = m_frameSvg->elementSize(m_prefix % "top").height();
    const int leftWidth = m_frameSvg->elementSize(m_prefix % "left").width();
    const int leftHeight = m_frameSvg->elementSize(m_prefix % "left").height();
    const int rightWidth = m_frameSvg->elementSize(m_prefix % "right").width();
    const int rightHeight = m_frameSvg->elementSize(m_prefix % "right").height();
    const int bottomWidth = m_frameSvg->elementSize(m_prefix % "bottom").width();
    const int bottomHeight = m_frameSvg->elementSize(m_prefix % "bottom").height();

    const int topOffset = 0;
    const int leftOffset = 0;
    const int contentWidth = width() - leftWidth - rightWidth;
    const int contentHeight = height() - topHeight - bottomHeight;
    int contentTop = 0;
    int contentLeft = 0;
    int rightOffset = contentWidth + leftWidth;
    int bottomOffset = contentHeight;

    //TODO
    // borders
    // avoid repainting textures
    // render images at the right size to begin with
    // tiling
    // compose over borders


    //corners
    //if (enabledBorders...)
    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(TopLeft));

        //TODO only do this is texture is dirty, and cache the texture object
        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "topleft"));
        textureNode->setTexture(texture);

        textureNode->setRect(leftOffset, topOffset , leftWidth, topHeight);

        contentLeft = leftWidth;
        contentTop = topHeight;
        bottomOffset += topHeight;

    }

    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(TopRight));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "topright"));
        textureNode->setTexture(texture);

        textureNode->setRect(rightOffset, topOffset , rightWidth, topHeight);

        contentLeft = leftWidth;
        contentTop = topHeight;
    }

    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(BottomLeft));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "bottomleft"));
        textureNode->setTexture(texture);

        textureNode->setRect(leftOffset, bottomOffset , rightWidth, bottomHeight);

        contentLeft = leftWidth;
    }

    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(BottomRight));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "bottomright"));
        textureNode->setTexture(texture);

        textureNode->setRect(rightOffset, bottomOffset , rightWidth, bottomHeight);

        contentLeft = leftWidth;
    }



    // Sides
    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(Left));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "left"));

        textureNode->setTexture(texture);
        textureNode->setRect(leftOffset, contentTop, leftWidth, contentHeight);
    }

        {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(Top));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "top"));

        textureNode->setTexture(texture);
        textureNode->setRect(contentLeft, topOffset , contentWidth, topHeight);
    }

    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(Right));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "right"));

        textureNode->setTexture(texture);
        textureNode->setRect(rightOffset, contentTop , rightWidth, contentHeight);
    }

    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(Bottom));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "bottom"));

        textureNode->setTexture(texture);
        textureNode->setRect(contentLeft, bottomOffset , contentWidth, bottomHeight);
    }

    //center
    {
        QSGSimpleTextureNode *textureNode =  static_cast<QSGSimpleTextureNode*>(centralNode->childAtIndex(Center));

        QSGTexture *texture = window()->createTextureFromImage(m_frameSvg->image(m_prefix % "center"));

        textureNode->setTexture(texture);
        textureNode->setRect(leftWidth, topHeight, contentWidth, contentHeight);
    }



    return centralNode;
}

void FrameSvgItem::geometryChanged(const QRectF &newGeometry,
                                          const QRectF &oldGeometry)
{
    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(newGeometry.size());
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void FrameSvgItem::doUpdate()
{
    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    update();
}

void FrameSvgItem::setImplicitWidth(qreal width)
{
    if (implicitWidth() == width) {
        return;
    }

    QQuickItem::setImplicitWidth(width);

    emit implicitWidthChanged();
}

qreal FrameSvgItem::implicitWidth() const
{
    return QQuickItem::implicitWidth();
}

void FrameSvgItem::setImplicitHeight(qreal height)
{
    if (implicitHeight() == height) {
        return;
    }

    QQuickItem::setImplicitHeight(height);

    emit implicitHeightChanged();
}

qreal FrameSvgItem::implicitHeight() const
{
    return QQuickItem::implicitHeight();
}

Plasma::FrameSvg *FrameSvgItem::frameSvg() const
{
    return m_frameSvg;
}

void FrameSvgItem::componentComplete()
{
    QQuickItem::componentComplete();
    m_frameSvg->resizeFrame(QSize(width(), height()));
}

} // Plasma namespace

#include "framesvgitem.moc"
