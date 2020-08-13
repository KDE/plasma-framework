/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef DATACONTAINERSENGINE_H
#define DATACONTAINERSENGINE_H

#include <Plasma/DataEngine>

class DataContainersEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    DataContainersEngine(QObject *parent, const QVariantList &args);

protected:
    bool sourceRequestEvent(const QString &source) override;
    bool updateSourceEvent(const QString &source) override;
};

#endif
