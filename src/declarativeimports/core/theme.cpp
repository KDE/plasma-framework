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

#include "theme.h"

#include <QQmlPropertyMap>

#include <KGlobalSettings>
#include <KIconLoader>
#include <QApplication>



//********** Theme *************

ThemeProxy::ThemeProxy(QObject *parent)
    : QObject(parent)
{
    m_defaultIconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);

    m_iconSizes = new QQmlPropertyMap(this);
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("panel", QVariant(KIconLoader::global()->currentSize(KIconLoader::Panel)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    m_theme = new Plasma::Theme(this);
    connect(m_theme, SIGNAL(themeChanged()), this, SIGNAL(themeChanged()));
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()), this, SLOT(iconLoaderSettingsChanged()));

    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &ThemeProxy::defaultFontChanged);
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &ThemeProxy::smallestFontChanged);
}

ThemeProxy::~ThemeProxy()
{
}

QString ThemeProxy::themeName() const
{
    return m_theme->themeName();
}

QFont ThemeProxy::defaultFont() const
{
    return QApplication::font();
}

QFont ThemeProxy::smallestFont() const
{
    return KGlobalSettings::smallestReadableFont();
}

QSizeF ThemeProxy::mSize(const QFont &font) const
{
    return QFontMetrics(font).boundingRect("M").size();
}


bool ThemeProxy::useGlobalSettings() const
{
    return m_theme->useGlobalSettings();
}

QString ThemeProxy::wallpaperPath() const
{
    return m_theme->wallpaperPath();
}

QString ThemeProxy::wallpaperPathForSize(int width, int height) const
{
    return m_theme->wallpaperPath(QSize(width, height));
}

QColor ThemeProxy::textColor() const
{
    return m_theme->color(Plasma::Theme::TextColor);
}

QColor ThemeProxy::highlightColor() const
{
    return m_theme->color(Plasma::Theme::HighlightColor);
}

QColor ThemeProxy::backgroundColor() const
{
    return m_theme->color(Plasma::Theme::BackgroundColor);
}

QColor ThemeProxy::buttonTextColor() const
{
    return m_theme->color(Plasma::Theme::ButtonTextColor);
}

QColor ThemeProxy::buttonBackgroundColor() const
{
    return m_theme->color(Plasma::Theme::ButtonBackgroundColor);
}

QColor ThemeProxy::linkColor() const
{
    return m_theme->color(Plasma::Theme::LinkColor);
}

QColor ThemeProxy::visitedLinkColor() const
{
    return m_theme->color(Plasma::Theme::VisitedLinkColor);
}

QColor ThemeProxy::buttonHoverColor() const
{
    return m_theme->color(Plasma::Theme::ButtonHoverColor);
}

QColor ThemeProxy::buttonFocusColor() const
{
    return m_theme->color(Plasma::Theme::ButtonFocusColor);
}

QColor ThemeProxy::viewTextColor() const
{
    return m_theme->color(Plasma::Theme::ViewTextColor);
}

QColor ThemeProxy::viewBackgroundColor() const
{
    return m_theme->color(Plasma::Theme::ViewBackgroundColor);
}

QColor ThemeProxy::viewHoverColor() const
{
    return m_theme->color(Plasma::Theme::ViewHoverColor);
}

QColor ThemeProxy::viewFocusColor() const
{
    return m_theme->color(Plasma::Theme::ViewFocusColor);
}

QString ThemeProxy::styleSheet() const
{
    return m_theme->styleSheet(QString());
}

int ThemeProxy::smallIconSize() const
{
    return KIconLoader::SizeSmall;
}

int ThemeProxy::smallMediumIconSize() const
{
    return KIconLoader::SizeSmallMedium;
}

int ThemeProxy::mediumIconSize() const
{
    return KIconLoader::SizeMedium;
}

int ThemeProxy::largeIconSize() const
{
    return KIconLoader::SizeLarge;
}

int ThemeProxy::hugeIconSize() const
{
    return KIconLoader::SizeHuge;
}

int ThemeProxy::enormousIconSize() const
{
    return KIconLoader::SizeEnormous;
}

void ThemeProxy::iconLoaderSettingsChanged()
{
    m_defaultIconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);

    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));


    emit defaultIconSizeChanged();
    emit iconSizesChanged();
}

int ThemeProxy::defaultIconSize() const
{
    return m_defaultIconSize;
}

QQmlPropertyMap *ThemeProxy::iconSizes() const
{
    return m_iconSizes;
}

#include "moc_theme.cpp"

