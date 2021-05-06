/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scripting/appletscript.h"

#include "kconfig.h"

#include "applet.h"
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
#include "package.h"
#endif
#include "private/applet_p.h"

namespace Plasma
{
class AppletScriptPrivate
{
public:
    Applet *applet;
};

AppletScript::AppletScript(QObject *parent)
    : ScriptEngine(parent)
    , d(new AppletScriptPrivate)
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
        Q_EMIT applet()->configNeedsSaving();
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
Package AppletScript::package() const
{
    Q_ASSERT(d->applet);
    return d->applet->package();
}
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 81)
KPluginInfo AppletScript::description() const
{
    Q_ASSERT(d->applet);
    return d->applet->pluginInfo();
}
#endif

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
