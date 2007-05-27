/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DATASOURCE_H
#define PLASMA_DATASOURCE_H

#include <QtCore/QHash>
#include <QtCore/QObject>

#include <plasma_export.h>
#include <dataengine.h>

namespace Plasma
{
/**
 * @brief A set of data exported via a DatEngine
 *
 * Plasma::DataSource wraps the data exported by a DataEngine
 * implementation, providing a generic wrapper for the data.
 *
 * A DataSource may have zero or more associated pieces of data which
 * are keyed by strings. The data itself is stored as QVariants. This allows
 * easy and flexible retrieval of the information associated with this object
 * without writing DataSource or DataEngine specific code in visualizations.
 **/
class PLASMA_EXPORT DataSource : public QObject
{
    Q_OBJECT

    public:
        typedef QHash<QString, DataSource*> Dict;
        typedef QHash<QString, Dict> Grouping;

        /**
         * Constructs a default DataSource, which has no name or data
         * associated with it
         **/
        explicit DataSource(QObject* parent = 0);
        virtual ~DataSource();

        /**
         * Returns the data for this DataSource
         **/
        const DataEngine::Data data() const;

        /**
         * Set a value for a key. This also marks this source as needing
         * to signal an update. After calling this, a call to checkForUpdate()
         * is done by the engine. This allows for batching updates.
         *
         * @param key a string used as the key for the data
         * @param value a QVariant holding the actual data
         **/
        void setData(const QString& key, const QVariant& value);

        /**
         * Checks for whether the data has changed and therefore an update
         * signal needs to be emitted.
         **/
        void checkForUpdate();

    Q_SIGNALS:
        /**
         * Emitted when the data has been updated, allowing visualization to
         * reflect the new data.
         **/
        void updated(const QString& source, const Plasma::DataEngine::Data& data);

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
