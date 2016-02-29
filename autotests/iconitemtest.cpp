/******************************************************************************
*   Copyright 2016 David Rosca <nowrep@gmail.com>                             *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "iconitemtest.h"

#include <QIcon>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItemGrabResult>
#include <QtTest/QSignalSpy>

#include <KIconLoader>
#include <KIconEngine>

#include "plasma/theme.h"
#include "plasma/svg.h"

static bool imageIsEmpty(const QImage &img)
{
    for (int i = 0; i < img.width(); ++i) {
        for (int j = 0; j < img.height(); ++j) {
            if (img.pixel(i, j) != 0) {
                return false;
            }
        }
    }
    return true;
}

void IconItemTest::initTestCase()
{
    // make our theme in search path
    qputenv("XDG_DATA_DIRS",
            qgetenv("XDG_DATA_DIRS") + ":" + QFINDTESTDATA("data").toLocal8Bit());

    // set default icon theme to test-theme
    QStandardPaths::setTestModeEnabled(true);

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if(!QDir(configPath).mkpath(QStringLiteral("."))) {
        qFatal("Failed to create test configuration directory.");
    }

    QFile::remove(configPath);

    KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
    cg.writeEntry("Theme", "test-theme");
    KConfigGroup plasmaConfig(KSharedConfig::openConfig("plasmarc"), "Theme");
    plasmaConfig.writeEntry("name", "default");

    m_view = new QQuickView();
    m_view->setSource(QUrl::fromLocalFile(QFINDTESTDATA("data/view.qml")));
    m_view->show();
    QTest::qWaitForWindowExposed(m_view);
}

void IconItemTest::cleanup()
{
    qDeleteAll(m_view->rootObject()->childItems());
}

QQuickItem *IconItemTest::createIconItem()
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

    QQuickItem *item = qobject_cast<QQuickItem*>(component.create(m_view->engine()->rootContext()));
    Q_ASSERT(item && qstrcmp(item->metaObject()->className(), "IconItem") == 0);
    item->setParentItem(m_view->rootObject());
    return item;
}

QImage IconItemTest::grabImage(QQuickItem *item)
{
    QSharedPointer<QQuickItemGrabResult> grab = item->grabToImage();
    QSignalSpy spy(grab.data(), SIGNAL(ready()));
    spy.wait();
    return grab->image();
}

Plasma::Svg *IconItemTest::findPlasmaSvg(QQuickItem *item)
{
    return item->findChild<Plasma::Svg *>();
}

void IconItemTest::changeTheme(Plasma::Theme *theme, const QString &themeName)
{
    if (theme->themeName() != themeName) {
        QSignalSpy spy(theme, SIGNAL(themeChanged()));
        theme->setThemeName(themeName);
        spy.wait();
    }
}

// ------ Tests
void IconItemTest::invalidIcon()
{
    QString name("tst-plasma-framework-invalid-icon-name");
    KIconLoader iconLoader("tst_plasma-framework");
    if (iconLoader.hasIcon(name)) {
        QSKIP("Current icon theme has 'tst-plasma-framework-invalid-icon-name' icon.");
    }

    QQuickItem *item = createIconItem();
    item->setProperty("source", name);
    QVERIFY(!item->property("valid").toBool());
    QVERIFY(imageIsEmpty(grabImage(item)));
}

void IconItemTest::usesPlasmaTheme()
{
    Plasma::Theme theme;
    if (!theme.themeName().startsWith("default")) {
        QSKIP("Current Plasma theme is not Breeze.");
    }

    // usesPlasmaTheme = true (default)
    QQuickItem *item1 = createIconItem();
    item1->setProperty("source", "konversation");
    QVERIFY(item1->property("valid").toBool());

    Plasma::Svg svg;
    svg.setContainsMultipleImages(true);
    svg.setImagePath("icons/konversation");

    QImage img1 = grabImage(item1);
    QImage img2 = svg.image(QSize(item1->width(), item1->height()), "konversation");
    QVERIFY(!imageIsEmpty(img1));
    QVERIFY(!imageIsEmpty(img2));
    QCOMPARE(img1, img2);

    // usesPlasmaTheme = false
    QQuickItem *item2 = createIconItem();
    item2->setProperty("usesPlasmaTheme", false);
    item2->setProperty("source", "konversation");

    img1 = grabImage(item2);
    // This depends on konversation icon being different in Plasma Breeze theme
    // and our test icon theme
    QVERIFY(img1 != img2);
}

void IconItemTest::animation()
{
    // animated = true (default)
    QQuickItem *item1 = createIconItem();
    item1->setProperty("source", "user-away");
    // first icon is not animated
    QImage userAwayImg = grabImage(item1);

    item1->setProperty("source", "user-busy");
    grabImage(item1);
    item1->setProperty("source", "user-away");
    // animation from user-busy -> user-away
    QVERIFY(userAwayImg != grabImage(item1));

    // animated = false
    QQuickItem *item2 = createIconItem();
    item2->setProperty("animated", false);
    item2->setProperty("source", "user-busy");
    QImage userBusyImg = grabImage(item2);

    item2->setProperty("source", "user-away");
    QCOMPARE(userAwayImg, grabImage(item2));

    item2->setProperty("source", "user-busy");
    QCOMPARE(userBusyImg, grabImage(item2));
}

void IconItemTest::animationAfterHide()
{
    QEXPECT_FAIL("", "Needs 'no animation after hide' patch", Abort);

    QQuickItem *item1 = createIconItem();
    QQuickItem *item2 = createIconItem();
    item1->setProperty("source", "user-away");
    item2->setProperty("source", "user-busy");
    // first icon is not animated
    QImage userAwayImg = grabImage(item1);
    QImage userBusyImg = grabImage(item2);

    item1->setProperty("source", "user-busy");
    grabImage(item1);
    item1->setProperty("visible", "false");
    item1->setProperty("visible", "true");
    item1->setProperty("source", "user-away");
    // icon was hidden, no animation
    QCOMPARE(userAwayImg, grabImage(item1));

    item1->setProperty("source", "user-busy");
    QVERIFY(userBusyImg != grabImage(item1));
}

void IconItemTest::bug_359388()
{
    QString name("bug359388");
    KIconLoader iconLoader("tst_plasma-framework");
    QIcon customThemeIcon(new KIconEngine(name, &iconLoader));
    if (iconLoader.hasIcon(name)) {
        QSKIP("Current icon theme has 'bug359388' icon.");
    }

    iconLoader.addAppDir("tst_plasma-framework", QFINDTESTDATA("data/bug359388"));

    QQuickItem *item1 = createIconItem();
    item1->setProperty("source", customThemeIcon);
    QVERIFY(item1->property("valid").toBool());

    QQuickItem *item2 = createIconItem();
    item2->setProperty("source", QIcon(QFINDTESTDATA("data/bug359388/hicolor/22x22/apps/" + name + ".svg")));
    QVERIFY(item2->property("valid").toBool());

    QCOMPARE(grabImage(item1), grabImage(item2));
}

void IconItemTest::loadSvg()
{
    QString name("tst-plasma-framework-test-icon");

    QQuickItem *item = createIconItem();
    item->setProperty("animated", false);
    item->setSize(QSize(22, 22));
    item->setProperty("source", name);
    QVERIFY(item->property("valid").toBool());

    Plasma::Svg *svg;
    svg = findPlasmaSvg(item);
    Q_ASSERT(svg);
    QCOMPARE(svg->imagePath(), QFINDTESTDATA("data/icons/test-theme/apps/22/" + name + ".svg"));

    // we only have 32x32 and 22x22 version in the theme, thus 32x32 is a better match.
    item->setSize(QSize(64, 64));
    // just to update the icon
    grabImage(item);
    svg = findPlasmaSvg(item);
    Q_ASSERT(svg);
    QCOMPARE(svg->imagePath(), QFINDTESTDATA("data/icons/test-theme/apps/32/" + name + ".svg"));
}

void IconItemTest::themeChange()
{
    // Icon from Plasma theme
    QQuickItem *item1 = createIconItem();
    item1->setProperty("animated", false);
    item1->setProperty("source", "zoom-fit-height");
    Plasma::Svg *svg1 = item1->findChild<Plasma::Svg*>();
    changeTheme(svg1->theme(), "breeze-light");
    QImage img1 = grabImage(item1);
    changeTheme(svg1->theme(), "breeze-dark");
    QImage img2 = grabImage(item1);
    QVERIFY(img1 != img2);

    // Icon from icon theme
    QQuickItem *item2 = createIconItem();
    item2->setProperty("animated", false);
    item2->setProperty("width", 22);
    item2->setProperty("height", 22);
    item2->setProperty("source", "tst-plasma-framework-test-icon");
    Plasma::Svg *svg2 = item2->findChild<Plasma::Svg*>();
    changeTheme(svg2->theme(), "breeze-light");
    img1 = grabImage(item2);
    changeTheme(svg2->theme(), "breeze-dark");
    img2 = grabImage(item2);
    QVERIFY(img1 != img2);
}

QTEST_MAIN(IconItemTest)

