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

#include <QDebug>
#include <QPaintEngine>
#include <QPainter>
#include <QPropertyAnimation>

#include <KIconLoader>
#include <KIconEffect>

#include <plasma/paintutils.h>
#include <plasma/svg.h>


QPixmap transition(const QPixmap &from, const QPixmap &to, qreal amount)
{
    if (from.isNull() && to.isNull()) {
        return from;
    }

    if (qFuzzyCompare(amount + 1, qreal(1.0))) {
        return from;
    }

    QRect startRect(from.rect());
    QRect targetRect(to.rect());
    QSize pixmapSize = startRect.size().expandedTo(targetRect.size());
    QRect toRect = QRect(QPoint(0,0), pixmapSize);
    targetRect.moveCenter(toRect.center());
    startRect.moveCenter(toRect.center());

    //paint to in the center of from
    QColor color;
    color.setAlphaF(amount);

    // If the native paint engine supports Porter/Duff compositing and CompositionMode_Plus
    QPaintEngine *paintEngine = from.paintEngine();
    if (paintEngine &&
        paintEngine->hasFeature(QPaintEngine::PorterDuff) &&
        paintEngine->hasFeature(QPaintEngine::BlendModes)) {
        QPixmap startPixmap(pixmapSize);
        startPixmap.fill(Qt::transparent);

        QPixmap targetPixmap(pixmapSize);
        targetPixmap.fill(Qt::transparent);

        QPainter p;
        p.begin(&targetPixmap);
        p.drawPixmap(targetRect, to);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(targetRect, color);
        p.end();

        p.begin(&startPixmap);
        p.drawPixmap(startRect, from);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(startRect, color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(targetRect, targetPixmap);
        p.end();

        return startPixmap;
    }
#warning Cannot use XRender with QPixmap anymore. Find equivalent with Qt API.
#if 0 // HAVE_X11 && defined(HAVE_XRENDER)
    // We have Xrender support
    else if (paintEngine && paintEngine->hasFeature(QPaintEngine::PorterDuff)) {
        // QX11PaintEngine doesn't implement CompositionMode_Plus in Qt 4.3,
        // which we need to be able to do a transition from one pixmap to
        // another.
        //
        // In order to avoid the overhead of converting the pixmaps to images
        // and doing the operation entirely in software, this function has a
        // specialized path for X11 that uses Xrender directly to do the
        // transition. This operation can be fully accelerated in HW.
        //
        // This specialization can be removed when QX11PaintEngine supports
        // CompositionMode_Plus.
        QPixmap source(targetPixmap), destination(startPixmap);

        source.detach();
        destination.detach();

        Display *dpy = QX11Info::display();

        XRenderPictFormat *format = XRenderFindStandardFormat(dpy, PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = 1; // RepeatNormal

        // Create a 1x1 8 bit repeating alpha picture
        Pixmap pixmap = XCreatePixmap(dpy, destination.handle(), 1, 1, 8);
        Picture alpha = XRenderCreatePicture(dpy, pixmap, format, CPRepeat, &pa);
        XFreePixmap(dpy, pixmap);

        // Fill the alpha picture with the opacity value
        XRenderColor xcolor;
        xcolor.alpha = quint16(0xffff * amount);
        XRenderFillRectangle(dpy, PictOpSrc, alpha, &xcolor, 0, 0, 1, 1);

        // Reduce the alpha of the destination with 1 - opacity
        XRenderComposite(dpy, PictOpOutReverse, alpha, None, destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        // Add source * opacity to the destination
        XRenderComposite(dpy, PictOpAdd, source.x11PictureHandle(), alpha,
                         destination.x11PictureHandle(),
                         toRect.x(), toRect.y(), 0, 0, 0, 0, destination.width(), destination.height());

        XRenderFreePicture(dpy, alpha);
        return destination;
    }
#endif
    else {
        // Fall back to using QRasterPaintEngine to do the transition.
        QImage under(pixmapSize, QImage::Format_ARGB32_Premultiplied);
        under.fill(Qt::transparent);
        QImage over(pixmapSize, QImage::Format_ARGB32_Premultiplied);
        over.fill(Qt::transparent);

        QPainter p;
        p.begin(&over);
        p.drawPixmap(targetRect, to);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.drawPixmap(startRect, from);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(startRect, color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(toRect.topLeft(), over);
        p.end();

        return QPixmap::fromImage(under);
    }
}

IconItem::IconItem(QQuickItem *parent)
    : QQuickPaintedItem(parent),
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

    setFlag(ItemHasContents, true);

    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitWidthChanged()));
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()),
            this, SIGNAL(implicitHeightChanged()));


    connect(this, SIGNAL(enabledChanged()),
            this, SLOT(loadPixmap()));

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
        if (!m_svgIcon) {
            m_svgIcon = new Plasma::Svg(this);
        }
        //try as a svg toolbar icon
        m_svgIcon->setImagePath("toolbar-icons/" + source.toString().split("-").first());

        //try as a svg normal icon (like systray)
        if (!m_svgIcon->isValid() || !m_svgIcon->hasElement(m_source.toString())) {
            m_svgIcon->setImagePath("icons/" + source.toString().split("-").first());
        }
        m_svgIcon->setContainsMultipleImages(true);

        //success?
        if (m_svgIcon->isValid() && m_svgIcon->hasElement(m_source.toString())) {
            m_icon = QIcon();

        //ok, svg not available
        } else {
            m_icon = QIcon::fromTheme(source.toString());
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

    if (width() > 0 && height() > 0) {
        loadPixmap();
    }

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

void IconItem::paint(QPainter *painter)
{
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
        result = transition(result,
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
    if (size<=0) {
        //m_iconPixmaps.clear();
        m_animation->stop();
        update();
        return;
    } else if (m_svgIcon) {
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
        m_animation->stop();
        update();
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
        m_iconPixmaps.pop_front();
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
        if (newGeometry.width() > 0 && newGeometry.height() > 0) {
            loadPixmap();
        }

        QQuickItem::geometryChanged(newGeometry, oldGeometry);
    }
}

#include "iconitem.moc"
