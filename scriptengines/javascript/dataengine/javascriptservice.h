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

namespace Plasma
{
    class Package;
} // namespace Plasma

class JavaScriptDataEngine;

class JavaScriptService : public Plasma::Service
{
    Q_OBJECT

public:
    JavaScriptService(const QString &serviceName, JavaScriptDataEngine *engine);
    ~JavaScriptService();

    bool wasFound() const;
    void setScriptValue(QScriptValue &v);

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters);
    void registerOperationsScheme();

private:
     QWeakPointer<JavaScriptDataEngine> m_dataEngine;
     QScriptValue m_scriptValue;
};

#endif

