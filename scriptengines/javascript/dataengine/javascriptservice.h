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

#ifndef JAVASCRIPTSERVICE_H
#define JAVASCRIPTSERVICE_H

#include <QWeakPointer>
#include <QScriptValue>

#include <Plasma/Package>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

namespace Plasma
{
    class Package;
} // namespace Plasma

class JavaScriptDataEngine;

class JavaScriptServiceJob : public Plasma::ServiceJob
{
    Q_OBJECT
    Q_PROPERTY(QString destination READ destination)
    Q_PROPERTY(QString operationName READ operationName)
    Q_PROPERTY(QVariantMap parameters READ parameters)
    Q_PROPERTY(QVariant result READ result WRITE setResult)
    Q_PROPERTY(int error READ error WRITE setError)
    Q_PROPERTY(QString errorText READ errorText WRITE setErrorText)
    Q_PROPERTY(QScriptValue start READ startFunction WRITE setStartFunction)

public:
    JavaScriptServiceJob(QScriptEngine *engine, const QString &destination, const QString &operation,
                         const QMap<QString, QVariant> &parameters, QObject *parent = 0);

    void start();

    QScriptValue startFunction() const;
    void setStartFunction(const QScriptValue &v);

    QScriptValue scriptValue() const;

private:
    QScriptValue m_startFunction;
    QScriptValue m_thisObject;
};

class JavaScriptService : public Plasma::Service
{
    Q_OBJECT
    Q_PROPERTY(QString destination READ destination WRITE setDestination)
    Q_PROPERTY(QStringList operationNames READ operationNames)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QScriptValue setupJob READ setupJobFunction WRITE setSetupJobFunction)

public:
    JavaScriptService(const QString &serviceName, JavaScriptDataEngine *engine);
    ~JavaScriptService();

    bool wasFound() const;

    QScriptValue setupJobFunction() const;
    void setSetupJobFunction(const QScriptValue &v);

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters);
    void registerOperationsScheme();

private:
     QWeakPointer<JavaScriptDataEngine> m_dataEngine;
     QScriptValue m_setupFunc;
};

#endif

