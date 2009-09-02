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

#ifndef SERVICEPROVIDER_H
#define SERVICEPROVIDER_H

namespace Plasma 
{

class Service;

class ServiceProvider : public Jolie::Interface
{
    Q_OBJECT

    public:
        ServiceProvider(Service *service);

    protected:
        messageReceived(Jolie::Message message);

    private:
        void sendOperationNames();
        void sendOperationDescription(const QString &operationName);
        void startOperationCall(const QString &destination, const QByteArray &description)

        Service *m_service;
};

} //namespace Plasma

#endif //SERVICEPROVIDER_H
