/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmudnson@kde.org>
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

#include <QDebug>
#include <QPaintEngine>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QPixmap>

#include <kiconloader.h>
#include <kiconeffect.h>

#include "fadingnode_p.h"
#include <QuickAddons/ManagedTextureNode>
#include "units.h"

IconItem::IconItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_svgIcon(0),
      m_smooth(false),
      m_active(false),
      m_colorGroup(Plasma::Theme::NormalColorGroup),
      m_animValue(0)
{
    m_loadPixmapTimer.setSingleShot(true);
    m_loadPixmapTimer.setInterval(150);
    connect(&m_loadPixmapTimer, &QTimer::timeout, this, &IconItem::loadPixmap);

    m_animation = new QPropertyAnimation(this);
    connect(m_animation, SIGNAL(valueChanged(QVariant)),
            this, SLOT(valueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()),
            this, SLOT(animationFinished()));
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250); //FIXME from theme

    setFlag(ItemHasContents, true);

    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitWidthChanged()));
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitHeightChanged()));

    connect(this, SIGNAL(enabledChanged()),
            &m_loadPixmapTimer, SLOT(start()));

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

    const bool oldValid = isValid();

    if (source.canConvert<QIcon>()) {
        m_icon = source.value<QIcon>();
        m_imageIcon = QImage();
        m_pixmapIcon = QPixmap();
        delete m_svgIcon;
        m_svgIcon = 0;

    } else if (source.canConvert<QString>()) {
        if (source.toString().isEmpty()) {
            delete m_svgIcon;
            m_svgIcon = 0;
            m_icon = QIcon();
            emit validChanged();
            loadPixmap();
            return;
        }

        //If a url in the form file:// is passed, take the image pointed by that from disk
        QUrl url = QUrl(source.toString());
        if (url.isLocalFile()) {
            m_icon = QIcon();
            m_imageIcon = QImage(url.path());
            m_pixmapIcon = QPixmap();
            delete m_svgIcon;
            m_svgIcon = 0;
        } else {
            if (!m_svgIcon) {
                m_svgIcon = new Plasma::Svg(this);
                m_svgIcon->setColorGroup(m_colorGroup);
            }
            //try as a svg icon
            m_svgIcon->setImagePath("icons/" + source.toString().split("-").first());

            m_svgIcon->setContainsMultipleImages(true);

            //success?
            if (m_svgIcon->isValid() && m_svgIcon->hasElement(m_source.toString())) {
                m_icon = QIcon();
                connect(m_svgIcon, SIGNAL(repaintNeeded()), this, SLOT(loadPixmap()));

                //ok, svg not available
            } else {
                m_icon = QIcon::fromTheme(source.toString());
                delete m_svgIcon;
                m_svgIcon = 0;
                m_imageIcon = QImage();
                m_pixmapIcon = QPixmap();

            }
        }

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

    if (width() > 0 && height() > 0) {
        if (!oldValid) {
            loadPixmap();
        } else {
            m_loadPixmapTimer.start();
        }
    }

    emit sourceChanged();
    emit validChanged();
}

QVariant IconItem::source() const
{
    return m_source;
}

void IconItem::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_colorGroup == group) {
        return;
    }

    m_colorGroup = group;

    if (m_svgIcon) {
        m_svgIcon->setColorGroup(group);
    }

    emit colorGroupChanged();
}

Plasma::Theme::ColorGroup IconItem::colorGroup() const
{
    return m_colorGroup;
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
    if (isComponentComplete()) {
        loadPixmap();
    }
    emit activeChanged();
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
    return !m_icon.isNull() || m_svgIcon || !m_pixmapIcon.isNull() || !m_imageIcon.isNull();
}

QSGNode* IconItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (m_iconPixmap.isNull() || width() == 0 || height() == 0) {
        delete oldNode;
        return Q_NULLPTR;
    }

    if (m_animation->state() == QAbstractAnimation::Running) {
        FadingNode *animatingNode = dynamic_cast<FadingNode*>(oldNode);

        if (!animatingNode || m_textureChanged) {
            delete oldNode;

            QSGTexture *source = window()->createTextureFromImage(m_iconPixmap.toImage());
            QSGTexture *target = window()->createTextureFromImage(m_oldIconPixmap.toImage());
            animatingNode = new FadingNode(source, target);
            m_sizeChanged = true;
            m_textureChanged = false;
        }

        animatingNode->setProgress(m_animValue);

        if (m_sizeChanged) {
            const int iconSize = Units::roundToIconSize(qMin(boundingRect().size().width(), boundingRect().size().height()));
            const QRect destRect(QPointF(boundingRect().center() - QPointF(iconSize/2, iconSize/2)).toPoint(),
                                 QSize(iconSize, iconSize));

            animatingNode->setRect(destRect);
            m_sizeChanged = false;
        }

        return animatingNode;
    } else {
        ManagedTextureNode *textureNode = dynamic_cast<ManagedTextureNode*>(oldNode);

        if (!textureNode || m_textureChanged) {
            delete oldNode;
            textureNode = new ManagedTextureNode;
            textureNode->setTexture(QSharedPointer<QSGTexture>(window()->createTextureFromImage(m_iconPixmap.toImage())));
            m_sizeChanged = true;
            m_textureChanged = false;
        }

        if (m_sizeChanged) {
            const int iconSize = Units::roundToIconSize(qMin(boundingRect().size().width(), boundingRect().size().height()));
            const QRect destRect(QPointF(boundingRect().center() - QPointF(iconSize/2, iconSize/2)).toPoint(),
                                 QSize(iconSize, iconSize));

            textureNode->setRect(destRect);
            m_sizeChanged = false;
        }
        return textureNode;
    }
}

void IconItem::valueChanged(const QVariant &value)
{
    m_animValue = value.toReal();
    update();
}

void IconItem::animationFinished()
{
    m_oldIconPixmap = QPixmap();
    m_textureChanged = true;
    update();
}

void IconItem::loadPixmap()
{
    if (!isComponentComplete()) {
        return;
    }

    const int size = Units::roundToIconSize(qMin(width(), height()));

    //final pixmap to paint
    QPixmap result;
    if (size <= 0) {
        m_iconPixmap = QPixmap();
        m_animation->stop();
        update();
        return;
    } else if (m_svgIcon) {
        m_svgIcon->resize(size, size);
        result = m_svgIcon->pixmap(m_source.toString());
    } else if (!m_icon.isNull()) {
        result = m_icon.pixmap(QSize(size, size));
    } else if (!m_pixmapIcon.isNull()) {
        result = m_pixmapIcon;
    } else if (!m_imageIcon.isNull()) {
        result = QPixmap::fromImage(m_imageIcon);
    } else {
        m_iconPixmap = QPixmap();
        m_animation->stop();
        update();
        return;
    }

    if (!isEnabled()) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::DisabledState);
    } else if (m_active) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
    }

    m_oldIconPixmap = m_iconPixmap;
    m_iconPixmap = result;
    m_textureChanged = true;

    //don't animate initial setting
    if (!m_oldIconPixmap.isNull() && !m_sizeChanged) {
        m_animation->setStartValue((qreal)0);
        m_animation->setEndValue((qreal)1);
        m_animation->start();
    } else {
        m_animValue = 1.0;
        m_animation->stop();
    }
    update();
}

void IconItem::geometryChanged(const QRectF &newGeometry,
                               const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_sizeChanged = true;
        update();
        if (newGeometry.width() > 0 && newGeometry.height() > 0) {
            if (!m_loadPixmapTimer.isActive() && isComponentComplete()) {
                m_loadPixmapTimer.start();
            }
        }
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void IconItem::componentComplete()
{
    QQuickItem::componentComplete();
    loadPixmap();
}

