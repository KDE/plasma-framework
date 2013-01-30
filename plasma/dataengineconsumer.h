/*
 *   Copyright 2012 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DATAENGINECONSUMER_H
#define PLASMA_DATAENGINECONSUMER_H

#include <QtCore/QUrl>

#include <plasma/plasma_export.h>

namespace Plasma
{

class DataEngine;
class DataEngineConsumerPrivate;

/**
 * @class DataEngineConsumer plasma/dataengineconsumer.h <Plasma/DataEngineConsumer>
 *
 * @brief A class that makes it safe and easy to use DataEngines
 *
 * DataEngineConsumer provides access to DataEngines, which are internally reference
 * counted and shared between all users of them. The only public method provided is
 * dataEngine which returns engines upon request.
 *
 * When the DataEngineConsumer class is deleted, all engines accessed using it are
 * de-referenced and possibly deleted (in the case that there are no other users of
 * the engine in question).
 *
 * DataEngineConsumer can be subclassed by other C++ classes to allow this simple
 * API to be used directly from these classes in a convenient manner.
 **/
class PLASMA_EXPORT DataEngineConsumer
{
public:
    /**
     * Constructs a DataEngineConsumer
     **/
    DataEngineConsumer();
    ~DataEngineConsumer();

    /**
     * Returns a Plasma::DataEngine. It never returns a null pointer, and the
     * DataEngine returned should not be deleted. All DataEngines will be dereferenced
     * once this DataEngineConsumer instance is deleted.
     *
     * It is safe and fast to request the same engine more than once.
     *
     * @param name the name of the DataEngine. This corresponds to the plugin name
     *             of the DataEngine.
     * @param location if a non-empty URI is passed in, then a connection with a
     *                 remote DataEngine at the location is attempted to be made.
     *                 The returned pointer is a proxy object for this connection.
     *                 In the common case, location is always an empty URI (QUrl())
     *                 and the DataEngine is loaded locally.
     */
    DataEngine *dataEngine(const QString &name, const QUrl &location = QUrl());

private:
    DataEngineConsumerPrivate * const d;
};

} // namespace Plasma

#endif


