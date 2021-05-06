/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_DATAENGINESCRIPT_H
#define PLASMA_DATAENGINESCRIPT_H

#include <KPluginInfo>

#include <plasma/dataengine.h>
#include <plasma/plasma_export.h>
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
    explicit DataEngineScript(QObject *parent = nullptr);
    ~DataEngineScript() override;

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
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    /**
     * @return absolute path to the main script file for this plasmoid
     */
    QString mainScript() const override;

    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    PLASMA_DEPRECATED_VERSION(5, 83, "Use kpackage API instead")
    Package package() const override;
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * @return the KPluginInfo associated with this dataengine
     *
     * @deprecated since 5.67 use metadata()
     */
    PLASMA_DEPRECATED_VERSION(5, 67, "Use KPluginMetaData metadata()")
    KPluginInfo description() const;
#endif

    /**
     * @return the KPluginMetaData associated with this dataengine
     *
     * @since 5.67
     */
    KPluginMetaData metadata() const;

    void setData(const QString &source, const QString &key, const QVariant &value);
    void setData(const QString &source, const QVariant &value);

    /**
     * Adds a set of values to a source
     * @param source the name of the source to set the data on
     * @values a key/value collection to add to the source
     * @since 4.5
     */
    void setData(const QString &source, const DataEngine::Data &values);

    void removeAllData(const QString &source);
    void removeData(const QString &source, const QString &key);
    void setMinimumPollingInterval(int minimumMs);
    int minimumPollingInterval() const;
    void setPollingInterval(uint frequency);
    void removeAllSources();
    void addSource(DataContainer *source);
    DataEngine::SourceDict containerDict() const;
    void removeSource(const QString &source);
    void updateAllSources();
    void forceImmediateUpdateOfAllVisualizations();

private:
    DataEngineScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE(libname, classname) K_PLUGIN_FACTORY(factory, registerPlugin<classname>();)

#define K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE_WITH_JSON(libname, classname, jsonFile)                                                                         \
    K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)                                                                                \
    K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)
} // Plasma namespace

#endif
