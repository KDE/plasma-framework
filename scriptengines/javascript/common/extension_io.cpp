/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
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

#include "scriptenv.h"

#include <QDir>

#include <KGlobalSettings>
#include <KIO/Job>
#include <KRun>
#include <KStandardDirs>

QScriptValue ScriptEnv::openUrl(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    QScriptValue v = context->argument(0);
    KUrl url = v.isString() ? KUrl(v.toString()) : qscriptvalue_cast<KUrl>(v);

    if (!url.isValid()) {
        return false;
    }

    ScriptEnv *env = ScriptEnv::findScriptEnv(engine);
    if (!env) {
        return false;
    }

    if (!(env->m_allowedUrls & AppLaunching) &&
        !((env->m_allowedUrls & HttpUrls) && (url.protocol() == "http" || url.protocol() == "https"))) {
        return false;
    }

    new KRun(url, 0);
    return true;
}

// TODO these should throw an exception
QScriptValue ScriptEnv::getUrl(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return engine->undefinedValue();
    }

    QScriptValue v = context->argument(0);
    KUrl url = v.isString() ? KUrl(v.toString()) : qscriptvalue_cast<KUrl>(v);

    if (!url.isValid()) {
        return engine->undefinedValue();
    }

    ScriptEnv *env = ScriptEnv::findScriptEnv(engine);
    if (!env) {
        //kDebug() << "findScriptEnv failed";
        return engine->undefinedValue();
    }

    if (url.isLocalFile()) {
        if (!(env->m_allowedUrls & LocalUrls)) {
            return engine->undefinedValue();
        }
    } else if (!(env->m_allowedUrls & NetworkUrls) &&
               !((env->m_allowedUrls & HttpUrls) && (url.protocol() == "http" || url.protocol() == "https"))) {
        return engine->undefinedValue();
    }

    KIO::Job *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    return engine->newQObject(job);
}

QScriptValue ScriptEnv::userDataPath(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return QDir::homePath();
    }

    const QString type = context->argument(0).toString();
    if (type.isEmpty()) {
        return QDir::homePath();
    }

    if (context->argumentCount() > 1) {
        const QString filename = context->argument(1).toString();
        return KStandardDirs::locateLocal(type.toLatin1(), filename);
    }

    if (type.compare("desktop", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::desktopPath();
    } else if (type.compare("autostart", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::autostartPath();
    } else if (type.compare("documents", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::documentPath();
    } else if (type.compare("music", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::musicPath();
    } else if (type.compare("video", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::videosPath();
    } else if (type.compare("downloads", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::downloadPath();
    } else if (type.compare("pictures", Qt::CaseInsensitive) == 0) {
        return KGlobalSettings::picturesPath();
    }

    return QString();
}

void ScriptEnv::registerGetUrl(QScriptValue &obj)
{
    QScriptValue get = obj.property("getUrl");
    if (!get.isValid()) {
        obj.setProperty("getUrl", m_engine->newFunction(ScriptEnv::getUrl));
    }
}

void ScriptEnv::registerOpenUrl(QScriptValue &obj)
{
    QScriptValue value = obj.property("openUrl");
    if (!value.isValid()) {
        obj.setProperty("openUrl", m_engine->newFunction(ScriptEnv::openUrl));
    }
}


