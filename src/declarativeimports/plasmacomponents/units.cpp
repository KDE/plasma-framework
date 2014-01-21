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


Units::Units (QObject *parent)
    : QObject(parent),
      m_gridUnit(-1)
{
    themeChanged();
    connect(&m_theme, SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
}

Units::~Units()
{
}

qreal Units::gridUnit() const
{
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").width();
    qDebug() << "FontMetrics: " << QApplication::font().pixelSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " like spacing" << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
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
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").width();
    qDebug() << "FontMetrics: " << QApplication::font().pixelSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " like spacing" << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
    if (gridUnit != m_gridUnit) {
        m_gridUnit = gridUnit;
        emit gridUnitChanged();
    }
}

#include "units.moc"

