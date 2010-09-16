/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
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

#include "javascriptdataengine.h"

#include <QScriptEngine>

#include <Plasma/Package>

#include "javascriptservice.h"
#include "common/authorization.h"
#include "common/scriptenv.h"
#include "simplebindings/dataengine.h"
#include "simplebindings/i18n.h"

JavaScriptDataEngine::JavaScriptDataEngine(QObject *parent, const QVariantList &args)
    : DataEngineScript(parent)
{
    Q_UNUSED(args);

    m_qscriptEngine = new QScriptEngine(this);
    m_env = new ScriptEnv(this, m_qscriptEngine);

    connect(m_env, SIGNAL(reportError(ScriptEnv*,bool)), this, SLOT(reportError(ScriptEnv*,bool)));
}

bool JavaScriptDataEngine::init()
{
    QScriptValue global = m_qscriptEngine->globalObject();

    bindI18N(m_qscriptEngine);

    m_iface = m_qscriptEngine->newQObject(this);
    m_iface.setScope(global);
    m_env->addMainObjectProperties(m_iface);

    global.setProperty("engine", m_iface);

    global.setProperty("setData", m_qscriptEngine->newFunction(JavaScriptDataEngine::jsSetData));
    global.setProperty("removeAllData", m_qscriptEngine->newFunction(JavaScriptDataEngine::jsRemoveAllData));
    global.setProperty("removeData", m_qscriptEngine->newFunction(JavaScriptDataEngine::jsRemoveData));
    global.setProperty("removeAllSources", m_qscriptEngine->newFunction(JavaScriptDataEngine::jsRemoveAllSources));
    global.setProperty("Service", m_qscriptEngine->newFunction(JavaScriptDataEngine::serviceCtor));

    registerDataEngineMetaTypes(m_qscriptEngine);

    Authorization auth;
    if (!m_env->importExtensions(description(), m_iface, auth)) {
        return false;
    }

    return m_env->include(mainScript());
}

QScriptEngine *JavaScriptDataEngine::engine() const
{
    return m_qscriptEngine;
}

void JavaScriptDataEngine::jsSetMaxSourceCount(int count)
{
    setMaxSourceCount(count);
}

int JavaScriptDataEngine::maxSourceCount() const
{
    return dataEngine()->maxSourceCount();
}

void JavaScriptDataEngine::jsSetMinimumPollingInterval(int interval)
{
    setMinimumPollingInterval(interval);
}

int JavaScriptDataEngine::jsMinimumPollingInterval() const
{
    return minimumPollingInterval();
}

void JavaScriptDataEngine::jsSetPollingInterval(int interval)
{
    m_pollingInterval = interval;
    setPollingInterval(interval);
}

int JavaScriptDataEngine::pollingInterval() const
{
    return m_pollingInterval;
}

QScriptValue JavaScriptDataEngine::jsSetData(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 2) {
        return context->throwError(i18n("setData() takes at least two arguments"));
    }

    QString error;
    JavaScriptDataEngine *iFace = extractIFace(engine, error);

    if (!iFace) {
        return context->throwError(error);
    }

    const QString source = context->argument(0).toString();
    if (context->argument(1).isArray()) {
        QScriptValueIterator it(context->argument(1));
        DataEngine::Data data;
        while (it.hasNext()) {
            it.next();
            data.insert(it.name(), it.value().toVariant());
        }

        iFace->setData(source, data);
    } else {
        QString value = context->argument(1).toString();
        if (context->argumentCount() > 2) {
            const QString key = value;
            value = context->argument(2).toString();
            iFace->setData(source, key, value);
        } else {
            iFace->setData(source, value);
        }
    }

    return engine->newVariant(true);
}

JavaScriptDataEngine *JavaScriptDataEngine::extractIFace(QScriptEngine *engine, QString &error)
{
    JavaScriptDataEngine *interface = 0;
    QScriptValue engineValue = engine->globalObject().property("engine");
    QObject *engineObject = engineValue.toQObject();

    if (!engineObject) {
        error = i18n("Could not extract the DataEngineObject");
    } else {
        interface = qobject_cast<JavaScriptDataEngine *>(engineObject);
        if (!interface) {
            error = i18n("Could not extract the DataEngine");
        }
    }

    return interface;
}

QScriptValue JavaScriptDataEngine::jsRemoveAllData(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1) {
        return context->throwError(i18n("removeAllData() takes at least one argument (the source name)"));
    }

    QString source = context->argument(0).toString();
    QString error;
    JavaScriptDataEngine *iFace = extractIFace(engine, error);

    if (iFace) {
        iFace->removeAllData(source);
        return engine->newVariant(true);
    }

    return context->throwError(error);
}

QScriptValue JavaScriptDataEngine::jsRemoveData(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 2) {
        return context->throwError(i18n("removeData() takes at least two arguments (the source and key names)"));
    }

    QString source = context->argument(0).toString();
    QString key = context->argument(1).toString();
    QString error;
    JavaScriptDataEngine *iFace = extractIFace(engine, error);

    if (iFace) {
        iFace->removeData(source, key);
        return engine->newVariant(true);
    }

    return context->throwError(error);
}

QScriptValue JavaScriptDataEngine::jsRemoveAllSources(QScriptContext *context, QScriptEngine *engine)
{
    QString error;
    JavaScriptDataEngine *iFace = extractIFace(engine, error);

    if (iFace) {
        iFace->removeAllSources();
        return engine->newVariant(true);
    }

    return context->throwError(error);
}

QScriptValue JavaScriptDataEngine::serviceCtor(QScriptContext *context, QScriptEngine *engine)
{
    QString error;
    JavaScriptDataEngine *iFace = extractIFace(engine, error);
    if (!iFace) {
        return context->throwError(error);
    }

    if (context->argumentCount() < 1) {
        return context->throwError(i18n("Service requires at least one parameter: the name of the service"));
    }

    const QString &serviceName = context->argument(0).toString();
    if (serviceName.isEmpty()) {
        return context->throwError(i18n("Service requires at least one parameter: the name of the service"));
    }

    JavaScriptService *service = new JavaScriptService(serviceName, iFace);
    if (service->wasFound()) {
        QScriptValue v = engine->newQObject(service, QScriptEngine::QtOwnership, QScriptEngine::ExcludeSuperClassContents);
        return v;
    }

    delete service;
    return context->throwError(i18n("Requested service %1 was not found in the Package.", serviceName));
}

QScriptValue JavaScriptDataEngine::callFunction(const QString &functionName, const QScriptValueList &args)
{
    QScriptValue func = m_iface.property(functionName);
    return m_env->callFunction(func, args, m_iface);
}

void JavaScriptDataEngine::reportError(ScriptEnv *env, bool fatal) const
{
    Q_UNUSED(fatal)

    kDebug() << "Error: " << env->engine()->uncaughtException().toString()
             << " at line " << env->engine()->uncaughtExceptionLineNumber() << endl;
    kDebug() << env->engine()->uncaughtExceptionBacktrace();
}

QStringList JavaScriptDataEngine::sources() const
{
    JavaScriptDataEngine *unconst = const_cast<JavaScriptDataEngine *>(this);
    QScriptValueList args;
    QScriptValue rv = unconst->callFunction("sources", args);
    if (rv.isValid() && (rv.isVariant() || rv.isArray())) {
        return rv.toVariant().toStringList();
    }

    return DataEngineScript::sources();
}

bool JavaScriptDataEngine::sourceRequestEvent(const QString &name)
{
    QScriptValueList args;
    args << name;
    m_env->callEventListeners("sourceRequestEvent", args);
    QScriptValue rv = callFunction("sourceRequestEvent", args);
    if (rv.isValid() && rv.isBool()) {
        return rv.toBool();
    }

    return false;
}

bool JavaScriptDataEngine::updateSourceEvent(const QString &source)
{
    QScriptValueList args;
    args << source;
    m_env->callEventListeners("updateSourcEvent", args);
    QScriptValue rv = callFunction("updateSourceEvent", args);
    if (rv.isValid() && rv.isBool()) {
        return rv.toBool();
    }

    return false;
}

Plasma::Service *JavaScriptDataEngine::serviceForSource(const QString &source)
{
    QScriptValueList args;
    args << source;
    QScriptValue rv = callFunction("serviceForSource", args);
    if (rv.isValid() && rv.isQObject()) {
        Plasma::Service *service = qobject_cast<Plasma::Service *>(rv.toQObject());
        if (service) {
            if (service->destination().isEmpty()) {
                service->setDestination(source);
            }

            return service;
        } else {
            delete rv.toQObject();
        }
    }

    return 0;
}

QString JavaScriptDataEngine::filePath(const char *type, const QString &file) const
{
    const QString path = m_env->filePathFromScriptContext(type, file);
    if (!path.isEmpty()) {
        return path;
    }

    return package()->filePath(type, file);
}

bool JavaScriptDataEngine::include(const QString &script)
{
    const QString path = filePath("scripts", script);
    if (path.isEmpty()) {
        return false;
    }

    return m_env->include(path);
}

K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE(javascriptdataengine, JavaScriptDataEngine)

#include <javascriptdataengine.moc>

