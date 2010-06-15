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
#include <QMetaEnum>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include "animator.h"
#include "javascriptanimation_p.h"
#include "bindings/animationgroup_p.h"

namespace Plasma
{

QScriptValue constructEasingCurveClass(QScriptEngine *engine);

namespace AnimationScriptEngine
{

QScriptEngine* inst = 0;
QHash<QString, QScriptValue> s_animFuncs;
QSet<QString> s_animFailures;
QString s_prefix;

QScriptValue animation(const QString &anim)
{
    return s_animFuncs.value(anim);
}

bool isAnimationRegistered(const QString &anim)
{
    return s_animFuncs.contains(anim);
}

void addToLoadFailures(const QString &anim)
{
    s_animFailures.insert(anim);
}

bool animationFailedToLoad(const QString &anim)
{
    return s_animFailures.contains(anim);
}

void clearAnimations()
{
    s_animFuncs.clear();
    s_animFailures.clear();
    delete inst;
    inst = 0;
}

QScriptValue registerAnimation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() > 1) {
        const QString name = s_prefix + context->argument(0).toString();

        if (!s_animFuncs.contains(name)) {
            const QScriptValue func = context->argument(1);
            if (func.isFunction()) {
                s_animFuncs.insert(name, func);
            }
        }
    }

    return engine->undefinedValue();
}

QObject *extractParent(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    return context->thisObject().property("__plasma_javascriptanimation").toQObject();
}

QScriptValue animationGroup(QScriptContext *context, QScriptEngine *engine)
{
    QObject *parent = extractParent(context, engine);
    if (!parent) {
        return engine->undefinedValue();
    }

    QSequentialAnimationGroup *group = new SequentialAnimationGroup(parent);
    return engine->newQObject(group);
}

QScriptValue parallelAnimationGroup(QScriptContext *context, QScriptEngine *engine)
{
    QObject *parent = extractParent(context, engine);
    if (!parent) {
        return engine->undefinedValue();
    }

    ParallelAnimationGroup *group = new ParallelAnimationGroup(parent);
    return engine->newQObject(group);
}

void registerEnums(QScriptValue &scriptValue, const QMetaObject &meta)
{
    //manually create enum values. ugh
    QScriptEngine *engine = scriptValue.engine();
    for (int i = 0; i < meta.enumeratorCount(); ++i) {
        QMetaEnum e = meta.enumerator(i);
        //kDebug() << e.name();
        for (int i=0; i < e.keyCount(); ++i) {
            //kDebug() << e.key(i) << e.value(i);
            scriptValue.setProperty(e.key(i), QScriptValue(engine, e.value(i)));
        }
    }
}

QScriptValue animation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("animation() takes one argument"));
    }

    QObject *parent = extractParent(context, engine);
    QAbstractAnimation *anim = 0;
    if (context->argument(0).isString()) {
        const QString animName = context->argument(0).toString();
        anim = Plasma::Animator::create(animName, parent);
    } else {
        int animId = context->argument(0).toInt32();
        if (animId == JavascriptAnimation::PauseAnimation) {
            anim = new QPauseAnimation(parent);
        } else if (animId == JavascriptAnimation::PropertyAnimation) {
            anim = new QPropertyAnimation(parent);
        } else {
            anim = Plasma::Animator::create(static_cast<Animator::Animation>(animId), parent);
        }
    }

    if (anim) {
        QScriptValue value = engine->newQObject(anim);
        registerEnums(value, *anim->metaObject());
        return value;
    }

    return context->throwError(i18n("%1 is not a known animation type", context->argument(0).isString()));
}

QScriptEngine *globalEngine()
{
    if (!inst) {
        inst = new QScriptEngine;
        QScriptValue global = inst->globalObject();
        global.setProperty("registerAnimation", inst->newFunction(AnimationScriptEngine::registerAnimation));
        global.setProperty("AnimationGroup", inst->newFunction(AnimationScriptEngine::animationGroup));
        global.setProperty("ParallelAnimationGroup", inst->newFunction(AnimationScriptEngine::parallelAnimationGroup));
        global.setProperty("QEasingCurve", constructEasingCurveClass(inst));
        kDebug() << "........... first js animation, creating the engine!";
    }

    return inst;
}

bool loadScript(const QString &path, const QString &prefix)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "failed to open script file" << path;
        return false;
    }

    QTextStream buffer(&file);
    QString tmp(buffer.readAll());

    QScriptEngine *engine = AnimationScriptEngine::globalEngine();
    s_prefix = prefix;
    QScriptValue def(engine->evaluate(tmp, path));
    s_prefix.clear();
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

