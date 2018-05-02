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
#include <QtCore/QTimer>

#include <kjob.h>
#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>

class QAbstractItemModel;

namespace Plasma
{

class DataContainerPrivate;

/**
 * @class DataContainer plasma/datacontainer.h <Plasma/DataContainer>
 *
 * @brief A set of data exported via a DataEngine
 *
 * Plasma::DataContainer wraps the data exported by a DataEngine
 * implementation, providing a generic wrapper for the data.
 *
 * A DataContainer may have zero or more associated pieces of data which
 * are keyed by strings. The data itself is stored as QVariants. This allows
 * easy and flexible retrieval of the information associated with this object
 * without writing DataContainer or DataEngine specific code in visualizations.
 *
 * If you are creating your own DataContainer objects (and are passing them to
 * DataEngine::addSource()), you normally just need to listen to the
 * updateRequested() signal (as well as any other methods you might have of
 * being notified of new data) and call setData() to actually update the data.
 * Then you need to either trigger the scheduleSourcesUpdated signal of the
 * parent DataEngine or call checkForUpdate() on the DataContainer.
 *
 * You also need to set a suitable name for the source with setObjectName().
 * See DataEngine::addSource() for more information.
 *
 * Note that there is normally no need to subclass DataContainer, except as
 * a way of encapsulating the data retrieval for a source, since all notifications
 * are done via signals rather than virtual methods.
 **/
class PLASMA_EXPORT DataContainer : public QObject
{
    friend class DataEngine;
    friend class DataEnginePrivate;
    Q_OBJECT

public:
    /**
     * Constructs a default DataContainer that has no name or data
     * associated with it
     **/
    explicit DataContainer(QObject *parent = nullptr);
    virtual ~DataContainer();

    /**
     * Returns the data for this DataContainer
     **/
    const DataEngine::Data data() const;

    /**
     * Set a value for a key.
     *
     * This also marks this source as needing to signal an update.
     *
     * If you call setData() directly on a DataContainer, you need to
     * either trigger the scheduleSourcesUpdated() slot for the
     * data engine it belongs to or call checkForUpdate() on the
     * DataContainer.
     *
     * @param key a string used as the key for the data
     * @param value a QVariant holding the actual data. If a invalid
     *              QVariant is passed in and the key currently exists in the
     *              data, then the data entry is removed
     **/
    void setData(const QString &key, const QVariant &value);

    /**
     * Removes all data currently associated with this source
     *
     * If you call removeAllData() on a DataContainer, you need to
     * either trigger the scheduleSourcesUpdated() slot for the
     * data engine it belongs to or call checkForUpdate() on the
     * DataContainer.
     **/
    void removeAllData();

    /**
     * Associates a model with this DataContainer. Use this for data
     * that is intended to be a long list of items.
     *
     * The ownership of the model is transferred to the DataContainer,
     * so the model will be deletd when a new one is set or when the
     * DataContainer itself is deleted, so it will be deleted when there won't be any
     * visualization associated to this source.
     *
     * Normally you should set the model from DataEngine::setModel instead from here.
     *
     * @param model the model that will be associated with this DataContainer
     */
    void setModel(QAbstractItemModel *model);

    /**
     * @return the model owned by this DataSource
     */
    QAbstractItemModel *model();

    /**
     * @return true if the visualization is currently connected
     */
    bool visualizationIsConnected(QObject *visualization) const;

    /**
     * Connects an object to this DataContainer.
     *
     * May be called repeatedly for the same visualization without
     * side effects
     *
     * @param visualization the object to connect to this DataContainer
     * @param pollingInterval the time in milliseconds between updates
     * @param alignment the clock position to align updates to
     **/
    void connectVisualization(QObject *visualization, uint pollingInterval,
                              Plasma::Types::IntervalAlignment alignment);

    /**
     * sets this data container to be automatically stored.
     * @param whether this data container should be stored
     * @since 4.6
     */
    void setStorageEnabled(bool store);

    /**
     * @return true if the data container has been marked for storage
     * @since 4.6
     */
    bool isStorageEnabled() const;

    /**
     * @return true if the data container has been updated, but not stored
     */
    bool needsToBeStored() const;

    /**
     * sets that the data container needs to be stored or not.
     * @param whether the data container needs to be stored
     */
    void setNeedsToBeStored(bool store);

    /**
     * @return the DataEngine that the DataContainer is
     * a child of.
     */
    DataEngine *getDataEngine();

    /**
     * @return true if one or more visualizations is connected to this DataContainer
     */
    bool isUsed() const;

public Q_SLOTS:
    /**
     * Disconnects an object from this DataContainer.
     *
     * Note that if this source was created by DataEngine::sourceRequestEvent(),
     * it will be deleted by DataEngine once control returns to the event loop.
     **/
    void disconnectVisualization(QObject *visualization);

    /**
     * Forces immediate update signals to all visualizations
     * @since 4.4
     */
    void forceImmediateUpdate();

Q_SIGNALS:
    /**
     * Emitted when the data has been updated, allowing visualizations to
     * reflect the new data.
     *
     * Note that you should not normally emit this directly.  Instead, use
     * checkForUpdate() or the DataEngine::scheduleSourcesUpdated() slot.
     *
     * @param source the objectName() of the DataContainer (and hence the name
     *               of the source) that updated its data
     * @param data   the updated data
     **/
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

    /**
     * A new model has been associated to this source,
     * visualizations can safely use it as long they are connected to this source.
     *
     * @param source the objectName() of the DataContainer (and hence the name
     *               of the source) that owns the model
     * @param model the QAbstractItemModel instance
     */
    void modelChanged(const QString &source, QAbstractItemModel *model);

    /**
     * Emitted when the last visualization is disconnected.
     *
     * Note that if this source was created by DataEngine::sourceRequestEvent(),
     * it will be deleted by DataEngine once control returns to the event loop
     * after this signal is emitted.
     *
     * @param source  the name of the source that became unused
     **/
    void becameUnused(const QString &source);

    /**
     * Emitted when an update is requested.
     *
     * If a polling interval was passed connectVisualization(), this signal
     * will be emitted every time the interval expires.
     *
     * Note that if you create your own DataContainer (and pass it to
     * DataEngine::addSource()), you will need to listen to this signal
     * and refresh the data when it is triggered.
     *
     * @param source  the datacontainer the update was requested for.  Useful
     *                for classes that update the data for several containers.
     **/
    void updateRequested(DataContainer *source);

protected:
    /**
     * Checks whether any data has changed and, if so, emits dataUpdated().
     **/
    void checkForUpdate();

    /**
     * Returns how long ago, in msecs, that the data in this container was last updated.
     *
     * This is used by DataEngine to compress updates that happen more quickly than the
     * minimum polling interval by calling setNeedsUpdate() instead of calling
     * updateSourceEvent() immediately.
     **/
    uint timeSinceLastUpdate() const;

    /**
     * Indicates that the data should be treated as dirty the next time hasUpdates() is called.
     *
     * This is needed for the case where updateRequested() is triggered but we don't want to
     * update the data immediately because it has just been updated.  The second request won't
     * be fulfilled in this case, because we never updated the data and so never called
     * checkForUpdate().  So we claim it needs an update anyway.
     **/
    void setNeedsUpdate(bool update = true);

protected Q_SLOTS:
    /**
     * @reimp from QObject
     */
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    friend class SignalRelay;
    friend class DataContainerPrivate;
    friend class DataEngineManager;
    DataContainerPrivate *const d;

    Q_PRIVATE_SLOT(d, void storeJobFinished(KJob *job))
    Q_PRIVATE_SLOT(d, void populateFromStoredData(KJob *job))
    Q_PRIVATE_SLOT(d, void retrieve())
};

} // Plasma namespace

#endif // multiple inclusion guard
