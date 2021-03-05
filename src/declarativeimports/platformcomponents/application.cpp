/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "application.h"
#include "application_p.h"

#include "utils/d_ptr_implementation.h"

#include <QCoreApplication>
#include <QDebug>

Application::Private::Private(Application *parent)
    : running(false)
    , q(parent)
{
    connect(&process, &QProcess::stateChanged, this, &Private::stateChanged);
    connect(&process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(errorFound(QProcess::ProcessError)));
    process.setProcessChannelMode(QProcess::MergedChannels);
}

void Application::Private::stateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState)
    // running = (newState != QProcess::NotRunning);
    // q->runningChanged(running);
}

void Application::Private::errorFound(QProcess::ProcessError err)
{
    Q_UNUSED(err)
    qWarning() << "Error" << process.error() << "while starting" << application;
}

Application::Application(QObject *parent)
    : QObject(parent)
    , d(this)
{
}

Application::~Application()
{
    terminate();
}

bool Application::running() const
{
    return d->running;
}

QString Application::application() const
{
    return d->application;
}

void Application::setApplication(const QString &application)
{
    // TODO: make an application whitelist
    // and ensure those are invoked from the system
    // prefix (plasma's)
    if (d->process.state() != QProcess::NotRunning) {
        return;
    }

    d->application = application;

    Q_EMIT applicationChanged(application);

    if (d->running) {
        start();
    }
}

void Application::setRunning(bool run)
{
    d->running = run;

    if (run) {
        start();
    } else {
        terminate();
    }
}

void Application::start()
{
    if (!d->running) {
        return;
    }

    if (d->application.isEmpty()) {
        qWarning() << "Cannot run an empty application";
        return;
    }

    d->process.start(d->application, QStringList());
}

void Application::terminate()
{
    d->process.terminate();
}
