/***********************************************************************/
/* engine.h 							       */
/* 								       */
/* Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com>   */
/* 								       */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public	       */
/* License as published by the Free Software Foundation; either	       */
/* version 2.1 of the License, or (at your option) any later version.  */
/*   								       */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.		       */
/*  								       */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA						       */
/***********************************************************************/

/**
 * @file   engine.h
 * @author Adenilson Cavalcanti
 * @date   Wed Feb 24 21:49:45 2010
 *
 * @brief
 *
 *
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

