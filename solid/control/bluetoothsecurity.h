/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2007 Juan González <jaguilera@opsiland.info>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_CONTROL_BLUETOOTHSECURITY_H
#define SOLID_CONTROL_BLUETOOTHSECURITY_H

#include <QtCore/QObject>
#include <QtCore/QPair>

#include "singletondefs.h"
#include "solid_control_export.h"

namespace Solid
{
namespace Control
{
    /**
     * This is the interface a class must implement to act as a Passkey Agent.
     * A class that acts as a PassKeyAgent will handle the process of pairing 
     * (stablish a trusted connection) with a remote device.
     */
    class SOLIDCONTROL_EXPORT BluetoothPasskeyAgent : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString remote READ remote)
        public:
            /**
             * Initializes the passkey agent
             * @param parent pointer to the parent object for auto deletion
             * @param remote address of the remote device for this agent in 00:00:00:00:00:00 format. If empty the agent will act as the default passkey agent for all devices.
             */
            BluetoothPasskeyAgent(QObject *parent = 0,const QString &remote="");
            //FIXME This method shouldn't be virtual, but the linker complains (and halts) if it isn't
            /**
             * Gets the UBI of the remote device this agent authenticates. 
             * @return Universal Bluetooth Identifier of the associated agent. If empty the agent authenticates any remote device that requires it.
             */
            virtual QString remote();
        public Q_SLOTS:
            /**
             * This method gets called when the bluetooth system needs to get the passkey for a remote authentication.
             * @param ubi Universal Bluetooth Identifier of the remote device
             * @param isNumeric Indicates whether a numerical passkey is expected.
             * @return The passkey to be used in this authentication, or an empty string if the request has been rejected/canceled
             */
            virtual QString requestPasskey(const QString &ubi, bool isNumeric);
            /**
             * This method gets called when the bluetooth system needs to verify a passkey. The verification is
             * done by showing the value to the passkey agent.
             * @param ubi Universal Bluetooth Identifier of the remote device
             * @param passkey The passkey to confirm.
             * @return Whether the passkey accepts or not the remote passkey
             */
            virtual bool confirmPasskey(const QString &ubi, const QString &passkey);
            /**
             * Gets called when the bluetooth system needs to show a passkey. No answer is
             * expected from the agent.
             * @param ubi Universal Bluetooth Identifier of the remote device
             * @param passkey The passkey to show
             */
            virtual void displayPasskey(const QString &ubi, const QString &passkey);
            /**
             * Indicates keypresses from a remote device,this can happen, for example, when pairing with a keyboard.
             * @param ubi Universal Bluetooth Identifier of the remote device that emitted the keypress
             */
            virtual void keypress(const QString &ubi);
            /**
             * Indicates to the agent that the authentication has been completed.
             * @param ubi Universal Bluetooth Identifier of the remote device that has been authenticated
             */
            virtual void completedAuthentication(const QString &ubi);
            /**
             * Indicates to the agent that the authentication has been canceled before completion.
             * @param ubi Universal Bluetooth Identifier of the remote device which authentication was 
             */
            virtual void cancelAuthentication(const QString &ubi);
        private:
            /// UBI of the remote device this passkey agent handles authentication for.
            QString m_remote;
    };
    
    /**
     * This is the interface a class must implement to act as an Authorization Agent.
     * An Authorization Agent handles requests to access local services from a remote
     * device.
     */
    class SOLIDCONTROL_EXPORT BluetoothAuthorizationAgent: public QObject
    {
        Q_OBJECT
        public:
            /**
             * Used to keep the QObject deletion chain working.
             * @param parent The parent of this object that will take care of deletion.
             */
            BluetoothAuthorizationAgent(QObject *parent);
        public Q_SLOTS:
            /**
             * This method gets called when the bluetooth system wants to get an authorization for accessing a local service.
             * @param localUbi Universal Bluetooth Identifier of the local device providing the service
             * @param remoteAddress of the remote device that wants to use our services
             * @param serviceUuid Universal unique identifier for the local service
             * @return whether the remote device is authorized to use the local service.
             */
            virtual bool authorize(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid)=0;
            /**
             * Cancels the currently active authorization request for the given local/remote device.
             * @param localUbi Universal Bluetooth Identifier of the local device providing the service
             * @param remoteAddress of the remote device that wanted to use our services but canceled before completion.
             * @param serviceUuid Universal unique identifier for the local service
             */
            virtual void cancel(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid)=0;
    };
    
    class BluetoothSecurityPrivate;
    /**
     * Represents a bluetooth security interface which handles passkey request by the backend.
     * To get an instance of this class use Solid::Control::BluetoothManager::security().
     */
    class SOLIDCONTROL_EXPORT BluetoothSecurity : public QObject
    {
    Q_OBJECT
    public:
        /**
         * Constructs an invalid bluetooth security object 
         */
        BluetoothSecurity();

        /**
         * Constructs a new bluetooth security object taking its data from a backend.
         *
         * @param backendObject the object given by the backend
         */
        explicit BluetoothSecurity(QObject *backendObject);

        /**
         * Destroys the device.
         */
        ~BluetoothSecurity();

        /**
         * Assigns a bluetooth security object to this bluetooth security object and returns a reference to it.
         *
         * @param device the bluetooth security object to assign
         * @return a reference to the bluetooth security object 
                     */
        BluetoothSecurity &operator=(const BluetoothSecurity &object);
    public Q_SLOTS:
        /**
         * Sets the passkeyagent that will handle authentication (for pairing) requests in this process
         * @param agent The PasskeyAgent for this process. If there is a previously registered agent it WON'T be deleted, track/parent your objects.
         */
        void setPasskeyAgent(Solid::Control::BluetoothPasskeyAgent *agent);
        /**
         * Sets the authorizationagent that will handle service usage authorization requests in this process.
         * @param agent The AuthorizationAgent for this process.If there is a previously registered agent it WON'T be deleted, track/parent your objects.
         */
        void setAuthorizationAgent(Solid::Control::BluetoothAuthorizationAgent *agent);
    private:
        Q_PRIVATE_SLOT(d, void _k_destroyed(QObject *))

        BluetoothSecurityPrivate * const d;
    };
} //Control
} //Solid

#endif
