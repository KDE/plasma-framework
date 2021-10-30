/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_SERVICE_H
#define PLASMA_SERVICE_H

#include <QHash>
#include <QObject>
#include <QVariant>

#include <KConfigGroup>

#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class QIODevice;
class QWidget;
class QUrl;
class QQuickItem;

namespace Plasma
{
class ServiceJob;
class ServicePrivate;

/**
 * @class Service plasma/service.h <Plasma/Service>
 *
 * @short This class provides a generic API for write access to settings or services.
 *
 * Plasma::Service allows interaction with a "destination", the definition of which
 * depends on the Service itself. For a network settings Service this might be a
 * profile name ("Home", "Office", "Road Warrior") while a web based Service this
 * might be a username ("aseigo", "stranger65").
 *
 * A Service provides one or more operations, each of which provides some sort
 * of interaction with the destination. Operations are described using config
 * XML which is used to create a KConfig object with one group per operation.
 * The group names are used as the operation names, and the defined items in
 * the group are the parameters available to be set when using that operation.
 *
 * A service is started with a KConfigGroup (representing a ready to be serviced
 * operation) and automatically deletes itself after completion and signaling
 * success or failure. See KJob for more information on this part of the process.
 *
 * Services may either be loaded "stand alone" from plugins, or from a DataEngine
 * by passing in a source name to be used as the destination.
 *
 * Sample use might look like:
 *
 * @code
 * Plasma::DataEngine *twitter = dataEngine("twitter");
 * Plasma::Service *service = twitter.serviceForSource("aseigo");
 * QVariantMap op = service->operationDescription("update");
 * op.insert("tweet", "Hacking on plasma!");
 * Plasma::ServiceJob *job = service->startOperationCall(op);
 * connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobCompeted()));
 * @endcode
 *
 * Please remember, the service needs to be deleted when it will no longer be
 * used. This can be done manually or by these (perhaps easier) alternatives:
 *
 * If it is needed throughout the lifetime of the object:
 * @code
 * service->setParent(this);
 * @endcode
 *
 * If the service will not be used after just one operation call, use:
 * @code
 * connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
 * @endcode
 *
 */
class PLASMA_EXPORT Service : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Service)
    Q_PROPERTY(QString destination READ destination WRITE setDestination)
    Q_PROPERTY(QStringList operationNames READ operationNames)
    Q_PROPERTY(QString name READ name)

public:
    /**
     * Destructor
     */
    ~Service() override;

    /**
     * Sets the destination for this Service to operate on
     *
     * @param destination specific to each Service, this sets which
     *                  target or address for ServiceJobs to operate on
     */
    Q_INVOKABLE void setDestination(const QString &destination);

    /**
     * @return the target destination, if any, that this service is associated with
     */
    Q_INVOKABLE QString destination() const;

    /**
     * @return the possible operations for this profile
     */
    Q_INVOKABLE QStringList operationNames() const;

    /**
     * Retrieves the parameters for a given operation
     *
     * @param operationName the operation to retrieve parameters for
     * @return QVariantMap containing the parameters
     */
    Q_INVOKABLE QVariantMap operationDescription(const QString &operationName);

    /**
     * Called to create a ServiceJob which is associated with a given
     * operation and parameter set.
     *
     * @return a started ServiceJob; the consumer may connect to relevant
     *         signals before returning to the event loop
     */
    Q_INVOKABLE ServiceJob *startOperationCall(const QVariantMap &description, QObject *parent = nullptr);

    /**
     * Query to find if an operation is enabled or not.
     *
     * @param operation the name of the operation to check
     * @return true if the operation is enabled, false otherwise
     */
    Q_INVOKABLE bool isOperationEnabled(const QString &operation) const;

    /**
     * The name of this service
     */
    Q_INVOKABLE QString name() const;

Q_SIGNALS:
    /**
     * Emitted when this service is ready for use
     */
    void serviceReady(Plasma::Service *service);

    /**
     * Emitted when an operation got enabled or disabled
     */
    void operationEnabledChanged(const QString &operation, bool enabled);

protected:
    /**
     * Default constructor
     *
     * @param parent the parent object for this service
     */
    explicit Service(QObject *parent = nullptr);

    /**
     * Constructor for plugin loading
     */
    Service(QObject *parent, const QVariantList &args);

    /**
     * Called when a job should be created by the Service.
     *
     * @param operation which operation to work on
     * @param parameters the parameters set by the user for the operation
     * @return a ServiceJob that can be started and monitored by the consumer
     */
    virtual ServiceJob *createJob(const QString &operation, QVariantMap &parameters) = 0;

    /**
     * By default this is based on the file in plasma/services/name.operations, but can be
     * reimplemented to use a different mechanism.
     *
     * It should result in a call to setOperationsScheme(QIODevice *);
     */
    virtual void registerOperationsScheme();

    /**
     * Sets the XML used to define the operation schema for
     * this Service.
     */
    void setOperationsScheme(QIODevice *xml);

    /**
     * Sets the name of the Service; useful for Services not loaded from plugins,
     * which use the plugin name for this.
     *
     * @param name the name to use for this service
     */
    void setName(const QString &name);

    /**
     * Enables a given service by name
     *
     * @param operation the name of the operation to enable or disable
     * @param enable true if the operation should be enabled, false if disabled
     */
    void setOperationEnabled(const QString &operation, bool enable);

private:
    ServicePrivate *const d;

    friend class DataEnginePrivate;
    friend class PluginLoader;
};

} // namespace Plasma

/**
 * Register a service when it is contained in a loadable module
 * @deprecated Since 5.88, use K_PLUGIN_CLASS_WITH_JSON instead
 */
#define K_EXPORT_PLASMA_SERVICE(libname, classname) K_PLUGIN_FACTORY(factory, registerPlugin<classname>();)

/// @deprecated Since 5.88, use K_PLUGIN_CLASS_WITH_JSON instead
#define K_EXPORT_PLASMA_SERVICE_WITH_JSON(libname, classname, jsonFile) K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)

#endif // multiple inclusion guard
