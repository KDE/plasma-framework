/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGINLOADERTEST_H
#define PLUGINLOADERTEST_H

#include <QCoreApplication>
#include <QTest>
#include <plasma/dataengine.h>

#include <plasma/pluginloader.h>
class PluginTest : public QObject
{
    Q_OBJECT
public:
    PluginTest();

private Q_SLOTS:
    void listEngines();
    void listAppletCategories();
    void listContainmentActions();
    void listContainmentsOfType();

    void loadDataEngine();

    void benchmarkloading()
    {
        QBENCHMARK {
            const QStringList applets = {
                "org.kde.plasma.binaryclock",
                "org.kde.plasma.calculator",
                "org.kde.plasma.colorpicker",
                "org.kde.plasma.comic",
                "org.kde.plasma_applet_dict",
                "org.kde.plasma.diskquota",
                "org.kde.plasma.fifteenpuzzle",
                "org.kde.plasma.fuzzyclock",
                "org.kde.plasma.grouping",
                "org.kde.plasma.private.grouping",
                "org.kde.plasma.keyboardindicator",
                "org.kde.plasma.kickerdash",
                "org.kde.plasma.konsoleprofiles",
                "org.kde.plasma.mediaframe",
                "org.kde.plasma.nightcolorcontrol",
                "org.kde.plasma.notes",
                "org.kde.plasma.quicklaunch",
                "org.kde.plasma.quickshare",
                "org.kde.plasma.timer",
                "org.kde.plasma.userswitcher",
                "org.kde.plasma.weather",
                "org.kde.plasma.webbrowser",

            };

            for (const QString &applet : applets) {
                Plasma::Applet *a = Plasma::PluginLoader::self()->loadApplet(applet, 0, {});
                Q_UNUSED(a)
            }
        }
    }

private:
    bool m_buildonly;
};

class EngineTest : public QObject
{
    Q_OBJECT
public:
    EngineTest()
    {
    }

public Q_SLOTS:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
};

#endif
