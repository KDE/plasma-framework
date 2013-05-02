/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "service.h"
#include "private/service_p.h"

#include "config-plasma.h"

#include <QFile>
#include <QTimer>
#include <QQuickItem>

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>

#include <dnssd/publicservice.h>
#include <dnssd/servicebrowser.h>
#include <qstandardpaths.h>

#include "configloader.h"
#include "version.h"
#include "private/configloader_p.h"
#include "private/configloaderhandler_p.h"
#include "pluginloader.h"

namespace Plasma
{

class ConfigLoaderHandlerMap : public ConfigLoaderHandler
{
public:
    ConfigLoaderHandlerMap(ConfigLoader *config, ConfigLoaderPrivate *d)
        : ConfigLoaderHandler(config, d)
    {}

    void addItem();

//private:
    QVariantMap m_map;
};

void ConfigLoaderHandlerMap::addItem()
{
    if (name().isEmpty()) {
        if (key().isEmpty()) {
            return;
        }

        setName(key());
    } else if (key().isEmpty()) {
        if (name().isEmpty()) {
            return;
        }

        setKey(name());
    }

    QVariantMap map;
    if (m_map.contains(currentGroup())) {
        map = m_map[currentGroup()].value<QVariantMap>();
    } else {
        m_map[currentGroup()] = QVariantMap();
    }


    if (type() == "bool") {
        bool defaultVal = defaultValue().toLower() == "true";
        map[key()] = defaultVal;
    } else if (type() == "color") {
        map[key()] = QColor(defaultValue());
    } else if (type() == "datetime") {
        map[key()] = QDateTime::fromString(defaultValue());
    } else if (type() == "enum") {
        key() = (key().isEmpty()) ? name() : key();
        map[key()] = defaultValue().toUInt();
    } else if (type() == "font") {
        map[key()] = QFont(defaultValue());
    } else if (type() == "int") {
        map[key()] = defaultValue().toInt();
    } else if (type() == "password") {
        map[key()] = defaultValue();
    } else if (type() == "path") {
        map[key()] = defaultValue();
    } else if (type() == "string") {
        map[key()] = defaultValue();
    } else if (type() == "stringlist") {
        //FIXME: the split() is naive and will break on lists with ,'s in them
        map[key()] = defaultValue().split(',');
    } else if (type() == "uint") {
        map[key()] = defaultValue().toUInt();
    } else if (type() == "url") {
        setKey((key().isEmpty()) ? name() : key());
        map[key()] = QUrl::fromUserInput(defaultValue());
    } else if (type() == "double") {
        map[key()] = defaultValue().toDouble();
    } else if (type() == "intlist") {
        QStringList tmpList = defaultValue().split(',');
        QList<int> defaultList;
        foreach (const QString &tmp, tmpList) {
            defaultList.append(tmp.toInt());
        }
        map[key()] = QVariant::fromValue(defaultList);
    } else if (type() == "longlong") {
        map[key()] = defaultValue().toLongLong();
    } else if (type() == "point") {
        QPoint defaultPoint;
        QStringList tmpList = defaultValue().split(',');
        if (tmpList.size() >= 2) {
            defaultPoint.setX(tmpList[0].toInt());
            defaultPoint.setY(tmpList[1].toInt());
        }
        map[key()] = defaultPoint;
    } else if (type() == "rect") {
        QRect defaultRect;
        QStringList tmpList = defaultValue().split(',');
        if (tmpList.size() >= 4) {
            defaultRect.setCoords(tmpList[0].toInt(), tmpList[1].toInt(),
                                  tmpList[2].toInt(), tmpList[3].toInt());
        }
        map[key()] = tmpList;
    } else if (type() == "size") {
        QSize defaultSize;
        QStringList tmpList = defaultValue().split(',');
        if (tmpList.size() >= 2) {
            defaultSize.setWidth(tmpList[0].toInt());
            defaultSize.setHeight(tmpList[1].toInt());
        }
        map[key()] = tmpList;
    } else if (type() == "ulonglong") {
        map[key()] = defaultValue().toULongLong();
    }

    m_map[currentGroup()] = map;
}

Service::Service(QObject *parent)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    ConfigLoaderPrivate *cl = new ConfigLoaderPrivate;
    ConfigLoaderHandlerMap map(0, cl);
}

Service::Service(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    Q_UNUSED(args)
    ConfigLoaderPrivate *cl = new ConfigLoaderPrivate;
    ConfigLoaderHandlerMap map(0, cl);
}

Service::~Service()
{
    delete d;
}

void ServicePrivate::associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.remove(static_cast<QWidget*>(obj));
}

void ServicePrivate::associatedItemDestroyed(QObject *obj)
{
    associatedItems.remove(static_cast<QQuickItem*>(obj));
}

KConfigGroup ServicePrivate::dummyGroup()
{
    if (!dummyConfig) {
        dummyConfig = new KConfig(QString(), KConfig::SimpleConfig);
    }

    return KConfigGroup(dummyConfig, "DummyGroup");
}

void Service::setDestination(const QString &destination)
{
    d->destination = destination;
}

QString Service::destination() const
{
    return d->destination;
}

QStringList Service::operationNames() const
{
    if (!d->config) {
#ifndef NDEBUG
        kDebug() << "No valid operations scheme has been registered";
#endif
        return QStringList();
    }

    return d->config->groupList();
}

KConfigGroup Service::operationDescription(const QString &operationName)
{
    if (!d->config) {
#ifndef NDEBUG
        kDebug() << "No valid operations scheme has been registered";
#endif
        return d->dummyGroup();
    }

    d->config->writeConfig();
    KConfigGroup params(d->config->config(), operationName);
    //kDebug() << "operation" << operationName
    //         << "requested, has keys" << params.keyList() << "from"
    //         << d->config->config()->name();
    return params;
}

QHash<QString, QVariant> Service::parametersFromDescription(const KConfigGroup &description)
{
    QHash<QString, QVariant> params;

    if (!d->config || !description.isValid()) {
        return params;
    }

    const QString op = description.name();
    foreach (const QString &key, description.keyList()) {
        KConfigSkeletonItem *item = d->config->findItem(op, key);
        if (item) {
            params.insert(key, description.readEntry(key, item->property()));
        }
    }

    return params;
}

ServiceJob *Service::startOperationCall(const KConfigGroup &description, QObject *parent)
{
    // TODO: nested groups?
    ServiceJob *job = 0;
    const QString op = description.isValid() ? description.name() : QString();

    if (!d->config) {
#ifndef NDEBUG
        kDebug() << "No valid operations scheme has been registered";
#endif
    } else if (!op.isEmpty() && d->config->hasGroup(op)) {
        if (d->disabledOperations.contains(op)) {
#ifndef NDEBUG
            kDebug() << "Operation" << op << "is disabled";
#endif
        } else {
            QHash<QString, QVariant> params = parametersFromDescription(description);
            job = createJob(op, params);
        }
    } else {
#ifndef NDEBUG
    kDebug() << op << "is not a valid group; valid groups are:" << d->config->groupList();
#endif
    }

    if (!job) {
        job = new NullServiceJob(d->destination, op, this);
    }

    job->setParent(parent ? parent : this);
    QTimer::singleShot(0, job, SLOT(autoStart()));
    return job;
}

void Service::associateItem(QQuickItem *widget, const QString &operation)
{
    if (!widget) {
        return;
    }

    disassociateItem(widget);
    d->associatedItems.insert(widget, operation);
    connect(widget, SIGNAL(destroyed(QObject*)),
            this, SLOT(associatedItemDestroyed(QObject*)));

    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateItem(QQuickItem *widget)
{
    if (!widget) {
        return;
    }

    disconnect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(associatedItemDestroyed(QObject*)));
    d->associatedItems.remove(widget);
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;

    // now reset the config, which may be based on our name
    delete d->config;
    d->config = 0;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    registerOperationsScheme();

    emit serviceReady(this);
}

void Service::setOperationEnabled(const QString &operation, bool enable)
{
    if (!d->config || !d->config->hasGroup(operation)) {
        return;
    }

    if (enable) {
        d->disabledOperations.remove(operation);
    } else {
        d->disabledOperations.insert(operation);
    }

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }

    {
        QHashIterator<QQuickItem *, QString> it(d->associatedItems);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }
}

bool Service::isOperationEnabled(const QString &operation) const
{
    return d->config && d->config->hasGroup(operation) && !d->disabledOperations.contains(operation);
}

void Service::setOperationsScheme(QIODevice *xml)
{
    delete d->config;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    KSharedConfigPtr c = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    d->config = new ConfigLoader(c, xml, this);
    d->config->d->setWriteDefaults(true);

    emit operationsChanged();

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }

    {
        QHashIterator<QQuickItem *, QString> it(d->associatedItems);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }
}

void Service::registerOperationsScheme()
{
    if (d->config) {
        // we've already done our job. let's go home.
        return;
    }

    if (d->name.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "No name found";
#endif
        return;
    }

    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/services/" + d->name + ".operations");

    if (path.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "Cannot find operations description:" << d->name << ".operations";
#endif
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

} // namespace Plasma



#include "moc_service.cpp"
