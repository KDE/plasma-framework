/*
 * Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QUrl>
#include <QDebug>

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
    qDebug() << "goin to fetch" << source << url << url.scheme();
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
K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(org.kde.examples.customDataContainers, DataContainersEngine, "plasma-dataengine-example-customDataContainers.json")

// include the moc file so the build system makes it for us
#include "customDataContainersEngine.moc"

