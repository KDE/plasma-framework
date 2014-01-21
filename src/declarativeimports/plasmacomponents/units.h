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

class Units : public QObject
{
    Q_OBJECT

    /**
     * The fundamental unit of space that should be used for sizes, expressed in pixels.
     * Given the screen has an accurate DPI settings, it corresponds to a millimeter
     */
    Q_PROPERTY(qreal gridUnit READ gridUnit NOTIFY gridUnitChanged())

    /**
     * icon sizes depending from the context: use those if possible
     * Access with theme.iconSizes.desktop theme.iconSizes.small etc.
     * available keys are:
     * * desktop
     * * toolbar
     * * small
     * * dialog
     */
    Q_PROPERTY(QQmlPropertyMap *iconSizes READ iconSizes NOTIFY iconSizesChanged)

public:
    Units(QObject *parent = 0);
    ~Units();

    qreal gridUnit() const;
    qreal dpiRatio() const;

    QQmlPropertyMap *iconSizes() const;

    /**
     * @returns the number of pixels value density independent pixels correspond to.
     */
    Q_INVOKABLE qreal dp(qreal value) const;

    /**
     * @returns the number of pixels value grid units correspond to.
     */
    Q_INVOKABLE qreal gu(qreal value) const;


Q_SIGNALS:
    void gridUnitChanged();
    void iconSizesChanged();

private Q_SLOTS:
    void themeChanged();
    void iconLoaderSettingsChanged();

private:
    int m_gridUnit;
    Plasma::Theme m_theme;
    qreal m_dpiRatio;
    QQmlPropertyMap *m_iconSizes;
};

#endif //UNITS_H

