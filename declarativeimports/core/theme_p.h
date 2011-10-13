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

#include <QObject>

#include <KUrl>
#include <QFont>
#include <QColor>

class FontProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool bold READ bold NOTIFY boldChanged)
    Q_PROPERTY(Capitalization capitalization READ capitalization  NOTIFY capitalizationChanged )
    Q_PROPERTY(QString family READ family NOTIFY familyChanged )
    Q_PROPERTY(bool italic READ italic NOTIFY italicChanged )
    Q_PROPERTY(qreal letterSpacing READ letterSpacing NOTIFY letterSpacingChanged )
    Q_PROPERTY(int pixelSize READ pixelSize NOTIFY pixelSizeChanged )
    Q_PROPERTY(qreal pointSize READ pointSize NOTIFY pointSizeChanged )
    Q_PROPERTY(bool strikeOut READ strikeOut NOTIFY strikeOutChanged )
    Q_PROPERTY(bool underline READ underline NOTIFY underlineChanged )
    Q_PROPERTY(Weight weight READ weight NOTIFY weightChanged )
    Q_PROPERTY(qreal wordSpacing READ wordSpacing NOTIFY wordSpacingChanged )

    Q_ENUMS(Capitalization)
    Q_ENUMS(Weight)

public:
    enum Capitalization {
        MixedCase = 0,
        AllUppercase = 1,
        AllLowercase = 2,
        SmallCaps = 3,
        Capitalize = 4
    };

    enum Weight {
        Light = 25,
        Normal = 50,
        DemiBold = 63,
        Bold = 75,
        Black = 87
    };

    FontProxy(QObject *parent = 0);
    ~FontProxy();
    static FontProxy *self();

    bool bold() const;
    Capitalization capitalization() const;
    QString family() const;
    bool italic() const;
    qreal letterSpacing() const;
    int pixelSize() const;
    qreal pointSize() const;
    bool strikeOut() const;
    bool underline() const;
    Weight weight() const;
    qreal wordSpacing() const;

Q_SIGNALS:
    void boldChanged();
    void capitalizationChanged();
    void familyChanged();
    void italicChanged();
    void letterSpacingChanged();
    void pixelSizeChanged();
    void pointSizeChanged();
    void strikeOutChanged();
    void underlineChanged();
    void weightChanged();
    void wordSpacingChanged();
};

class ThemeProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)
    Q_PROPERTY(bool windowTranslucentEnabled READ windowTranslucencyEnabled NOTIFY themeChanged)
    Q_PROPERTY(KUrl homepage READ homepage NOTIFY themeChanged)
    Q_PROPERTY(bool useGlobalSettings READ useGlobalSettings NOTIFY themeChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY themeChanged)

    //fonts
    Q_PROPERTY(QObject *font READ font CONSTANT)

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

public:
    ThemeProxy(QObject *parent = 0);
    ~ThemeProxy();

    QString themeName() const;
    QObject *font() const;
    bool windowTranslucencyEnabled() const;
    KUrl homepage() const;
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

Q_SIGNALS:
    void themeChanged();
};

#endif
