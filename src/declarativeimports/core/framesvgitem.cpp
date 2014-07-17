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

#include <plasma/private/framesvg_p.h>

#include "svgtexturenode.h"

#include <cmath> //floor()

namespace Plasma
{
class FrameItemNode : public SVGTextureNode
{
public:
    FrameItemNode(FrameSvgItem* frameSvg, FrameSvg::EnabledBorders borders, QSGNode* parent)
        : SVGTextureNode()
        , m_frameSvg(frameSvg)
        , m_border(borders)
        , m_lastParent(parent)
    {
        m_lastParent->appendChildNode(this);
        fetchPrefix();
    }

    void fetchPrefix()
    {
        QString elementId = m_frameSvg->actualPrefix() + FrameSvgPrivate::borderToElementId(m_border);

        FrameData* frameData = m_frameSvg->frameData();
        QSize someSize = m_frameSvg->sectionRect(frameData, m_border, m_frameSvg->frameSvg()->frameSize().toSize()).size();

        // Q_ASSERT(!someSize.isEmpty());

        //QImage image = m_frameSvg->frameSvg()->image(someSize, elementId);
        QImage image(someSize, QImage::Format_ARGB32_Premultiplied);
        QPainter p(&image);
        p.setCompositionMode(QPainter::CompositionMode_Source);

        p.drawPixmap(image.rect(), m_frameSvg->frameSvg()->framePixmap(), m_frameSvg->sectionRect(frameData, m_border, m_frameSvg->frameSvg()->frameSize().toSize()));

        p.end();

        setVisible(!image.isNull());
        if(!image.isNull()) {
            QSGTexture* texture = m_frameSvg->window()->createTextureFromImage(image);
            setTexture(texture);
        } else {
            qDebug() << "not painting " << elementId;
        }
    }

    void reposition()
    {
        FrameData* frameData = m_frameSvg->frameData();
        if (!frameData)
            return;

        setRect(m_frameSvg->sectionRect(frameData, m_border, QSize(m_frameSvg->width(), m_frameSvg->height())));
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
      m_sizeChanged(false),
      m_fastPath(true)
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

    bool hasOverlay = !actualPrefix().startsWith(QLatin1String("mask-")) && m_frameSvg->hasElement(actualPrefix() % "overlay");
    m_fastPath = !hasOverlay;
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

    if (1||m_fastPath) {
        if (m_textureChanged) {
            delete oldNode;
            oldNode = 0;
        }

        if (!oldNode) {
            oldNode = new QSGNode;

            new FrameItemNode(this, FrameSvg::NoBorder, oldNode);

            if (m_topHeight) {
                new FrameItemNode(this, FrameSvg::TopBorder, oldNode);

                if (m_leftWidth) {
                    new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::LeftBorder, oldNode);
                }
                if (m_rightWidth) {
                    new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::RightBorder, oldNode);
                }
            }

            if (m_bottomHeight) {
                new FrameItemNode(this, FrameSvg::BottomBorder, oldNode);

                if (m_leftWidth) {
                    new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::LeftBorder, oldNode);
                }
                if (m_rightWidth) {
                    new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::RightBorder, oldNode);
                }
            }

            if (m_leftWidth) {
                new FrameItemNode(this, FrameSvg::LeftBorder, oldNode);
            }
            if (m_rightWidth) {
                new FrameItemNode(this, FrameSvg::RightBorder, oldNode);
            }

            m_sizeChanged = true;
            m_textureChanged = false;
        }

        FrameData* frame = frameData();
        if (frame && m_sizeChanged)
        {
            for(int i = 0; i<oldNode->childCount(); ++i) {
                FrameItemNode* it = static_cast<FrameItemNode*>(oldNode->childAtIndex(i));
                it->reposition();
            }

            m_sizeChanged = false;
        } else if(!frame) {
            qWarning() << "no frame for" << imagePath() << prefix();
        }
    } else {
        SVGTextureNode *textureNode = dynamic_cast<SVGTextureNode *>(oldNode);
        if (!textureNode) {
            delete oldNode;
            textureNode = new SVGTextureNode;
            textureNode->setFiltering(QSGTexture::Nearest);
             //force updating the texture on our newly created node
            oldNode = textureNode;
        }

        if ((m_textureChanged || m_sizeChanged) || textureNode->texture()->textureSize() != m_frameSvg->size()) {
            QImage image = m_frameSvg->framePixmap().toImage();
            QSGTexture *texture = window()->createTextureFromImage(image);
            texture->setFiltering(QSGTexture::Nearest);
            textureNode->setTexture(texture);
            textureNode->setRect(0, 0, width(), height());

            m_textureChanged = false;
            m_sizeChanged = false;
        }
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

FrameData* FrameSvgItem::frameData() const
{
    //We need to do that prefix, otherwise we are fetching the requested prefix, which might be different
    return m_frameSvg->d->frames.value(actualPrefix());
}

QString FrameSvgItem::actualPrefix() const
{
    return m_frameSvg->d->prefix;
}

void FrameSvgItem::updateBorderSizes()
{
    m_leftWidth = m_frameSvg->elementSize(actualPrefix()%"left").width();
    m_topHeight = m_frameSvg->elementSize(actualPrefix()%"top").height();
    m_rightWidth = m_frameSvg->elementSize(actualPrefix()%"right").width();
    m_bottomHeight = m_frameSvg->elementSize(actualPrefix()%"bottom").height();
}

QRect FrameSvgItem::sectionRect(FrameData* frame, Plasma::FrameSvg::EnabledBorders borders, const QSize &size)
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
