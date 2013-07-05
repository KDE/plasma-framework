/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "application.h"
#include "application_p.h"

#include "utils/d_ptr_implementation.h"

#include <QDebug>

Application::Private::Private(Application * parent)
    : q(parent)
{
    connect(
        &process, SIGNAL(stateChanged(QProcess::ProcessState)),
        this,     SLOT(stateChanged(QProcess::ProcessState))
        );
    process.setProcessChannelMode(QProcess::MergedChannels);
}

void Application::Private::stateChanged(QProcess::ProcessState newState)
{
    //running = (newState != QProcess::NotRunning);
    //q->runningChanged(running);
}

Application::Application(QObject * parent)
    : QObject(parent), d(this)
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

void Application::setApplication(const QString & application)
{
    qDebug() << "setting the application to" << application;

    // TODO: make an application whitelist
    // and ensure those are invoked from the system
    // prefix (plasma's)
    if (d->process.state() != QProcess::NotRunning) {
        return;
    }

    qDebug() << "setting the application to" << application;

    d->application = application;

    emit applicationChanged(application);

    if (d->running) {
        start();
    }
}

void Application::setRunning(bool run)
{
    qDebug() << "running?" << run;
    d->running = run;

    if (run)
        start();
    else
        terminate();
}

void Application::start()
{
    qDebug() << "starting?";
    if (d->application.isEmpty()) return;

    qDebug() << "starting.";
    d->process.start(d->application);

    qDebug() <<
    d->process.waitForStarted();

    qDebug() << "do we have an error " << d->process.error();
}

void Application::terminate()
{
    d->process.terminate();
}

