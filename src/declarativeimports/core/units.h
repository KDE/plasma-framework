/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
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
    Q_PROPERTY(qreal gridUnit READ gridUnit NOTIFY gridUnitChanged())

    /**
     * units.iconSizes provides access to platform-dependent icon sizing
     *
     * The icon sizes provided are normalized for different DPI, so icons
     * will scale depending on the DPI.
     *
     * Icon sizes from KIconLoader:
     * * small
     * * smallMedium
     * * medium
     * * large
     * * huge
     * * enormous
     *
     * Icon sizes depending on the context: use those if possible
     * Access with units.iconSizes.desktop units.iconSizes.small etc.
     * available keys are:
     * * desktop
     * * toolbar
     * * dialog
     */
    Q_PROPERTY(QQmlPropertyMap *iconSizes READ iconSizes NOTIFY iconSizesChanged)

    // layout hints
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)
    Q_PROPERTY(int largeSpacing READ largeSpacing NOTIFY spacingChanged)

    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY devicePixelRatioChanged)

public:
    Units(QObject *parent = 0);
    ~Units();

    bool eventFilter(QObject *watched, QEvent *event);

    qreal gridUnit() const;

    void setDevicePixelRatio(const qreal scale);
    qreal devicePixelRatio() const;

    QQmlPropertyMap *iconSizes() const;

    int smallSpacing() const;
    int largeSpacing() const;

    /**
     * @returns the number of pixels value density independent pixels correspond to.
     */
    Q_INVOKABLE qreal dp(qreal value) const;

    /**
     * @returns the number of pixels value grid units correspond to.
     */
    Q_INVOKABLE qreal gu(qreal value) const;

    /**
     * @returns the dpi value for the item's screen
     */
    Q_INVOKABLE qreal dpi(QQuickItem *item);

    Q_INVOKABLE void printScreenInfo(QQuickItem *item);

Q_SIGNALS:
    void devicePixelRatioChanged();
    void gridUnitChanged();
    void iconSizesChanged();
    void spacingChanged();

private Q_SLOTS:
    void themeChanged();
    void iconLoaderSettingsChanged();

private:
    void updateSpacing();

    Plasma::Theme m_theme;

    int m_gridUnit;
    qreal m_devicePixelRatio;
    qreal m_dpi;

    QQmlPropertyMap *m_iconSizes;

    int m_smallSpacing;
    int m_largeSpacing;
};

#endif //UNITS_H

