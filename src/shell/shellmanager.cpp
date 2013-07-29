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

#include "shellmanager.h"

#include <utils/d_ptr_implementation.h>

#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QList>
#include <QTimer>

#include <QQmlEngine>
#include <QQmlComponent>

#include <config-prefix.h>

static const QString s_shellsDir(
        QString(CMAKE_INSTALL_PREFIX) + "/" + LIB_INSTALL_DIR + "/" + "qml/org/kde/plasma/shells/");

//
// ShellManager
//

class ShellManager::Private {
public:
    Private()
        : currentHandler(nullptr)
    {
        shellUpdateDelay.setInterval(100);
        shellUpdateDelay.setSingleShot(true);
    }

    QList<QObject *> handlers;
    QObject * currentHandler;
    QTimer shellUpdateDelay;
};

ShellManager::ShellManager()
{
    connect(
        &d->shellUpdateDelay, &QTimer::timeout,
        this, &ShellManager::updateShell
    );

    loadHandlers();
}

ShellManager::~ShellManager()
{
    // if (d->currentHandler)
    //     d->currentHandler->unload();
}

void ShellManager::loadHandlers()
{
    // For the time being, we are 'loading' static shells
    // TODO: Make this plugin-based
    static QQmlEngine * engine = new QQmlEngine(this);

    for (const auto & dir: QDir(s_shellsDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        for (const auto & qml: QDir(s_shellsDir + dir).entryList(QStringList() << "*.qml")) {
            qDebug() << "Making a new instance of " << qml;

            QQmlComponent handlerComponent(engine,
                    QUrl::fromLocalFile(s_shellsDir + dir + "/" + qml)
                );
            auto handler = handlerComponent.create();

            // Writing out the errors
            for (const auto & error: handlerComponent.errors()) {
                qDebug() << "Error: " << error;
            }

            if (handler) {
                qDebug() << "We got the handler";

                registerHandler(handler);
            }
        }
    }

    updateShell();
}

void ShellManager::registerHandler(QObject * handler)
{
    connect(
        handler, &QObject::destroyed,
        this,    &ShellManager::deregisterHandler
    );

    connect(
        handler, SIGNAL(updated()),
        this,    SLOT(requestShellUpdate())
    );

    d->handlers.push_back(handler);
}

void ShellManager::deregisterHandler(QObject * handler)
{
    if (d->handlers.contains(handler)) {
        d->handlers.removeAll(handler);

        handler->disconnect(this);
    }

    if (d->currentHandler == handler)
        d->currentHandler = nullptr;
}

void ShellManager::requestShellUpdate()
{
    qDebug() << "Somebody wants us to check whether we should change the current shell";
    d->shellUpdateDelay.start();
}

void ShellManager::updateShell()
{
    d->shellUpdateDelay.stop();

    qDebug() << "We got a request to update the current shell";

    if (d->handlers.isEmpty()) {
        qFatal("We have no shell handlers installed");
        return;
    }

    // Finding the handler that has the priority closest to zero.
    // We will return a handler even if there are no willing ones.

    auto handler =* std::min_element(d->handlers.cbegin(), d->handlers.cend(),
            [] (QObject * left, QObject * right)
            {
                auto willing = [] (QObject * handler)
                {
                    qDebug() << "willing? " << handler->property("willing");
                    return handler->property("willing").toBool();
                };

                auto priority = [] (QObject * handler)
                {
                    qDebug() << "priority? " << handler->property("priority");
                    return handler->property("priority").toInt();
                };

                return
                    // If one is willing and the other is not,
                    // return it - it has the priority
                    willing(left) && !willing(right) ? true :
                    !willing(left) && willing(right) ? false :
                    // otherwise just compare the priorities
                    priority(left) < priority(right);
            }
         );

    if (handler == d->currentHandler) return;

    // Activating the new handler and killing the old one
    qDebug() << "Activating the new handler and killing the old one";

    if (d->currentHandler) {
        qDebug() << "Loaded?" << d->currentHandler->property("loaded");
        d->currentHandler->setProperty("loaded", false);
    }

    d->currentHandler = handler;

    qDebug() << "Loaded?" << d->currentHandler->property("loaded");
    qDebug() << "Loaded?" << d->currentHandler->property("willing");
    qDebug() << "Loaded?" << d->currentHandler->property("priority");

    d->currentHandler->setProperty("loaded", true);
}

ShellManager * ShellManager::instance()
{
    static ShellManager manager;
    return &manager;
}

