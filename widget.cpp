/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "widget.h"

#include <QAction>
#include <QGraphicsLinearLayout>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

Widget::Widget(Plasma::Applet *applet, QObject *parent)
    : Applet(parent),
      m_applet(applet)
{
    setCurrentConfigGroup(QStringList());
}

Widget::~Widget()
{
}

uint Widget::id() const
{
    if (m_applet) {
        return m_applet.data()->id();
    }

    return 0;
}

QString Widget::type() const
{
    if (m_applet) {
        return m_applet.data()->pluginName();
    }

    return QString();
}

void Widget::remove()
{
    if (m_applet) {
        m_applet.data()->destroy();
        m_applet.clear();
    }
}

void Widget::setGlobalShortcut(const QString &shortcut)
{
    if (m_applet) {
        m_applet.data()->setGlobalShortcut(KShortcut(shortcut));
    }
}

QString Widget::globalShorcut() const
{
    if (m_applet) {
        return m_applet.data()->globalShortcut().toString();
    }

    return QString();
}

Plasma::Applet *Widget::applet() const
{
    return m_applet.data();
}

int Widget::index() const
{
    if (!m_applet) {
        return -1;
    }

    Plasma::Applet *applet = m_applet.data();
    Plasma::Containment *c = applet->containment();
    if (!c) {
        return -1;
    }

    QGraphicsLayout *layout = c->layout();
    if (!layout) {
        return - 1;
    }

    for (int i = 0; i < layout->count(); ++i) {
        if (layout->itemAt(i) == applet) {
            return i;
        }
    }

    return -1;
}

void Widget::setIndex(int index)
{
    if (!m_applet) {
        return;
    }

    Plasma::Applet *applet = m_applet.data();
    Plasma::Containment *c = applet->containment();
    if (!c) {
        return;
    }

    //FIXME: this is hackish. would be nice to define this for gridlayouts too
    QGraphicsLinearLayout *layout = dynamic_cast<QGraphicsLinearLayout *>(c->layout());
    if (!layout) {
        return;
    }

    layout->insertItem(index, applet);
}

QRectF Widget::geometry() const
{
    if (m_applet) {
        return m_applet.data()->geometry();
    }

    return QRectF();
}

void Widget::setGeometry(const QRectF &geometry)
{
    if (m_applet) {
        m_applet.data()->setGeometry(geometry);
    }
}

void Widget::showConfigurationInterface()
{
    if (m_applet) {
        m_applet.data()->showConfigurationInterface();
    }
}


#include "widget.moc"

