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

#ifndef __ENGINE__
#define __ENGINE__

#include <QScriptEngine>
#include <QScriptString>
#include <QScriptValue>
#include <QScriptContext>
#include <QDebug>

/* Plasma-shell will have an engine */
class AnimationEngine
{
public:
    static QScriptEngine* globalEngine();
    static QScriptValue animation(const QString &anim);

protected:
    AnimationEngine() { }

private:
    static QScriptEngine *inst;
    static QScriptValue registerAnimation(QScriptContext *context, QScriptEngine *engine);
    static QHash<QString, QScriptValue> s_animFuncs;
};

#endif
