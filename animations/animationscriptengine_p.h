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

#ifndef __ENGINE__
#define __ENGINE__

#include <QScriptEngine>
#include <QScriptString>
#include <QScriptValue>
#include <QScriptContext>
#include <QDebug>

/* Plasma-shell will have an engine */
namespace Plasma
{

namespace AnimationScriptEngine
{

void clearAnimations();
bool isAnimationRegistered(const QString &anim);
QScriptEngine* globalEngine();
QScriptValue animation(const QString &anim);
bool loadScript(const QString &path, const QString &prefix = QString());
void addToLoadFailures(const QString &anim);
bool animationFailedToLoad(const QString &anim);

}

} // namespace Plasma

#endif
