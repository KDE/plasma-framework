/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef DATAENGINE_P_H
#define DATAENGINE_P_H

#include <QQueue>
#include <QTime>

#include <kplugininfo.h>

class QTime;

namespace Plasma
{

class Service;

class DataEnginePrivate
{
    public:
        DataEnginePrivate(DataEngine *e, KService::Ptr service);
        ~DataEnginePrivate();
        DataContainer *source(const QString &sourceName, bool createWhenMissing = true);
        void connectSource(DataContainer *s, QObject *visualization, uint pollingInterval,
                           Plasma::IntervalAlignment align, bool immediateCall = true);
        DataContainer *requestSource(const QString &sourceName, bool *newSource = 0);
        void trimQueue();
        void internalUpdateSource(DataContainer*);
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
         * @param methods ways to announce this engine on the network.
         */
        void publish(AnnouncementMethods methods, const QString &name);

        /**
         * remove this engine from the network.
         */
        void unpublish(const QString &name = QString());

        /**
         * @return whether or not this engine is published.
         */
        bool isPublished() const;

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

        DataEngine *q;
        KPluginInfo dataEngineDescription;
        int refCount;
        int checkSourcesTimerId;
        int updateTimerId;
        int minPollingInterval;
        QTime updateTimestamp;
        DataEngine::SourceDict sources;
        QQueue<DataContainer*> sourceQueue;
        QString icon;
        uint limit;
        bool valid;
        DataEngineScript *script;
        QString engineName;
        QString serviceName;
        Package *package;
        Service *publishedService;
};

} // Plasma namespace

#endif // multiple inclusion guard
