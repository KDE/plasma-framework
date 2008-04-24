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

#ifndef PLASMA_DATACONTAINER_H
#define PLASMA_DATACONTAINER_H

#include <QtCore/QHash>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>

namespace Plasma
{
/**
 * @brief A set of data exported via a DataEngine
 *
 * Plasma::DataContainer wraps the data exported by a DataEngine
 * implementation, providing a generic wrapper for the data.
 *
 * A DataContainer may have zero or more associated pieces of data which
 * are keyed by strings. The data itself is stored as QVariants. This allows
 * easy and flexible retrieval of the information associated with this object
 * without writing DataContainer or DataEngine specific code in visualizations.
 **/
class PLASMA_EXPORT DataContainer : public QObject
{
    Q_OBJECT

    public:
        //typedef QHash<QString, DataEngine::SourceDict> Grouping;

        /**
         * Constructs a default DataContainer, which has no name or data
         * associated with it
         **/
        explicit DataContainer(QObject* parent = 0);
        virtual ~DataContainer();

        /**
         * Returns the data for this DataContainer
         **/
        const DataEngine::Data data() const;

        /**
         * Set a value for a key. This also marks this source as needing
         * to signal an update. After calling this, a call to checkForUpdate()
         * is done by the engine. This allows for batching updates.
         *
         * @param key a string used as the key for the data
         * @param value a QVariant holding the actual data. If a null or invalid
         *              QVariant is passed in and the key currently exists in the
         *              data, then the data entry is removed
         **/
        void setData(const QString& key, const QVariant& value);

        /**
         * Removes all data currently associated with this source
         **/
        void removeAllData();

        /**
         * Checks for whether the data has changed and therefore an update
         * signal needs to be emitted.
         **/
        void checkForUpdate();

        /**
         * Returns how long ago, in msecs, that the data in this container was last updated
         **/
        int timeSinceLastUpdate() const;

        /**
         * @internal
         **/
        bool hasUpdates() const;

        /**
         * Indicates that the data should be treated as dirty the next time hasUpdates() is called.
         *
         * why? because if one SignalRelay times out just after another, the minimum update
         * interval stops a real update from being done - but that relay still needs to be given
         * data, because it won't have been in the queue and won't have gotten that last update.
         * when it checks hasUpdates() we'll lie, and then everything will return to normal.
         **/
        void setNeedsUpdate(bool update = true);

        /**
         * @return true if the visualization is currently connected
         */
        bool visualizationIsConnected(QObject *visualization) const;

        /**
         * Connects an object to this DataContainer. May be called repeatedly
         * for the same visualization without side effects
         *
         * @param visualization the object to connect to this DataContainer
         * @param pollingInterval the time in milliseconds between updates
         **/
        void connectVisualization(QObject* visualization, uint pollingInterval, Plasma::IntervalAlignment alignment);

    public Q_SLOTS:
        /**
         * Check if the DataContainer is still in use.
         * If not the signal "becameUnused" will be emitted.
         * Warning: The DataContainer may be invalid after calling this function.
         */
        void checkUsage();

        /**
         * Disconnects an object from this DataContainer.
         **/
        void disconnectVisualization(QObject* visualization);

    Q_SIGNALS:
        /**
         * Emitted when the data has been updated, allowing visualization to
         * reflect the new data.
         **/
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

        /**
         * Emitted when this source becomes unused
         **/
        void becameUnused(const QString& source);

        /**
         * Emitted when the source, usually due to an internal timer firing,
         * requests to be updated.
         **/
        void updateRequested(DataContainer *source);

    private:
        friend class SignalRelay;
        class Private;
        Private* const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
