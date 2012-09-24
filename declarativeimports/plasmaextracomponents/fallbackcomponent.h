/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FALLBACKCOMPONENT_H
#define FALLBACKCOMPONENT_H


#include <QObject>
#include <QCache>
#include <QStringList>

class FallbackComponent : public QObject
{
    Q_OBJECT

Q_PROPERTY(QString basePath READ basePath WRITE setBasePath NOTIFY onBasePathChanged)
Q_PROPERTY(QStringList candidates READ candidates WRITE setCandidates NOTIFY onCandidatesChanged)

public:
    FallbackComponent(QObject *parent = 0);

    Q_INVOKABLE QString filePath(const QString& key);

    QString basePath() const;
    void setBasePath(const QString &basePath);


    QStringList candidates() const;
    void setCandidates(const QStringList &candidates);


Q_SIGNALS:
    void onBasePathChanged();
    void onCandidatesChanged();

private:
    QCache<QString, QString> m_possiblePaths;
    QString m_basePath;
    QStringList m_candidates;
};

#endif
