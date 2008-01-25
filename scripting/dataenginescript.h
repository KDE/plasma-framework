/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DATAENGINESCRIPT_H
#define PLASMA_DATAENGINESCRIPT_H

#include <plasma/plasma_export.h>
#include <plasma/scripting/scriptengine.h>

namespace Plasma
{

class DataEngine;

class PLASMA_EXPORT DataEngineScript : public ScriptEngine
{
    Q_OBJECT

public:
    /**
     * Default constructor for a DataEngineScript.
     * Subclasses should not attempt to access the Plasma::DataEngine
     * associated with this DataEngineScript in the constructor. All
     * such set up that requires the DataEngine itself should be done
     * in the init() method.
     */
    explicit DataEngineScript(QObject *parent = 0);
    ~DataEngineScript();

    /**
     * Sets the Plasma::DataEngine associated with this DataEngineScript
     */
    void setDataEngine(DataEngine *dataEngine);

    /**
     * Returns the Plasma::DataEngine associated with this script component
     */
    DataEngine* dataEngine() const;

private:
    class Private;
    Private * const d;
};

#define K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_dataenginescriptengine_" #libname))

} //Plasma namespace

#endif
