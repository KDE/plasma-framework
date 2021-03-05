/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FALLBACKCOMPONENT_H
#define FALLBACKCOMPONENT_H

#include <QCache>
#include <QObject>
#include <QStringList>

/**
 * Component that loads a qml file based on a chain of different fallback paths
 * @deprecated this can be done with qmlregistertype on c++ side or a Loader on QML side
 */
class FallbackComponent : public QObject
{
    Q_OBJECT

    /**
     * Prefix of the path
     * This should be something like "plasma","kwin","plasmate",etc
     * If the basePath is "plasma", it will be set for the data of plasma like,
     * or it can be an absolute path
     **/
    Q_PROPERTY(QString basePath READ basePath WRITE setBasePath NOTIFY basePathChanged)

    /**
     * The possible candidates in order to have a complete path.
     * basepath/candidate, where candidate is the first one in the list of candidates
     * in order of importance that matches an existing file
     **/
    Q_PROPERTY(QStringList candidates READ candidates WRITE setCandidates NOTIFY candidatesChanged)

public:
    explicit FallbackComponent(QObject *parent = nullptr);

    /**
     * This method must be called after the basePath and the candidates property
     * This method resolves a file path based on the base path and the candidates.
     * it searches for a file named key under basepath/candidate/key, and returns
     * the path constructed with the first candidate that matches, if any.
     *
     * @param key the name of the file to search for
     **/
    Q_INVOKABLE QString filePath(const QString &key = QString());

    QString basePath() const;
    void setBasePath(const QString &basePath);

    QStringList candidates() const;
    void setCandidates(const QStringList &candidates);

Q_SIGNALS:
    void basePathChanged();
    void candidatesChanged();

private:
    QCache<QString, QString> m_possiblePaths;
    QString m_basePath;
    QStringList m_candidates;
};

#endif
