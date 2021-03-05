/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configmodeltest.h"

#include "plasma/applet.h"
#include "plasma/package.h"
#include "plasmaquick/configmodel.h"
#include "plasmaquick/private/configcategory_p.cpp"

#include <KConfigLoader>

#include <QQmlComponent>
#include <QQmlEngine>

void ConfigModelTest::configSchemeFromPackage()
{
    Plasma::Applet *applet = Plasma::Applet::loadPlasmoid(QFINDTESTDATA("data/testconfigpackage"));

    QCOMPARE(applet->configScheme()->groupList(), QStringList() << QStringLiteral("General"));
    QCOMPARE(applet->configScheme()->findItemByName("testIntEntry")->property().toInt(), 23);
    QCOMPARE(applet->configScheme()->findItemByName("testStringEntry")->property().toString(), QStringLiteral("string-value"));

    qmlRegisterType<PlasmaQuick::ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<PlasmaQuick::ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");

    QQmlEngine engine;
    QQmlComponent *component = new QQmlComponent(&engine, applet->kPackage().fileUrl("configmodel"), this);
    QObject *object = component->create(engine.rootContext());
    PlasmaQuick::ConfigModel *configModel = qobject_cast<PlasmaQuick::ConfigModel *>(object);

    QCOMPARE(configModel->rowCount(), 1);
    QCOMPARE(configModel->get(0).toMap().value(QStringLiteral("name")).toString(), QStringLiteral("General"));
    QCOMPARE(configModel->get(0).toMap().value(QStringLiteral("icon")).toString(), QStringLiteral("plasma"));
    QCOMPARE(configModel->get(0).toMap().value(QStringLiteral("source")).toString(), QStringLiteral("ConfigGeneral.qml"));
    QCOMPARE(configModel->get(0).toMap().value(QStringLiteral("pluginName")).toString(), QString());
    QVERIFY(!configModel->get(0).toMap().value(QStringLiteral("kcm")).value<void *>());

    delete component;
    delete applet;
}

void ConfigModelTest::emptySourceWithApplet()
{
    Plasma::Applet *applet = Plasma::Applet::loadPlasmoid(QFINDTESTDATA("data/testconfigpackage"));
    PlasmaQuick::ConfigModel model;

    model.appendCategory(QStringLiteral("plasma"), QStringLiteral("name"), QString(), QString());

    QVERIFY(model.hasIndex(0, 0));
    QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toString(), QString());
    QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toString(), QString());

    model.setApplet(applet);

    QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toString(), QString());
    QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toString(), QString());

    delete applet;
}

void ConfigModelTest::notEmptySourceWithApplet()
{
    const QString pkgPath = QFINDTESTDATA("data/testconfigpackage");

    {
        Plasma::Applet *applet = Plasma::Applet::loadPlasmoid(pkgPath);
        PlasmaQuick::ConfigModel model;

        // Relative source
        model.appendCategory(QStringLiteral("plasma"), QStringLiteral("name"), QStringLiteral("ConfigGeneral.qml"), QString());

        QVERIFY(model.hasIndex(0, 0));
        QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toString(), QStringLiteral("ConfigGeneral.qml"));
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toString(), QStringLiteral("ConfigGeneral.qml"));

        model.setApplet(applet);

        const QUrl fullPath = QUrl::fromLocalFile(pkgPath + QStringLiteral("/contents/ui/ConfigGeneral.qml"));
        QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toUrl(), fullPath);
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toUrl(), fullPath);

        delete applet;
    }

    {
        Plasma::Applet *applet = Plasma::Applet::loadPlasmoid(pkgPath);
        PlasmaQuick::ConfigModel model;

        // Absolute source
        const QString source = QStringLiteral("/test/contents/ui/ConfigGeneral.qml");
        model.appendCategory(QStringLiteral("plasma"), QStringLiteral("name"), source, QString());

        QVERIFY(model.hasIndex(0, 0));
        QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toString(), source);
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toString(), source);

        model.setApplet(applet);

        QCOMPARE(model.data(model.index(0, 0), PlasmaQuick::ConfigModel::SourceRole).toString(), source);
        QCOMPARE(model.get(0).toMap().value(QStringLiteral("source")).toString(), source);

        delete applet;
    }
}

QTEST_MAIN(ConfigModelTest)
