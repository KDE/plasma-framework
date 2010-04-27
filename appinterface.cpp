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

#include "appinterface.h"

#include <QEventLoop>
#include <QTimer>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/DataEngineManager>

#include "scriptengine.h"

AppInterface::AppInterface(Plasma::Corona *corona, QObject *parent)
    : QObject(parent),
      m_corona(corona)
{

}

int AppInterface::screenCount() const
{
    return m_corona->numScreens();
}

QRectF AppInterface::screenGeometry(int screen) const
{
    return m_corona->screenGeometry(screen);
}

QList<int> AppInterface::activityIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> containments;

    foreach (Plasma::Containment *c, m_corona->containments()) {
        if (!ScriptEngine::isPanel(c)) {
            containments.append(c->id());
        }
    }

    return containments;
}

QList<int> AppInterface::panelIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> panels;

    foreach (Plasma::Containment *c, m_corona->containments()) {
        //kDebug() << "checking" << (QObject*)c << isPanel(c);
        if (ScriptEngine::isPanel(c)) {
            panels.append(c->id());
        }
    }

    return panels;
}

void AppInterface::lockCorona(bool locked)
{
    m_corona->setImmutability(locked ? Plasma::UserImmutable : Plasma::Mutable);
}

bool AppInterface::coronaLocked() const
{
    return m_corona->immutability() != Plasma::Mutable;
}

void AppInterface::sleep(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

bool AppInterface::hasBattery() const
{
  Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
  Plasma::DataEngine *power = engines->loadEngine("powermanagement");

  const QStringList batteries = power->query("Battery")["sources"].toStringList();
  engines->unloadEngine("powermanagement");
  return !batteries.isEmpty();
}

QStringList AppInterface::knownWidgetTypes() const
{
    QStringList widgets;
    KPluginInfo::List info = Plasma::Applet::listAppletInfo();

    foreach (const KPluginInfo &info, info) {
        widgets.append(info.pluginName());
    }

    return widgets;
}

QStringList AppInterface::knownActivityTypes() const
{
    return knownContainmentTypes("desktop");
}

QStringList AppInterface::knownPanelTypes() const
{
    return knownContainmentTypes("panel");
}

QStringList AppInterface::knownContainmentTypes(const QString &type) const
{
    QStringList containments;
    KPluginInfo::List info = Plasma::Containment::listContainmentsOfType(type);

    foreach (const KPluginInfo &info, info) {
        containments.append(info.pluginName());
    }

    return containments;
}

#include "appinterface.moc"

