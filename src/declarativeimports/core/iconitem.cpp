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

#include <kiconloader.h>
#include <kiconeffect.h>
#include <KIconTheme>

#include "fadingnode_p.h"
#include <QuickAddons/ManagedTextureNode>
#include "units.h"

IconItem::IconItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_svgIcon(nullptr),
      m_status(Plasma::Svg::Normal),
      m_smooth(true),
      m_active(false),
      m_animated(true),
      m_usesPlasmaTheme(true),
      m_roundToIconSize(true),
      m_textureChanged(false),
      m_sizeChanged(false),
      m_allowNextAnimation(false),
      m_blockNextAnimation(false),
      m_implicitHeightSetByUser(false),
      m_implicitWidthSetByUser(false),
      m_colorGroup(Plasma::Theme::NormalColorGroup),
      m_animValue(0)
{
    m_animation = new QPropertyAnimation(this);
    connect(m_animation, &QPropertyAnimation::valueChanged,
            this, &IconItem::valueChanged);
    connect(m_animation, &QPropertyAnimation::finished,
            this, &IconItem::animationFinished);
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250); //FIXME from theme

    setFlag(ItemHasContents, true);

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged,
            this, &IconItem::updateImplicitSize);

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    connect(this, &QQuickItem::enabledChanged,
            this, &IconItem::onEnabledChanged);
#endif

    connect(this, &IconItem::implicitWidthChanged, this, &IconItem::implicitWidthChanged2);
    connect(this, &IconItem::implicitHeightChanged, this, &IconItem::implicitHeightChanged2);

    updateImplicitSize();
}

IconItem::~IconItem()
{
}

void IconItem::updateImplicitSize()
{
    if (!m_imageIcon.isNull()) {
        const QSize &s = m_imageIcon.size();

        if (s.isValid()) {
            if (!m_implicitWidthSetByUser && !m_implicitHeightSetByUser) {
                setImplicitSize(s.width(), s.height());
            } else if (!m_implicitWidthSetByUser) {
                setImplicitWidth(s.width());
            } else if (!m_implicitHeightSetByUser) {
                setImplicitHeight(s.height());
            }

            return;
        }
    } else if (m_svgIcon) { // FIXME: Check Svg::isValid()? Considered expensive by apidox.
        //resize() resets the icon to its implicit size, specified
        m_svgIcon->resize();
        QSize s;
        const QString &sourceString = m_source.toString();
        //plasma theme icon, where one file contains multiple images
        if (m_svgIcon->hasElement(sourceString)) {
            s = m_svgIcon->elementSize(sourceString);
        //normal icon: one image per file, page size is icon size
        } else {
            s = m_svgIcon->size();
        }
        if (s.isValid()) {
            if (!m_implicitWidthSetByUser && !m_implicitHeightSetByUser) {
                setImplicitSize(s.width(), s.height());
            } else if (!m_implicitWidthSetByUser) {
                setImplicitWidth(s.width());
            } else if (!m_implicitHeightSetByUser) {
                setImplicitHeight(s.height());
            }

            return;
        }
    }

    // Fall back to initializing implicit size to the Dialog size.
    const int implicitSize = KIconLoader::global()->currentSize(KIconLoader::Dialog);

    if (!m_implicitWidthSetByUser && !m_implicitHeightSetByUser) {
        setImplicitSize(implicitSize, implicitSize);
    } else if (!m_implicitWidthSetByUser) {
        setImplicitWidth(implicitSize);
    } else if (!m_implicitHeightSetByUser) {
        setImplicitHeight(implicitSize);
    }
}

void IconItem::setSource(const QVariant &source)
{
    if (source == m_source) {
        return;
    }

    const bool oldValid = isValid();

    m_source = source;
    QString sourceString = source.toString();

    // If the QIcon was created with QIcon::fromTheme(), try to load it as svg
    if (source.canConvert<QIcon>() && !source.value<QIcon>().name().isEmpty()) {
        sourceString = source.value<QIcon>().name();
    }

    if (!sourceString.isEmpty()) {
        // If a file:// URL or a absolute path is passed, take the image pointed by that from disk
        QString localFile;
        if (sourceString.startsWith(QLatin1String("file:"))) {
            localFile = QUrl(sourceString).toLocalFile();
        } else if (sourceString.startsWith(QLatin1Char('/'))) {
            localFile = sourceString;
        }

        if (!localFile.isEmpty()) {
            m_icon = QIcon();
            m_imageIcon = QImage(localFile);
            m_svgIconName.clear();
            delete m_svgIcon;
            m_svgIcon = nullptr;
        } else {
            if (!m_svgIcon) {
                m_svgIcon = new Plasma::Svg(this);
                m_svgIcon->setColorGroup(m_colorGroup);
                m_svgIcon->setStatus(m_status);
                m_svgIcon->setDevicePixelRatio((window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));
                connect(m_svgIcon, &Plasma::Svg::repaintNeeded, this, &IconItem::schedulePixmapUpdate);
            }

            if (m_usesPlasmaTheme) {
                //try as a svg icon from plasma theme
                m_svgIcon->setImagePath(QLatin1String("icons/") + sourceString.section(QLatin1Char('-'), 0, 0));
                m_svgIcon->setContainsMultipleImages(true);
            }

            //success?
            if (m_usesPlasmaTheme && m_svgIcon->isValid() && m_svgIcon->hasElement(sourceString)) {
                m_icon = QIcon();
                m_svgIconName = sourceString;
                //ok, svg not available from the plasma theme
            } else {
                //try to load from iconloader an svg with Plasma::Svg
                const auto *iconTheme = KIconLoader::global()->theme();
                QString iconPath;
                if (iconTheme) {
                    iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svg"), qMin(width(), height()), KIconLoader::MatchBest);
                    if (iconPath.isEmpty()) {
                        iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svgz"), qMin(width(), height()), KIconLoader::MatchBest);
                    }
                } else {
                    qWarning() << "KIconLoader has no theme set";
                }

                if (!iconPath.isEmpty()) {
                    m_svgIcon->setImagePath(iconPath);
                    m_svgIconName = sourceString;
                //fail, use QIcon
                } else {
                    //if we started with a QIcon use that.
                    m_icon = source.value<QIcon>();
                    if (m_icon.isNull()) {
                        m_icon = QIcon::fromTheme(sourceString);
                    }
                    m_svgIconName.clear();
                    delete m_svgIcon;
                    m_svgIcon = nullptr;
                    m_imageIcon = QImage();
                }
            }
        }

    } else if (source.canConvert<QIcon>()) {
        m_icon = source.value<QIcon>();
        m_imageIcon = QImage();
        m_svgIconName.clear();
        delete m_svgIcon;
        m_svgIcon = nullptr;
    } else if (source.canConvert<QImage>()) {
        m_icon = QIcon();
        m_imageIcon = source.value<QImage>();
        m_svgIconName.clear();
        delete m_svgIcon;
        m_svgIcon = nullptr;
    } else {
        m_icon = QIcon();
        m_imageIcon = QImage();
        m_svgIconName.clear();
        delete m_svgIcon;
        m_svgIcon = nullptr;
    }

    if (width() > 0 && height() > 0) {
        schedulePixmapUpdate();
    }

    updateImplicitSize();

    emit sourceChanged();

    if (isValid() != oldValid) {
        emit validChanged();
    }
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


void IconItem::setOverlays(const QStringList &overlays)
{
    if (overlays == m_overlays) {
        return;
    }
    m_overlays = overlays;
    schedulePixmapUpdate();
    emit overlaysChanged();
}

QStringList IconItem::overlays() const
{
    return m_overlays;
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
        m_allowNextAnimation = true;
        schedulePixmapUpdate();
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

bool IconItem::isAnimated() const
{
    return m_animated;
}

void IconItem::setAnimated(bool animated)
{
    if (m_animated == animated) {
        return;
    }

    m_animated = animated;
    emit animatedChanged();
}

bool IconItem::usesPlasmaTheme() const
{
    return m_usesPlasmaTheme;
}

void IconItem::setUsesPlasmaTheme(bool usesPlasmaTheme)
{
    if (m_usesPlasmaTheme == usesPlasmaTheme) {
        return;
    }

    m_usesPlasmaTheme = usesPlasmaTheme;

    // Reload icon with new settings
    const QVariant src = m_source;
    m_source.clear();
    setSource(src);

    update();
    emit usesPlasmaThemeChanged();
}

bool IconItem::roundToIconSize() const
{
    return m_roundToIconSize;
}

void IconItem::setRoundToIconSize(bool roundToIconSize)
{
    if (m_roundToIconSize == roundToIconSize) {
        return;
    }

    const QSize oldPaintedSize = paintedSize();

    m_roundToIconSize = roundToIconSize;
    emit roundToIconSizeChanged();

    if (oldPaintedSize != paintedSize()) {
        emit paintedSizeChanged();
    }

    schedulePixmapUpdate();
}

bool IconItem::isValid() const
{
    return !m_icon.isNull() || m_svgIcon || !m_imageIcon.isNull();
}

int IconItem::paintedWidth() const
{
    return paintedSize(boundingRect().size()).width();
}

int IconItem::paintedHeight() const
{
    return paintedSize(boundingRect().size()).height();
}

QSize IconItem::paintedSize(const QSizeF &containerSize) const
{
    const QSize &actualContainerSize = (containerSize.isValid() ? containerSize : boundingRect().size()).toSize();

    const QSize paintedSize = m_iconPixmap.size().scaled(actualContainerSize, Qt::KeepAspectRatio);

    const int width = paintedSize.width();
    const int height = paintedSize.height();

    if (width == height) {
        if (m_roundToIconSize) {
            return QSize(Units::roundToIconSize(width), Units::roundToIconSize(height));
        } else {
            return QSize(width, height);
        }
    }

    // if we don't have a square image, we still want it to be rounded to icon size
    // but we cannot just blindly round both as we might erroneously change a 50x45 image to be 48x32
    // instead, round the bigger of the two and then downscale the smaller with the ratio
    if (width > height) {
        const int roundedWidth = m_roundToIconSize ? Units::roundToIconSize(width) : width;
        return QSize(roundedWidth, qRound(height * (roundedWidth / static_cast<qreal>(width))));
    } else {
        const int roundedHeight = m_roundToIconSize ? Units::roundToIconSize(height) : height;
        return QSize(qRound(width * (roundedHeight / static_cast<qreal>(height))), roundedHeight);
    }
}

void IconItem::setStatus(Plasma::Svg::Status status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    if (m_svgIcon) {
        m_svgIcon->setStatus(status);
    }
    emit statusChanged();
}

Plasma::Svg::Status IconItem::status() const
{
    return m_status;
}

void IconItem::setImplicitHeight2(int height)
{
    m_implicitHeightSetByUser = true;
    setImplicitHeight(height);
    emit implicitHeightChanged2();
}

void IconItem::setImplicitWidth2(int width)
{
    m_implicitWidthSetByUser = true;
    setImplicitWidth(width);
    emit implicitWidthChanged2();
}

void IconItem::updatePolish()
{
    QQuickItem::updatePolish();
    loadPixmap();
}

QSGNode* IconItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (m_iconPixmap.isNull() || width() == 0.0 || height() == 0.0) {
        delete oldNode;
        return nullptr;
    }

    if (m_animation->state() == QAbstractAnimation::Running) {
        FadingNode *animatingNode = dynamic_cast<FadingNode*>(oldNode);

        if (!animatingNode || m_textureChanged) {
            delete oldNode;

            QSGTexture *source = window()->createTextureFromImage(m_oldIconPixmap.toImage(), QQuickWindow::TextureCanUseAtlas);
            source->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
            QSGTexture *target = window()->createTextureFromImage(m_iconPixmap.toImage(), QQuickWindow::TextureCanUseAtlas);
            target->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
            animatingNode = new FadingNode(source, target);
            m_sizeChanged = true;
            m_textureChanged = false;
        }

        animatingNode->setProgress(m_animValue);

        if (m_sizeChanged) {
            const QSize newSize = paintedSize();
            const QRect destRect(QPointF(boundingRect().center() - QPointF(newSize.width(), newSize.height()) / 2).toPoint(), newSize);
            animatingNode->setRect(destRect);
            m_sizeChanged = false;
        }

        return animatingNode;
    } else {
        ManagedTextureNode *textureNode = dynamic_cast<ManagedTextureNode*>(oldNode);

        if (!textureNode || m_textureChanged) {
            delete oldNode;
            textureNode = new ManagedTextureNode;
            textureNode->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
            textureNode->setTexture(QSharedPointer<QSGTexture>(window()->createTextureFromImage(m_iconPixmap.toImage(), QQuickWindow::TextureCanUseAtlas)));
            m_sizeChanged = true;
            m_textureChanged = false;
        }

        if (m_sizeChanged) {
            const QSize newSize = paintedSize();
            const QRect destRect(QPointF(boundingRect().center() - QPointF(newSize.width(), newSize.height()) / 2).toPoint(), newSize);
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

void IconItem::onEnabledChanged()
{
    m_allowNextAnimation = true;
    schedulePixmapUpdate();
}

void IconItem::animationFinished()
{
    m_oldIconPixmap = QPixmap();
    m_textureChanged = true;
    update();
}

void IconItem::schedulePixmapUpdate()
{
    polish();
}

void IconItem::loadPixmap()
{
    if (!isComponentComplete()) {
        return;
    }

    int size = qMin(qRound(width()), qRound(height()));
    if (m_roundToIconSize) {
        size = Units::roundToIconSize(size);
    }

    //final pixmap to paint
    QPixmap result;
    if (size <= 0) {
        m_iconPixmap = QPixmap();
        m_animation->stop();
        update();
        return;
    } else if (m_svgIcon) {
        m_svgIcon->setDevicePixelRatio(window() ? window()->devicePixelRatio() : qApp->devicePixelRatio());
        m_svgIcon->resize(size, size);
        if (!m_svgIconName.isEmpty() && m_svgIcon->hasElement(m_svgIconName)) {
            result = m_svgIcon->pixmap(m_svgIconName);
        } else if (!m_svgIconName.isEmpty()) {
            const auto *iconTheme = KIconLoader::global()->theme();
            if (iconTheme) {
                QString iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svg"), size, KIconLoader::MatchBest);
                if (iconPath.isEmpty()) {
                    iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svgz"), size, KIconLoader::MatchBest);
                }

                if (!iconPath.isEmpty()) {
                    m_svgIcon->setImagePath(iconPath);
                }
            } else {
                qWarning() << "KIconLoader has no theme set";
            }

            result = m_svgIcon->pixmap();
        }
    } else if (!m_icon.isNull()) {
        result = m_icon.pixmap(QSize(size, size) * (window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));
    } else if (!m_imageIcon.isNull()) {
        result = QPixmap::fromImage(m_imageIcon);
    } else {
        m_iconPixmap = QPixmap();
        m_animation->stop();
        update();
        return;
    }

    // Strangely KFileItem::overlays() returns empty string-values, so
    // we need to check first whether an overlay must be drawn at all.
    // It is more efficient to do it here, as KIconLoader::drawOverlays()
    // assumes that an overlay will be drawn and has some additional
    // setup time.
    foreach (const QString& overlay, m_overlays) {
        if (!overlay.isEmpty()) {
            // There is at least one overlay, draw all overlays above m_pixmap
            // and cancel the check
            KIconLoader::global()->drawOverlays(m_overlays, result, KIconLoader::Desktop);
            break;
        }
    }

    if (!isEnabled()) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::DisabledState);
    } else if (m_active) {
        result = KIconLoader::global()->iconEffect()->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
    }

    const QSize oldPaintedSize = paintedSize();

    m_oldIconPixmap = m_iconPixmap;
    m_iconPixmap = result;
    m_textureChanged = true;

    if (oldPaintedSize != paintedSize()) {
        emit paintedSizeChanged();
    }

    //don't animate initial setting
    bool animated = (m_animated || m_allowNextAnimation) && !m_oldIconPixmap.isNull() && !m_sizeChanged && !m_blockNextAnimation;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    if (QQuickWindow::sceneGraphBackend() == QLatin1String("software")) {
        animated = false;
    }
#endif

    if (animated) {
        m_animValue = 0.0;
        m_animation->setStartValue((qreal)0);
        m_animation->setEndValue((qreal)1);
        m_animation->start();
        m_allowNextAnimation = false;
    } else {
        m_animValue = 1.0;
        m_animation->stop();
        m_blockNextAnimation = false;
    }
    update();
}

void IconItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemVisibleHasChanged && value.boolValue) {
        m_blockNextAnimation = true;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    } else if (change == ItemEnabledHasChanged) {
        onEnabledChanged();
#endif
    } else if (change == ItemSceneChange && value.window) {
        schedulePixmapUpdate();
    }

    QQuickItem::itemChange(change, value);
}

void IconItem::geometryChanged(const QRectF &newGeometry,
                               const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_sizeChanged = true;
        if (newGeometry.width() > 0 && newGeometry.height() > 0) {
            schedulePixmapUpdate();
        } else {
            update();
        }

        if (paintedSize(oldGeometry.size()) != paintedSize(newGeometry.size())) {
            emit paintedSizeChanged();
        }
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void IconItem::componentComplete()
{
    QQuickItem::componentComplete();
    schedulePixmapUpdate();
}
