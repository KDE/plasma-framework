/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef HTTPCONTAINER_H
#define HTTPCONTAINER_H

#include <QUrl>

#include <Plasma/DataContainer>

namespace KIO
{
class Job;
}

class HttpContainer : public Plasma::DataContainer
{
    Q_OBJECT

public:
    HttpContainer(const QUrl &url, QObject *parent = nullptr);

    void fetchUrl(bool reload = true);

private Q_SLOTS:
    void data(KIO::Job *job, const QByteArray &data);
    void fetchFinished(KJob *);

private:
    const QUrl m_url;
    QPointer<KJob> m_job;
    QByteArray m_data;
};

#endif
