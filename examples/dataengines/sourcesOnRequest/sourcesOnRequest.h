/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef SOURCESONREQUESTENGINE_H
#define SOURCESONREQUESTENGINE_H

#include <Plasma/DataEngine>

class SourcesOnRequestEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    SourcesOnRequestEngine(QObject *parent, const QVariantList &args);

protected:
    bool sourceRequestEvent(const QString &source) override;
    bool updateSourceEvent(const QString &source) override;
};

#endif
