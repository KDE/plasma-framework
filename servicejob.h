/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_SERVICEJOB_H
#define PLASMA_SERVICEJOB_H

#include <QVariant>

#include <KJob>
#include <KService>

namespace Plasma
{

/**
 * @brief This class provides jobs for use with Plasma::Service
 */
class ServiceJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Default constructor
     *
     * @arg parent the parent object for this service
     */
    ServiceJob(const QString &destination, const QString &operation,
               const QMap<QString, QVariant> &parameters, QObject *parent = 0);

    /**
     * Destructor
     */
    ~ServiceJob();

    /**
     * @return the destination, if any, that this service is associated with
     */
    QString destination() const;

    /**
     * @return the operation this job should perform
     */
    QString operationName() const;

    /**
     * @return the parameters for the job
     */
    QMap<QString, QVariant> parameters() const;

    /**
     * @return the resulting value from the operation
     */
    QVariant result() const;

protected:
    /**
     * Sets the result for an operation.
     */
    void setResult(const QVariant &result);

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard

