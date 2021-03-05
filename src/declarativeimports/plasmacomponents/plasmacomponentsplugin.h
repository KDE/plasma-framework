/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMACOMPONENTSPLUGIN_H
#define PLASMACOMPONENTSPLUGIN_H

#include <QQmlExtensionPlugin>
#include <QSet>

class QQmlEngine;
class QQuickItem;
class PlasmaComponentsPlugin;

class EngineBookKeeping : public QObject
{
    Q_OBJECT

public:
    EngineBookKeeping();
    static EngineBookKeeping *self();

    void insertEngine(QQmlEngine *engine);
    QQmlEngine *engine() const;

private Q_SLOTS:
    void engineDestroyed(QObject *deleted);

private:
    QSet<QQmlEngine *> m_engines;
};

class PlasmaComponentsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

#endif
