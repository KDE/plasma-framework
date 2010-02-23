/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
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
#include <QtGui/QGraphicsAnchorLayout>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsWidget>

#include <Plasma/Applet>

#include "../backportglobal.h"
#include "appletinterface.h"

Q_DECLARE_METATYPE(QScript::Pointer<QGraphicsItem>::wrapped_pointer_type)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsLayout*)
Q_DECLARE_METATYPE(QGraphicsLayoutItem*)
Q_DECLARE_METATYPE(QGraphicsAnchorLayout*)
Q_DECLARE_METATYPE(QGraphicsGridLayout*)
DECLARE_POINTER_METATYPE(QGraphicsLinearLayout)

DECLARE_VOID_NUMBER_METHOD(QGraphicsLinearLayout, removeAt)
DECLARE_VOID_NUMBER_METHOD(QGraphicsLinearLayout, addStretch)
DECLARE_VOID_NUMBER_NUMBER_METHOD(QGraphicsLinearLayout, insertStretch)
DECLARE_VOID_NUMBER_NUMBER_METHOD(QGraphicsLinearLayout, setItemSpacing)
DECLARE_VOID_QUAD_NUMBER_METHOD(QGraphicsLinearLayout, setContentsMargins)

/////////////////////////////////////////////////////////////

//    Q_DECLARE_METATYPE(QGraphicsLayoutItem*)
QGraphicsLayoutItem *extractLayoutItem(QScriptContext *ctx, int index = 0)
{
    QScriptValue v = ctx->argument(index);
    if (ctx->argumentCount() == 0 || v.isQObject()) {
        QObject *object = v.toQObject();
        QGraphicsWidget *w = qobject_cast<QGraphicsWidget *>(object);
        if (!w) {
            AppletInterface *interface = qobject_cast<AppletInterface*>(object);
            if (!interface) {
                interface =  qobject_cast<AppletInterface*>(ctx->engine()->globalObject().property("plasmoid").toQObject());
            }

            if (interface) {
                w = interface->applet();
            }
        }

        return w;
    }

    QVariant variant = v.toVariant();
    QGraphicsLayoutItem *item = variant.value<QGraphicsLayoutItem *>();
    //this is horribly ugly code, but when a QLinearLayout* is stuffed into a QVariant,
    //QVariant does not know that it is a QGraphicsLayoutItem. repeat for all subclasses
    //of same
    if (!item) {
        item = variant.value<QGraphicsLayout *>();

        if (!item) {
            item = variant.value<QGraphicsLinearLayout *>();

            if (!item) {
                item = variant.value<QGraphicsGridLayout *>();

                if (!item) {
                    item = variant.value<QGraphicsAnchorLayout *>();
                }
            }
        }
    }

    return item;
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsLayoutItem *parent = extractLayoutItem(ctx);

    if (!parent) {
        return ctx->throwError(i18n("The parent must be a QGraphicsLayoutItem"));
    }

    return qScriptValueFromValue(eng, new QGraphicsLinearLayout(parent));
}

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, orientation) {
    if (ctx->argumentCount() > 0) {
        self->setOrientation(static_cast<Qt::Orientation>(ctx->argument(0).toInt32()));
    }

    return QScriptValue(eng, static_cast<int>(self->orientation()));
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, setAlignment) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    self->setAlignment(item, static_cast<Qt::Alignment>(ctx->argument(1).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, insertItem) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx, 1);

    if (!item) {
        return eng->undefinedValue();
    }

    self->insertItem(ctx->argument(0).toInt32(), item);
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, removeItem) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    self->removeItem(item);
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, setStretchFactor) {
    QGraphicsLayoutItem *item = ctx->argument(0).toVariant().value<QGraphicsLayoutItem*>();

    if (!item) {
        return eng->undefinedValue();
    }

    self->setStretchFactor(item, static_cast<Qt::Orientation>(ctx->argument(1).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, addItem) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx);
    if (!item) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QGraphicsLinearLayout.prototype.addItem: argument is not a GraphicsLayoutItem");
    }

    self->addItem(item);
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsItem, toString) {
    return QScriptValue(eng, "QGraphicsLinearLayout");
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, count) {
    return QScriptValue(eng, self->count());
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, itemAt) {
    if (ctx->argumentCount() < 1) {
        return eng->undefinedValue();
    }

    int index = ctx->argument(0).toInt32();
    return qScriptValueFromValue(eng, self->itemAt(index));
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, spacing) {
    if (ctx->argumentCount() > 0) {
        int pixels = ctx->argument(0).toInt32();
        self->setSpacing(pixels);
    }

    return QScriptValue(eng, self->spacing());
} END_DECLARE_METHOD

/////////////////////////////////////////////////////////////

QScriptValue constructLinearLayoutClass(QScriptEngine *eng)
{
//    QScriptValue proto = QScript::wrapGVPointer<QGraphicsLinearLayout>(eng, new QGraphicsLinearLayout(), );
    qRegisterMetaType<QGraphicsLayoutItem*>();
    QGraphicsLayoutItem * i = new QGraphicsLinearLayout;
    QVariant v;
    ///v.setValue<QGraphicsLayoutItem*>(i);
    v.setValue<void*>(i);
    QScriptValue proto = QScript::wrapPointer<QGraphicsLinearLayout>(eng, new QGraphicsLinearLayout(), QScript::UserOwnership);
    const QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    const QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("count", eng->newFunction(count), getter);
    proto.setProperty("spacing", eng->newFunction(spacing), getter | setter);
    proto.setProperty("orientation", eng->newFunction(orientation), getter | setter);
    ADD_METHOD(proto, itemAt);
    ADD_METHOD(proto, removeAt);
    ADD_METHOD(proto, addStretch);
    ADD_METHOD(proto, setStretchFactor);
    ADD_METHOD(proto, setAlignment);
    ADD_METHOD(proto, insertStretch);
    ADD_METHOD(proto, setItemSpacing);
    ADD_METHOD(proto, setContentsMargins);
    ADD_METHOD(proto, addItem);
    ADD_METHOD(proto, removeItem);
    ADD_METHOD(proto, insertItem);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsLinearLayout>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    //ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIsMovable);

    return ctorFun;
}
