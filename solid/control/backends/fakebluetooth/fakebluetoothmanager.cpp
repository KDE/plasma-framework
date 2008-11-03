/*  This file is part of the KDE project
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


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
#include <QFile>
#include <QtXml/QtXml>
#include <QLatin1String>

#include <kstandarddirs.h>
#include <kdebug.h>

#include "fakebluetoothmanager.h"

FakeBluetoothManager::FakeBluetoothManager(QObject * parent, const QStringList &) : Solid::Control::Ifaces::BluetoothManager(parent)
{
    kDebug() ;

    mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakebluetooth.xml");

//     QDBusConnection::sessionBus().registerObject("/org/kde/solid/fake", this, QDBusConnection::ExportNonScriptableSlots);

    parseBluetoothFile();
}

FakeBluetoothManager::FakeBluetoothManager(QObject * parent, const QStringList &, const QString &xmlFile) : Solid::Control::Ifaces::BluetoothManager(parent)
{
    kDebug() ;
    mXmlFile = xmlFile;
    if (mXmlFile.isEmpty()) {
        kDebug() << "Falling back to installed bluetoothing xml";
        mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakebluetooth.xml");
    }
    parseBluetoothFile();
}

FakeBluetoothManager::~FakeBluetoothManager()
{
    foreach (FakeBluetoothInterface *interface, mBluetoothInterfaces) {
        delete interface;
    }

    mBluetoothInterfaces.clear();
}

QStringList FakeBluetoothManager::bluetoothInterfaces() const
{
    return mBluetoothInterfaces.keys();
}

QObject * FakeBluetoothManager::createInterface(const QString  & ubi)
{
    if (mBluetoothInterfaces.contains(ubi))
        return mBluetoothInterfaces[ubi];
    else
        return 0;
}

QString FakeBluetoothManager::defaultInterface() const
{
    return "";
}

void FakeBluetoothManager::parseBluetoothFile()
{
    QFile machineFile(mXmlFile);
    if (!machineFile.open(QIODevice::ReadOnly)) {
        kDebug() << "Error while opening " << mXmlFile;
        return;
    }

    QDomDocument fakeDocument;
    QString error;
    int line;
    if (!fakeDocument.setContent(&machineFile, &error, &line)) {
        kDebug() << "Error while creating the QDomDocument: " << error << " line: " <<
        line <<  endl;
        machineFile.close();
        return;
    }
    machineFile.close();

    kDebug() << "Parsing fake computer XML: " << mXmlFile;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while (!node.isNull()) {
        QDomElement tempElement = node.toElement();
        if (!tempElement.isNull() && tempElement.tagName() == QLatin1String("interface")) {
            FakeBluetoothInterface *tempDevice = parseDeviceElement(tempElement);
            if (tempDevice) {
                mBluetoothInterfaces.insert(tempDevice->ubi(), tempDevice);
// Use the DeviceManager for now, the udi/ubi should
//                emit deviceAdded(tempDevice->ubi());
            }
        }
        node = node.nextSibling();
    }
}

FakeBluetoothInterface *FakeBluetoothManager::parseDeviceElement(const QDomElement &deviceElement)
{
    FakeBluetoothInterface *interface = 0;
    QMap<QString, QVariant> propertyMap;
    QString ubi = deviceElement.attribute("ubi");
    propertyMap.insert("ubi", ubi);
    kDebug() << "Listing device: " << ubi;
    propertyMap.insert("ubi", QVariant(ubi));

    QDomNode childNode = deviceElement.firstChild();
    while (!childNode.isNull()) {
        QDomElement childElement = childNode.toElement();
        //kDebug() << "found child=" << childElement.tagName();
        if (!childElement.isNull() && childElement.tagName() == QLatin1String("property")) {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = childElement.attribute("key");
            propertyValue = QVariant(childElement.text());
            //    kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString();
            propertyMap.insert(propertyKey, propertyValue);
        } else if (!childElement.isNull() && childElement.tagName() == QLatin1String("device")) {
            QString ubi = childElement.attribute("ubi");
            kDebug() << "Listing properties: " << ubi;
            FakeBluetoothRemoteDevice * remoteDevice = new FakeBluetoothRemoteDevice(parseBluetoothElement(childElement));
            mBluetoothRemoteDevices.insert(ubi, remoteDevice);
        }
        childNode = childNode.nextSibling();
    }
    //kDebug() << "Done listing. ";

    kDebug() << "Creating FakeBluetoothInterface for " << ubi;
    interface = new FakeBluetoothInterface(propertyMap);

    // Inject Remote devices....
    foreach (FakeBluetoothRemoteDevice *device, mBluetoothRemoteDevices) {
        kDebug() << "Injecting " << device->name();
        interface->injectDevice(device->ubi(), device);
    }

    mBluetoothRemoteDevices.clear();

    return interface;
}

QMap<QString, QVariant> FakeBluetoothManager::parseBluetoothElement(const QDomElement &deviceElement)
{
    QMap<QString, QVariant> propertyMap;

    QString ubi = deviceElement.attribute("ubi");
    propertyMap.insert("ubi", ubi);

    QDomNode propertyNode = deviceElement.firstChild();
    while (!propertyNode.isNull()) {
        QDomElement propertyElement = propertyNode.toElement();
        if (!propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property")) {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());
//            kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString();
            propertyMap.insert(propertyKey, propertyValue);
        }

        propertyNode = propertyNode.nextSibling();
    }
    return propertyMap;
}

FakeBluetoothInterface *FakeBluetoothManager::createBluetoothInterface(const QString &ubi)
{
    if (mBluetoothInterfaces.contains(ubi))
        return mBluetoothInterfaces[ubi];
    else
        return 0;
}

QStringList FakeBluetoothManager::bluetoothInputDevices() const
{
    return QStringList();
}

FakeBluetoothInputDevice *FakeBluetoothManager::createBluetoothInputDevice(const QString &ubi)
{
    if (mBluetoothInputDevices.contains(ubi))
        return mBluetoothInputDevices[ubi];
    else
        return 0;
}

KJob *FakeBluetoothManager::setupInputDevice(const QString  & /*ubi */)
{
    // TODO
    return NULL;
}
Solid::Control::Ifaces::BluetoothSecurity *FakeBluetoothManager::security(const QString &/*interface*/)
{
    return NULL;
}

void FakeBluetoothManager::removeInputDevice(const QString  & /*ubi */)
{
    //TODO
}


#include "fakebluetoothmanager.moc"

