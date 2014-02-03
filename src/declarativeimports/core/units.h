/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef UNITS_H
#define UNITS_H

#include <QObject>
#include <QQmlPropertyMap>

#include <Plasma/Theme>

class QQuickItem;

class Units : public QObject
{
    Q_OBJECT

    /**
     * The fundamental unit of space that should be used for sizes, expressed in pixels.
     * Given the screen has an accurate DPI settings, it corresponds to a millimeter
     */
    Q_PROPERTY(int gridUnit READ gridUnit NOTIFY gridUnitChanged())

    /**
     * units.iconSizes provides access to platform-dependent icon sizing
     *
     * The icon sizes provided are normalized for different DPI, so icons
     * will scale depending on the DPI.
     *
     * Icon sizes from KIconLoader, adjusted to devicePixelRatio:
     * * small
     * * smallMedium
     * * medium
     * * large
     * * huge
     * * enormous
     *
     * Not devicePixelRation-adjusted::
     * * desktop
     */
    Q_PROPERTY(QQmlPropertyMap *iconSizes READ iconSizes NOTIFY iconSizesChanged)

    // layout hints

    /**
     * units.largeSpacing is the amount of spacing that should be used around bigger UI elements,
     * for example between the content and the border. Internally, this size depends on the size of
     * the default font as rendered on the screen, so it takes user-configured font size and DPI
     * into account.
     */
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)

    /**
     * units.smallSpacing is the amount of spacing that should be used inside bigger UI elements,
     * for example between an icon and the corresponding text. Internally, this size depends on
     * the size of the default font as rendered on the screen, so it takes user-configured font
     * size and DPI into account.
     */
    Q_PROPERTY(int largeSpacing READ largeSpacing NOTIFY spacingChanged)

    /**
     * The ratio between physical and device-independent pixels. This value does not depend on the \
     * size of the configured font. If you want to take font sizes into account when scaling elements,
     * use theme.mSize(theme.defaultFont), units.smallSpacing and units.largeSpacing.
     * The devicePixelRatio follows the definition of "device independent pixel" by Microsoft.
     */
    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio NOTIFY devicePixelRatioChanged)

    /**
     * units.longDuration should be used for longer, screen-covering animations, for opening and
     * closing of dialogs and other "not too small" animations
     */
    Q_PROPERTY(int longDuration READ longDuration NOTIFY durationChanged)

    /**
     * units.longDuration should be used for short animations, such as accentuating a UI event,
     * hover events, etc..
     */
    Q_PROPERTY(int shortDuration READ shortDuration NOTIFY durationChanged)

public:
    Units(QObject *parent = 0);
    ~Units();

    bool eventFilter(QObject *watched, QEvent *event);

    /**
     * @return pixel value for a grid Unit. Depends on DPI and font size.
     */
    int gridUnit() const;

    /**
     * @return The ratio between physical and device-independent pixels.
     */
    qreal devicePixelRatio() const;

    /**
     * @return map with iconsizes, indexed by name
     */
    QQmlPropertyMap *iconSizes() const;

    /**
     * @return Pixel value for large spacing between elements.
     * @since 5.0
     */
    int smallSpacing() const;

    /**
     * @return Pixel value for large spacing between elements.
     * @since 5.0
     */
    int largeSpacing() const;

    /**
     * @return Duration for long animations, in milliseconds.
     * @since 5.0
     */
    int longDuration() const;

    /**
     * @return Duration for short animations, in milliseconds.
     * @since 5.0
     */
    int shortDuration() const;

Q_SIGNALS:
    void devicePixelRatioChanged();
    void gridUnitChanged();
    void iconSizesChanged();
    void spacingChanged();
    void durationChanged();

private Q_SLOTS:
    void themeChanged();
    void iconLoaderSettingsChanged();

private:
    void updateDevicePixelRatio();
    void updateSpacing();
    /**
     * @return The dpi-adjusted size for a given icon size
     */
    int devicePixelIconSize(const int size) const;

    Plasma::Theme m_theme;

    int m_gridUnit;
    qreal m_devicePixelRatio;
    qreal m_dpi;

    QQmlPropertyMap *m_iconSizes;

    int m_smallSpacing;
    int m_largeSpacing;

    int m_longDuration;
};

#endif //UNITS_H

