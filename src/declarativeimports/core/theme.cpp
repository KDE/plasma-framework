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

#include <QFontMetrics>
#include <kiconloader.h>
#include <QApplication>
#include <QScreen>
#include <QQuickWindow>

#include <QDebug>

//********** Theme *************

ThemeProxy::ThemeProxy(QQmlEngine *parent)
    : Plasma::Theme(parent),
      m_engine(parent)
{
    m_iconSizes = new QQmlPropertyMap(this);
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("panel", QVariant(KIconLoader::global()->currentSize(KIconLoader::Panel)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    connect(this, &Plasma::Theme::themeChanged, this, &ThemeProxy::themeChanged);
    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()), this, SLOT(iconLoaderSettingsChanged()));
}

ThemeProxy::~ThemeProxy()
{
}

void ThemeProxy::iconLoaderSettingsChanged()
{
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    emit defaultIconSizeChanged();
    emit iconSizesChanged();
}

QQmlPropertyMap *ThemeProxy::iconSizes() const
{
    return m_iconSizes;
}

int ThemeProxy::dpi(QQuickItem* item)
{
    int  _dpi = 1337;
    qDebug() << " ----- dpi() ---- ";
    if (item) {
        QScreen* screen = item->window()->screen();
        if (screen) {
            qDebug() << "screen geo: " << screen->availableGeometry();
            _dpi = screen->physicalDotsPerInch();
            qDebug() << "   refreshRate     : " << screen->refreshRate();
            qDebug() << "   devicePixelRatio: " << screen->devicePixelRatio();
            qDebug() << "   depth           : " << screen->depth();
            qDebug() << "   dpi X:            " << screen->physicalDotsPerInchX();
            qDebug() << "   dpi Y:            " << screen->physicalDotsPerInchY();
            qDebug() << "   ->> dpi:          " << _dpi;
        }
    }
    return _dpi;
}



#include "moc_theme.cpp"
