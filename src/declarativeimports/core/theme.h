/***************************************************************************
 *   Copyright 2010 Marco Martin <mart@kde.org>                            *
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
#ifndef THEME_PROXY_P
#define THEME_PROXY_P

#include <QApplication>
#include <QObject>

#include <QFont>
#include <QColor>
#include <QJSValue>
#include <QQmlEngine>

#include <Plasma/Theme>

class QQmlPropertyMap;

/**
 * QML wrapper for kdelibs Plasma::Theme
 *
 * Exposed as `Theme` in QML.
 */
class ThemeProxy : public Plasma::Theme
{
    Q_OBJECT

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

    // layout hints
    Q_PROPERTY(int smallSpacing READ smallSpacing CONSTANT)
    Q_PROPERTY(int largeSpacing READ largeSpacing CONSTANT)

public:
    ThemeProxy(QQmlEngine *parent = 0);
    ~ThemeProxy();

    /**
     * @return The size of an uppercase M in a font, defaultFont() by default
     */
    Q_INVOKABLE QSizeF mSize(const QFont &font = QApplication::font()) const;

    Q_INVOKABLE QString wallpaperPathForSize(int width=-1, int height=-1) const;

    QString styleSheet() const;

    QQmlPropertyMap *iconSizes() const;

    int smallSpacing() const;
    int largeSpacing() const;

private Q_SLOTS:
    void iconLoaderSettingsChanged();

Q_SIGNALS:
    void themeChanged();
    void iconSizesChanged();

private:
    void updateSpacing();
    int m_smallSpacing;
    int m_largeSpacing;
    QQmlPropertyMap *m_iconSizes;
    QQmlEngine *m_engine;
};

#endif
