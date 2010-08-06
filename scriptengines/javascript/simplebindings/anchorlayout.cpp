/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
 *   Copyright 2009 Artur Duque de Souza <asouza@kde.org>
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
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsAnchorLayout>

#include <Plasma/Applet>

#include "backportglobal.h"

Q_DECLARE_METATYPE(QScript::Pointer<QGraphicsItem>::wrapped_pointer_type)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsAnchor*)
Q_DECLARE_METATYPE(QGraphicsLayoutItem*)
DECLARE_POINTER_METATYPE(QGraphicsAnchorLayout)

// QGraphicsAnchorLayout
DECLARE_VOID_NUMBER_METHOD(QGraphicsAnchorLayout, setSpacing)
DECLARE_NUMBER_GET_SET_METHODS(QGraphicsAnchorLayout, horizontalSpacing, setHorizontalSpacing)
DECLARE_NUMBER_GET_SET_METHODS(QGraphicsAnchorLayout, verticalSpacing, setVerticalSpacing)
DECLARE_VOID_NUMBER_METHOD(QGraphicsAnchorLayout, removeAt)


/////////////////////////////////////////////////////////////

QGraphicsLayoutItem *extractLayoutItem(QScriptContext *ctx, int index = 0, bool noExistingLayout = false);

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsLayoutItem *parent = extractLayoutItem(ctx, 0, true);
    //FIXME: don't leak memory when parent is 0
    return qScriptValueFromValue(eng, new QGraphicsAnchorLayout(parent));
}

BEGIN_DECLARE_METHOD(QGraphicsAnchorLayout, addAnchor) {
    QGraphicsLayoutItem *item1 = extractLayoutItem(ctx, 0);
    QGraphicsLayoutItem *item2 = extractLayoutItem(ctx, 2);

    if (!item1 || !item2) {
        return eng->undefinedValue();
    }

    QGraphicsAnchor *anchor = self->addAnchor(item1, static_cast<Qt::AnchorPoint>(ctx->argument(1).toInt32()),
                                              item2, static_cast<Qt::AnchorPoint>(ctx->argument(3).toInt32()));

    return eng->newQObject(anchor, QScriptEngine::QtOwnership);
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsAnchorLayout, anchor) {
    QGraphicsLayoutItem *item1 = extractLayoutItem(ctx, 0);
    QGraphicsLayoutItem *item2 = extractLayoutItem(ctx, 2);

    if (!item1 || !item2) {
        return eng->undefinedValue();
    }

    QGraphicsAnchor *anchor = self->anchor(item1, static_cast<Qt::AnchorPoint>(ctx->argument(1).toInt32()),
                                           item2, static_cast<Qt::AnchorPoint>(ctx->argument(3).toInt32()));

    return eng->newQObject(anchor, QScriptEngine::QtOwnership);
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsAnchorLayout, addCornerAnchors) {
    QGraphicsLayoutItem *item1 = extractLayoutItem(ctx, 0);
    QGraphicsLayoutItem *item2 = extractLayoutItem(ctx, 2);

    if (!item1 || !item2) {
        return eng->undefinedValue();
    }

    self->addCornerAnchors(item1, static_cast<Qt::Corner>(ctx->argument(1).toInt32()),
                           item2, static_cast<Qt::Corner>(ctx->argument(3).toInt32()));

    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsAnchorLayout, addAnchors) {
    QGraphicsLayoutItem *item1 = extractLayoutItem(ctx, 0);
    QGraphicsLayoutItem *item2 = extractLayoutItem(ctx, 1);

    if (!item1 || !item2) {
        return eng->undefinedValue();
    }

    self->addAnchors(item1, item2, static_cast<Qt::Orientation>(ctx->argument(2).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsItem, toString) {
    return QScriptValue(eng, "QGraphicsAnchorLayout");
} END_DECLARE_METHOD

/////////////////////////////////////////////////////////////

class PrototypeAnchorLayout : public QGraphicsAnchorLayout
{
public:
    PrototypeAnchorLayout()
    { }
};

QScriptValue constructAnchorLayoutClass(QScriptEngine *eng)
{
    QScriptValue proto =
        QScript::wrapPointer<QGraphicsAnchorLayout>(eng,
                                                    new QGraphicsAnchorLayout(),
                                                    QScript::UserOwnership);
    const QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    const QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;

    proto.setProperty("horizontalSpacing", eng->newFunction(horizontalSpacing), getter);
    proto.setProperty("horizontalSpacing", eng->newFunction(setHorizontalSpacing), setter);
    proto.setProperty("verticalSpacing", eng->newFunction(verticalSpacing), getter);
    proto.setProperty("verticalSpacing", eng->newFunction(setVerticalSpacing), setter);

    ADD_METHOD(proto, setSpacing);
    ADD_METHOD(proto, removeAt);
    ADD_METHOD(proto, addAnchor);
    ADD_METHOD(proto, anchor);
    ADD_METHOD(proto, addAnchors);
    ADD_METHOD(proto, addCornerAnchors);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsAnchorLayout>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
