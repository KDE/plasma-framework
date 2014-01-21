/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org            >                *
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

#include "units.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QtGlobal>
#include <cmath>

#include <KIconLoader>

Units::Units (QObject *parent)
    : QObject(parent),
      m_gridUnit(-1)
{
    m_iconSizes = new QQmlPropertyMap(this);
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("panel", QVariant(KIconLoader::global()->currentSize(KIconLoader::Panel)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()), this, SLOT(iconLoaderSettingsChanged()));

    themeChanged();
    connect(&m_theme, SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
}

Units::~Units()
{
}

void Units::iconLoaderSettingsChanged()
{
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    emit iconSizesChanged();
}

QQmlPropertyMap *Units::iconSizes() const
{
    return m_iconSizes;
}


qreal Units::dpiRatio() const
{
    const qreal ratio = (qreal)QApplication::desktop()->physicalDpiX() / (qreal)96;
    return ratio;
}

qreal Units::gridUnit() const
{
    qDebug() << "FontMetrics: " << QApplication::font().pixelSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " like spacing" << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
    
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").width();
    return m_gridUnit;
}

qreal Units::dp(qreal value) const
{
    //Usual "default" is 96 dpi
    //that magic ratio follows the definition of "device independent pixel" by Microsoft
    const qreal ratio = (qreal)QApplication::desktop()->physicalDpiX() / (qreal)96;

    if (value <= 2.0) {
        return qRound(value * floor(ratio));
    } else {
        return qRound(value * ratio);
    }
}

qreal Units::gu(qreal value) const
{
    return qRound(m_gridUnit * value);
}

void Units::themeChanged()
{
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").height();
    qDebug() << "FontMetrics: " << QApplication::font().pixelSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " like spacing" << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
    if (gridUnit != m_gridUnit) {
        m_gridUnit = gridUnit;
        emit gridUnitChanged();
    }
}

#include "units.moc"

