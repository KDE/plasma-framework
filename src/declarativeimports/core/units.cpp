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
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <cmath>

#include <KIconLoader>

Units::Units (QObject *parent)
    : QObject(parent),
      m_gridUnit(-1)
{
    //Usual "default" is 96 dpi
    //that magic ratio follows the definition of "device independent pixel" by Microsoft
    m_dpi = QApplication::desktop()->physicalDpiX();
    m_dpiScale = (qreal)m_dpi / (qreal)96;

    updateSpacing();

    m_iconSizes = new QQmlPropertyMap(this);
    iconLoaderSettingsChanged();

    connect(KIconLoader::global(), SIGNAL(iconLoaderSettingsChanged()), this, SLOT(iconLoaderSettingsChanged()));

    themeChanged();
    connect(&m_theme, SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
    installEventFilter(qApp);
}

Units::~Units()
{
}

void Units::iconLoaderSettingsChanged()
{
    m_iconSizes->insert("default", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));
    m_iconSizes->insert("toolbar", KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    m_iconSizes->insert("small", KIconLoader::global()->currentSize(KIconLoader::Small));
    m_iconSizes->insert("dialog", KIconLoader::global()->currentSize(KIconLoader::Dialog));

    m_iconSizes->insert("smallMedium", KIconLoader::SizeSmallMedium);
    m_iconSizes->insert("medium", KIconLoader::SizeMedium);
    m_iconSizes->insert("large", KIconLoader::SizeLarge);
    m_iconSizes->insert("huge", KIconLoader::SizeHuge);
    m_iconSizes->insert("enormous", KIconLoader::SizeEnormous);

    emit iconSizesChanged();
}

QQmlPropertyMap *Units::iconSizes() const
{
    return m_iconSizes;
}

qreal Units::dpiScale() const
{

    return m_dpiScale;
}

void Units::setDpiScale(const qreal scale)
{
    if (m_dpiScale != scale) {
        m_dpiScale = scale;
        qDebug() << "Setting dpi scale to " << scale;
        emit dpiScaleChanged();
    }
}

qreal Units::gridUnit() const
{
    qDebug() << "FontMetrics: " << QApplication::font().pixelSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " like spacing" << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
    qDebug() << "m_dpi: " << m_dpi;
    qDebug() << "m_dpiScale: " << m_dpiScale;
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").width();
    return m_gridUnit;
}

qreal Units::dp(qreal value) const
{
    if (value <= 2.0) {
        return qRound(value * floor(m_dpiScale));
    } else {
        return qRound(value * m_dpiScale);
    }
}

qreal Units::gu(qreal value) const
{
    return qRound(m_gridUnit * value);
}

void Units::themeChanged()
{
    const int gridUnit = QFontMetrics(QApplication::font()).boundingRect("M").height();
    if (gridUnit != m_gridUnit) {
        m_gridUnit = gridUnit;
        emit gridUnitChanged();
    }
}

qreal Units::dpi(QQuickItem* item)
{
    int  _dpi = 96;
    if (item) {
        QScreen* screen = item->window()->screen();
        if (screen) {
            _dpi = screen->physicalDotsPerInch();
        }
    }
    return _dpi;
}

void Units::printScreenInfo(QQuickItem* item)
{
    int  _dpi = dpi(item);
    qDebug() << " ----- printScreenInfo() ---- ";
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
    qDebug() << "FontMetrics: " << QApplication::font().pointSize() << QFontMetrics(QApplication::font()).boundingRect("M");
    qDebug() << " MRect" << QFontMetrics(QApplication::font()).boundingRect("M").size();
    qDebug() << " gridUnit: " << QFontMetrics(QApplication::font()).boundingRect("M").size().height();
}

int Units::smallSpacing() const
{
    return m_smallSpacing;
}

int Units::largeSpacing() const
{
    return m_largeSpacing;
}

void Units::updateSpacing()
{
    const int _s = QFontMetrics(QApplication::font()).boundingRect("M").size().height();
    if (_s != m_largeSpacing) {
        m_smallSpacing = qMax(2, (int)(_s / 8)); // 1/8 of msize.height, at least 2
        m_largeSpacing = _s; // msize.height
        emit spacingChanged();
    }
}

bool Units::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
            updateSpacing();
        }
    }
    return QObject::eventFilter(watched, event);
}

#include "units.moc"

