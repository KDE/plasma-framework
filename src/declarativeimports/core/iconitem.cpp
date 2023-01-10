/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmudnson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iconitem.h"

#include <cmath>

#include <QDebug>
#include <QPaintEngine>
#include <QPainter>
#include <QPalette>
#include <QPropertyAnimation>
#include <QQuickWindow>
#include <QSGImageNode>

#include <KIconEffect>
#include <KIconLoader>
#include <KIconTheme>

#include "theme.h"
#include "units.h"

class IconItemSource
{
public:
    explicit IconItemSource(IconItem *iconItem)
        : m_iconItem(iconItem)
    {
    }
    virtual ~IconItemSource()
    {
    }

    virtual bool isValid() const = 0;
    virtual const QSize size() const = 0;
    virtual QPixmap pixmap(const QSize &size) = 0;

protected:
    QQuickWindow *window()
    {
        return m_iconItem->window();
    }

    IconItem *m_iconItem;
};

class NullSource : public IconItemSource
{
public:
    explicit NullSource(IconItem *iconItem)
        : IconItemSource(iconItem)
    {
    }

    bool isValid() const override
    {
        return false;
    }

    const QSize size() const override
    {
        return QSize();
    }

    QPixmap pixmap(const QSize &size) override
    {
        Q_UNUSED(size)
        return QPixmap();
    }
};

class QIconSource : public IconItemSource
{
public:
    explicit QIconSource(const QIcon &icon, IconItem *iconItem)
        : IconItemSource(iconItem)
        , m_icon(icon)
    {
    }

    bool isValid() const override
    {
        return !m_icon.isNull();
    }

    const QSize size() const override
    {
        return QSize();
    }

    QPixmap pixmap(const QSize &size) override
    {
        KIconLoader::global()->setCustomPalette(Plasma::Theme::globalPalette());
        QPixmap result = m_icon.pixmap(window(), m_icon.actualSize(size));
        KIconLoader::global()->resetPalette();
        return result;
    }

private:
    QIcon m_icon;
};

class QImageSource : public IconItemSource
{
public:
    explicit QImageSource(const QImage &imageIcon, IconItem *iconItem)
        : IconItemSource(iconItem)
        , m_imageIcon(imageIcon)
    {
    }

    bool isValid() const override
    {
        return !m_imageIcon.isNull();
    }

    const QSize size() const override
    {
        const QSize s = m_imageIcon.size();
        if (s.isValid()) {
            return s;
        }

        return QSize();
    }

    QPixmap pixmap(const QSize &size) override
    {
        Q_UNUSED(size)
        return QPixmap::fromImage(m_imageIcon);
    }

private:
    QImage m_imageIcon;
};

class SvgSource : public IconItemSource
{
public:
    explicit SvgSource(const QString &sourceString, IconItem *iconItem)
        : IconItemSource(iconItem)
        , m_svgIcon(new Plasma::Svg(iconItem))
    {
        m_svgIcon->setColorGroup(iconItem->colorGroup());
        m_svgIcon->setStatus(iconItem->status());
        m_svgIcon->setDevicePixelRatio(devicePixelRatio());
        QObject::connect(m_svgIcon, &Plasma::Svg::repaintNeeded, iconItem, &IconItem::schedulePixmapUpdate);
        QObject::connect(iconItem, &IconItem::statusChanged, m_svgIcon, [=] {
            if (m_svgIcon) {
                m_svgIcon->setStatus(iconItem->status());
            }
        });
        QObject::connect(iconItem, &IconItem::colorGroupChanged, m_svgIcon, [=] {
            if (m_svgIcon) {
                m_svgIcon->setColorGroup(iconItem->colorGroup());
            }
        });

        if (iconItem->usesPlasmaTheme()) {
            // try as a svg icon from plasma theme
            m_svgIcon->setImagePath(QLatin1String("icons/") + sourceString.section(QLatin1Char('-'), 0, 0));
            m_svgIcon->setContainsMultipleImages(true);
        }

        // success?
        if (iconItem->usesPlasmaTheme() && m_svgIcon->isValid() && m_svgIcon->hasElement(sourceString)) {
            m_svgIconName = sourceString;
            // ok, svg not available from the plasma theme
        } else {
            // try to load from iconloader an svg with Plasma::Svg
            const auto *iconTheme = KIconLoader::global()->theme();
            QString iconPath;
            if (iconTheme) {
                iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svg"), qMin(iconItem->width(), iconItem->height()), KIconLoader::MatchBest);
                if (iconPath.isEmpty()) {
                    iconPath = iconTheme->iconPath(sourceString + QLatin1String(".svgz"), qMin(iconItem->width(), iconItem->height()), KIconLoader::MatchBest);
                }
            } else {
                qWarning() << "KIconLoader has no theme set";
            }

            if (!iconPath.isEmpty()) {
                m_svgIcon->setImagePath(iconPath);
                m_svgIconName = sourceString;
            } else {
                // fail, cleanup
                delete m_svgIcon;
            }
        }
    }

    ~SvgSource() override
    {
        if (m_svgIcon) {
            // the parent IconItem can outlive this IconItemSource, so delete our Plasma::Svg object
            // explicitly to avoid leaving unreferenced Plasma::Svg objects parented to the IconItem
            delete m_svgIcon;
        }
    }

    bool isValid() const override
    {
        return m_svgIcon;
    }

    const QSize size() const override
    {
        QSize s;
        if (m_svgIcon) { // FIXME: Check Svg::isValid()? Considered expensive by apidox.
            // resize() resets the icon to its implicit size, specified
            m_svgIcon->resize();

            // plasma theme icon, where one file contains multiple images
            if (m_svgIcon->hasElement(m_svgIconName)) {
                s = m_svgIcon->elementSize(m_svgIconName);
                // normal icon: one image per file, page size is icon size
            } else {
                s = m_svgIcon->size();
            }
        }
        return s;
    }

    QPixmap pixmap(const QSize &size) override
    {
        m_svgIcon->setDevicePixelRatio(devicePixelRatio());
        m_svgIcon->resize(size);
        if (!m_svgIconName.isEmpty() && m_svgIcon->hasElement(m_svgIconName)) {
            return m_svgIcon->pixmap(m_svgIconName);
        } else if (!m_svgIconName.isEmpty()) {
            const auto *iconTheme = KIconLoader::global()->theme();
            if (iconTheme) {
                QString iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svg"), size.width(), KIconLoader::MatchBest);
                if (iconPath.isEmpty()) {
                    iconPath = iconTheme->iconPath(m_svgIconName + QLatin1String(".svgz"), size.width(), KIconLoader::MatchBest);
                }

                if (!iconPath.isEmpty()) {
                    m_svgIcon->setImagePath(iconPath);
                }
            } else {
                qWarning() << "KIconLoader has no theme set";
            }

            return m_svgIcon->pixmap();
        }

        return QPixmap();
    }

private:
    qreal devicePixelRatio()
    {
        return std::ceil(window() ? window()->devicePixelRatio() : qApp->devicePixelRatio());
    }

    QPointer<Plasma::Svg> m_svgIcon;
    QString m_svgIconName;
};

IconItem::IconItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_iconItemSource(new NullSource(this))
    , m_status(Plasma::Svg::Normal)
    , m_active(false)
    , m_animated(true)
    , m_usesPlasmaTheme(true)
    , m_roundToIconSize(true)
    , m_textureChanged(false)
    , m_sizeChanged(false)
    , m_allowNextAnimation(false)
    , m_blockNextAnimation(false)
    , m_implicitHeightSetByUser(false)
    , m_implicitWidthSetByUser(false)
    , m_colorGroup(Plasma::Theme::NormalColorGroup)
    , m_animValue(0)
{
    m_animation = new QPropertyAnimation(this);
    connect(m_animation, &QPropertyAnimation::valueChanged, this, &IconItem::valueChanged);
    connect(m_animation, &QPropertyAnimation::finished, this, &IconItem::animationFinished);
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_animation->setDuration(Units::instance().longDuration());
    connect(&Units::instance(), &Units::durationChanged, m_animation, [=]() {
        m_animation->setDuration(Units::instance().longDuration());
    });

    setFlag(ItemHasContents, true);

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &IconItem::updateImplicitSize);

    connect(this, &IconItem::implicitWidthChanged, this, &IconItem::implicitWidthChanged2);
    connect(this, &IconItem::implicitHeightChanged, this, &IconItem::implicitHeightChanged2);

    updateImplicitSize();
}

IconItem::~IconItem()
{
}

void IconItem::updateImplicitSize()
{
    if (m_iconItemSource->isValid()) {
        const QSize s = m_iconItemSource->size();

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

    disconnect(KIconLoader::global(), &KIconLoader::iconChanged, this, &IconItem::iconLoaderIconChanged);

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
            if (sourceString.endsWith(QLatin1String(".svg")) //
                || sourceString.endsWith(QLatin1String(".svgz")) //
                || sourceString.endsWith(QLatin1String(".ico"))) {
                QIcon icon = QIcon(localFile);
                m_iconItemSource.reset(new QIconSource(icon, this));
            } else {
                QImage imageIcon = QImage(localFile);
                m_iconItemSource.reset(new QImageSource(imageIcon, this));
            }
        } else {
            m_iconItemSource.reset(new SvgSource(sourceString, this));

            if (!m_iconItemSource->isValid()) {
                // if we started with a QIcon use that.
                QIcon icon = source.value<QIcon>();
                if (icon.isNull()) {
                    icon = QIcon::fromTheme(sourceString);
                }
                m_iconItemSource.reset(new QIconSource(icon, this));

                // since QIcon is rendered by KIconLoader, watch for when its configuration changes now and reload as needed.
                connect(KIconLoader::global(), &KIconLoader::iconChanged, this, &IconItem::iconLoaderIconChanged);
            }
        }

    } else if (source.canConvert<QIcon>()) {
        m_iconItemSource.reset(new QIconSource(source.value<QIcon>(), this));
    } else if (source.canConvert<QImage>()) {
        m_iconItemSource.reset(new QImageSource(source.value<QImage>(), this));
    } else {
        m_iconItemSource.reset(new NullSource(this));
    }

    if (width() > 0 && height() > 0) {
        schedulePixmapUpdate();
    }

    updateImplicitSize();

    Q_EMIT sourceChanged();

    if (isValid() != oldValid) {
        Q_EMIT validChanged();
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
    Q_EMIT colorGroupChanged();
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
    Q_EMIT overlaysChanged();
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
    Q_EMIT activeChanged();
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
    Q_EMIT animatedChanged();
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
    Q_EMIT usesPlasmaThemeChanged();
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
    Q_EMIT roundToIconSizeChanged();

    if (oldPaintedSize != paintedSize()) {
        Q_EMIT paintedSizeChanged();
    }

    schedulePixmapUpdate();
}

bool IconItem::isValid() const
{
    return m_iconItemSource->isValid();
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
    Q_EMIT statusChanged();
}

Plasma::Svg::Status IconItem::status() const
{
    return m_status;
}

void IconItem::setImplicitHeight2(int height)
{
    m_implicitHeightSetByUser = true;
    setImplicitHeight(height);
    Q_EMIT implicitHeightChanged2();
}

void IconItem::setImplicitWidth2(int width)
{
    m_implicitWidthSetByUser = true;
    setImplicitWidth(width);
    Q_EMIT implicitWidthChanged2();
}

void IconItem::updatePolish()
{
    QQuickItem::updatePolish();
    loadPixmap();
}

QSGNode *IconItem::createSubtree(qreal initialOpacity)
{
    auto opacityNode = new QSGOpacityNode{};
    opacityNode->setFlag(QSGNode::OwnedByParent, true);
    opacityNode->setOpacity(initialOpacity);

    auto imageNode = window()->createImageNode();
    imageNode->setFlag(QSGNode::OwnedByParent, true);
    imageNode->setTexture(window()->createTextureFromImage(m_iconPixmap.toImage()));
    imageNode->setOwnsTexture(true);
    imageNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    opacityNode->appendChildNode(imageNode);

    return opacityNode;
}

void IconItem::updateSubtree(QSGNode *node, qreal opacity)
{
    auto opacityNode = static_cast<QSGOpacityNode *>(node);
    opacityNode->setOpacity(opacity);

    auto imageNode = static_cast<QSGImageNode *>(opacityNode->firstChild());
    imageNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
}

QSGNode *IconItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (m_iconPixmap.isNull() || width() == 0.0 || height() == 0.0) {
        delete oldNode;
        return nullptr;
    }

    if (!oldNode) {
        oldNode = new QSGNode{};
    }

    if (m_animation->state() == QAbstractAnimation::Running) {
        if (oldNode->childCount() < 2) {
            oldNode->appendChildNode(createSubtree(0.0));
            m_textureChanged = true;
        }

        // Rather than doing a perfect crossfade, first fade in the new texture
        // then fade out the old texture. This is done to avoid the underlying
        // color bleeding through when both textures are at ~0.5 opacity, which
        // causes flickering if the two textures are very similar.
        updateSubtree(oldNode->firstChild(), 2.0 - m_animValue * 2.0);
        updateSubtree(oldNode->lastChild(), m_animValue * 2.0);
    } else {
        if (oldNode->childCount() == 0) {
            oldNode->appendChildNode(createSubtree(1.0));
            m_textureChanged = true;
        }

        if (oldNode->childCount() > 1) {
            auto toRemove = oldNode->firstChild();
            oldNode->removeChildNode(toRemove);
            delete toRemove;
        }

        updateSubtree(oldNode->firstChild(), 1.0);
    }

    if (m_textureChanged) {
        auto child = oldNode->lastChild();
        auto imageNode = static_cast<QSGImageNode *>(child->firstChild());
        imageNode->setTexture(window()->createTextureFromImage(m_iconPixmap.toImage()));
        m_textureChanged = false;
        m_sizeChanged = true;
    }

    if (m_sizeChanged) {
        const QSize newSize = paintedSize();
        const QRect destRect(QPointF(boundingRect().center() - QPointF(newSize.width(), newSize.height()) / 2).toPoint(), newSize);

        for (int i = 0; i < oldNode->childCount(); ++i) {
            auto imageNode = static_cast<QSGImageNode *>(oldNode->childAtIndex(i)->firstChild());
            imageNode->setRect(destRect);
        }

        m_sizeChanged = false;
    }

    return oldNode;
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

void IconItem::iconLoaderIconChanged(int group)
{
    Q_UNUSED(group);
    schedulePixmapUpdate();
}

void IconItem::windowVisibleChanged(bool visible)
{
    if (visible) {
        m_blockNextAnimation = true;
    }
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

    // final pixmap to paint
    QPixmap result;
    if (size <= 0) {
        m_iconPixmap = QPixmap();
        m_animation->stop();
        update();
        return;
    }
    if (m_iconItemSource->isValid()) {
        result = m_iconItemSource->pixmap(QSize(size, size));
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
    for (const QString &overlay : std::as_const(m_overlays)) {
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
        Q_EMIT paintedSizeChanged();
    }

    // don't animate initial setting
    bool animated = (m_animated || m_allowNextAnimation) && !m_oldIconPixmap.isNull() && !m_sizeChanged && !m_blockNextAnimation;

    if (QQuickWindow::sceneGraphBackend() == QLatin1String("software")) {
        animated = false;
    }

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
    } else if (change == ItemEnabledHasChanged) {
        onEnabledChanged();
    } else if (change == ItemSceneChange && value.window) {
        if (m_window) {
            disconnect(m_window.data(), &QWindow::visibleChanged, this, &IconItem::windowVisibleChanged);
        }
        m_window = value.window;
        if (m_window) {
            connect(m_window.data(), &QWindow::visibleChanged, this, &IconItem::windowVisibleChanged);
        }
        schedulePixmapUpdate();
    }

    QQuickItem::itemChange(change, value);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void IconItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void IconItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#endif
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_sizeChanged = true;
        if (newGeometry.width() > 0 && newGeometry.height() > 0) {
            schedulePixmapUpdate();
        } else {
            update();
        }

        if (paintedSize(oldGeometry.size()) != paintedSize(newGeometry.size())) {
            Q_EMIT paintedSizeChanged();
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
#else
    QQuickItem::geometryChange(newGeometry, oldGeometry);
#endif
}

void IconItem::componentComplete()
{
    QQuickItem::componentComplete();
    schedulePixmapUpdate();
}
