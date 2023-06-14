/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "sharedqmlengine.h"

#include <QQmlContext>
#include <qstringlistmodel.h>

namespace Plasma
{
class Applet;
}

namespace PlasmaQuick
{

class AppletContext : public QQmlContext
{
    Q_OBJECT
public:
    AppletContext(QQmlEngine *engine, Plasma::Applet *applet, SharedQmlEngine *parent);
    ~AppletContext();

    Plasma::Applet *applet() const;
    SharedQmlEngine *sharedQmlEngine() const;

private:
    Plasma::Applet *m_applet;
    SharedQmlEngine *m_sharedEngine;
};

}
