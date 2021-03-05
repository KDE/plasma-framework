/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_SERVICEJOB_H
#define PLASMA_SERVICEJOB_H

#include <QVariant>

#include <KJob>

#include <plasma/plasma_export.h>

namespace Plasma
{
class ServiceJobPrivate;

/**
 * @class ServiceJob plasma/servicejob.h <Plasma/ServiceJob>
 *
 * @short This class provides jobs for use with Plasma::Service
 *
 * Unlike KJob, you can do the work in start(), since Plasma::Service already
 * delays the call to start() until the event loop is reached.
 *
 * If the job is quick enough that it is not worth reporting the progress,
 * you just need to implement start() to do the task, then call emitResult()
 * at the end of it.  If the task does not complete successfully, you should
 * set a non-zero error code with setError(int) and an error message with
 * setErrorText(QString).
 *
 * If the job is longer (involving network access, for instance), you should
 * report the progress at regular intervals.  See the KJob documentation for
 * information on how to do this.
 */
class PLASMA_EXPORT ServiceJob : public KJob
{
    Q_OBJECT
    Q_PROPERTY(QString destination READ destination)
    Q_PROPERTY(QString operationName READ operationName)
    Q_PROPERTY(QVariant result READ result)

public:
    /**
     * Default constructor
     *
     * @param destination the subject that the job is acting on
     * @param operation   the action that the job is performing on the @p destination
     * @param parameters  the parameters of the @p action
     * @param parent      the parent object for this service
     */
    ServiceJob(const QString &destination, const QString &operation, const QVariantMap &parameters, QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~ServiceJob() override;

    /**
     * @return the subject that the job is acting on
     */
    QString destination() const;

    /**
     * @return the operation the job is performing on the destination
     */
    QString operationName() const;

    /**
     * @return the parameters for the operation
     */
    QVariantMap parameters() const;

    /**
     * Returns the result of the operation
     *
     * The result will be invalid if the job has not completed yet, or
     * if the job does not have a meaningful result.
     *
     * Note that this should not be used to find out whether the operation
     * was successful.  Instead, you should check the value of error().
     *
     * @return the result of the operation
     */
    QVariant result() const;

    /**
     * Default implementation of start, which simply sets the results to false.
     * This makes it easy to create a "failure" job.
     */
    Q_INVOKABLE void start() override;

protected:
    /**
     * Sets the result for an operation.
     */
    void setResult(const QVariant &result);

private:
    Q_PRIVATE_SLOT(d, void autoStart())
    Q_PRIVATE_SLOT(d, void preventAutoStart())

    ServiceJobPrivate *const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
