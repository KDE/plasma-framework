/*  This file is part of the KDE project
    Copyright (C) 2006,2008 Will Stephenson <wstephenson@kde.org>

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
#include "fakenetworkmanager.h"

#include <QFile>
#include <QtXml/QtXml>
#include <QLatin1String>

#include <kstandarddirs.h>
#include <kdebug.h>

#include "fakeaccesspoint.h"
#include "fakewirednetworkinterface.h"
#include "fakewirelessnetworkinterface.h"

FakeNetworkManager::FakeNetworkManager(QObject * parent, const QStringList  &) : Solid::Control::Ifaces::NetworkManager(parent)
{
    mUserNetworkingEnabled = true;
    mUserWirelessEnabled = true;
    mRfKillEnabled = false;
    mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakenetworking.xml");

//     QDBusConnection::sessionBus().registerObject("/org/kde/solid/fake", this, QDBusConnection::ExportNonScriptableSlots);

    parseNetworkingFile();
}

FakeNetworkManager::FakeNetworkManager(QObject * parent, const QStringList &, const QString &xmlFile) : Solid::Control::Ifaces::NetworkManager(parent)
{
    mUserNetworkingEnabled = true;
    mUserWirelessEnabled = true;

    mXmlFile = xmlFile;
    if (mXmlFile.isEmpty())
    {
       kDebug() << "Falling back to installed networking xml";
       mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakenetworking.xml");
    }
    parseNetworkingFile();
}

FakeNetworkManager::~FakeNetworkManager()
{
}

Solid::Networking::Status FakeNetworkManager::status() const
{
    return Solid::Networking::Unknown;
}

QStringList FakeNetworkManager::networkInterfaces() const
{
    return mNetworkInterfaces.keys();
}

QStringList FakeNetworkManager::activeNetworkInterfaces() const
{
    QStringList activeDevices;
    QMapIterator<QString, FakeNetworkInterface *> it(mNetworkInterfaces);
    while (it.hasNext())
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if (netDevice->isActive())
            activeDevices.append(netDevice->uni());
    }
    return activeDevices;
}

QObject * FakeNetworkManager::createNetworkInterface(const QString  & undi)
{
    if (mNetworkInterfaces.contains(undi))
        return mNetworkInterfaces[undi];
    else
        return 0;
}

bool FakeNetworkManager::isWirelessEnabled() const
{
    return mUserWirelessEnabled;
}

bool FakeNetworkManager::isNetworkingEnabled() const
{
    QMapIterator<QString, FakeNetworkInterface *> it(mNetworkInterfaces);
    while (it.hasNext())
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if (netDevice->isActive())
            return true;
    }
    return false;
}

bool FakeNetworkManager::isWirelessHardwareEnabled() const
{
    return mRfKillEnabled;
}

void FakeNetworkManager::setWirelessEnabled(bool enabled)
{
    mUserWirelessEnabled = enabled;
}

void FakeNetworkManager::setNetworkingEnabled(bool enabled)
{
    QMapIterator<QString, FakeNetworkInterface *> it(mNetworkInterfaces);
    while (it.hasNext())
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
    //    if ((netDevice->type() == Solid::Control::NetworkInterface::Ieee80211 && mUserWirelessEnabled)
      //     || netDevice->type() == Solid::Control::NetworkInterface::Ieee8023)
        //    netDevice->setActive(enabled);
    }
    mUserNetworkingEnabled = enabled;
}

void FakeNetworkManager::parseNetworkingFile()
{
    QFile machineFile(mXmlFile);
    if (!machineFile.open(QIODevice::ReadOnly))
    {
        kDebug() << "Error while opening " << mXmlFile;
        return;
    }

    QDomDocument fakeDocument;
    if (!fakeDocument.setContent(&machineFile))
    {
        kDebug() << "Error while creating the QDomDocument.";
        machineFile.close();
        return;
    }
    machineFile.close();

    kDebug() << "Parsing fake computer XML: " << mXmlFile;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while (!node.isNull())
    {
        QDomElement tempElement = node.toElement();
        if (!tempElement.isNull() && tempElement.tagName() == QLatin1String("device"))
        {
            FakeNetworkInterface *tempDevice = parseDeviceElement(tempElement);
            if(tempDevice)
            {
               mNetworkInterfaces.insert(tempDevice->uni(), tempDevice);
// Use the DeviceManager for now, the udi/uni should
//                emit deviceAdded(tempDevice->uni());
            }
        } else if (tempElement.tagName() == QLatin1String("property")) {
            QString propertyKey = tempElement.attribute("key");
            QVariant propertyValue = QVariant(tempElement.text());
            if ( propertyKey== QLatin1String("networking")) {
                mUserNetworkingEnabled = propertyValue.toBool();
            } else if ( propertyKey== QLatin1String("wireless")) {
                mUserWirelessEnabled = propertyValue.toBool();
            } else if ( propertyKey== QLatin1String("rfkill")) {
                mRfKillEnabled = propertyValue.toBool();
            }
        }
        node = node.nextSibling();
    }
}

FakeNetworkInterface *FakeNetworkManager::parseDeviceElement(const QDomElement &deviceElement)
{
    FakeNetworkInterface *device = 0;
    QMap<QString,QVariant> propertyMap;
    QString uni = deviceElement.attribute("uni");
    propertyMap.insert("uni", uni);
    kDebug() << "Listing device: " << uni;
    propertyMap.insert("uni", QVariant(uni));
    QList< FakeAccessPoint * > networks;
    bool wireless = false;
    QDomNode childNode = deviceElement.firstChild();
    while (!childNode.isNull())
    {
        QDomElement childElement = childNode.toElement();
        //kDebug() << "found child=" << childElement.tagName();
        if (!childElement.isNull() && childElement.tagName() == QLatin1String("property"))
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = childElement.attribute("key");
            propertyValue = QVariant(childElement.text());
            if ( propertyValue == "ieee80211" ) {
                wireless = true;
            }
            //kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString();
            propertyMap.insert(propertyKey, propertyValue);
        }
        else if (!childElement.isNull() && childElement.tagName() == QLatin1String("accesspoint"))
        {
            QString uni = childElement.attribute("uni");
            kDebug() << "Listing properties: " << uni;
            FakeAccessPoint * wifi = new FakeAccessPoint(parseAPElement(childElement), this);
            networks.append(wifi);
        }
        childNode = childNode.nextSibling();
    }
    //kDebug() << "Done listing. ";

/*    if (!propertyMap.isEmpty())
    { */
        kDebug() << "Creating FakeNetworkDevice for " << uni;
        if (wireless) {
            FakeWirelessNetworkInterface * wifi = new FakeWirelessNetworkInterface(propertyMap);
            foreach( FakeAccessPoint * net, networks)
            {
                kDebug() << "Injecting " << net->uni();
                wifi->injectAccessPoint(net);
            }
            device = wifi;
        } else {
            device = new FakeWiredNetworkInterface(propertyMap);
        }


//     }

    return device;
}

QMap<QString,QVariant> FakeNetworkManager::parseAPElement(const QDomElement &deviceElement)
{
    QMap<QString,QVariant> propertyMap;

    QDomNode propertyNode = deviceElement.firstChild();
    while (!propertyNode.isNull())
    {
        QDomElement propertyElement = propertyNode.toElement();
        if (!propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property"))
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());
            //kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString();
            propertyMap.insert(propertyKey, propertyValue);
        }

        propertyNode = propertyNode.nextSibling();
    }
    return propertyMap;
}

void FakeNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters)
{
    mActiveConnections.append(connectionUni);
    QTimer::singleShot(0, this, SIGNAL(activeConnectionsChanged()));
}

void FakeNetworkManager::deactivateConnection(const QString & activeConnection)
{
    mActiveConnections.removeAll(activeConnection);
}

QStringList FakeNetworkManager::activeConnections() const
{
    return mActiveConnections;
}

#include "fakenetworkmanager.moc"

