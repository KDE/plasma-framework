/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmudnson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ICONITEM_H
#define ICONITEM_H

#include <QIcon>
#include <QPixmap>
#include <QPointer>
#include <QQuickItem>
#include <QTimer>
#include <QVariant>

#include <plasma/svg.h>

class QPropertyAnimation;
class IconItemSource;
class SvgSource;

/**
 * @class IconItem
 * @short Displays an icon, either from the standard QIcon system or where applicable from the theme SVG files
 */
class IconItem : public QQuickItem
{
    Q_OBJECT

    /**
     * Sets the icon to be displayed. Source can be one of:
     *  - iconName (as a string)
     *  - URL (for now, only file:// URL or a absolute path are supported)
     *  - QImage
     *  - QPixmap
     *  - QIcon
     *
     * When passing an icon name (or a QIcon with an icon name set) it will:
     *  - load the plasma variant if usesPlasmaTheme is set and exists
     *  - otherwise try to load the icon as an SVG so colorscopes apply
     *  - load the icon as normal
     */
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)

    /**
     * Specifies the color group to use for this icon
     * This only applies to icons loaded from the plasma theme
     */
    Q_PROPERTY(Plasma::Theme::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY colorGroupChanged)

    /**
     * Specifies the overlay(s) for this icon
     */
    Q_PROPERTY(QStringList overlays READ overlays WRITE setOverlays NOTIFY overlaysChanged)

    /**
     * Apply a visual indication that this icon is active.
     * Typically used to indicate that it is hovered
     */
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

    /**
     * Sets the image in a selected status.
     * Svgs can be colored with system color themes, if the status is selected,
     * the TextColor will become HighlightedText color and BackgroundColor
     * will become HighlightColor, making the svg graphics (for instance an icon)
     * will look correct together selected text
     * @see Plasma::Svg::status
     * @since 5.23
     */
    Q_PROPERTY(Plasma::Svg::Status status READ status WRITE setStatus NOTIFY statusChanged)

    /**
     * If set, icon will blend when the source is changed
     */
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated NOTIFY animatedChanged)

    /**
     * If set, icon will try and use icons from the Plasma theme if possible
     */
    Q_PROPERTY(bool usesPlasmaTheme READ usesPlasmaTheme WRITE setUsesPlasmaTheme NOTIFY usesPlasmaThemeChanged)

    /**
     * If set, icon will round the painted size to defined icon sizes. Default is true.
     */
    Q_PROPERTY(bool roundToIconSize READ roundToIconSize WRITE setRoundToIconSize NOTIFY roundToIconSizeChanged)

    /**
     * True if a valid icon is set. False otherwise.
     */
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

    /**
     * The width of the icon that is actually painted
     * Icons are drawn at standard icon sizes (eg. 16,32,64) centered within the item
     */
    Q_PROPERTY(int paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)

    /**
     * The height of the icon actually being drawn.
     * Icons are drawn at standard icon sizes (eg. 16,32,64) centered within the item
     */
    Q_PROPERTY(int paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)

    Q_PROPERTY(int implicitHeight READ implicitHeight WRITE setImplicitHeight2 NOTIFY implicitHeightChanged2)

    Q_PROPERTY(int implicitWidth READ implicitWidth WRITE setImplicitWidth2 NOTIFY implicitWidthChanged2)

public:
    explicit IconItem(QQuickItem *parent = nullptr);
    ~IconItem() override;

    void setSource(const QVariant &source);
    QVariant source() const;

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    void setOverlays(const QStringList &overlays);
    QStringList overlays() const;

    bool isActive() const;
    void setActive(bool active);

    bool isAnimated() const;
    void setAnimated(bool animated);

    bool usesPlasmaTheme() const;
    void setUsesPlasmaTheme(bool usesPlasmaTheme);

    bool roundToIconSize() const;
    void setRoundToIconSize(bool roundToIconSize);

    bool isValid() const;

    int paintedWidth() const;
    int paintedHeight() const;

    void setStatus(Plasma::Svg::Status status);
    Plasma::Svg::Status status() const;

    void setImplicitHeight2(int height);
    void setImplicitWidth2(int height);

    void updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;

    void itemChange(ItemChange change, const ItemChangeData &value) override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

    void componentComplete() override;

Q_SIGNALS:
    void overlaysChanged();
    void activeChanged();
    void sourceChanged();
    void animatedChanged();
    void usesPlasmaThemeChanged();
    void roundToIconSizeChanged();
    void validChanged();
    void colorGroupChanged();
    void paintedSizeChanged();
    void statusChanged();
    void implicitHeightChanged2();
    void implicitWidthChanged2();

private Q_SLOTS:
    void schedulePixmapUpdate();
    void animationFinished();
    void valueChanged(const QVariant &value);
    void onEnabledChanged();
    void iconLoaderIconChanged(int group);
    void windowVisibleChanged(bool visible);

private:
    void loadPixmap();
    QSize paintedSize(const QSizeF &containerSize = QSizeF()) const;
    void updateImplicitSize();

    QSGNode *createSubtree(qreal initialOpacity);
    void updateSubtree(QSGNode *node, qreal opacity);

    // all the ways we can set an source. Only one of them will be valid
    std::unique_ptr<IconItemSource> m_iconItemSource;
    // this contains the raw variant it was passed
    QVariant m_source;
    Plasma::Svg::Status m_status;

    bool m_active;
    bool m_animated;
    bool m_usesPlasmaTheme;
    bool m_roundToIconSize;

    bool m_textureChanged;
    bool m_sizeChanged;
    bool m_allowNextAnimation;
    bool m_blockNextAnimation;
    bool m_implicitHeightSetByUser;
    bool m_implicitWidthSetByUser;

    QPixmap m_iconPixmap;
    QPixmap m_oldIconPixmap;

    QStringList m_overlays;

    Plasma::Theme::ColorGroup m_colorGroup;

    // animation on pixmap change
    QPropertyAnimation *m_animation;
    qreal m_animValue;

    QPointer<QWindow> m_window;

    // to access schedulePixmapUpdate private slot
    friend class SvgSource;
};

#endif
