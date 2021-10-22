/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQmlContext>

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
    AppletContext(QQmlEngine *engine, Plasma::Applet *applet);
    ~AppletContext();

    Plasma::Applet *applet() const;

private:
    Plasma::Applet *m_applet;
};

}

