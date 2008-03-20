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

#ifndef PLASMA_DATAENGINE_H
#define PLASMA_DATAENGINE_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <KDE/KGenericFactory>
#include <KDE/KService>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>

namespace Plasma
{

class DataContainer;
class DataEngineScript;

/**
 * @class DataEngine
 * @brief Data provider for plasmoids (Plasma plugins)
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
    friend class DataEngineScript;
    Q_OBJECT
    Q_PROPERTY( QStringList sources READ sources )
    Q_PROPERTY( bool valid READ isValid )
    Q_PROPERTY( QString icon READ icon WRITE setIcon )

    public:
        typedef QHash<QString, DataEngine*> Dict;
        typedef QHash<QString, QVariant> Data;
        typedef QHashIterator<QString, QVariant> DataIterator;
        typedef QHash<QString, DataContainer*> SourceDict;

        /**
         * Constructor.
         *
         * @param parent The parent object.
         * @param service pointer to the service that describes the engine
         **/
        explicit DataEngine(QObject* parent = 0, KService::Ptr service = KService::Ptr(0));
        DataEngine(QObject* parent, const QVariantList& args);
        virtual ~DataEngine();

        /**
         * This method is called when the DataEngine is started. When this
         * method is called the DataEngine is fully constructed and ready to be
         * used. This method should be reimplemented by DataEngine subclasses
         * which have the need to perform a startup routine.
         **/
        virtual void init();

        /**
         * @return a list of all the data sources available via this DataEngine
         *         Whether these sources are currently available (which is what
         *         the default implementation provides) or not is up to the
         *         DataEngine to decide.
         **/
        virtual QStringList sources() const;

        /**
         * Returns the engine name for the DataEngine
         */
        QString engineName() const;

        /**
         * Connects a source to an object for data updates. The object must
         * have a slot with the following signature:
         *
         * dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
         *
         * The data is a QHash of QVariants keyed by QString names, allowing
         * one data source to provide sets of related data.
         *
         * @param source the name of the data source
         * @param visualization the object to connect the data source to
         * @param updateInterval the frequency, in milliseconds, with which to signal updates;
         *                        a value of 0 (the default) means to update only
         *                        when there is new data spontaneously generated
         *                        (e.g. by the engine); any other value results in
         *                        periodic updates from this source. This value is
         *                        per-visualization and can be handy for items that require
         *                        constant updates such as scrolling graphs or clocks.
         * @param intervalAlignedTo the number of ms to aling the interval to
         **/
        Q_INVOKABLE void connectSource(const QString& source, QObject* visualization,
                                       uint updateInterval = 0,
                                       Plasma::IntervalAlignment intervalAlignment = NoAlignment) const;

        /**
         * Connects all currently existing sources to an object for data updates.
         * The object must have a slot with the following signature:
         *
         * SLOT(dataUpdated(QString,Plasma::DataEngine::Data))
         *
         * The data is a QHash of QVariants keyed by QString names, allowing
         * one data source to provide sets of related data.
         *
         * This method may be called multiple times for the same visualization
         * without side-effects. This can be useful to change the updateInterval.
         *
         * Note that this method does not automatically connect sources that
         * may appear later on. Connecting and responding to the sourceAdded sigal
         * is still required to achieve that.
         *
         * @param visualization the object to connect the data source to
         * @param updateInterval the frequency, in milliseconds, with which to signal updates;
         *                        a value of 0 (the default) means to update only
         *                        when there is new data spontaneously generated
         *                        (e.g. by the engine); any other value results in
         *                        periodic updates from this source. This value is
         *                        per-visualization and can be handy for items that require
         *                        constant updates such as scrolling graphs or clocks.
         **/
        Q_INVOKABLE void connectAllSources(QObject* visualization, uint updateInterval = 0,
                                           Plasma::IntervalAlignment intervalAlignment = NoAlignment) const;

        /**
         * Disconnects a source to an object that was receiving data updates.
         *
         * @param source the name of the data source
         * @param visualization the object to connect the data source to
         **/
        Q_INVOKABLE void disconnectSource(const QString& source, QObject* visualization) const;

        /**
         * Retrevies a pointer to the DataContainer for a given source. This method
         * should not be used if possible. An exception is for script engines that
         * can not provide a QMetaObject as required by connectSource for the initial
         * call to dataUpdated. Using this method, such engines can provide their own
         * connectSource API.
         *
         * @arg source the name of the source.
         * @return pointer to a DataContainer, or zero on failure
         **/
        Q_INVOKABLE DataContainer* containerForSource(const QString &source);

        /**
         * Gets the Data associated with a data source.
         *
         * The data is a QHash of QVariants keyed by QString names, allowing
         * one data source to provide sets of related data.
         *
         * @param source the data source to retrieve the data for
         * @return the Data associated with the source; if the source doesn't
         *         exist an empty data set is returned
         **/
        Q_INVOKABLE DataEngine::Data query(const QString& source) const;

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
         * Returns true if this engine is valid, otherwise returns false
         **/
        bool isValid() const;

        /**
         * Returns true if the data engine is empty, which is to say that it has no
         * data sources currently.
         */
        bool isEmpty() const;

        /**
         * Sets the icon for this data engine
         **/
        void setIcon(const QString& icon);

        /**
         * @return the name of the icon for this data engine; and empty string
         *         is returned if there is no associated icon.
         **/
        QString icon() const;

    Q_SIGNALS:
        /**
         * Emitted when a new data source is created
         * @param source the name of the new data source
         **/
        void newSource(const QString& source);

        /**
         * Emitted when a data source is removed.
         * @param source the name of the data source that was removed
         **/
        void sourceRemoved(const QString& source);

    protected:
        /**
         * When a source that does not currently exist is requested by the
         * consumer, this method is called to give the DataEngine the
         * opportunity to create one.
         *
         * The name of the data source (e.g. the source parameter passed into
         * setData) it must be the same as the name passed to sourceRequested
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
         * @return true if a DataContainer was set up, false otherwise
         */
        virtual bool sourceRequested(const QString &name);

        /**
         * Called by internal updating mechanisms to trigger the engine
         * to refresh the data contained in a given source. Reimplement this
         * method when using facilities such as setUpdateInterval.
         * @see setUpdateInterval
         *
         * @param source the name of the source that should be updated
         * @return true if the data was changed, or false if there was no
         *         change or if the change will occur later
         **/
        virtual bool updateSource(const QString& source);

        /**
         * Sets a value for a data source. If the source
         * doesn't exist then it is created.
         *
         * Note: Passing an empty or invalid QVariant as the value is
         * functionally equivalent to calling removeSource(source);
         *
         * @param source the name of the data source
         * @param value the data to associated with the source
         **/
        void setData(const QString &source, const QVariant &value);

        /**
         * Sets a value for a data source. If the source
         * doesn't exist then it is created.
         *
         * Note: Passing an empty or invalid QVariant as the value is
         * functionally equivalent to calling removeSource(source);
         *
         * @param source the name of the data source
         * @param key the key to use for the data
         * @param value the data to associated with the source
         **/
        void setData(const QString& source, const QString& key, const QVariant& value);

        /**
         * Adds a set of data to a data source. If the source
         * doesn't exist then it is created.
         *
         * Note: Passing an empty or invalid QVariant as the value is
         * functionally equivalent to calling removeSource(source);
         *
         * @param source the name of the data source
         * @param data the data to add to the source
         **/
        void setData(const QString &source, const Data &data);

        /**
         * Clears all the data associated with a data source.
         *
         * @param source the name of the data source
         **/
        void clearData(const QString& source);

        /**
         * Removes a data entry from a source
         *
         * @param source the name of the data source
         * @param key the data entry to remove
         **/
        void removeData(const QString& source, const QString& key);

        /**
         * Adds an already constructed data source. The DataEngine takes
         * ownership of the DataContainer object.
         * @param source the DataContainer to add to the DataEngine
         **/
        void addSource(DataContainer* source);

        /**
         * Sets an upper limit on the number of data sources to keep in this engine.
         * If the limit is exceeded, then the oldest data source, as defined by last
         * update, is dropped.
         *
         * @param limit the maximum number of sources to keep active
         **/
        void setSourceLimit(uint limit);

        /**
         * Sets the minimum amount of time, in milliseconds, that must pass between
         * successive updates of data. This can help prevent too many updates happening
         * due to multiple update requests coming in, which can be useful for
         * expensive (time- or resource-wise) update mechanisms.
         *
         * @arg minimumMs the minimum time lapse, in milliseconds, between updates.
         *                A value less than 0 means to never perform automatic updates,
         *                a value of 0 means update immediately on every update request,
         *                a value >0 will result in a minimum time lapse being enforced.
         **/
        void setMinimumUpdateInterval(int minimumMs);

        /**
         * @return the minimum time between updates. @see setMinimumupdateInterval
         **/
        int minimumUpdateInterval() const;

        /**
         * Sets up an internal update tick for all data sources. On every update,
         * updateSource will be called for each applicable source.
         * @see updateSource
         *
         * @param frequency the time, in milliseconds, between updates. A value of 0
         *                  will stop internally triggered updates.
         **/
        void setUpdateInterval(uint frequency);

        /**
         * Returns the current update frequency.
         * @see setUpdateInterval
         NOTE: This is not implemented to prevent having to store the value internally.
               When there is a good use case for needing access to this value, we can
               add another member to the Private class and add this method.
        uint updateInterval();
         **/

        /**
         * Removes all data sources
         **/
        void clearSources();

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
        SourceDict sourceDict() const;

        /**
         * Reimplemented from QObject
         **/
        void timerEvent(QTimerEvent *event);

    protected Q_SLOTS:
        /**
         * Call this method when you call setData directly on a DataContainer instead
         * of using the DataEngine::setData methods.
         * If this method is not called, no dataUpdated(..) signals will be emitted!
         */
        void checkForUpdates();

        /**
         * Removes a data source.
         * @param source the name of the data source to remove
         **/
        void removeSource(const QString& source);

        /**
         * @internal
         **/
        void internalUpdateSource(DataContainer* source);

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_DATAENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_engine_" #libname))
#endif // multiple inclusion guard

