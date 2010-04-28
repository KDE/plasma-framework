/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Adenilson Cavalcanti <cavalcantii@gmail.com>
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

/* TODO:
 *
 * - cleanup debug messages
 */

#include "animationscriptengine_p.h"

#include <QFile>
#include <QTextStream>

#include <kdebug.h>

namespace Plasma
{

namespace AnimationScriptEngine
{

QScriptEngine* inst = 0;
QHash<QString, QScriptValue> s_animFuncs;

QScriptValue animation(const QString &anim)
{
    return s_animFuncs.value(anim);
}

bool isAnimationRegistered(const QString &anim)
{
    return s_animFuncs.contains(anim);
}

void clearAnimations()
{
    s_animFuncs.clear();
    delete inst;
    inst = 0;
}

QScriptValue registerAnimation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() > 1) {
        const QString name = context->argument(0).toString();

        if (!s_animFuncs.contains(name)) {
            const QScriptValue func = context->argument(1);
            if (func.isFunction()) {
                s_animFuncs.insert(name, func);
            }
        }
    }

    return engine->undefinedValue();
}

QScriptEngine *globalEngine()
{
    if (!inst) {
        inst = new QScriptEngine;
        QScriptValue global = inst->globalObject();
        global.setProperty("registerAnimation", inst->newFunction(AnimationScriptEngine::registerAnimation));
	qDebug() << "........... first js animation, creating the engine!";
    }

    return inst;
}

bool loadScript(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "failed to open script file" << path;
        return false;
    }

    QTextStream buffer(&file);
    QString tmp(buffer.readAll());

    QScriptEngine *engine = AnimationScriptEngine::globalEngine();
    QScriptValue def(engine->evaluate(tmp, path));
    if (engine->hasUncaughtException()) {
        const QScriptValue error = engine->uncaughtException();
        QString file = error.property("fileName").toString();
        const QString failureMsg = QString("Error in %1 on line %2.\n%3")
                                          .arg(file)
                                          .arg(error.property("lineNumber").toString())
                                          .arg(error.toString());
        kError() << failureMsg;
        return false;
    }

    return true;
}

} // namespace AnimationEngine
} // namespace Plasma

