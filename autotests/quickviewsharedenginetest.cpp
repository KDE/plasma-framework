/*
    This file is part of the test suite of the Qt Toolkit.
    SPDX-FileCopyrightText: 2014 Digia Plc and/or its subsidiary(-ies) <http://www.qt-project.org/legal>

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LGPL-3.0-only WITH Qt-LGPL-exception-1.1 OR LicenseRef-Qt-Commercial
*/

#include <QDebug>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QSignalSpy>
#include <QTest>
#include <QWindow>
#include <QtTest/qtestcase.h>
#include <quickviewsharedengine.h>

class QQmlDataTest : public QObject
{
    Q_OBJECT
public:
    QQmlDataTest()
    {
    }

    QString testFile(const QString &fileName) const
    {
        if (m_directory.isEmpty()) {
            qFatal("QQmlDataTest::initTestCase() not called.");
        }
        QString result = m_dataDirectory;
        result += QLatin1Char('/');
        result += fileName;
        return result;
    }
    inline QString testFile(const char *fileName) const
    {
        return testFile(QLatin1String(fileName));
    }
    inline QUrl testFileUrl(const QString &fileName) const
    {
        return QUrl::fromLocalFile(testFile(fileName));
    }
    inline QUrl testFileUrl(const char *fileName) const
    {
        return testFileUrl(QLatin1String(fileName));
    }

    inline QString dataDirectory() const
    {
        return m_dataDirectory;
    }
    inline QUrl dataDirectoryUrl() const
    {
        return m_dataDirectoryUrl;
    }
    inline QString directory() const
    {
        return m_directory;
    }

    static QByteArray msgComponentError(const QQmlComponent &, const QQmlEngine *engine = nullptr);

public Q_SLOTS:
    virtual void initTestCase()
    {
        QVERIFY2(!m_dataDirectory.isEmpty(), "'input' directory not found");
        m_directory = QFileInfo(m_dataDirectory).absolutePath();
        QVERIFY2(QDir::setCurrent(m_directory), qPrintable(QLatin1String("Could not chdir to ") + m_directory));
    }

private:
    static QQmlDataTest *m_instance;

    const QString m_dataDirectory = QFINDTESTDATA("input");
    const QUrl m_dataDirectoryUrl = QUrl::fromLocalFile(m_dataDirectory + QLatin1Char('/'));

    QString m_directory;
};

class QuickViewSharedEngineTest : public QQmlDataTest
{
    Q_OBJECT
public:
    QuickViewSharedEngineTest();

private Q_SLOTS:
    void resizemodeitem();
    void errors();
    void engine();
};

QuickViewSharedEngineTest::QuickViewSharedEngineTest()
{
}

void QuickViewSharedEngineTest::resizemodeitem()
{
    QWindow window;
    window.setGeometry(0, 0, 400, 400);

    auto view = new PlasmaQuick::QuickViewSharedEngine(&window);
    QVERIFY(view);
    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeRootObjectToView);
    QCOMPARE(QSize(0, 0), view->initialSize());
    view->setSource(testFileUrl("resizemodeitem.qml"));
    QQuickItem *item = qobject_cast<QQuickItem *>(view->rootObject());
    QVERIFY(item);
    window.show();

    view->showNormal();

    // initial size from root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(view->size(), QSize(200, 200));
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->size(), view->initialSize());

    // size update from view
    view->resize(QSize(80, 100));

    QTRY_COMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QCOMPARE(view->size(), QSize(80, 100));
    QCOMPARE(view->size(), view->sizeHint());

    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeViewToRootObject);

    // size update from view disabled
    view->resize(QSize(60, 80));
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QTest::qWait(50);
    QCOMPARE(view->size(), QSize(60, 80));

    // size update from root object
    item->setWidth(250);
    item->setHeight(350);
    QCOMPARE(item->width(), 250.0);
    QCOMPARE(item->height(), 350.0);
    QTRY_COMPARE(view->size(), QSize(250, 350));
    QCOMPARE(view->size(), QSize(250, 350));
    QCOMPARE(view->size(), view->sizeHint());

    // reset window
    window.hide();
    delete view;
    view = new PlasmaQuick::QuickViewSharedEngine(&window);
    QVERIFY(view);
    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeViewToRootObject);
    view->setSource(testFileUrl("resizemodeitem.qml"));
    item = qobject_cast<QQuickItem *>(view->rootObject());
    QVERIFY(item);
    window.show();

    view->showNormal();

    // initial size for root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->size(), view->initialSize());

    // size update from root object
    item->setWidth(80);
    item->setHeight(100);
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QTRY_COMPARE(view->size(), QSize(80, 100));
    QCOMPARE(view->size(), QSize(80, 100));
    QCOMPARE(view->size(), view->sizeHint());

    // size update from root object disabled
    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeRootObjectToView);
    item->setWidth(60);
    item->setHeight(80);
    QCOMPARE(view->width(), 80);
    QCOMPARE(view->height(), 100);
    QCOMPARE(QSize(item->width(), item->height()), view->sizeHint());

    // size update from view
    view->resize(QSize(200, 300));
    QTRY_COMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 300.0);
    QCOMPARE(view->size(), QSize(200, 300));
    QCOMPARE(view->size(), view->sizeHint());

    delete view;

    // if we set a specific size for the view then it should keep that size
    // for SizeRootObjectToView mode.
    view = new PlasmaQuick::QuickViewSharedEngine(&window);
    view->resize(300, 300);
    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeRootObjectToView);
    QCOMPARE(QSize(0, 0), view->initialSize());
    view->setSource(testFileUrl("resizemodeitem.qml"));
    qWarning() << "bbb" << view->size();
    view->resize(300, 300);
    qWarning() << "ccc" << view->size();
    item = qobject_cast<QQuickItem *>(view->rootObject());
    QVERIFY(item);

    view->showNormal();
    QTest::qWait(50);

    // initial size from root object
    QCOMPARE(item->width(), 300.0);
    QCOMPARE(item->height(), 300.0);
    QCOMPARE(view->size(), QSize(300, 300));
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->initialSize(), QSize(200, 200)); // initial object size

    delete view;
}

QStringList messages;
void messageHandler(QtMsgType, const QMessageLogContext &, const QString &message)
{
    messages.append(message);
};
void QuickViewSharedEngineTest::errors()
{
    qInstallMessageHandler(messageHandler);

    auto view = new PlasmaQuick::QuickViewSharedEngine;
    QVERIFY(view);
    view->setSource(testFileUrl("error1.qml"));
    QCOMPARE(view->status(), QQmlComponent::Error);
    QCOMPARE(messages.count(), 1);
    delete view;
}

void QuickViewSharedEngineTest::engine()
{
    // test if engines are actually shared
    // test if the engine gets deleted when the last view dies
    auto view = new PlasmaQuick::QuickViewSharedEngine();
    QQmlEngine *engine = view->engine().get();

    auto view2 = new PlasmaQuick::QuickViewSharedEngine();
    QQmlEngine *engine2 = view2->engine().get();

    QCOMPARE(engine, engine2);
    QSignalSpy engineDestroyedSpy(engine, &QObject::destroyed);

    delete view;
    delete view2;

    QCOMPARE(engineDestroyedSpy.count(), 1);
}

QTEST_MAIN(QuickViewSharedEngineTest)

#include "quickviewsharedenginetest.moc"
