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
    : Plasma::Theme(parent)
{
    m_defaultIconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);

    m_iconSizes = new QQmlPropertyMap(this);
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("panel", QVariant(KIconLoader::global()->currentSize(KIconLoader::Panel)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    connect(this, &Plasma::Theme::themeChanged, this, &ThemeProxy::themeChanged);
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()), this, SLOT(iconLoaderSettingsChanged()));

    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &ThemeProxy::defaultFontChanged);
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &ThemeProxy::smallestFontChanged);
}

ThemeProxy::~ThemeProxy()
{
}

QString ThemeProxy::themeName() const
{
    return Plasma::Theme::themeName();
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
    return Plasma::Theme::useGlobalSettings();
}

QString ThemeProxy::wallpaperPath() const
{
    return Plasma::Theme::wallpaperPath();
}

QString ThemeProxy::wallpaperPathForSize(int width, int height) const
{
    return Plasma::Theme::wallpaperPath(QSize(width, height));
}

QColor ThemeProxy::textColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor);
}

QColor ThemeProxy::highlightColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightColor);
}

QColor ThemeProxy::backgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor);
}

QColor ThemeProxy::buttonTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonTextColor);
}

QColor ThemeProxy::buttonBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonBackgroundColor);
}

QColor ThemeProxy::linkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::LinkColor);
}

QColor ThemeProxy::visitedLinkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::VisitedLinkColor);
}

QColor ThemeProxy::buttonHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonHoverColor);
}

QColor ThemeProxy::buttonFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonFocusColor);
}

QColor ThemeProxy::viewTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewTextColor);
}

QColor ThemeProxy::viewBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewBackgroundColor);
}

QColor ThemeProxy::viewHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewHoverColor);
}

QColor ThemeProxy::viewFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewFocusColor);
}

QString ThemeProxy::styleSheet() const
{
    return Plasma::Theme::styleSheet(QString());
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

