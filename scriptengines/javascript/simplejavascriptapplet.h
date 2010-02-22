/*
 *   Copyright 2007, 2010 Richard J. Moore <rich@kde.org>
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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QScriptValue>

#include <Plasma/Animator>
#include <Plasma/AppletScript>
#include <Plasma/DataEngine>

#include "simplebindings/uiloader.h"

class QScriptContext;
class QScriptEngine;

class AppletInterface;
class ScriptEnv;

namespace Plasma
{
    class ExtenderItem;
} // namespace Plasma

class SimpleJavaScriptApplet : public Plasma::AppletScript
{
    Q_OBJECT

public:
    SimpleJavaScriptApplet(QObject *parent, const QVariantList &args);
    ~SimpleJavaScriptApplet();
    bool init();

    static void reportError(ScriptEnv *error, bool fatal = false);

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    QList<QAction*> contextualActions();
    void constraintsEvent(Plasma::Constraints constraints);
    bool include(const QString &path);
    QSet<QString> loadedExtensions() const;
    QScriptValue variantToScriptValue(QVariant var);

    static QString findImageFile(QScriptEngine *engine, const QString &file);

public Q_SLOTS:
    void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
    void configChanged();
    void executeAction(const QString &name);
    void collectGarbage();
    void extenderItemRestored(Plasma::ExtenderItem* item);

private Q_SLOTS:
    void engineReportsError(ScriptEnv *engine, bool fatal);

private:
    bool importExtensions();
    bool importBuiltinExtension(const QString &extension);
    void setupObjects();
    void callFunction(const QString &functionName, const QScriptValueList &args = QScriptValueList());
    static void populateAnimationsHash();

    static QString findSvg(QScriptEngine *engine, const QString &file);
    static QScriptValue animation(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue animationGroup(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue parallelAnimationGroup(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18n(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18nc(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18np(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18ncp(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue dataEngine(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue service(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue loadui(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaSvg(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaFrameSvg(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaExtenderItem(QScriptContext *context, QScriptEngine *engine);

    void installWidgets(QScriptEngine *engine);
    static QScriptValue createWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue widgetAdjustSize(QScriptContext *context, QScriptEngine *engine);

    static AppletInterface *extractAppletInterface(QScriptEngine *engine);
    static QGraphicsWidget *extractParent(QScriptContext *context,
                                          QScriptEngine *engine,
                                          int parentIndex = 0,
                                          bool *parentedToApplet = 0);

private:
    static KSharedPtr<UiLoader> s_widgetLoader;
    static QHash<QString, Plasma::Animator::Animation> s_animationDefs;
    ScriptEnv *m_env;
    QScriptEngine *m_engine;
    QScriptValue m_self;
    QVariantList m_args;
    AppletInterface *m_interface;
    friend class AppletInterface;
};

#endif // SCRIPT_H

