/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fallbackcomponent.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include <QDebug>

FallbackComponent::FallbackComponent(QObject *parent)
    : QObject(parent)
{
}

QString FallbackComponent::basePath() const
{
    return m_basePath;
}

void FallbackComponent::setBasePath(const QString &basePath)
{
    if (basePath != m_basePath) {
        m_basePath = basePath;
        Q_EMIT basePathChanged();
    }
}

QStringList FallbackComponent::candidates() const
{
    return m_candidates;
}

void FallbackComponent::setCandidates(const QStringList &candidates)
{
    m_candidates = candidates;
    Q_EMIT candidatesChanged();
}

QString FallbackComponent::filePath(const QString &key)
{
    QString resolved;

    for (const QString &path : std::as_const(m_candidates)) {
        // qDebug() << "Searching for:" << path + path;
        if (m_possiblePaths.contains(path + key)) {
            resolved = *m_possiblePaths.object(path + key);
            if (!resolved.isEmpty()) {
                break;
            } else {
                continue;
            }
        }

        QDir tmpPath(m_basePath);

        if (tmpPath.isAbsolute()) {
            resolved = m_basePath + path + key;
        } else {
            resolved = QStandardPaths::locate(QStandardPaths::GenericDataLocation, m_basePath + QLatin1Char('/') + path + key);
        }

        m_possiblePaths.insert(path + key, new QString(resolved));
        if (!resolved.isEmpty()) {
            break;
        }
    }

    return resolved;
}
