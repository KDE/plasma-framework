/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "scripting/appletscript.h"

#include "kconfig.h"

#include "applet.h"
#include "package.h"
#include "private/applet_p.h"

namespace Plasma
{

class AppletScriptPrivate
{
public:
    Applet *applet;
};

AppletScript::AppletScript(QObject *parent)
    : ScriptEngine(parent),
      d(new AppletScriptPrivate)
{
    d->applet = nullptr;
}

AppletScript::~AppletScript()
{
    delete d;
}

void AppletScript::setApplet(Plasma::Applet *applet)
{
    d->applet = applet;
}

Applet *AppletScript::applet() const
{
    Q_ASSERT(d->applet);
    return d->applet;
}

void AppletScript::constraintsEvent(Plasma::Types::Constraints constraints)
{
    Q_UNUSED(constraints);
}

QList<QAction *> AppletScript::contextualActions()
{
    return QList<QAction *>();
}

void AppletScript::setHasConfigurationInterface(bool hasInterface)
{
    if (applet()) {
        applet()->setHasConfigurationInterface(hasInterface);
    }
}

void AppletScript::setConfigurationRequired(bool req, const QString &reason)
{
    if (applet()) {
        applet()->setConfigurationRequired(req, reason);
    }
}

void AppletScript::setLaunchErrorMessage(const QString &reason)
{
    if (applet()) {
        applet()->setLaunchErrorMessage(reason);
    }
}

void AppletScript::configNeedsSaving() const
{
    if (applet()) {
        emit applet()->configNeedsSaving();
    }
}

void AppletScript::configChanged()
{
}

QString AppletScript::mainScript() const
{
    Q_ASSERT(d->applet);
    return d->applet->kPackage().filePath("mainscript");
}

Package AppletScript::package() const
{
    Q_ASSERT(d->applet);
    return d->applet->package();
}

KPluginInfo AppletScript::description() const
{
    Q_ASSERT(d->applet);
    return d->applet->pluginInfo();
}

Plasma::Types::ContainmentType AppletScript::containmentType() const
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        return cont->containmentType();
    } else {
        return Plasma::Types::NoContainmentType;
    }
}

void AppletScript::setContainmentType(Plasma::Types::ContainmentType type)
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        cont->setContainmentType(type);
    }
}

} // Plasma namespace

#include "moc_appletscript.cpp"
