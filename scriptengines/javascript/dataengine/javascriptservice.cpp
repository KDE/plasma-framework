/*
 *   Copyright 2010 Aaron J. Seigo <aseigo@kde.org>
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

#include "javascriptservice.h"

#include <QFile>
#include <Plasma/ServiceJob>

#include "common/scriptenv.h"
#include "javascriptdataengine.h"

JavaScriptServiceJob::JavaScriptServiceJob(QScriptEngine *engine, const QString &destination, const QString &operation,
                     const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(destination, operation, parameters, parent),
      m_thisObject(engine->newQObject(this, QScriptEngine::QtOwnership, QScriptEngine::ExcludeSuperClassContents))
{
}

void JavaScriptServiceJob::start()
{
    if (!m_startFunction.isFunction()) {
        setResult(false);
        return;
    }

    m_startFunction.call(m_thisObject);
}

QScriptValue JavaScriptServiceJob::scriptValue() const
{
    return m_thisObject;
}

QScriptValue JavaScriptServiceJob::startFunction() const
{
    return m_startFunction;
}

void JavaScriptServiceJob::setStartFunction(const QScriptValue &v)
{
    m_startFunction = v;
}

JavaScriptService::JavaScriptService(const QString &serviceName, JavaScriptDataEngine *engine)
    : Plasma::Service(engine),
      m_dataEngine(engine)
{
    setName(serviceName);
}

JavaScriptService::~JavaScriptService()
{
//    kDebug();
}

Plasma::ServiceJob *JavaScriptService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (m_setupFunc.isFunction() && m_dataEngine && isOperationEnabled(operation)) {
        JavaScriptServiceJob *job = new JavaScriptServiceJob(m_dataEngine.data()->engine(), destination(), operation, parameters, this);
        QScriptValueList args;
        args << job->scriptValue();
        m_setupFunc.call(QScriptValue(), args);
        return job;
    }

    return 0;
}

bool JavaScriptService::wasFound() const
{
    return m_dataEngine;
}

void JavaScriptService::registerOperationsScheme()
{
    if (!m_dataEngine) {
        return;
    }

    const QString path = m_dataEngine.data()->filePath("services", name() + ".operations");

    if (path.isEmpty()) {
        kDebug() << "Cannot find operations description:" << name() << ".operations";
        m_dataEngine.clear();
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

QScriptValue JavaScriptService::setupJobFunction() const
{
    return m_setupFunc;
}

void JavaScriptService::setSetupJobFunction(const QScriptValue &v)
{
    m_setupFunc = v;
}

#include "javascriptservice.moc"

