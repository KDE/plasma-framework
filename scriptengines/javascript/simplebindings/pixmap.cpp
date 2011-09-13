/*
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
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
#include <QtGui/QPixmap>
#include "backportglobal.h"
#include "plasmoid/appletinterface.h"

Q_DECLARE_METATYPE(QPixmap*)
Q_DECLARE_METATYPE(QPixmap)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 1 && ctx->argument(0).isString()) {
        // a path on disk in the package
        AppletInterface *interface = AppletInterface::extract(eng);
        const QString path = interface ? interface->file("images", ctx->argument(0).toString()) : QString();
        return qScriptValueFromValue(eng, QPixmap(path));
    }

    if (ctx->argumentCount() == 2) {
        qreal x = ctx->argument(0).toNumber();
        qreal y = ctx->argument(1).toNumber();
        return qScriptValueFromValue(eng, QPixmap(x, y));
    }

    return qScriptValueFromValue(eng, QPixmap());
}

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, rect)
    return qScriptValueFromValue(eng, QRectF(self->rect()));
}

static QScriptValue null(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, null);
    return QScriptValue(eng, self->isNull());
}

static QScriptValue scaled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, scaled);
    qreal x = ctx->argument(0).toNumber();
    qreal y = ctx->argument(1).toNumber();
    return qScriptValueFromValue(eng, self->scaled(x, y));
}

QScriptValue constructQPixmapClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QPixmap());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    proto.setProperty("null", eng->newFunction(null), getter);
    proto.setProperty("rect", eng->newFunction(rect), getter);
    proto.setProperty("scaled", eng->newFunction(scaled));

    eng->setDefaultPrototype(qMetaTypeId<QPixmap>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPixmap*>(), proto);

    return eng->newFunction(ctor, proto);
}
