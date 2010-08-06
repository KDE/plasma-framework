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

void JavaScriptService::setScriptValue(QScriptValue &v)
{
    m_scriptValue = v;
}

Plasma::ServiceJob *JavaScriptService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    QMapIterator<QString, QVariant> it(parameters);
    if (m_dataEngine) {
        ScriptEnv *env = ScriptEnv::findScriptEnv(m_dataEngine.data()->engine());
        if (env) {
            QScriptValueList args;
            args << m_scriptValue << operation
                 << qScriptValueFromValue(m_dataEngine.data()->engine(), parameters);
            //TODO: Parameters!
            QScriptValue func = m_scriptValue.property("createJob");
            /*
            kDebug() << "same thing?" << v.equals(sv) << func.isFunction() <<
                sv.property("createJob").isFunction();
                */
            QScriptValue rv = env->callFunction(func, args, m_scriptValue);
            Plasma::ServiceJob *job = dynamic_cast<Plasma::ServiceJob *>(rv.toQObject());
            if (job) {
                return job;
            }
        }
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

#include "javascriptservice.moc"

