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

#ifndef JOLIEMESSAGEHELPER_H
#define JOLIEMESSAGEHELPER_H

#include <QByteArray>
#include <QtJolie/Message>
#include <QtJolie/Value>
#include <kdebug.h>
#include <klocalizedstring.h>

namespace JolieMessage {
    namespace Field {
        static const QByteArray DESTINATION  = "Destination";
        static const QByteArray ENABLEDOPERATIONS
                                             = "EnabledOperations";
        static const QByteArray IDENTITY     = "Credentials";
        static const QByteArray IDENTITYID   = "CredentialsID";
        static const QByteArray PARAMETERS   = "Parameters";
        static const QByteArray PIN          = "PIN";
        static const QByteArray RESULT       = "Result";
        static const QByteArray SIGNATURE    = "Signature";
        static const QByteArray TOKEN        = "Token";
        static const QByteArray UUID         = "UUID";
        static const QByteArray OPERATION    = "Operation";
        static const QByteArray OPERATIONSDESCRIPTION
                                             = "OperationsDescription";
    }

    namespace Error {
        static const QByteArray INVALIDTOKEN = "InvalidToken";
        static const QByteArray REQUIREPIN   = "RequirePIN";
        static const QByteArray ACCESSDENIED = "AccessDenied";
    }

    inline QString errorMessage(const QByteArray &error)
    {
        if (error == Error::INVALIDTOKEN) {
            return i18nc("Error message, access to a remote service failed.",
                         "Invalid token.");
        } else if (error == Error::REQUIREPIN) {
            return i18nc("Error message, access to a remote service failed.",
                         "Matching password required.");
        } else if (error == Error::ACCESSDENIED) {
            return i18nc("Error message, access to a remote service failed.",
                         "Access denied.");
        }
        return i18n("Unknown error.");
    }

    inline QByteArray field(const QByteArray &fieldName, const Jolie::Message &message)
    {
        if (!message.data().children(fieldName).isEmpty()) {
            return message.data().children(fieldName).first().toByteArray();
        } else {
            return QByteArray();
        }
    }

    inline QByteArray payload(const Jolie::Message &message)
    {
        QByteArray result;
        //result = "payload!";
        result.append(message.operationName());
        result.append(field(Field::PARAMETERS, message));
        result.append(field(Field::IDENTITY, message));
        result.append(field(Field::IDENTITYID, message));
        result.append(field(Field::OPERATION, message));
        result.append(field(Field::OPERATIONSDESCRIPTION, message));
        result.append(field(Field::PIN, message));
        result.append(field(Field::TOKEN, message));
        return result;
    }

    inline QString print(const Jolie::Message &message)
    {
        QString result;
        result =
        QString("\n=== JOLIE MESSAGE ===\nId = %1\nOperation = %2\nResource = %3\nData= %4\n")
                .arg(QString::number(message.id()))
                .arg(QString(message.operationName()))
                .arg(QString(message.resourcePath()))
                .arg(QString(message.data().toByteArray()));

        result += "=====================\n";

        foreach (const QByteArray &child, message.data().childrenNames()) {
            result += "\n******" + child + "******\n";
            foreach (const Jolie::Value &value, message.data().children(child)) {
                if (child == Field::TOKEN || child == Field::PARAMETERS
                                          || child == Field::SIGNATURE) {
                    result += value.toByteArray().toBase64();
                } else {
                    result += value.toByteArray();
                }
            }
        }
        
        result += "\n== END OF MESSAGE ==\n";
        
        return result;
    }
}

#endif
    
