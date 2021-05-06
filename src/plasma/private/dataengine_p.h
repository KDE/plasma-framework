/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DATAENGINE_P_H
#define DATAENGINE_P_H

#include <QElapsedTimer>

#include <KPluginMetaData>

namespace Plasma
{
class Service;

class DataEnginePrivate
{
public:
    DataEnginePrivate(DataEngine *e, const KPluginMetaData &info, const QVariantList &args = QVariantList());
    ~DataEnginePrivate();
    DataContainer *source(const QString &sourceName, bool createWhenMissing = true);
    void connectSource(DataContainer *s, QObject *visualization, uint pollingInterval, Plasma::Types::IntervalAlignment align, bool immediateCall = true);
    DataContainer *requestSource(const QString &sourceName, bool *newSource = nullptr);
    void internalUpdateSource(DataContainer *);
    void setupScriptSupport();

    /**
     * Reference counting method. Calling this method increases the count
     * by one.
     **/
    void ref();

    /**
     * Reference counting method. Calling this method decreases the count
     * by one.
     **/
    void deref();

    /**
     * Reference counting method. Used to determine if this DataEngine is
     * used.
     * @return true if the reference count is non-zero
     **/
    bool isUsed() const;

    /**
     * a datacontainer has been destroyed, clean up stuff
     */
    void sourceDestroyed(QObject *object);

    /**
     * stores the source
     * @param sourceName the name of the source to store
     */
    void storeSource(DataContainer *source) const;

    /**
     * stores all sources marked for storage
     */
    void storeAllSources();

    /**
     * retrieves source data
     * @param the data container to populate
     */
    void retrieveStoredData(DataContainer *s);

    /**
     * Call this method when you call setData directly on a DataContainer instead
     * of using the DataEngine::setData methods.
     * If this method is not called, no dataUpdated(..) signals will be emitted!
     */
    void scheduleSourcesUpdated();

    DataEngine *q;
    KPluginMetaData dataEngineDescription;
    int refCount;
    int checkSourcesTimerId;
    int updateTimerId;
    int minPollingInterval;
    QElapsedTimer updateTimer;
    DataEngine::SourceDict sources;
    bool valid;
    DataEngineScript *script;
    QString serviceName;
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    Package *package = nullptr;
#endif
    QString waitingSourceRequest;
};

} // Plasma namespace

#endif // multiple inclusion guard
