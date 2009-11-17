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

#include "serviceprovider_p.h"

#include "authorizationrule_p.h"
#include "authorizationmanager_p.h"
#include "joliemessagehelper_p.h"

#include "config-plasma.h"

#include <plasma/remote/authorizationinterface.h>
#include <plasma/remote/authorizationmanager.h>
#include <plasma/remote/authorizationrule.h>
#include <plasma/remote/credentials.h>
#include <plasma/service.h>
#include <plasma/servicejob.h>
#include <plasma/private/servicejob_p.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>

#include <QtJolie/Server>

#ifdef ENABLE_REMOTE_WIDGETS
#include <QtCrypto>
#endif

#include <kdebug.h>
#include <kstandarddirs.h>

namespace Plasma 
{

ServiceProvider::ServiceProvider(const QString &name, Service *service)
    : Jolie::AbstractAdaptor(service),
      m_service(service)
{
    connect(service, SIGNAL(finished(Plasma::ServiceJob *)),
            this, SLOT(operationCompleted(Plasma::ServiceJob*)));

    m_providerName = name;
    AuthorizationManager::self()->d->server->registerAdaptor(m_providerName.toUtf8(), this);
    kDebug() << "registered service provider " << m_providerName;
}

ServiceProvider::~ServiceProvider()
{
    AuthorizationManager::self()->d->server->unregisterAdaptor(m_providerName.toUtf8());
}

void ServiceProvider::startOperationCall(Jolie::Message message)
{
    kDebug() << "starting operation call";

    KConfigGroup description =
       m_service->operationDescription(QString(JolieMessage::field(JolieMessage::Field::OPERATION, message)));

    //deserialize the parameters
    QByteArray parametersByteArray;
    parametersByteArray = JolieMessage::field(JolieMessage::Field::PARAMETERS, message);
    kDebug() << "parameters byte array: " << parametersByteArray.toBase64();
    QBuffer buffer(&parametersByteArray);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    QMap<QString, QVariant> parameters;
    in >> parameters;

    if (!description.isValid()) {
        kDebug() << "invalid description.";
    }

    kDebug() << "====PARAMETERS====";

    //write the parameters into the operation description
    foreach (const QString &key, parameters.keys()) {
        kDebug() << "key = " << key << ", value = " << parameters.value(key);
        description.writeEntry(key, parameters.value(key));
    }

    m_service->setDestination(JolieMessage::field(JolieMessage::Field::DESTINATION, message));
    ServiceJob *job = m_service->startOperationCall(description);
    QString identityID = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    job->d->identity = AuthorizationManager::self()->d->getCredentials(identityID);
    kDebug() << "adding into messagemap:" << ((QObject*)job);
    m_messageMap[job] = message;
}

void ServiceProvider::sendOperations(Jolie::Message message)
{
    kDebug() << "send operations.";
    //kDebug() << printJolieMessage(message);
    Jolie::Message response(message.resourcePath(), message.operationName(), message.id());

    //FIXME: this is duplicated from Plasma::Service
    QString path = KStandardDirs::locate("data", "plasma/services/" + m_service->name() +
".operations");

    if (path.isEmpty()) {
        kDebug() << "Cannot find operations description:" << m_service->name() << ".operations";
        response.setFault(Jolie::Fault("NoOperationsDescription"));
    } else {
        kDebug() << "file = " << path;
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        Jolie::Value value;
        value.children(JolieMessage::Field::OPERATIONSDESCRIPTION) << Jolie::Value(file.readAll());
        file.close();
        response.setData(value);
    }

    QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
    response = appendToken(response, id, uuid);
    kDebug() << "caller = " << id.toBase64();

    //hack around the not yet async service adaptor api in qtjolie
    if (m_descriptorMap.contains(id + uuid)) {
        kDebug() << "descriptor found, sending message";
        AuthorizationManager::self()->d->server->sendReply(
            m_descriptorMap.value(id + uuid), response);
    } else {
        kDebug() << "no valid entry in descriptormap.";
    }
}

void ServiceProvider::sendEnabledOperations(Jolie::Message message)
{
    kDebug() << "send enabled operations.";
    Jolie::Message response(message.resourcePath(), message.operationName(), message.id());

    QStringList enabledOperationsList;
    foreach (const QString &operation, m_service->operationNames()) {
        if (m_service->isOperationEnabled(operation)) {
            enabledOperationsList << operation;
        }
    }

    kDebug() << "enabled operations: " << enabledOperationsList;

    QByteArray enabledOperationsArray;
    QDataStream out(&enabledOperationsArray, QIODevice::WriteOnly);
    out << enabledOperationsList;

    Jolie::Value value;
    value.children(JolieMessage::Field::ENABLEDOPERATIONS) << Jolie::Value(enabledOperationsArray);
    response.setData(value);

    QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
    response = appendToken(response, id, uuid);
    kDebug() << "caller = " << id.toBase64();

    //hack around the not yet async service adaptor api in qtjolie
    if (m_descriptorMap.contains(id + uuid)) {
        kDebug() << "descriptor found, sending message";
        AuthorizationManager::self()->d->server->sendReply(
            m_descriptorMap.value(id + uuid), response);
    } else {
        kDebug() << "no valid entry in descriptormap.";
    }
}

QString ServiceProvider::resourceName() const
{
    return m_providerName;
}

void ServiceProvider::relay(Jolie::Server *server, int descriptor,
                                      const Jolie::Message &message)
{
    Q_UNUSED(server)

    if (message.operationName() == "startConnection") {
        kDebug() << "reset token";
        //add the identity
        Credentials identity;
        QByteArray identityByteArray = JolieMessage::field(JolieMessage::Field::IDENTITY, message);
        QDataStream stream(&identityByteArray, QIODevice::ReadOnly);
        stream >> identity;
        AuthorizationManager::self()->d->addCredentials(identity);

        Jolie::Message response(message.resourcePath(), message.operationName(), message.id());
        QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
        response = appendToken(response, identity.id().toAscii(), uuid);
        AuthorizationManager::self()->d->server->sendReply(descriptor, response);

        return;
    }
    
    if (JolieMessage::field(JolieMessage::Field::TOKEN, message).isEmpty()) {
        Jolie::Message response(message.resourcePath(), message.operationName(), message.id());
        response.setFault(Jolie::Fault(JolieMessage::Error::INVALIDTOKEN));
        AuthorizationManager::self()->d->server->sendReply(descriptor, response);
        return;
    }
    
    //m_descriptor = descriptor;
    QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
    m_descriptorMap[id + uuid] = descriptor;
    authorize(message, m_tokens[id + uuid]);
}

void ServiceProvider::operationCompleted(Plasma::ServiceJob *job)
{
    kDebug() << "operation completed.";
    if (!m_messageMap.contains(job)) {
        kDebug() << "service not in map!";
        return;
    }

    kDebug() << "found message in message map!";

    Jolie::Message message = m_messageMap.take(job);
    Jolie::Message response(message.resourcePath(), message.operationName(), message.id());

    QVariant variantResult = job->result();
    kDebug() << "got a result: " << variantResult;
    QByteArray byteArrayResult;
    QBuffer buffer(&byteArrayResult);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << variantResult;

    Jolie::Value data;
    data.children(JolieMessage::Field::RESULT) << Jolie::Value(byteArrayResult);
    response.setData(data);
    if (job->error()) {
        response.setFault(Jolie::Fault(job->errorString().toAscii()));
    }

    QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
    response = appendToken(response, id, uuid);

    //hack around the not yet async service adaptor api in qtjolie
    if (m_descriptorMap.contains(id + uuid)) {
        kDebug() << "descriptor found, sending message";
        AuthorizationManager::self()->d->server->sendReply(
            m_descriptorMap.value(id + uuid), response);
    } else {
        kDebug() << "no valid entry in descriptormap.";
    }
}

void ServiceProvider::ruleChanged(Plasma::AuthorizationRule *rule)
{
    int i = 0;
    foreach (const Jolie::Message &message, m_messagesPendingAuthorization) {
        QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
        //Credentials identity = AuthorizationManager::self()->d->getCredentials(id);

        bool matches = rule->d->matches(message.resourcePath(), id);
        if (matches && rule->policy() == AuthorizationRule::PinRequired &&
            JolieMessage::field(JolieMessage::Field::PIN, message) != rule->pin()) {
            kDebug() << "we need a pin";
            authorizationFailed(message, JolieMessage::Error::REQUIREPIN);
            m_messagesPendingAuthorization.removeAt(i);
            return;
        /**
        } else if (matches && rule->policy() == AuthorizationRule::PinRequired) {
            kDebug() << "AUTHORIZATION: Service is freely accessable for verified caller.";
            rule->setPolicy(AuthorizationRule::Allow);
            authorizationSuccess(message);
            //TODO: it might be nicer to do a removeAll once Jolie::Message implements ==
            m_messagesPendingAuthorization.removeAt(i);
            return;
        */
        } else if (matches && rule->policy() == AuthorizationRule::Allow) {
            kDebug() << "AUTHORIZATION: Service is freely accessable for verified caller.";
            authorizationSuccess(message);
            //TODO: it might be nicer to do a removeAll once Jolie::Message implements ==
            m_messagesPendingAuthorization.removeAt(i);
            return;
        } else if (matches && rule->policy() == AuthorizationRule::Deny) {
            kDebug() << "AUTHORIZATION: Service is never accessable for verified caller.";
            authorizationFailed(message, JolieMessage::Error::ACCESSDENIED);
            m_messagesPendingAuthorization.removeAt(i);
            return;
        } else {
            i++;
        }
    }
}

Jolie::Message ServiceProvider::appendToken(Jolie::Message message,
                                            const QByteArray &caller,
                                            const QByteArray &uuid)
{
#ifdef ENABLE_REMOTE_WIDGETS
    m_tokens[caller + uuid] = QCA::Random::randomArray(256).toByteArray();
#endif
    //kDebug() << "setting token: " << m_tokens[caller + uuid].toBase64()
             //<< " for caller: " << caller.toBase64()
             //<< " with uuid caller: " << uuid.toBase64();
    
    Jolie::Value data = message.data();
    data.children(JolieMessage::Field::TOKEN) << Jolie::Value(m_tokens[caller + uuid]);
    message.setData(data);
    return message;
}


void ServiceProvider::authorize(const Jolie::Message &message, const QByteArray &validToken)
{
    kDebug() << "VALIDATING MESSAGE:";
    //kDebug() << JolieMessage::print(message);

    //Authorization step 1: is the service accessable to all callers? In that case we can skip the
    //verification of the signature
    kDebug() << "STEP1";
    AuthorizationRule *rule =
        AuthorizationManager::self()->d->matchingRule(message.resourcePath(), Credentials());

    if (rule && rule->policy() == AuthorizationRule::Allow) {
        kDebug() << "AUTHORIZATION: Service is freely accessable.";
        authorizationSuccess(message);
        return;
    } else if (rule && rule->policy() == AuthorizationRule::Deny) {
        kDebug() << "AUTHORIZATION: Service is never accessable.";
        authorizationFailed(message, JolieMessage::Error::ACCESSDENIED);
        return;
    }

    //Authorization step 2: see if the token matches. If it doesn't we can't safely identify the
    //caller and are finished.
    kDebug() << "STEP2";
    if (JolieMessage::field(JolieMessage::Field::TOKEN, message) != validToken && !validToken.isEmpty()) {
        kDebug() << "AUTHORIZATION: Message token doesn't match.";
        kDebug() << "expected: " << validToken.toBase64();
        authorizationFailed(message, JolieMessage::Error::INVALIDTOKEN);
        return;
    }

    QByteArray payload = JolieMessage::payload(message);
    QByteArray signature = JolieMessage::field(JolieMessage::Field::SIGNATURE, message);
    Credentials identity = AuthorizationManager::self()->d->getCredentials(
            JolieMessage::field(JolieMessage::Field::IDENTITYID, message));

    if (!identity.isValid()) {
        kDebug() << "no identity";
        authorizationFailed(message, JolieMessage::Error::INVALIDTOKEN);
        return;
    }

    kDebug() << "STEP3";
    //Authorization step 3: see if we have the key and can validate the signature. If we can't,
    //either the public key has changed, or somebody is doing something nasty, and we're finished.
    if ((!identity.isValidSignature(signature, payload))) {
        kDebug() << "AUTHORIZATION: signature invalid.";
        authorizationFailed(message, JolieMessage::Error::ACCESSDENIED);
        return;
    }

    kDebug() << "STEP4";
    //Authorization step 4: if we have a valid signature, see if we've got a matching rule
    rule = AuthorizationManager::self()->d->matchingRule(message.resourcePath(), identity);
    if (rule && rule->policy() == AuthorizationRule::PinRequired) {
        kDebug() << "we expect a pin!";
        QByteArray pin = JolieMessage::field(JolieMessage::Field::PIN, message);
        if (rule->pin() == QString(pin)) {
            authorizationSuccess(message);
            rule->setPolicy(AuthorizationRule::Allow);
        } else {
            authorizationFailed(message, JolieMessage::Error::ACCESSDENIED);
            AuthorizationManager::self()->d->rules.removeAll(rule);
            delete rule;
        }
    } else if (rule && rule->policy() == AuthorizationRule::Allow) {
        kDebug() << "AUTHORIZATION: Service is freely accessable for validated sender.";
        authorizationSuccess(message);
        return;
    } else if (rule && rule->policy() == AuthorizationRule::Deny) {
        kDebug() << "AUTHORIZATION: Service is not accessable for validated sender.";
        authorizationFailed(message, JolieMessage::Error::ACCESSDENIED);
        return;
    } else {
        //- let the shell set the rule matching this request:
        kDebug() << "STEP6";
        kDebug() << "leave it up to the authorization interface";
        m_messagesPendingAuthorization << message;
        AuthorizationRule *newRule =
            new AuthorizationRule(QString(message.resourcePath()), identity.id());
        connect(newRule, SIGNAL(changed(Plasma::AuthorizationRule*)), this,
                         SLOT(ruleChanged(Plasma::AuthorizationRule*)));
        AuthorizationManager::self()->d->rules.append(newRule);
        AuthorizationManager::self()->d->authorizationInterface->authorizationRequest(*newRule);
    }
}

void ServiceProvider::authorizationSuccess(const Jolie::Message &message)
{
    kDebug() << "message with operationName " << message.operationName() << " allowed!";

    //would be lovely if this kind of stuff could be autogenerated code from xml like in dbus
    //adaptors
    if (message.operationName() == "getOperations") {
        sendOperations(message);
    } else if (message.operationName() == "getEnabledOperations") {
        sendEnabledOperations(message);
    } else if (message.operationName() == "startOperationCall") {
        startOperationCall(message);
    }
}

void ServiceProvider::authorizationFailed(const Jolie::Message &message, const QByteArray &error)
{
    kDebug() << "message with operationName " << message.operationName() << " NOT allowed!";
    Jolie::Message response(message.resourcePath(), message.operationName(), message.id());
    response.setFault(Jolie::Fault(error));

    QByteArray id = JolieMessage::field(JolieMessage::Field::IDENTITYID, message);
    QByteArray uuid = JolieMessage::field(JolieMessage::Field::UUID, message);
    AuthorizationManager::self()->d->server->sendReply(
                            m_descriptorMap.value(id + uuid), response);
    return;
}

} //namespace Plasma

#include "serviceprovider_p.moc"
