/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNITS_H
#define UNITS_H

#include <QObject>
#include <QQmlPropertyMap>

#include <Plasma/Theme>

#include <KConfigWatcher>

class QQuickItem;

class SharedAppFilter : public QObject
{
    Q_OBJECT
public:
    explicit SharedAppFilter(QObject *parent = nullptr);
    ~SharedAppFilter() override;

Q_SIGNALS:
    void fontChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
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
     * Given the screen has an accurate DPI settings, it corresponds to the height of
     * the font's boundingRect.
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
     * * desktop (DEPRECATED: use iconSizeHints instead)
     *
     */
    // note the iconSizeChanged signal indicates that one (or more) of these icons have changed
    // but the property map itself remains constant
    Q_PROPERTY(QQmlPropertyMap *iconSizes READ iconSizes CONSTANT)

    /**
     * units.iconSizeHints provides access to user-configurable icon size hints,
     * to be used where appropriate in the user interface.
     *
     * Conceptually, an icon size hint is a key that has one of the sizes from
     * @iconSizes property as value.
     *
     * Currently available hints:
     * * panel
     * * desktop
     */
    // note the iconSizeHintsChanged signal indicates that one (or more) of these icons have changed
    // but the property map itself remains constant
    Q_PROPERTY(QQmlPropertyMap *iconSizeHints READ iconSizeHints CONSTANT)

    // layout hints

    /**
     * This property holds the amount of spacing that should be used between smaller UI elements,
     * such as a small icon and a label in a button.
     * Internally, this size depends on the size of the default font as rendered on the screen,
     * so it takes user-configured font size and DPI into account.
     */
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)

    /**
     * This property holds the amount of spacing that should be used between medium UI elements,
     * such as buttons and text fields in a toolbar.
     * Internally, this size depends on the size of the default font as rendered on the screen,
     * so it takes user-configured font size and DPI into account.
     */
    Q_PROPERTY(int mediumSpacing READ mediumSpacing NOTIFY spacingChanged)

    /**
     * This property is functionally identical to gridUnit. To avoid confusion, it should not be used.
     *
     * TODO KF6: This should be changed to work like Kirigami::Units::largeSpacing.
     */
    Q_PROPERTY(int largeSpacing READ largeSpacing NOTIFY spacingChanged)

    /**
     * The ratio between physical and device-independent pixels. This value does not depend on the \
     * size of the configured font. If you want to take font sizes into account when scaling elements,
     * use PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont), PlasmaCore.Units.smallSpacing and PlasmaCore.Units.largeSpacing.
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

    /**
     * units.veryShortDuration should be used for elements that should animate near instantly,
     * but should have a hint of smoothness
     */
    Q_PROPERTY(int veryShortDuration READ veryShortDuration NOTIFY durationChanged)

    /**
     * units.veryLongDuration should be used for specialty animations that benefit
     * from being even longer than longDuration.
     */
    Q_PROPERTY(int veryLongDuration READ veryLongDuration NOTIFY durationChanged)

    /**
     * Time in milliseconds equivalent to the theoretical human moment, which can be used
     * to determine whether how long to wait until the user should be informed of something,
     * or can be used as the limit for how long something should wait before being
     * automatically initiated.
     *
     * Some examples:
     *
     * - When the user types text in a search field, wait no longer than this duration after
     *   the user completes typing before starting the search
     * - When loading data which would commonly arrive rapidly enough to not require interaction,
     *   wait this long before showing a spinner
     *
     * This might seem an arbitrary number, but given the psychological effect that three
     * seconds seems to be what humans consider a moment (and in the case of waiting for
     * something to happen, a moment is that time when you think "this is taking a bit long,
     * isn't it?"), the idea is to postpone for just before such a conceptual moment. The reason
     * for the two seconds, rather than three, is to function as a middle ground: Not long enough
     * that the user would think that something has taken too long, for also not so fast as to
     * happen too soon.
     *
     * See also
     * https://www.psychologytoday.com/blog/all-about-addiction/201101/tick-tock-tick-hugs-and-life-in-3-second-intervals
     * (the actual paper is hidden behind an academic paywall and consequently not readily
     * available to us, so the source will have to be the blog entry above)
     *
     * @since 5.81
     */
    Q_PROPERTY(int humanMoment READ humanMoment CONSTANT)

public:
    /// @cond INTERNAL_DOCS

    ~Units() override;

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
     * @return map with user-configurable icon size hints, indexed by name
     * @since 5.33
     */
    QQmlPropertyMap *iconSizeHints() const;

    /**
     * @return Pixel value for large spacing between elements.
     * @since 5.0
     */
    int smallSpacing() const;

    /**
     * @return Pixel value for medium spacing between elements.
     * @since 5.92
     */
    int mediumSpacing() const;

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

    /**
     * @return Duration for very long animations, in milliseconds.
     * @since 5.69
     */
    int veryLongDuration() const;

    /**
     * @return Duration for instantaneous animations, in milliseconds.
     * @since 5.78
     */
    int veryShortDuration() const;

    /**
     * @return Duration for very long wait times
     * @since 5.81
     */
    int humanMoment() const;
    /// @endcond

    /**
     * @return a size rounded to the nearest inferior standard icon size.
     *           sizes larger than iconSizes.huge, it will be returned unmodified
     * @param int size the size we want to be rounded down
     * @see iconSizes
     */
    Q_INVOKABLE static int roundToIconSize(int size);

Q_SIGNALS:
    void devicePixelRatioChanged();
    void gridUnitChanged();
    void iconSizesChanged();
    void iconSizeHintsChanged();
    void spacingChanged();
    void durationChanged();

private Q_SLOTS:
    void iconLoaderSettingsChanged();
    void updateSpacing();

private:
    Units(QObject *parent = nullptr);
    Units(Units const &) = delete; // Copy construct
    Units(Units &&) = delete; // Move construct
    Units &operator=(Units const &) = delete; // Copy assign
    Units &operator=(Units &&) = delete; // Move assign

    void updateDevicePixelRatio();
    void updateAnimationSpeed();

    /**
     * @return the best-looking icon scale for the given device pixel ratio
     * Note that this function is only relevant when using Plasma scaling and
     * when using Qt scaling, it always returns 1.
     */
    static qreal bestIconScaleForDevicePixelRatio(const qreal ratio);

    /**
     * @return The dpi-adjusted size for a given icon size
     */
    int devicePixelIconSize(const int size) const;

    int m_gridUnit;
    qreal m_devicePixelRatio;

    QQmlPropertyMap *m_iconSizes;
    QQmlPropertyMap *m_iconSizeHints;
    static SharedAppFilter *s_sharedAppFilter;

    int m_smallSpacing;
    int m_mediumSpacing;
    int m_largeSpacing;

    KConfigWatcher::Ptr m_animationSpeedWatcher;
    int m_longDuration;
};

#endif // UNITS_H
