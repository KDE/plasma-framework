/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef SCRIPTENGINE
#define SCRIPTENGINE

#include <QScriptEngine>
#include <QScriptValue>

namespace Plasma
{
    class Applet;
    class Containment;
    class Corona;
} // namespace Plasma

namespace PlasmaKPartScripting
{

class ScriptEngine : public QScriptEngine
{
    Q_OBJECT

public:
    explicit ScriptEngine(Plasma::Corona *corona, QObject *parent = 0);
    ~ScriptEngine();

    static QStringList pendingUpdateScripts();
    static QStringList defaultLayoutScripts();

    bool evaluateScript(const QString &script, const QString &path = QString());
    QScriptValue wrap(Plasma::Applet *w);

    static ScriptEngine *envFor(QScriptEngine *engine);

Q_SIGNALS:
    void print(const QString &string);
    void printError(const QString &string);

private:
    void setupEngine();

    static QScriptValue print(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue fileExists(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue loadTemplate(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue theme(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue widgets(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue addWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue knownWidgetTypes(QScriptContext *context, QScriptEngine *engine);

private Q_SLOTS:
    void exception(const QScriptValue &value);

private:
    Plasma::Corona *m_corona;
    Plasma::Containment *m_containment;
    QScriptValue m_scriptSelf;

    static const int PLASMA_KPART_SCRIPTING_VERSION = 3;
};

}

#endif

