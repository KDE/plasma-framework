/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include "httpContainer.h"

#include <kio/job.h>

HttpContainer::HttpContainer(const QUrl &url, QObject *parent)
    : Plasma::DataContainer(parent)
    , m_url(url)
{
    // Since we are grabbing data over the network, we request a
    // backing store. This way, if the network is down or on first start
    // before we get our first reply back, if this source was previously
    // available, we'll still have our data. This is a very nice "freebie"
    // DataContainer gives us.
    setStorageEnabled(true);

    // Now, start an initial fetch.
    fetchUrl(false);
}

void HttpContainer::fetchUrl(bool reload)
{
    // Now we go about the business of fetching the URL with KIO
    m_data.clear();

    KIO::TransferJob *job = KIO::get(m_url, reload ? KIO::Reload : KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job *, QByteArray)), this, SLOT(data(KIO::Job *, QByteArray)));
    connect(job, SIGNAL(finished(KJob *)), this, SLOT(fetchFinished(KJob *)));

    if (m_job) {
        m_job.data()->kill();
    }

    m_job = job;
}

void HttpContainer::data(KIO::Job *job, const QByteArray &data)
{
    if (job == m_job.data()) {
        // we store the data as it arrives
        m_data.append(data);
    }
}

void HttpContainer::fetchFinished(KJob *job)
{
    if (!job->error()) {
        // We now set the data on the source with the retrieved data and some
        // additional stats. Note that we don't include the source name, as that
        // is implied as this object *is* the DataContainer. setData is called
        // with just key/value pairs.
        setData(QStringLiteral("Contents"), m_data);
        setData(QStringLiteral("Size"), job->processedAmount(KJob::Bytes));

        // Since we only create TransferJobs, it's safe to just static_cast here.
        // In many real-world situations, this isn't the safest thing to do and a
        // qobject_cast with a test on the result is often safer and cleaner.
        KIO::TransferJob *tjob = static_cast<KIO::TransferJob *>(job);
        setData(QStringLiteral("Error Page"), tjob->isErrorPage());
        setData(QStringLiteral("Mimetype"), tjob->mimetype());

        // Let DataContainer know we have data that needs storing
        setNeedsToBeStored(true);

        // Notify DataContainer that now is a good time to check to see that
        // data has been updated. This will cause visualizations to be updated.
        checkForUpdate();

        // Clean up behind ourselves so there isn't unnecessary memory usage
        m_data.clear();
    }
}
