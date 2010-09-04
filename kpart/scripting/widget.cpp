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

#include <QGraphicsLinearLayout>

#include <KService>
#include <KServiceTypeTrader>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

namespace PlasmaKPartScripting
{

class Widget::Private
{
public:
    Private()
        : configDirty(false)
    {
    }

    QWeakPointer<Plasma::Applet> applet;

    KConfigGroup configGroup;
    QStringList configGroupPath;
    KConfigGroup globalConfigGroup;
    QStringList globalConfigGroupPath;
    bool configDirty : 1;
};

Widget::Widget(Plasma::Applet *applet, QObject *parent)
    : QObject(parent),
      d(new Widget::Private)
{
    d->applet = applet;
}

Widget::~Widget()
{
    if (d->configDirty) {
        reloadConfig();
    }

    delete d;
}

void Widget::setCurrentConfigGroup(const QStringList &groupNames)
{
    Plasma::Applet *app = applet();
    if (!app) {
        d->configGroup = KConfigGroup();
        d->configGroupPath.clear();
        return;
    }

    d->configGroup = app->config();
    d->configGroupPath = groupNames;

    foreach (const QString &groupName, groupNames) {
        d->configGroup = KConfigGroup(&d->configGroup, groupName);
    }
}

QStringList Widget::currentConfigGroup() const
{
    return d->configGroupPath;
}

QStringList Widget::configKeys() const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.keyList();
    }

    return QStringList();
}

QStringList Widget::configGroups() const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.groupList();
    }

    return QStringList();
}

QVariant Widget::readConfig(const QString &key, const QVariant &def) const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.readEntry(key, def);
    } else {
        return QVariant();
    }
}

void Widget::writeConfig(const QString &key, const QVariant &value)
{
    if (d->configGroup.isValid()) {
        d->configGroup.writeEntry(key, value);
        d->configDirty = true;
    }
}

void Widget::setCurrentGlobalConfigGroup(const QStringList &groupNames)
{
    Plasma::Applet *app = applet();
    if (!app) {
        d->globalConfigGroup = KConfigGroup();
        d->globalConfigGroupPath.clear();
        return;
    }

    d->globalConfigGroup = app->globalConfig();
    d->globalConfigGroupPath = groupNames;

    foreach (const QString &groupName, groupNames) {
        d->globalConfigGroup = KConfigGroup(&d->globalConfigGroup, groupName);
    }
}

QStringList Widget::currentGlobalConfigGroup() const
{
    return d->globalConfigGroupPath;
}

QStringList Widget::globalConfigKeys() const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.keyList();
    }

    return QStringList();
}

QStringList Widget::globalConfigGroups() const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.groupList();
    }

    return QStringList();
}

QVariant Widget::readGlobalConfig(const QString &key, const QVariant &def) const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.readEntry(key, def);
    } else {
        return QVariant();
    }
}

void Widget::writeGlobalConfig(const QString &key, const QVariant &value)
{
    if (d->globalConfigGroup.isValid()) {
        d->globalConfigGroup.writeEntry(key, value);
        d->configDirty = true;
    }
}

void Widget::reloadConfig()
{
    Plasma::Applet *app = applet();
    if (app) {
        KConfigGroup cg = app->config();

        if (!app->isContainment()) {
            app->restore(cg);
        }

        app->configChanged();

        if (app->containment() && app->containment()->corona()) {
            app->containment()->corona()->requestConfigSync();
        }

        d->configDirty = false;
    }
}

QString Widget::version() const
{
    Plasma::Applet *app = applet();
    if (!app) {
        return QString();
    }

    QString type = app->pluginName();
    KService::List services = KServiceTypeTrader::self()->query("Plasma/Applet", "[X-KDE-PluginInfo-Name] == '" + type + '\'');
    if (services.isEmpty()) {
        return QString();
    }

    KPluginInfo info(services.first());
    return info.version();
}

Plasma::Applet *Widget::applet() const
{
    return d->applet.data();
}

uint Widget::id() const
{
    if (d->applet) {
        return d->applet.data()->id();
    }

    return 0;
}

QString Widget::type() const
{
    if (d->applet) {
        return d->applet.data()->pluginName();
    }

    return QString();
}

void Widget::remove()
{
    if (d->applet) {
        d->applet.data()->destroy();
        d->applet.clear();
    }
}

QRectF Widget::geometry() const
{
    if (d->applet) {
        return d->applet.data()->geometry();
    }

    return QRectF();
}

void Widget::setGeometry(const QRectF &geometry)
{
    if (d->applet) {
        d->applet.data()->setGeometry(geometry);
    }
}

void Widget::showConfigurationInterface()
{
    if (d->applet) {
        d->applet.data()->showConfigurationInterface();
    }
}

}

#include "widget.moc"

