/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
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

#include "iconitem.h"

#include <KIcon>
#include <KIconLoader>
#include <KIconEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

#include <Plasma/PaintUtils>
#include <Plasma/Svg>

IconItem::IconItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_svgIcon(0),
      m_smooth(false),
      m_active(false),
      m_animValue(0)
{
    m_animation = new QPropertyAnimation(this);
    connect(m_animation, SIGNAL(valueChanged(QVariant)),
            this, SLOT(valueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()),
            this, SLOT(animationFinished()));
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250);

    setFlag(QGraphicsItem::ItemHasNoContents, false);

    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SLOT(implicitWidthChanged()));
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SLOT(implicitHeightChanged()));


    //initialize implicit size to the Dialog size
    setImplicitWidth(KIconLoader::global()->currentSize(KIconLoader::Dialog));
    setImplicitHeight(KIconLoader::global()->currentSize(KIconLoader::Dialog));
}


IconItem::~IconItem()
{
}

void IconItem::setSource(const QVariant &source)
{
    if (source == m_source) {
        return;
    }

    m_source = source;

    if (source.canConvert<QIcon>()) {
        m_icon = source.value<QIcon>();
        m_imageIcon = QImage();
        m_pixmapIcon = QPixmap();
        delete m_svgIcon;
        m_svgIcon = 0;

    } else if (source.canConvert<QString>()) {
        m_svgIcon = new Plasma::Svg(this);
        //try as a svg toolbar icon
        m_svgIcon->setImagePath("toolbar-icons/" + source.toString().split("-").first());

        //try as a svg normal icon (like systray)
        if (!m_svgIcon->isValid()) {
            m_svgIcon->setImagePath("icons/" + source.toString().split("-").first());
        }
        m_svgIcon->setContainsMultipleImages(true);

        //success?
        if (m_svgIcon->isValid()) {
            m_icon = QIcon();

        //ok, svg not available
        } else {
            m_icon = KIcon(source.toString());
            delete m_svgIcon;
            m_svgIcon = 0;
        }

        m_imageIcon = QImage();
        m_pixmapIcon = QPixmap();

    } else if (source.canConvert<QPixmap>()) {
        m_icon = QIcon();
        m_imageIcon = QImage();
        m_pixmapIcon = source.value<QPixmap>();
        delete m_svgIcon;
        m_svgIcon = 0;

    } else if (source.canConvert<QImage>()) {
        m_icon = QIcon();
        m_imageIcon = source.value<QImage>();
        m_pixmapIcon = QPixmap();
        delete m_svgIcon;
        m_svgIcon = 0;

    } else {
        m_icon = QIcon();
        m_imageIcon = QImage();
        m_pixmapIcon = QPixmap();
        delete m_svgIcon;
        m_svgIcon = 0;
    }

    loadPixmap();

    emit sourceChanged();
    emit validChanged();
}

QVariant IconItem::source() const
{
    return m_source;
}

bool IconItem::isActive() const
{
    return m_active;
}

void IconItem::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    loadPixmap();
    emit activeChanged();
}

void IconItem::setImplicitWidth(qreal width)
{
    if (implicitWidth() == width) {
        return;
    }

    QDeclarativeItem::setImplicitWidth(width);

    emit implicitWidthChanged();
}

qreal IconItem::implicitWidth() const
{
    return QDeclarativeItem::implicitWidth();
}

void IconItem::setImplicitHeight(qreal height)
{
    if (implicitHeight() == height) {
        return;
    }

    QDeclarativeItem::setImplicitHeight(height);

    emit implicitHeightChanged();
}

qreal IconItem::implicitHeight() const
{
    return QDeclarativeItem::implicitHeight();
}

void IconItem::setSmooth(const bool smooth)
{
    if (smooth == m_smooth) {
        return;
    }
    m_smooth = smooth;
    update();
}

bool IconItem::smooth() const
{
    return m_smooth;
}

bool IconItem::isValid() const
{
    return !m_iconPixmaps.isEmpty();
}

void IconItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_iconPixmaps.isEmpty()) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_smooth);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_smooth);

    const QRect destRect(QPointF(boundingRect().center() - QPointF(m_iconPixmaps.first().width()/2, m_iconPixmaps.first().height()/2)).toPoint(),
                         m_iconPixmaps.first().size());

    if (m_animation->state() == QAbstractAnimation::Running) {
        QPixmap result = m_iconPixmaps.first();
        result = Plasma::PaintUtils::transition(result,
                               m_iconPixmaps.last(), m_animValue);
        painter->drawPixmap(destRect, result);
    //simpler logic for just paint
    } else {
        painter->drawPixmap(destRect, m_iconPixmaps.first());
    }

    painter->restore();
}

void IconItem::animationFinished()
{
    while (m_iconPixmaps.count() > 1) {
        m_iconPixmaps.pop_front();
    }
}

void IconItem::valueChanged(const QVariant &value)
{
    m_animValue = value.toReal();
    update();
}

void IconItem::loadPixmap()
{
    int size = qMin(width(), height());


    //FIXME: Heuristic: allow 24x24 for icons/ that are in the systray(ugly)
    if (m_svgIcon && m_svgIcon->imagePath().contains("icons/") &&
        size > KIconLoader::SizeSmallMedium &&
        size < KIconLoader::SizeMedium) {
        size = 24;

    //if size is less than 16, leave as is
    } else if (size < KIconLoader::SizeSmall) {
        //do nothing
    } else if (size < KIconLoader::SizeSmallMedium) {
        size = KIconLoader::SizeSmall;
    } else if (size < KIconLoader::SizeMedium) {
        size = KIconLoader::SizeSmallMedium;
    } else if (size < KIconLoader::SizeLarge) {
        size = KIconLoader::SizeMedium;
    } else if (size < KIconLoader::SizeHuge) {
        size = KIconLoader::SizeLarge;
    //if size is more than 64, leave as is
    }



    //final pixmap to paint
    QPixmap result;
    if (m_svgIcon) {
        m_svgIcon->resize(size, size);
        result = m_svgIcon->pixmap(m_source.toString());
    } else if (!m_source.isNull()) {
        result = m_icon.pixmap(QSize(size, size));
    } else if (!m_pixmapIcon.isNull()) {
        result = m_pixmapIcon;
    } else if (!m_imageIcon.isNull()) {
        result = QPixmap::fromImage(m_imageIcon);
    } else {
        m_iconPixmaps.clear();
        return;
    }

    if (!isEnabled()) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::DisabledState);
    } else if (m_active) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
    }

    //this happen only when loadPixmap has been called when an anim is running
    while (m_iconPixmaps.count() > 1) {
        m_iconPixmaps.pop_front();
    }

    m_iconPixmaps << result;
    //if there is only one image, don't animate
    //if an animation was already running, immediate transition, to not overload
    if (m_animation->state() == QAbstractAnimation::Running) {
        m_animation->stop();
    } else if (m_iconPixmaps.count() > 1) {
        m_animation->setStartValue((qreal)0);
        m_animation->setEndValue((qreal)1);
        m_animation->start();
    }
    update();
}

void IconItem::geometryChanged(const QRectF &newGeometry,
                               const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_iconPixmaps.clear();
        loadPixmap();
    }

    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

#include "iconitem.moc"
