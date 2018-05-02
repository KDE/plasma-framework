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

#include "sourcesOnRequest.h"

#include <Plasma/DataContainer>

#include <QStandardItemModel>

/*
 This DataEngine shows how to created sources on demand as they are requested
 and update them on visualization-requested update intervals.
 It also shows how to bind and arbitrary QAbstractItemModel to a source
*/

SourcesOnRequestEngine::SourcesOnRequestEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // We've passed the constructor's args to our parent class.
    // We're done for now!
}

bool SourcesOnRequestEngine::sourceRequestEvent(const QString &source)
{
    // When this method is called we can assume that:
    //   * the source does not exist yet
    //   * the source name parameter passed in is not an empty string
    //   * we are free to reject creating the source if we wish

    // We're going to reject any sources that start with the letter 'a'
    // to demonstrate how to reject a request in a DataEngine.
    if (source.startsWith(QLatin1Char('a')) || source.startsWith(QLatin1Char('A'))) {
        return false;
    }

    // For every other source, we're going to start an update count for it.
    // Critically, we create a source before returning that has the exact
    // same key as the source string. We MUST NOT create a source of a different
    // name as that will cause unexpected results for the visualization.
    // In such a case the DataEngine will remain happy and Do The Right Thing(tm)
    // but the visualization will not get the source connected to it as it
    // expects. So ALWAYS key the new data by the source string as below:
    setData(source, QStringLiteral("Update Count"), 0);

    if (!modelForSource(source)) {
        QStandardItemModel *m = new QStandardItemModel;
        m->appendRow(new QStandardItem(QStringLiteral("Item1, first update")));
        m->appendRow(new QStandardItem(QStringLiteral("Item2, first update")));
        m->appendRow(new QStandardItem(QStringLiteral("Item3, first update")));
        setModel(source, m);
    }

    // as we successfully set up the source, return true
    return true;
}

bool SourcesOnRequestEngine::updateSourceEvent(const QString &source)
{
    // Whenever a visualization has requested an update, such as when passing
    // an update interval to DataEngine::connectSource, this method will be called.
    // When this method is called we can assume that:
    //     * the source exists
    //     * it hasn't been updated more frequently than the minimum update interval
    //
    // If not data is set in this method, then the update is skipped for the visualiation
    // and that is just fine.
    //
    // We can also set data for other sources here if we wish, but as with
    // sourceRequestEvent this may not be what the visualization expects. Unlike in
    // sourceRequestEvent, however, this will result in expected behavior: visualizations
    // connected to the sources which have setData called for them will be notified
    // of these changes.
    const int updateCount = containerForSource(source)->data().value(QStringLiteral("Update Count")).toInt() + 1;
    setData(source, QStringLiteral("Update Count"), updateCount);

    QStandardItemModel *m = qobject_cast<QStandardItemModel *>(modelForSource(source));
    if (m) {
        m->clear();
        m->appendRow(new QStandardItem(QStringLiteral("Item1, update %1").arg(updateCount)));
        m->appendRow(new QStandardItem(QStringLiteral("Item2, update %1").arg(updateCount)));
        m->appendRow(new QStandardItem(QStringLiteral("Item3, update %1").arg(updateCount)));
    }

    // Since we updated the source immediately here, we need to return true so the DataEngine
    // knows to continue with the update notification for visualizations.
    return true;
}

// export the plugin; use the plugin name and the class name
K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(org.kde.examples.sourcesOnRequest, SourcesOnRequestEngine, "plasma-dataengine-example-sourcesOnRequest.json")

// include the moc file so the build system makes it for us
#include "sourcesOnRequest.moc"

