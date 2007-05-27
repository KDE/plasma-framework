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

#ifndef PLASMA_ENGINE_MANAGER_H
#define PLASMA_ENGINE_MANAGER_H

#include <QHash>
#include "dataengine.h"

namespace Plasma
{

/**
 * @brief DataEngine loader and life time manager
 *
 * Plasma::DataEngineManager provides facilities for listing, loading and
 * according to reference count unloading of DataEngines.
 **/
class PLASMA_EXPORT DataEngineManager
{
    public:
        /**
         * Singleton pattern accessor.
         */
        static DataEngineManager* self();

        /**
         * Default constructor. Usually the singleton method self() is the
         * preferred access mechanism.
         */
        DataEngineManager();
        ~DataEngineManager();

        /**
         * Returns a data engine object if one is loaded and available.
         * Otherwise, returns 0.
         *
         * @param name the name of the engine
         */
        Plasma::DataEngine* dataEngine(const QString& name) const;

        /**
         * Loads a data engine and increases the reference count on it.
         * This should be called once per object (or set of objects) using the
         * DataEngine. Afterwards, dataEngine should be used or the return
         * value cached. Call unloadDataEngine when finished with the engine.
         *
         * @param name the name of the engine
         * @return the data engine that was loaded, or 0 on failure.
         */
        Plasma::DataEngine* loadDataEngine(const QString& name);

        /**
         * Decreases the reference count on the engine. If the count reaches
         * zero, then the engine is deleted to save resources.
         */
        void unloadDataEngine(const QString& name);

        /**
         * Returns a listing of all known engines by name
         */
        static QStringList knownEngines();

    private:
        DataEngine::Dict m_engines;

        class Private;
        Private* const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
