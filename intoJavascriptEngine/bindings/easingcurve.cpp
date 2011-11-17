/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QEasingCurve>
#include <QMetaEnum>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptContext>
#include <QScriptable>

Q_DECLARE_METATYPE(QEasingCurve)
Q_DECLARE_METATYPE(QEasingCurve*)
#define ADD_ENUM_VALUE(__c__, __ns__, __v__) \
    __c__.setProperty(#__v__, QScriptValue(__c__.engine(), __ns__::__v__))

#define DECLARE_SELF(Class, __fn__) \
    Class* self = qscriptvalue_cast<Class*>(ctx->thisObject()); \
    if (!self) { \
        return ctx->throwError(QScriptContext::TypeError, \
            QString::fromLatin1("%0.prototype.%1: this object is not a %0") \
            .arg(#Class).arg(#__fn__)); \
    }

namespace Plasma
{

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        if (arg.isNumber()) {
            qint32 type = arg.toInt32();
            if (type > -1 && type < QEasingCurve::Custom) {
                return qScriptValueFromValue(eng, QEasingCurve(static_cast<QEasingCurve::Type>(type)));
            }
        }
    }

    return qScriptValueFromValue(eng, QEasingCurve());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QEasingCurve, toString);
    return QScriptValue(eng, QString::fromLatin1("QEasingCurve(type=%0)").arg(self->type()));
}

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QEasingCurve, type);

    if (ctx->argumentCount()) {
        QScriptValue arg = ctx->argument(0);

        qint32 type = -1;
        if (arg.isNumber()) {
            type = arg.toInt32();
        } else if (arg.isString()) {
            QMetaObject meta = QEasingCurve::staticMetaObject;
            QMetaEnum easingCurveEnum = meta.enumerator(meta.indexOfEnumerator("Type"));

            type = easingCurveEnum.keyToValue(arg.toString().toAscii().data());
        }
        if (type > -1 && type < QEasingCurve::Custom) {
            self->setType(static_cast<QEasingCurve::Type>(type));
        }
    }

    return QScriptValue(eng, self->type());
}

static QScriptValue valueForProgress(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QEasingCurve, valueForProgress);
    if (ctx->argumentCount() < 1 || !ctx->argument(0).isNumber()) {
        return eng->undefinedValue();
    }

    return self->valueForProgress(ctx->argument(0).toNumber());
}

QScriptValue constructEasingCurveClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QEasingCurve());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;

    proto.setProperty("type", eng->newFunction(type), getter | setter);
    proto.setProperty("toString", eng->newFunction(toString), getter);
    proto.setProperty("valueForProgress", eng->newFunction(valueForProgress), getter);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, Linear);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InQuad);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutQuad);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutQuad);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInQuad);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InCubic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutCubic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutCubic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInCubic);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InQuart);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutQuart);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutQuart);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInQuart);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InQuint);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutQuint);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutQuint);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInQuint);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InSine);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutSine);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutSine);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInSine);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InExpo);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutExpo);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutExpo);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInExpo);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InCirc);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutCirc);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutCirc);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInCirc);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InElastic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutElastic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutElastic);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInElastic);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InBack);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutBack);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutBack);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutInBack);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InBounce);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutBounce);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InOutBounce);

    ADD_ENUM_VALUE(ctorFun, QEasingCurve, InCurve);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, OutCurve);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, SineCurve);
    ADD_ENUM_VALUE(ctorFun, QEasingCurve, CosineCurve);

    eng->setDefaultPrototype(qMetaTypeId<QEasingCurve>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QEasingCurve*>(), proto);

    return ctorFun;
}

}

