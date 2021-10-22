/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

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
#include <kdeclarative/qmlobjectsharedengine.h>

#include <qloggingcategory.h>

namespace PlasmaQuick
{

AppletContext::AppletContext(QQmlEngine *engine, Plasma::Applet *applet)
    : QQmlContext(engine, applet),
      m_applet(applet)
{}

AppletContext::~AppletContext()
{}

Plasma::Applet *AppletContext::applet() const
{
    return m_applet;
}

}

#include "moc_appletcontext_p.cpp"
