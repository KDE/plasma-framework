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

#include "containment.h"

#include <QAction>

#include <Plasma/Corona>
#include <Plasma/Containment>

#include "scriptengine.h"
#include "widget.h"

Containment::Containment(Plasma::Containment *containment, QObject *parent)
    : Applet(parent),
      m_containment(containment)
{
    setCurrentConfigGroup(QStringList());
}

Containment::~Containment()
{
}

int Containment::screen() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment.data()->screen();
}

void Containment::setScreen(int screen)
{
    if (m_containment) {
        m_containment.data()->setScreen(screen);
    }
}
                
int Containment::desktop() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment.data()->desktop();
}

void Containment::setDesktop(int desktop)
{
    if (m_containment) {
        m_containment.data()->setScreen(m_containment.data()->screen(), desktop);
    }
}

QString Containment::formFactor() const
{
    if (!m_containment) {
        return "Planar";
    }

    switch (m_containment.data()->formFactor()) {
        case Plasma::Planar:
            return "planar";
            break;
        case Plasma::MediaCenter:
            return "mediacenter";
            break;
        case Plasma::Horizontal:
            return "horizontal";
            break;
        case Plasma::Vertical:
            return "vertical";
            break;
    }

    return "Planar";
}

QList<int> Containment::widgetIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> w;

    if (m_containment) {
        foreach (const Plasma::Applet *applet, m_containment.data()->applets()) {
            w.append(applet->id());
        }
    }

    return w;
}

QScriptValue Containment::widgetById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("widgetById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    Containment *c = qobject_cast<Containment*>(context->thisObject().toQObject());

    if (!c) {
        return engine->undefinedValue();
    }

    if (c->m_containment) {
        foreach (Plasma::Applet *w, c->m_containment.data()->applets()) {
            if (w->id() == id) {
                ScriptEngine *env = ScriptEngine::envFor(engine);
                return env->wrap(w, engine);
            }
        }
    }

    return engine->undefinedValue();
}

QScriptValue Containment::addWidget(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("widgetById requires a name of a widget or a widget object"));
    }

    Containment *c = qobject_cast<Containment*>(context->thisObject().toQObject());

    if (!c || !c->m_containment) {
        return engine->undefinedValue();
    }

    QScriptValue v = context->argument(0);
    Plasma::Applet *applet = 0;
    if (v.isString()) {
        applet = c->m_containment.data()->addApplet(v.toString());
        if (applet) {
            ScriptEngine *env = ScriptEngine::envFor(engine);
            return env->wrap(applet, engine);
        }
    } else if (Widget *widget = qobject_cast<Widget*>(v.toQObject())) {
        applet = widget->applet();
        c->m_containment.data()->addApplet(applet);
        return v;
    }

    return engine->undefinedValue();
}

uint Containment::id() const
{
    if (!m_containment) {
        return 0;
    }

    return m_containment.data()->id();
}

QString Containment::name() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment.data()->activity();
}

void Containment::setName(const QString &name)
{
    if (m_containment) {
        m_containment.data()->setActivity(name);
    }
}

QString Containment::type() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment.data()->pluginName();
}

void Containment::remove()
{
    if (m_containment) {
        m_containment.data()->destroy(false);
    }
}

void Containment::showConfigurationInterface()
{
    if (m_containment) {
        QAction *configAction = m_containment.data()->action("configure");
        if (configAction && configAction->isEnabled()) {
            configAction->trigger();
        }
    }
}

Plasma::Applet *Containment::applet() const
{
    return m_containment.data();
}

Plasma::Containment *Containment::containment() const
{
    return m_containment.data();
}

#include "containment.moc"

