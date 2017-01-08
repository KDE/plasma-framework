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

#ifndef ICONITEM_H
#define ICONITEM_H

#include <QIcon>
#include <QQuickItem>
#include <QPixmap>
#include <QVariant>
#include <QTimer>

#include <plasma/svg.h>

class QPropertyAnimation;

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
     *  - URL
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
     * See QQuickItem::smooth
     */
    Q_PROPERTY(bool smooth READ smooth WRITE setSmooth NOTIFY smoothChanged)

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

public:
    IconItem(QQuickItem *parent = 0);
    ~IconItem();

    void setSource(const QVariant &source);
    QVariant source() const;

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    void setOverlays(const QStringList &overlays);
    QStringList overlays() const;

    bool isActive() const;
    void setActive(bool active);

    void setSmooth(const bool smooth);
    bool smooth() const;

    bool isAnimated() const;
    void setAnimated(bool animated);

    bool usesPlasmaTheme() const;
    void setUsesPlasmaTheme(bool usesPlasmaTheme);

    bool isValid() const;

    int paintedWidth() const;
    int paintedHeight() const;

    void setStatus(Plasma::Svg::Status status);
    Plasma::Svg::Status status() const;

    void updatePolish() Q_DECL_OVERRIDE;
    QSGNode* updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * updatePaintNodeData) Q_DECL_OVERRIDE;

    void itemChange(ItemChange change, const ItemChangeData &value) Q_DECL_OVERRIDE;
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) Q_DECL_OVERRIDE;

    void componentComplete() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void overlaysChanged();
    void activeChanged();
    void sourceChanged();
    void smoothChanged();
    void animatedChanged();
    void usesPlasmaThemeChanged();
    void validChanged();
    void colorGroupChanged();
    void paintedSizeChanged();
    void statusChanged();

private Q_SLOTS:
    void schedulePixmapUpdate();
    void animationFinished();
    void valueChanged(const QVariant &value);
    void enabledChanged();

private:
    void loadPixmap();
    QSize paintedSize(const QSizeF &containerSize = QSizeF()) const;
    void updateImplicitSize();

    //all the ways we can set an source. Only one of them will be valid
    QIcon m_icon;
    Plasma::Svg *m_svgIcon;
    QString m_svgIconName;
    QPixmap m_pixmapIcon;
    QImage m_imageIcon;
    //this contains the raw variant it was passed
    QVariant m_source;
    Plasma::Svg::Status m_status;

    QSizeF m_implicitSize;

    bool m_smooth;
    bool m_active;
    bool m_animated;
    bool m_usesPlasmaTheme;

    bool m_textureChanged;
    bool m_sizeChanged;
    bool m_allowNextAnimation;
    bool m_blockNextAnimation;

    QPixmap m_iconPixmap;
    QPixmap m_oldIconPixmap;

    QStringList m_overlays;

    Plasma::Theme::ColorGroup m_colorGroup;

    //animation on pixmap change
    QPropertyAnimation *m_animation;
    qreal m_animValue;
};

#endif
