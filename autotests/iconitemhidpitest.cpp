/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>
#include <QtTest>

#include <KIconEngine>
#include <KIconLoader>
#include <KIconTheme>

#include "plasma/svg.h"
#include "plasma/theme.h"
#include "utils.h"

class IconItemHiDPITest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testScaleFactor();

private:
    QQuickItem *createIconItem();

    QQuickView *m_view = nullptr;
};

void IconItemHiDPITest::initTestCase()
{
    Plasma::TestUtils::installPlasmaTheme("breeze");
    Plasma::TestUtils::installPlasmaTheme("breeze-light");
    Plasma::TestUtils::installPlasmaTheme("breeze-dark");

    QVERIFY(qputenv("XDG_DATA_DIRS", qgetenv("XDG_DATA_DIRS") + ":" + QFINDTESTDATA("data").toLocal8Bit()));

    QIcon::setThemeSearchPaths({QFINDTESTDATA("data/icons")});
    QIcon::setThemeName("test-theme");

    KIconTheme::forceThemeForTests("test-theme");
    KIconTheme::reconfigure();
    KIconLoader::global()->reconfigure(QString());

    m_view = new QQuickView();
    m_view->setSource(QUrl::fromLocalFile(QFINDTESTDATA("data/view.qml")));
    m_view->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_view));

    if (!m_view->rootObject() || !m_view->rootObject()->grabToImage()) {
        QSKIP("Cannot grab item to image.");
    }
}

void IconItemHiDPITest::cleanupTestCase()
{
    delete m_view;
}

void IconItemHiDPITest::init()
{
    Plasma::Theme().setThemeName(QStringLiteral("default"));
}

void IconItemHiDPITest::cleanup()
{
    qDeleteAll(m_view->rootObject()->childItems());
}

QQuickItem *IconItemHiDPITest::createIconItem()
{
    QByteArray iconQml =
        "import QtQuick 2.0;"
        "import org.kde.plasma.core 2.0 as PlasmaCore;"
        "PlasmaCore.IconItem {"
        "    id: root;"
        "}";

    QQmlComponent component(m_view->engine());

    QSignalSpy spy(&component, SIGNAL(statusChanged(QQmlComponent::Status)));
    component.setData(iconQml, QUrl("test://iconTest"));
    if (component.status() != QQmlComponent::Ready) {
        spy.wait();
    }

    QQuickItem *item = qobject_cast<QQuickItem *>(component.create(m_view->engine()->rootContext()));
    Q_ASSERT(item && qstrcmp(item->metaObject()->className(), "IconItem") == 0);
    item->setParentItem(m_view->rootObject());
    return item;
}

void IconItemHiDPITest::testScaleFactor()
{
    QString name("tst-plasma-framework-test-icon");

    QQuickItem *item = createIconItem();
    item->setProperty("animated", false);
    item->setSize(QSize(64, 64));
    item->setProperty("source", name);
    QVERIFY(item->property("valid").toBool());

    Plasma::Svg *svg = item->findChild<Plasma::Svg *>();
    Q_ASSERT(svg);
    QCOMPARE(svg->imagePath(), QFINDTESTDATA("data/icons/test-theme/apps/32/" + name + ".svg"));
    if (m_view->devicePixelRatio() != 1.25) {
        qInfo() << m_view->devicePixelRatio();
        QSKIP("Can't test devicePixelRatio when system DPI is not 100%.");
    }
    QCOMPARE(svg->devicePixelRatio(), 2);
}

QTEST_MAIN(IconItemHiDPITest)

#include "iconitemhidpitest.moc"
