/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include "application.h"

#include <QProcess>

class Application::Private : public QObject
{
    Q_OBJECT
public:
    Private(Application *);

    QString application;
    QProcess process;
    bool running;

private Q_SLOTS:
    void stateChanged(QProcess::ProcessState newState);
    void errorFound(QProcess::ProcessError error);

private:
    Application *const q;
};

#endif /* APPLICATION_P_H */
