/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletcontext_p.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickWindow>
#include <QRandomGenerator>

#include <QDebug>

#include <KLocalizedString>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include <qloggingcategory.h>

namespace PlasmaQuick
{

AppletContext::AppletContext(QQmlEngine *engine, Plasma::Applet *applet, SharedQmlEngine *parent)
    : QQmlContext(engine, applet)
    , m_applet(applet)
    , m_sharedEngine(parent)
{
    setParent(parent);
}

AppletContext::~AppletContext()
{
}

Plasma::Applet *AppletContext::applet() const
{
    return m_applet;
}

SharedQmlEngine *AppletContext::sharedQmlEngine() const
{
    return m_sharedEngine;
}

}

#include "moc_appletcontext_p.cpp"
