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

#ifndef PLASMA_DATAENGINE_H
#define PLASMA_DATAENGINE_H

#include <QtCore/QAtomic>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <KGenericFactory>

#include <plasma/plasma_export.h>

namespace Plasma
{

class DataSource;

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
    Q_OBJECT

    public:
        typedef QHash<QString, DataEngine*> Dict;
        typedef QHash<QString, QVariant> Data;
        typedef QHashIterator<QString, QVariant> DataIterator;

        /**
         * Default constructor.
         *
         * @param parent The parent object.
         **/
        explicit DataEngine(QObject* parent = 0);
        virtual ~DataEngine();

        /**
         * @return a list of all the data sources currently available via this
         *         DataEngine
         **/
        virtual QStringList dataSources() const;

        /**
         * Connects a source to an object for data updates. The object must
         * have a slot with the following signature:
         *
         * SLOT(updated(QString, Plasma::DataEngine::Data))
         *
         * The data is a QHash of QVariants keyed by QString names, allowing
         * one data source to provide sets of related data.
         *
         * @param source the name of the data source
         * @param visualization the object to connect the data source to
         **/
        void connectSource(const QString& source, QObject* visualization) const;

        /**
         * Connects all sources to an object for data updates. The object must
         * have a slot with the following signature:
         *
         * SLOT(updated(QString, Plasma::DataEngine::Data))
         *
         * The data is a QHash of QVariants keyed by QString names, allowing
         * one data source to provide sets of related data.
         *
         * @param visualization the object to connect the data source to
         **/
        void connectAllSources(QObject* viualization) const;

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
        Data query(const QString& source) const;

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
         * @return true if the reference count is zero
         **/
        bool isUsed() const;

        /**
         * Returns true if this engine is valid, otherwise returns false
         **/
        bool isValid() const;

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
        void newDataSource(const QString& source);

        /**
         * Emitted when a data source is removed.
         * @param source the name of the data source that was removed
         **/
        void dataSourceRemoved(const QString& source);

    protected:
        /**
         * This method is called when the DataEngine is started. When this
         * method is called the DataEngine is fully constructed and ready to be
         * used. This method should be reimplemented by DataEngine subclasses
         * which have the need to perform a startup routine.
         **/
        virtual void init();

        /**
         * Sets a value for a data source. If the source
         * doesn't exist then it is created.
         *
         * @param source the name of the data source
         * @param value the data to associated with the source
         **/
        void setData(const QString& source, const QVariant& value);

        /**
         * Sets a value for a data source. If the source
         * doesn't exist then it is created.
         *
         * @param source the name of the data source
         * @param key the key to use for the data
         * @param value the data to associated with the source
         **/
        void setData(const QString& source, const QString& key, const QVariant& value);

        /**
         * Adds an already constructed data source. The DataEngine takes
         * ownership of the DataSource object.
         * @param source the DataSource to add to the DataEngine
         **/
        void addSource(DataSource* source);

        /**
         * Sets an upper limit on the number of data sources to keep in this engine.
         * If the limit is exceeded, then the oldest data source, as defined by last
         * update, is dropped.
         *
         * @param limit the maximum number of sources to keep active
         **/
        void setSourceLimit(uint limit);

/*        DataSource* domain(const QString &domain);
        void createDataSource(const QString& source,
                              const QString& domain = QString());*/

        /**
         * Removes a data source.
         * @param source the name of the data source to remove
         **/
        void removeDataSource(const QString& source);

        /**
         * Removes all data sources
         **/
        void clearAllDataSources();

        /**
         * Sets whether or not this engine is valid, e.g. can be used.
         * In practice, only the internal fall-back engine, the NullEngine
         * should have need for this.
         *
         * @param valid whether or not the engine is valid
         **/
        void setValid(bool valid);

    private Q_SLOTS:
        void checkForUpdates();

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_DATAENGINE(libname, classname) \
        K_EXPORT_COMPONENT_FACTORY(                    \
                        plasma_engine_##libname,        \
                        KGenericFactory<classname>("plasma_engine_" #libname))

#endif // multiple inclusion guard

