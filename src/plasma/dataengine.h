/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_DATAENGINE_H
#define PLASMA_DATAENGINE_H

#include <QHash>
#include <QObject>
#include <QStringList>

#include <KPluginInfo>
#include <KService>

#include <plasma/plasma.h>
#include <plasma/service.h>
#include <plasma/version.h>

class QAbstractItemModel;

namespace Plasma
{
class DataContainer;
class DataEngineScript;
class Package;
class Service;
class DataEnginePrivate;

/**
 * @class DataEngine plasma/dataengine.h <Plasma/DataEngine>
 *
 * @short Data provider for plasmoids (Plasma plugins)
 *
 * This is the base class for DataEngines, which provide access to bodies of
 * data via a common and consistent interface. The common use of a DataEngine
 * is to provide data to a widget for display. This allows a user interface
 * element to show all sorts of data: as long as there is a DataEngine, the
 * data is retrievable.
 *
 * DataEngines are loaded as plugins on demand and provide zero, one or more
 * data sources which are identified by name. For instance, a network
 * DataEngine might provide a data source for each network interface.
 **/
class PLASMA_EXPORT DataEngine : public QObject
{
    Q_OBJECT

public:
    typedef QHash<QString, DataEngine *> Dict;
    typedef QMap<QString, QVariant> Data;
    typedef QMapIterator<QString, QVariant> DataIterator;
    typedef QHash<QString, DataContainer *> SourceDict;

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * Constructor.
     *
     * @param parent The parent object.
     * @param plugin plugin info that describes the engine
     *
     * @deprecated since 5.67
     **/
    PLASMA_DEPRECATED_VERSION(5, 67, "Use KPluginMetaData")
    explicit DataEngine(const KPluginInfo &plugin, QObject *parent = nullptr);
#endif

    /**
     * Constructor.
     *
     * @param parent The parent object.
     * @param plugin metadata that describes the engine
     *
     * @since 5.67
     */
    explicit DataEngine(const KPluginMetaData &plugin, QObject *parent = nullptr);

    explicit DataEngine(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    ~DataEngine() override;

    /**
     * @return a list of all the data sources available via this DataEngine
     *         Whether these sources are currently available (which is what
     *         the default implementation provides) or not is up to the
     *         DataEngine to decide.
     **/
    virtual QStringList sources() const;

    /**
     * @param source the source to target the Service at
     * @return a Service that has the source as a destination. The service
     *         is parented to the DataEngine, but should be deleted by the
     *         caller when finished with it
     */
    Q_INVOKABLE virtual Service *serviceForSource(const QString &source);

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * @return description of the plugin that implements this DataEngine
     *
     * @deprecated since 5.67, use metadata
     */
    PLASMA_DEPRECATED_VERSION(5, 67, "Use metadata()")
    KPluginInfo pluginInfo() const;
#endif

    /**
     * @return description of the plugin that implements this DataEngine
     *
     * @since 5.67
     */
    KPluginMetaData metadata() const;

    /**
     * Connects a source to an object for data updates. The object must
     * have a slot with the following signature:
     * @code
     * void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
     * @endcode
     *
     * The data is a QHash of QVariants keyed by QString names, allowing
     * one data source to provide sets of related data.
     *
     * @param source the name of the data source
     * @param visualization the object to connect the data source to
     * @param pollingInterval the frequency, in milliseconds, with which to check for updates;
     *                        a value of 0 (the default) means to update only
     *                        when there is new data spontaneously generated
     *                        (e.g. by the engine); any other value results in
     *                        periodic updates from this source. This value is
     *                        per-visualization and can be handy for items that require
     *                        constant updates such as scrolling graphs or clocks.
     *                        If the data has not changed, no update will be sent.
     * @param intervalAlignment the number of ms to align the interval to
     **/
    Q_INVOKABLE void connectSource(const QString &source,
                                   QObject *visualization,
                                   uint pollingInterval = 0,
                                   Plasma::Types::IntervalAlignment intervalAlignment = Types::NoAlignment) const;

    /**
     * Connects all currently existing sources to an object for data updates.
     * The object must have a slot with the following signature:
     * @code
     * void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
     * @endcode
     *
     * The data is a QHash of QVariants keyed by QString names, allowing
     * one data source to provide sets of related data.
     *
     * This method may be called multiple times for the same visualization
     * without side-effects. This can be useful to change the pollingInterval.
     *
     * Note that this method does not automatically connect sources that
     * may appear later on. Connecting and responding to the sourceAdded signal
     * is still required to achieve that.
     *
     * @param visualization the object to connect the data source to
     * @param pollingInterval the frequency, in milliseconds, with which to check for updates;
     *                        a value of 0 (the default) means to update only
     *                        when there is new data spontaneously generated
     *                        (e.g. by the engine); any other value results in
     *                        periodic updates from this source. This value is
     *                        per-visualization and can be handy for items that require
     *                        constant updates such as scrolling graphs or clocks.
     *                        If the data has not changed, no update will be sent.
     * @param intervalAlignment the number of ms to align the interval to
     **/
    Q_INVOKABLE void
    connectAllSources(QObject *visualization, uint pollingInterval = 0, Plasma::Types::IntervalAlignment intervalAlignment = Types::NoAlignment) const;

    /**
     * Disconnects a source from an object that was receiving data updates.
     *
     * @param source the name of the data source
     * @param visualization the object to connect the data source to
     **/
    Q_INVOKABLE void disconnectSource(const QString &source, QObject *visualization) const;

    /**
     * Retrieves a pointer to the DataContainer for a given source. This method
     * should not be used if possible. An exception is for script engines that
     * can not provide a QMetaObject as required by connectSource for the initial
     * call to dataUpdated. Using this method, such engines can provide their own
     * connectSource API.
     *
     * @param source the name of the source.
     * @return pointer to a DataContainer, or zero on failure
     **/
    Q_INVOKABLE DataContainer *containerForSource(const QString &source);

    /**
     * @return The model associated to a source if any. The ownership of the model stays with the DataContainer.
     *         Returns 0 if there isn't any model associated or if the source doesn't exists.
     */
    QAbstractItemModel *modelForSource(const QString &source);

    /**
     * Returns true if this engine is valid, otherwise returns false
     *
     * @return true if the engine is valid
     **/
    bool isValid() const;

    /**
     * Returns true if the data engine is empty, which is to say that it has no
     * data sources currently.
     *
     * @return true if the engine has no sources currently
     */
    bool isEmpty() const;

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 83)
    /**
     * Accessor for the associated Package object if any.
     *
     * @return the Package object, or 0 if none
     **/
    PLASMA_DEPRECATED_VERSION(5, 83, "Use kpackage API instead")
    Package package() const;
#endif

Q_SIGNALS:
    /**
     * Emitted when a new data source is created
     *
     * Note that you do not need to emit this yourself unless
     * you are reimplementing sources() and want to advertise
     * that a new source is available (but hasn't been created
     * yet).
     *
     * @param source the name of the new data source
     **/
    void sourceAdded(const QString &source);

    /**
     * Emitted when a data source is removed.
     *
     * Note that you do not need to emit this yourself unless
     * you have reimplemented sources() and want to signal that
     * a source that was available but was never created is no
     * longer available.
     *
     * @param source the name of the data source that was removed
     **/
    void sourceRemoved(const QString &source);

protected:
    /**
     * When a source that does not currently exist is requested by the
     * consumer, this method is called to give the DataEngine the
     * opportunity to create one.
     *
     * The name of the data source (e.g. the source parameter passed into
     * setData) must be the same as the name passed to sourceRequestEvent
     * otherwise the requesting visualization may not receive notice of a
     * data update.
     *
     * If the source can not be populated with data immediately (e.g. due to
     * an asynchronous data acquisition method such as an HTTP request)
     * the source must still be created, even if it is empty. This can
     * be accomplished in these cases with the follow line:
     *
     *      setData(name, DataEngine::Data());
     *
     * @param source the name of the source that has been requested
     * @return true if a DataContainer was set up, false otherwise
     */
    virtual bool sourceRequestEvent(const QString &source);

    /**
     * Called by internal updating mechanisms to trigger the engine
     * to refresh the data contained in a given source. Reimplement this
     * method when using facilities such as setPollingInterval.
     * @see setPollingInterval
     *
     * @param source the name of the source that should be updated
     * @return true if the data was changed, or false if there was no
     *         change or if the change will occur later
     **/
    virtual bool updateSourceEvent(const QString &source);

    /**
     * Sets a value for a data source. If the source
     * doesn't exist then it is created.
     *
     * @param source the name of the data source
     * @param value the data to associated with the source
     **/
    void setData(const QString &source, const QVariant &value);

    /**
     * Sets a value for a data source. If the source
     * doesn't exist then it is created.
     *
     * @param source the name of the data source
     * @param key the key to use for the data
     * @param value the data to associated with the source
     **/
    void setData(const QString &source, const QString &key, const QVariant &value);

    /**
     * Adds a set of data to a data source. If the source
     * doesn't exist then it is created.
     *
     * @param source the name of the data source
     * @param data the data to add to the source
     **/
    void setData(const QString &source, const QVariantMap &data);

    /**
     * Removes all the data associated with a data source.
     *
     * @param source the name of the data source
     **/
    void removeAllData(const QString &source);

    /**
     * Removes a data entry from a source
     *
     * @param source the name of the data source
     * @param key the data entry to remove
     **/
    void removeData(const QString &source, const QString &key);

    /**
     * Associates a model to a data source. If the source
     * doesn't exist then it is created. The source will have the key "HasModel" to easily indicate there is a model present.
     *
     * The ownership of the model is transferred to the DataContainer,
     * so the model will be deleted when a new one is set or when the
     * DataContainer itself is deleted. As the DataContainer, it will be
     * deleted when there won't be any
     * visualization associated to this source.
     *
     * @param source the name of the data source
     * @param model the model instance
     */
    void setModel(const QString &source, QAbstractItemModel *model);

    /**
     * Adds an already constructed data source. The DataEngine takes
     * ownership of the DataContainer object. The objectName of the source
     * is used for the source name.
     *
     * @param source the DataContainer to add to the DataEngine
     **/
    void addSource(DataContainer *source);

    /**
     * Sets the minimum amount of time, in milliseconds, that must pass between
     * successive updates of data. This can help prevent too many updates happening
     * due to multiple update requests coming in, which can be useful for
     * expensive (time- or resource-wise) update mechanisms.
     *
     * The default minimumPollingInterval is -1, or "never perform automatic updates"
     *
     * @param minimumMs the minimum time lapse, in milliseconds, between updates.
     *                A value less than 0 means to never perform automatic updates,
     *                a value of 0 means update immediately on every update request,
     *                a value >0 will result in a minimum time lapse being enforced.
     **/
    void setMinimumPollingInterval(int minimumMs);

    /**
     * @return the minimum time between updates. @see setMinimumPollingInterval
     **/
    int minimumPollingInterval() const;

    /**
     * Sets up an internal update tick for all data sources. On every update,
     * updateSourceEvent will be called for each applicable source.
     * @see updateSourceEvent
     *
     * @param frequency the time, in milliseconds, between updates. A value of 0
     *                  will stop internally triggered updates.
     **/
    void setPollingInterval(uint frequency);

    /**
     * Removes all data sources
     **/
    void removeAllSources();

    /**
     * Sets whether or not this engine is valid, e.g. can be used.
     * In practice, only the internal fall-back engine, the NullEngine
     * should have need for this.
     *
     * @param valid whether or not the engine is valid
     **/
    void setValid(bool valid);

    /**
     * @return the list of active DataContainers.
     */
    QHash<QString, DataContainer *> containerDict() const;

    /**
     * Reimplemented from QObject
     **/
    void timerEvent(QTimerEvent *event) override;

    /**
     * Sets a source to be stored for easy retrieval
     * when the real source of the data (usually a network connection)
     * is unavailable.
     * @param source the name of the source
     * @param store if source should be stored
     * @since 4.6
     */
    void setStorageEnabled(const QString &source, bool store);

protected Q_SLOTS:
    /**
     * Removes a data source.
     * @param source the name of the data source to remove
     **/
    void removeSource(const QString &source);

    /**
     * Immediately updates all existing sources when called
     */
    void updateAllSources();

    /**
     * Forces an immediate update to all connected sources, even those with
     * timeouts that haven't yet expired. This should _only_ be used when
     * there was no data available, e.g. due to network non-availability,
     * and then it becomes available. Normal changes in data values due to
     * calls to updateSource or in the natural progression of the monitored
     * object (e.g. CPU heat) should not result in a call to this method!
     *
     * @since 4.4
     */
    void forceImmediateUpdateOfAllVisualizations();

private:
    friend class DataEnginePrivate;
    friend class DataEngineScript;
    friend class DataEngineManager;
    friend class PlasmoidServiceJob;
    friend class NullEngine;

    Q_PRIVATE_SLOT(d, void internalUpdateSource(DataContainer *source))
    Q_PRIVATE_SLOT(d, void sourceDestroyed(QObject *object))
    Q_PRIVATE_SLOT(d, void scheduleSourcesUpdated())

    DataEnginePrivate *const d;
};

} // Plasma namespace

/**
 * Register a data engine when it is contained in a loadable module
 */
/* clang-format off */
#define K_EXPORT_PLASMA_DATAENGINE(libname, classname) \
    K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
    K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#define K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(libname, classname, jsonFile) \
    K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();) \
    K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)
/* clang-format on */

#endif // multiple inclusion guard
