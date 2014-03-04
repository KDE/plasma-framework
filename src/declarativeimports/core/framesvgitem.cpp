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

#include "QDebug"


namespace Plasma
{

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
    : QQuickPaintedItem(parent)
{
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

FrameSvgItemMargins *FrameSvgItem::fixedMargins() const
{
    return m_fixedMargins;
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

bool FrameSvgItem::hasElementPrefix(const QString &prefix) const
{
    return m_frameSvg->hasElementPrefix(prefix);
}

void FrameSvgItem::paint(QPainter *painter)
{
    m_frameSvg->paintFrame(painter);
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


void FrameSvgItem::updateDevicePixelRatio()
{
    //devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
    //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
    m_frameSvg->setDevicePixelRatio(qMax((qreal)1.0, floor(m_units.devicePixelRatio())));
}

} // Plasma namespace

#include "framesvgitem.moc"
