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

    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)
    Q_PROPERTY(bool useGlobalSettings READ useGlobalSettings NOTIFY themeChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY themeChanged)

    //fonts
    Q_PROPERTY(QFont  defaultFont READ defaultFont NOTIFY defaultFontChanged)
    Q_PROPERTY(QFont  smallestFont READ smallestFont NOTIFY smallestFontChanged)

    // colors
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonTextColor READ buttonTextColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonBackgroundColor READ buttonBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor linkColor READ linkColor NOTIFY themeChanged)
    Q_PROPERTY(QColor visitedLinkColor READ visitedLinkColor NOTIFY themeChanged)
    Q_PROPERTY(QColor visitedLinkColor READ visitedLinkColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonHoverColor READ buttonHoverColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonFocusColor READ buttonFocusColor NOTIFY themeChanged)
    Q_PROPERTY(QColor viewTextColor READ viewTextColor NOTIFY themeChanged)
    Q_PROPERTY(QColor viewBackgroundColor READ viewBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor viewHoverColor READ viewHoverColor NOTIFY themeChanged)
    Q_PROPERTY(QColor viewFocusColor READ viewFocusColor NOTIFY themeChanged)
    Q_PROPERTY(QString styleSheet READ styleSheet NOTIFY themeChanged)

    // icon sizes
    Q_PROPERTY(int smallIconSize READ smallIconSize CONSTANT)
    Q_PROPERTY(int smallMediumIconSize READ smallMediumIconSize CONSTANT)
    Q_PROPERTY(int mediumIconSize READ mediumIconSize CONSTANT)
    Q_PROPERTY(int largeIconSize READ largeIconSize CONSTANT)
    Q_PROPERTY(int hugeIconSize READ hugeIconSize CONSTANT)
    Q_PROPERTY(int enormousIconSize READ enormousIconSize CONSTANT)
    Q_PROPERTY(int defaultIconSize READ defaultIconSize NOTIFY defaultIconSizeChanged)

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

    QString themeName() const;
    QFont defaultFont() const;
    QFont smallestFont() const;
    /**
     * @return The size of an uppercase M in a font, defaultFont() by default
     */
    Q_INVOKABLE QSizeF mSize(const QFont &font = QApplication::font()) const;

    bool useGlobalSettings() const;
    QString wallpaperPath() const;
    Q_INVOKABLE QString wallpaperPathForSize(int width=-1, int height=-1) const;

    QColor textColor() const;
    QColor highlightColor() const;
    QColor backgroundColor() const;
    QColor buttonTextColor() const;
    QColor buttonBackgroundColor() const;
    QColor linkColor() const;
    QColor visitedLinkColor() const;
    QColor buttonHoverColor() const;
    QColor buttonFocusColor() const;
    QColor viewTextColor() const;
    QColor viewBackgroundColor() const;
    QColor viewHoverColor() const;
    QColor viewFocusColor() const;
    QString styleSheet() const;

    int smallIconSize() const;
    int smallMediumIconSize() const;
    int mediumIconSize() const;
    int largeIconSize() const;
    int hugeIconSize() const;
    int enormousIconSize() const;
    int defaultIconSize() const;
    QQmlPropertyMap *iconSizes() const;

    int smallSpacing() const;
    int largeSpacing() const;

private Q_SLOTS:
    void iconLoaderSettingsChanged();

Q_SIGNALS:
    void themeChanged();
    void defaultIconSizeChanged();
    void iconSizesChanged();
    void defaultFontChanged();
    void smallestFontChanged();

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void updateSpacing();
    int m_smallSpacing;
    int m_largeSpacing;
    int m_defaultIconSize;
    QQmlPropertyMap *m_iconSizes;
    QQmlEngine *m_engine;
};

#endif
