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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QScriptValue>

#include <Plasma/Animator>
#include <Plasma/AppletScript>
#include <Plasma/DataEngine>

#include "simplebindings/uiloader.h"

class QScriptEngine;
class QScriptContext;

class AppletInterface;

class SimpleJavaScriptApplet : public Plasma::AppletScript
{
    Q_OBJECT

public:
    SimpleJavaScriptApplet( QObject *parent, const QVariantList &args );
    ~SimpleJavaScriptApplet();
    bool init();

    static void reportError(QScriptEngine *error, bool fatal = false);

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    QList<QAction*> contextualActions();
    void constraintsEvent(Plasma::Constraints constraints);
    bool include(const QString &path);
    QSet<QString> loadedExtensions() const;

    QScriptValue variantToScriptValue(QVariant var);

    static QString findImageFile(QScriptEngine *engine, const QString &file);

public slots:
    void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
    void configChanged();
    void executeAction(const QString &name);
    void collectGarbage();

private:
    bool importExtensions();
    bool importBuiltinExtesion(const QString &extension);
    void setupObjects();
    void callFunction(const QString &functionName, const QScriptValueList &args = QScriptValueList());
    static void populateAnimationsHash();

    static QString findSvg(QScriptEngine *engine, const QString &file);
    static QScriptValue animation(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue animationGroup(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18n(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18nc(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18np(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue jsi18ncp(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue dataEngine(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue service(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue loadui(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaSvg(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaFrameSvg(QScriptContext *context, QScriptEngine *engine);

    void installWidgets( QScriptEngine *engine );
    static QScriptValue createWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue notSupported(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue print(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue createPrototype(QScriptEngine *engine, const QString &name);
    static QScriptValue widgetAdjustSize(QScriptContext *context, QScriptEngine *engine);

    // run extension
    static QScriptValue runApplication(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue runCommand(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue openUrl(QScriptContext *context, QScriptEngine *engine);

private:
    static KSharedPtr<UiLoader> s_widgetLoader;
    static QHash<QString, Plasma::Animator::Animation> s_animationDefs;
    QScriptEngine *m_engine;
    QScriptValue m_self;
    QVariantList m_args;
    QSet<QString> m_extensions;
    AppletInterface *m_interface;
    friend class AppletInterface;
};


#endif // SCRIPT_H

