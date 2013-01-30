/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_DENYALLAUTHORIZATION_H
#define PLASMA_DENYALLAUTHORIZATION_H

#include "plasma_export.h"
#include "authorizationinterface.h"

#include <QtCore/QObject>

namespace Plasma
{

/**
 * @class DenyAllAuthorization plasma/denyallauthorization.h <Plasma/DenyAllAuthorization>
 *
 * @short Implementation of AuthorizationInterface that you can use if you don't want to allow
 * any remote access in your shell.
 *
 * @since 4.4?
 */
class PLASMA_EXPORT DenyAllAuthorization : public AuthorizationInterface
{
    public:
        DenyAllAuthorization();
        ~DenyAllAuthorization();
        virtual void authorizationRequest(AuthorizationRule &rule);
        virtual void clientPinRequest(ClientPinRequest &request);
};
} // Plasma namespace

#endif

