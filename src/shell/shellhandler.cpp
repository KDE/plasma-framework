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

#include "shellhandler.h"

#include <utils/d_ptr_implementation.h>

#include <vector>
#include <algorithm>

#include "desktopcorona.h"
#include "scripting/desktopscriptengine.h"

//
// ShellHandler
//

class ShellHandler::Private {
public:
    Private()
        : willing(false)
    {}

    bool willing;
    int priority;
};

ShellHandler::ShellHandler()
{
    ShellManager::instance()->registerHandler(this);
}

ShellHandler::~ShellHandler()
{
}

bool ShellHandler::willing() const
{
    return d->willing;
}

void ShellHandler::setWilling(bool willing)
{
    if (d->willing == willing) return;

    d->willing = willing;

    emit willingnessChanged(willing);
}

unsigned short ShellHandler::priority() const
{
    return d->priority;
}

void ShellHandler::setPriority(unsigned short priority)
{
    d->priority = priority;

    emit priorityChanged(priority);
}

//
// ShellManager
//

class ShellManager::Private {
public:
    Private()
        : currentHandler(nullptr)
    {
    }

    std::vector<ShellHandler::Ptr> handlers;
    ShellHandler * currentHandler;
};

ShellManager::ShellManager()
{

}

ShellManager::~ShellManager()
{
    if (d->currentHandler)
        d->currentHandler->unload();
}

void ShellManager::loadHandlers()
{
    // For the time being, we are 'loading' static shells
    // TODO: Make this plugin-based

}

void ShellManager::registerHandler(ShellHandler * handler)
{
    handler->setParent(this);

    connect(handler, SIGNAL(priorityChanged(int)),
            this, SLOT(updateShell()));
    connect(handler, SIGNAL(willingnessChanged(bool)),
            this, SLOT(updateShell()));

    d->handlers.push_back(ShellHandler::Ptr(handler));
}

void ShellManager::updateShell()
{
    if (d->handlers.empty()) {
        qFatal("We have no shell handlers installed");
        return;
    }

    // Finding the handler that has the priority closest to zero.
    // We will return a handler even if there are no willing ones.

    auto handler = std::min_element(d->handlers.cbegin(), d->handlers.cend(),
            [] (
                const ShellHandler::Ptr & left,
                const ShellHandler::Ptr & right
            ) {
                return
                    // If one is willing and the other is not,
                    // return it - it has the priority
                    left->willing() && !right->willing() ? true :
                    !left->willing() && right->willing() ? false :
                    // otherwise just compare the priorities
                    left->priority() < right->priority();
            }
         )->get();

    if (handler == d->currentHandler) return;

    // Activating the new handler and killing the old one

}

ShellManager * ShellManager::instance()
{
    static ShellManager manager;
    return &manager;
}

