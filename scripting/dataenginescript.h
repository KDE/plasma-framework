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

#include <kgenericfactory.h>
#include <kplugininfo.h>

#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>
#include <plasma/scripting/scriptengine.h>

namespace Plasma
{

class DataEngineScriptPrivate;
class Service;

/**
 * @class DataEngineScript plasma/scripting/dataenginescript.h <Plasma/Scripting/DataEngineScript>
 *
 * @short Provides a restricted interface for scripting a DataEngine
 */
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
    DataEngine *dataEngine() const;

    /**
     * @return a list of all the data sources available via this DataEngine
     *         Whether these sources are currently available (which is what
     *         the default implementation provides) or not is up to the
     *         DataEngine to decide. By default, this returns dataEngine()->sources()
     */
     virtual QStringList sources() const;

    /**
     * Called when the script should create a source that does not currently
     * exist.
     *
     * @param name the name of the source that should be created
     * @return true if a DataContainer was set up, false otherwise
     */
    virtual bool sourceRequestEvent(const QString &name);

    /**
     * Called when the script should refresh the data contained in a given
     * source.
     *
     * @param source the name of the source that should be updated
     * @return true if the data was changed, or false if there was no
     *         change or if the change will occur later
     **/
    virtual bool updateSourceEvent(const QString &source);

    /**
     * @param source the source to targe the Service at
     * @return a Service that has the source as a destination. The service
     *         is parented to the DataEngine, but may be deleted by the
     *         caller when finished with it
     */
    virtual Service *serviceForSource(const QString &source);

protected:
    /**
     * @return absolute path to the main script file for this plasmoid
     */
    QString mainScript() const;

    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    const Package *package() const;

    /**
     * @return the KPluginInfo associated with this plasmoid
     */
    KPluginInfo description() const;

    void setData(const QString &source, const QString &key,
                 const QVariant &value);
    void setData(const QString &source, const QVariant &value);
    void removeAllData(const QString &source);
    void removeData(const QString &source, const QString &key);
    void setMaxSourceCount(uint limit);
    void setMinimumPollingInterval(int minimumMs);
    int  minimumPollingInterval() const;
    void setPollingInterval(uint frequency);
    void removeAllSources();
    void addSource(DataContainer *source);
    DataEngine::SourceDict containerDict() const;
    void setName(const QString &name);
    void setIcon(const QString &icon);
    void scheduleSourcesUpdated();
    void removeSource(const QString &source);
    void updateAllSources();
    void forceImmediateUpdateOfAllVisualizations();

private:
    DataEngineScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_dataenginescriptengine_" #libname))

} //Plasma namespace

#endif
