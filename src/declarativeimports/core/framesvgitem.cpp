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

#include "framesvgitem.h"

#include <QQuickWindow>
#include <QSGTexture>
#include <QDebug>
#include <QPainter>
#include <QSGGeometry>

#include <plasma/private/framesvg_p.h>

#include "svgtexturenode.h"

#include <cmath> //floor()

namespace Plasma
{
class FrameItemNode : public SVGTextureNode
{
public:
    enum FitMode {
        Stretch,
        Tile
    };

    FrameItemNode(FrameSvgItem* frameSvg, FrameSvg::EnabledBorders borders, FitMode fitMode, QSGNode* parent)
        : SVGTextureNode()
        , m_frameSvg(frameSvg)
        , m_border(borders)
        , m_lastParent(parent)
        , m_fitMode(fitMode)
    {
        m_lastParent->appendChildNode(this);
        fetchPrefix();
    }

    void fetchPrefix()
    {
        //QString elementId = m_frameSvg->actualPrefix() + FrameSvgPrivate::borderToElementId(m_border);

        m_elementSize = m_frameSvg->sectionRect(m_border, m_frameSvg->frameSvg()->frameSize().toSize()).size();

        //FIXME: we should be sure the texture has a valid size at this point
        // Q_ASSERT(!m_elementSize.isEmpty());

        //QImage image = m_frameSvg->frameSvg()->image(m_elementSize, elementId);
        QImage image(m_elementSize, QImage::Format_ARGB32_Premultiplied);
        QPainter p(&image);
        p.setCompositionMode(QPainter::CompositionMode_Source);

        p.drawPixmap(image.rect(), m_frameSvg->frameSvg()->framePixmap(), m_frameSvg->sectionRect(m_border, m_frameSvg->frameSvg()->frameSize().toSize()));

        p.end();

        setVisible(!image.isNull());
        if(!image.isNull()) {
            QSGTexture* texture = m_frameSvg->window()->createTextureFromImage(image);
            setTexture(texture);
        } else {
            qWarning() << "this should never happen";
            //qDebug() << "not painting " << elementId;
        }

        if (m_fitMode == Tile) {
            if (m_border == FrameSvg::TopBorder || m_border == FrameSvg::BottomBorder || m_border == FrameSvg::NoBorder) {
                static_cast<QSGTextureMaterial*>(material())->setHorizontalWrapMode(QSGTexture::Repeat);
                static_cast<QSGOpaqueTextureMaterial*>(opaqueMaterial())->setHorizontalWrapMode(QSGTexture::Repeat);
            }

            if (m_border == FrameSvg::LeftBorder || m_border == FrameSvg::RightBorder || m_border == FrameSvg::NoBorder) {
                static_cast<QSGTextureMaterial*>(material())->setVerticalWrapMode(QSGTexture::Repeat);
                static_cast<QSGOpaqueTextureMaterial*>(opaqueMaterial())->setVerticalWrapMode(QSGTexture::Repeat);
            }
        }
    }

    void reposition()
    {
        setRect(m_frameSvg->sectionRect(m_border, QSize(m_frameSvg->width(), m_frameSvg->height())));

        QRectF frameRect = m_frameSvg->sectionRect(m_border, QSize(m_frameSvg->width(), m_frameSvg->height()));
        QRectF textureRect = QRectF(0,0,1,1);
        if (m_fitMode == Tile) {
            if (m_border == FrameSvg::TopBorder || m_border == FrameSvg::BottomBorder || m_border == FrameSvg::NoBorder) {
                textureRect.setWidth(frameRect.width() / m_elementSize.width());
            }
            if (m_border == FrameSvg::LeftBorder || m_border == FrameSvg::RightBorder || m_border == FrameSvg::NoBorder) {
                textureRect.setHeight(frameRect.height() / m_elementSize.height());
            }
        }
        QSGGeometry::updateTexturedRectGeometry(geometry(), frameRect, textureRect);
    }

    void setVisible(bool visible)
    {
        if (visible == bool(parent()))
            return;

        if (visible) {
            m_lastParent->appendChildNode(this);
        } else {
            m_lastParent->removeChildNode(this);
        }
    }

private:
    FrameSvgItem* m_frameSvg;
    FrameSvg::EnabledBorders m_border;
    QSGNode *m_lastParent;
    QSize m_elementSize;
    FitMode m_fitMode;
};


FrameSvgItemMargins::FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent)
    : QObject(parent),
      m_frameSvg(frameSvg),
      m_fixed(false)
{
    //qDebug() << "margins at: " << left() << top() << right() << bottom();
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

qreal FrameSvgItemMargins::left() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::LeftMargin);
    } else {
        return m_frameSvg->marginSize(Types::LeftMargin);
    }
}

qreal FrameSvgItemMargins::top() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::TopMargin);
    } else {
        return m_frameSvg->marginSize(Types::TopMargin);
    }
}

qreal FrameSvgItemMargins::right() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::RightMargin);
    } else {
        return m_frameSvg->marginSize(Types::RightMargin);
    }
}

qreal FrameSvgItemMargins::bottom() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::BottomMargin);
    } else {
        return m_frameSvg->marginSize(Types::BottomMargin);
    }
}

void FrameSvgItemMargins::update()
{
    emit marginsChanged();
}

void FrameSvgItemMargins::setFixed(bool fixed)
{
    if (fixed == m_fixed) {
        return;
    }

    m_fixed = fixed;
    emit marginsChanged();
}

bool FrameSvgItemMargins::isFixed() const
{
    return m_fixed;
}

FrameSvgItem::FrameSvgItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_textureChanged(false),
      m_sizeChanged(false)
{
    m_updateTexTimer.setSingleShot(true);
    m_updateTexTimer.setInterval(500);
    connect(&m_updateTexTimer, &QTimer::timeout, [=] {
        qDebug()<<"Updating the texture of" << m_frameSvg->imagePath();
        m_frameSvg->resizeFrame(QSize(width(), height()));
        doUpdate();
    });

    m_frameSvg = new Plasma::FrameSvg(this);
    m_margins = new FrameSvgItemMargins(m_frameSvg, this);
    m_fixedMargins = new FrameSvgItemMargins(m_frameSvg, this);
    m_fixedMargins->setFixed(true);
    setFlag(ItemHasContents, true);
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(doUpdate()));
    connect(&m_units, &Units::devicePixelRatioChanged, this, &FrameSvgItem::updateDevicePixelRatio);
}

FrameSvgItem::~FrameSvgItem()
{
}

void FrameSvgItem::setImagePath(const QString &path)
{
    if (m_frameSvg->imagePath() == path) {
        return;
    }

    m_frameSvg->setImagePath(path);
    m_frameSvg->setElementPrefix(m_prefix);
    updateDevicePixelRatio();

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
        updateBorderSizes();
        m_textureChanged = true;
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
        updateBorderSizes();
        m_textureChanged = true;
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

FrameSvgItemMargins *FrameSvgItem::fixedMargins() const
{
    return m_fixedMargins;
}

void FrameSvgItem::setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders)
{
    if (m_frameSvg->enabledBorders() == borders) {
        return;
    }

    m_frameSvg->setEnabledBorders(borders);
    emit enabledBordersChanged();
    updateBorderSizes();
    m_textureChanged = true;
    update();
}

Plasma::FrameSvg::EnabledBorders FrameSvgItem::enabledBorders() const
{
    return m_frameSvg->enabledBorders();
}

bool FrameSvgItem::hasElementPrefix(const QString &prefix) const
{
    return m_frameSvg->hasElementPrefix(prefix);
}

void FrameSvgItem::geometryChanged(const QRectF &newGeometry,
                                   const QRectF &oldGeometry)
{
    if (isComponentComplete()) {
        //m_frameSvg->resizeFrame(newGeometry.size());
        m_updateTexTimer.start();
        m_sizeChanged = true;
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

    m_textureChanged = true;
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

QSGNode *FrameSvgItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (!window() || !m_frameSvg || !m_frameSvg->hasElementPrefix(m_prefix)) {
        delete oldNode;
        return Q_NULLPTR;
    }

    if (m_textureChanged) {
        delete oldNode;
        oldNode = 0;
    }

    if (!oldNode) {
        oldNode = new QSGNode;

        const QString actualPrefix = !m_frameSvg->prefix().isEmpty() && m_frameSvg->hasElementPrefix(m_frameSvg->prefix()) ? m_frameSvg->prefix() % "-" : QString();


        FrameItemNode::FitMode borderFitMode = (m_frameSvg->hasElement("hint-stretch-borders") || m_frameSvg->hasElement(actualPrefix % "hint-stretch-borders")) ? FrameItemNode::Stretch : FrameItemNode::Tile;
        FrameItemNode::FitMode centerFitMode = (m_frameSvg->hasElement("hint-tile-center") || m_frameSvg->hasElement(actualPrefix % "hint-tile-center")) ? FrameItemNode::Tile: FrameItemNode::Stretch;

        new FrameItemNode(this, FrameSvg::NoBorder, centerFitMode, oldNode);

        if (m_topHeight) {
            new FrameItemNode(this, FrameSvg::TopBorder, borderFitMode, oldNode);

            if (m_leftWidth) {
                new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::LeftBorder, FrameItemNode::Stretch, oldNode);
            }
            if (m_rightWidth) {
                new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::RightBorder, FrameItemNode::Stretch, oldNode);
            }
        }

        if (m_bottomHeight) {
            new FrameItemNode(this, FrameSvg::BottomBorder, borderFitMode, oldNode);

            if (m_leftWidth) {
                new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::LeftBorder, FrameItemNode::Stretch, oldNode);
            }
            if (m_rightWidth) {
                new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::RightBorder, FrameItemNode::Stretch, oldNode);
            }
        }

        if (m_leftWidth) {
            new FrameItemNode(this, FrameSvg::LeftBorder, borderFitMode, oldNode);
        }
        if (m_rightWidth) {
            new FrameItemNode(this, FrameSvg::RightBorder, borderFitMode, oldNode);
        }

        m_sizeChanged = true;
        m_textureChanged = false;
    }

    if (m_sizeChanged) {
        for(int i = 0; i<oldNode->childCount(); ++i) {
            FrameItemNode* it = static_cast<FrameItemNode*>(oldNode->childAtIndex(i));
            it->reposition();
        }

        m_sizeChanged = false;
    }

    return oldNode;
}

void FrameSvgItem::componentComplete()
{
    QQuickItem::componentComplete();
    m_frameSvg->resizeFrame(QSize(width(), height()));
    updateBorderSizes();
    m_textureChanged = true;
}

void FrameSvgItem::updateDevicePixelRatio()
{
    //devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
    //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
    m_frameSvg->setDevicePixelRatio(qMax<qreal>(1.0, floor(m_units.devicePixelRatio())));
    updateBorderSizes();
    m_textureChanged = true;
}

void FrameSvgItem::updateBorderSizes()
{
    const QString actualPrefix = !m_frameSvg->prefix().isEmpty() && m_frameSvg->hasElementPrefix(m_frameSvg->prefix()) ? m_frameSvg->prefix() % "-" : QString();

    m_leftWidth = m_frameSvg->elementSize(actualPrefix % "left").width();
    m_topHeight = m_frameSvg->elementSize(actualPrefix % "top").height();
    m_rightWidth = m_frameSvg->elementSize(actualPrefix % "right").width();
    m_bottomHeight = m_frameSvg->elementSize(actualPrefix % "bottom").height();
}

QRect FrameSvgItem::sectionRect(Plasma::FrameSvg::EnabledBorders borders, const QSize &size)
{
    QRect contentRect = QRect(QPoint(0, 0), size).adjusted(m_leftWidth, m_topHeight, -m_rightWidth, -m_bottomHeight);
    updateBorderSizes();

    switch(borders) {
        case FrameSvg::NoBorder:
            return contentRect;
        case FrameSvg::TopBorder:
            return QRect(QPoint(contentRect.left(), 0), QSize(contentRect.width(), m_topHeight));
        case FrameSvg::BottomBorder:
            return QRect(contentRect.bottomLeft()+QPoint(0, 1), QSize(contentRect.width(), m_bottomHeight));
        case FrameSvg::LeftBorder:
            return QRect(QPoint(0, contentRect.top()), QSize(m_leftWidth, contentRect.height()));
        case FrameSvg::RightBorder:
            return QRect(contentRect.topRight()+QPoint(1,0), QSize(m_rightWidth, contentRect.height()));
        case FrameSvg::TopBorder | FrameSvg::LeftBorder:
            return QRect(QPoint(0, 0), QSize(m_leftWidth, m_topHeight));
        case FrameSvg::TopBorder | FrameSvg::RightBorder:
            return QRect(QPoint(contentRect.right()+1, 0), QSize(m_rightWidth, m_topHeight));
        case FrameSvg::BottomBorder | FrameSvg::LeftBorder:
            return QRect(QPoint(0, contentRect.bottom()+1), QSize(m_leftWidth, m_bottomHeight));
        case FrameSvg::BottomBorder | FrameSvg::RightBorder:
            return QRect(contentRect.bottomRight()+QPoint(1,1), QSize(m_rightWidth, m_bottomHeight));
        default:
            qWarning() << "unrecognized border" << borders;
            return QRect();
    }
    return QRect();
}

} // Plasma namespace

#include "framesvgitem.moc"
