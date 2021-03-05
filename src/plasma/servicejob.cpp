/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "servicejob.h"

#include <QDebug>

#include "private/servicejob_p.h"

namespace Plasma
{
ServiceJobPrivate::ServiceJobPrivate(ServiceJob *owner, const QString &dest, const QString &op, const QVariantMap &params)
    : q(owner)
    , destination(dest)
    , operation(op)
    , parameters(params)
    , m_allowAutoStart(true)
{
}

void ServiceJobPrivate::preventAutoStart()
{
    m_allowAutoStart = false;
}

void ServiceJobPrivate::autoStart()
{
    if (m_allowAutoStart) {
        m_allowAutoStart = false;

        if (q->isAutoDelete()) {
            // by checking for isAutoDelete, we prevent autostarting when
            // exec() is called or when the job owner has "taken control"
            // of the job by requesting it not be autodeleted
            q->start();
        }
    }
}

ServiceJob::ServiceJob(const QString &destination, const QString &operation, const QVariantMap &parameters, QObject *parent)
    : KJob(parent)
    , d(new ServiceJobPrivate(this, destination, operation, parameters))
{
    connect(this, SIGNAL(finished(KJob *)), this, SLOT(preventAutoStart()));
}

ServiceJob::~ServiceJob()
{
    delete d;
}

QString ServiceJob::destination() const
{
    return d->destination;
}

QString ServiceJob::operationName() const
{
    return d->operation;
}

QVariantMap ServiceJob::parameters() const
{
    return d->parameters;
}

QVariant ServiceJob::result() const
{
    return d->result;
}

void ServiceJob::setResult(const QVariant &result)
{
    d->result = result;
    emitResult();
}

void ServiceJob::start()
{
    setResult(false);
}

} // namespace Plasma

#include "moc_servicejob.cpp"
