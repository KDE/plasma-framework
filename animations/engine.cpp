/*
 * Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com>
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

#include "engine.h"

QScriptEngine* AnimationEngine::inst = 0;
QHash<QString, QScriptValue> AnimationEngine::s_animFuncs;

QScriptValue AnimationEngine::animation(const QString &anim)
{
    return s_animFuncs.value(anim);
}

QScriptValue AnimationEngine::registerAnimation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() > 1) {
        const QString name = context->argument(0).toString();
        const QScriptValue func = context->argument(1);
        if (func.isFunction()) {
            s_animFuncs.insert(name, func);
        }
    }

    return engine->undefinedValue();
}

QScriptEngine *AnimationEngine::globalEngine()
{
    if (!inst) {
        inst = new QScriptEngine;
        QScriptValue global = inst->globalObject();
        global.setProperty("registerAnimation", inst->newFunction(AnimationEngine::registerAnimation));
	qDebug() << "........... first js animation, creating the engine!";
    }

    return inst;
}

