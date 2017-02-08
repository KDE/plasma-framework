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

class SharedAppFilter : public QObject
{
   Q_OBJECT
public:
    SharedAppFilter(QObject *parent = 0);
    ~SharedAppFilter();

Q_SIGNALS:
    void fontChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
};

/**
 * @class Units
 * @short Expose sizes to QML
 */
class Units : public QObject
{
    Q_OBJECT

    /**
     * The fundamental unit of space that should be used for sizes, expressed in pixels.
     * Given the screen has an accurate DPI settings, it corresponds to a width of
     * the capital letter M
     */
    Q_PROPERTY(int gridUnit READ gridUnit NOTIFY gridUnitChanged)

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
     *
     */
    //note the iconSizeChanges signal indicates that one (or more) of these icons have changed
    //but the property map itself remains constant
    Q_PROPERTY(QQmlPropertyMap *iconSizes READ iconSizes CONSTANT)

    // layout hints

    /**
     * units.smallSpacing is the amount of spacing that should be used around smaller UI elements,
     * for example as spacing in Columns. Internally, this size depends on the size of
     * the default font as rendered on the screen, so it takes user-configured font size and DPI
     * into account.
     */
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)

    /**
     * units.largeSpacing is the amount of spacing that should be used inside bigger UI elements,
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
     * units.shortDuration should be used for short animations, such as accentuating a UI event,
     * hover events, etc..
     */
    Q_PROPERTY(int shortDuration READ shortDuration NOTIFY durationChanged)

public:
/// @cond INTERNAL_DOCS

    ~Units();

    /**
     * @return a reference to the global Units instance
     * @since 5.31
     */
    static Units &instance();

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
/// @endcond

    /**
     * @return a size rounded tothe nearest inferior standard icon size.
     *           sizes larger than iconSizes.huge, it will be returned unmodified
     * @param int size the size we want to be rounded down
     * @see iconSizes
     */
    Q_INVOKABLE static int roundToIconSize(int size);

Q_SIGNALS:
    void devicePixelRatioChanged();
    void gridUnitChanged();
    void iconSizesChanged();
    void spacingChanged();
    void durationChanged();

private Q_SLOTS:
    void iconLoaderSettingsChanged();
    void settingsFileChanged(const QString &file);
    void updateSpacing();

private:
    Units(QObject *parent = 0);
    Units(Units const&) = delete; // Copy construct
    Units(Units&&) = delete; // Move construct
    Units& operator=(Units const&) = delete; // Copy assign
    Units& operator=(Units &&) = delete; // Move assign

    void updateDevicePixelRatio();
    void updatePlasmaRCSettings();
    /**
     * @return The dpi-adjusted size for a given icon size
     */
    int devicePixelIconSize(const int size) const;

    int m_gridUnit;
    qreal m_devicePixelRatio;

    QQmlPropertyMap *m_iconSizes;
    static SharedAppFilter *s_sharedAppFilter;

    int m_smallSpacing;
    int m_largeSpacing;

    int m_longDuration;
};

#endif //UNITS_H

