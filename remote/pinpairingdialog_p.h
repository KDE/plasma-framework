/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#ifndef PINPAIRINGDIALOG_H
#define PINPAIRINGDIALOG_H

#include "plasma_export.h"
#include <QObject>

namespace Plasma
{

class AuthorizationRule;
class ClientPinRequest;
class PinPairingDialogPrivate;

class PLASMA_EXPORT PinPairingDialog : public QObject
{

Q_OBJECT

public:
    PinPairingDialog(AuthorizationRule &rule, QObject* parent = 0);
    PinPairingDialog(ClientPinRequest &request, QObject* parent = 0);
    ~PinPairingDialog();

private:
    PinPairingDialogPrivate *const d;

    Q_PRIVATE_SLOT(d, void slotAccept())
    Q_PRIVATE_SLOT(d, void slotReject())

    friend class PinPairingDialogPrivate;
};

}

#endif // PINPAIRINGDIALOG_H
