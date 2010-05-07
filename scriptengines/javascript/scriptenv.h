/*
 *   Copyright 2007-2008 Richard J. Moore <rich@kde.org>
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

#ifndef SCRIPTENV_H
#define SCRIPTENV_H

#include <QScriptEngine>
#include <QSet>

#include <KPluginInfo>

#include "authorization.h"

class ScriptEnv : public QObject
{
    Q_OBJECT
public:
    enum AllowedUrl { NoUrls = 0,
                      HttpUrls = 1,
                      NetworkUrls = 2,
                      LocalUrls = 4 };
    Q_DECLARE_FLAGS(AllowedUrls, AllowedUrl)

    ScriptEnv(QObject *parent, QScriptEngine *engine);
    ~ScriptEnv();

    /** Returns the QScriptEngine in use. */
    QScriptEngine *engine() const;

    /** Returns the ScriptEnv in use for a given QScriptEngine or 0. */
    static ScriptEnv *findScriptEnv(QScriptEngine *engine);

    static void registerEnums(QScriptValue &scriptValue, const QMetaObject &meta);
    bool include(const QString &path);

    bool importExtensions(const KPluginInfo &info, QScriptValue &obj, Authorization &authorizer);
    QSet<QString> loadedExtensions() const;

    bool checkForErrors(bool fatal);

Q_SIGNALS:
    void reportError(ScriptEnv *engine, bool fatal);

private:
    void registerGetUrl(QScriptValue &obj);
    bool importBuiltinExtension(const QString &extension, QScriptValue &obj);

    static QScriptValue debug(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue print(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue runApplication(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue runCommand(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue openUrl(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue getUrl(QScriptContext *context, QScriptEngine *engine);


private Q_SLOTS:
    void signalException();

private:
    QSet<QString> m_extensions;
    AllowedUrls m_allowedUrls;
    QScriptEngine *m_engine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ScriptEnv::AllowedUrls)

#endif

