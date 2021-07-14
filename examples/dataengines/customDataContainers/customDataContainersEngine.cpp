/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include <QDebug>
#include <QUrl>

#include "customDataContainersEngine.h"

#include "httpContainer.h"

/*
 This DataEngine shows how to use a subclass of DataContainer to create and
 manage sources. This is particularly useful when managing asynchronous requests,
 such as sources that reflect network, D-Bus, etc. results.
*/

DataContainersEngine::DataContainersEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // We've passed the constructor's args to our parent class.
    // We're done for now!
}

bool DataContainersEngine::sourceRequestEvent(const QString &source)
{
    // This engine will fetch webpages over http. First thing we do is check
    // the source to make sure it is indeed an http URL.
    QUrl url(source);
    qDebug() << "going to fetch" << source << url << url.scheme();
    if (!url.scheme().startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
        return false;
    }

    // Create a HttpContainer, which is a subclass of Plasma::DataContainer
    HttpContainer *container = new HttpContainer(url, this);

    // Set the object name to be the same as the source name; DataEngine
    // relies on this to identify the container. This could also be done
    // in HttpContainer's constructor, but for the sake of this example
    // we're dong it here to show that it must be done *before* the
    // DataContainer subclass is passed to addSource
    container->setObjectName(source);

    // Now we tell Plasma::DataEngine about this new container
    addSource(container);

    // Since we successfully set up the source, return true
    return true;
}

bool DataContainersEngine::updateSourceEvent(const QString &source)
{
    HttpContainer *container = qobject_cast<HttpContainer *>(containerForSource(source));
    if (container) {
        container->fetchUrl();
    }

    // HttpContainer is asynchronous, so the data hasn't actually been updated yet. So
    // we return false here to let the DataEngine know that nothing has changed quite yet.
    return false;
}

// export the plugin; use the plugin name and the class name
K_PLUGIN_CLASS_WITH_JSON(DataContainersEngine, "plasma-dataengine-example-customDataContainers.json")

// include the moc file so the build system makes it for us
#include "customDataContainersEngine.moc"
