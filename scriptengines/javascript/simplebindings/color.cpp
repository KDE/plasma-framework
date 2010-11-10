/*
 *   Copyright (c) 2009 Aaron J. Seigo <aseigo@kde.org>
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

#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QColor>

#include <KDebug>

#include <Plasma/Theme>

#include "backportglobal.h"

Q_DECLARE_METATYPE(QColor*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return qScriptValueFromValue(eng, QColor());
    } else if (ctx->argumentCount() == 1) {
        QString namedColor = ctx->argument(0).toString();
        return qScriptValueFromValue(eng, QColor(namedColor));
    }

    int r = 0;
    int g = 0;
    int b = 0;
    int a = 255;
    if (ctx->argumentCount() == 3) {
        r = ctx->argument(0).toInt32();
        g = ctx->argument(1).toInt32();
        b = ctx->argument(2).toInt32();
    }

    if (ctx->argumentCount() == 4) {
        a = ctx->argument(3).toInt32();
    }

    return qScriptValueFromValue(eng, QColor(r, g, b, a));
}

// red, green, blue, alpha, setRed, setGreen, setBlue, setAlpha, isValid, toString, name

static QScriptValue red(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, red);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setRed(arg.toInt32());
    }

    return QScriptValue(eng, self->red());
}

static QScriptValue green(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, green);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setGreen(arg.toInt32());
    }

    return QScriptValue(eng, self->green());
}

static QScriptValue blue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, blue);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setBlue(arg.toInt32());
    }

    return QScriptValue(eng, self->blue());
}

static QScriptValue alpha(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, alpha);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setAlpha(arg.toInt32());
    }

    return QScriptValue(eng, self->alpha());
}

static QScriptValue valid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, valid);
    return QScriptValue(eng, self->isValid());
}

static QScriptValue setThemeColor(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, themeColor);

    if (ctx->argumentCount() > 0) {
        const qint32 arg = ctx->argument(0).toInt32();
        if (arg >= 0 && arg <= Plasma::Theme::VisitedLinkColor) {
            kDebug() << "setting to: " << static_cast<Plasma::Theme::ColorRole>(arg);
            kDebug() << "color is: " << Plasma::Theme::defaultTheme()->color(static_cast<Plasma::Theme::ColorRole>(arg));
            self->setRgba(Plasma::Theme::defaultTheme()->color(static_cast<Plasma::Theme::ColorRole>(arg)).rgba());
        }
    }

    return ctx->thisObject();//.property("themeColor");
}

QScriptValue constructColorClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QColor());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("red", eng->newFunction(red), getter | setter);
    proto.setProperty("green", eng->newFunction(green), getter | setter);
    proto.setProperty("blue", eng->newFunction(blue), getter | setter);
    proto.setProperty("alpha", eng->newFunction(alpha), getter | setter);
    proto.setProperty("valid", eng->newFunction(valid), getter);
    ADD_METHOD(proto, setThemeColor);

    eng->setDefaultPrototype(qMetaTypeId<QColor>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QColor*>(), proto);

    return eng->newFunction(ctor, proto);
}
