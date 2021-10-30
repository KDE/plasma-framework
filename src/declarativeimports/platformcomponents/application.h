/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QString>

#include "utils/d_ptr.h"

/**
 * Class which handles an application execution.
 *
 * Example:
 * @code
 * Application {
 *     application: "xterm"
 *     running: terminalRunningCheckbox.checked
 * }
 * @endcode
 */
class Application : public QObject
{
    Q_OBJECT

    /**
     * The name or path of the applications
     */
    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY applicationChanged)

    /**
     * Indicates whether the user wants the application to be running or not.
     * It does not refer to the actual state of the application.
     */
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    QString application() const;
    bool running() const;

public:
    void setApplication(const QString &application);
    void setRunning(bool run);

public Q_SLOTS:
    void start();
    void terminate();

Q_SIGNALS:
    void applicationChanged(const QString &application);
    void runningChanged(bool running);

private:
    D_PTR;
};

#endif /* APPLICATION_H */
