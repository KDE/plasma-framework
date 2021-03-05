/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialogqmltest.h"

#include <plasma.h>

#include <QQmlContext>
#include <QQmlEngine>

#include <QSignalSpy>

// this test checks that we don't set visible to true until after we set the window flags
void DialogQmlTest::loadAndShow()
{
    QQmlEngine engine;

    QByteArray dialogQml =
        "import QtQuick 2.0\n"
        "import org.kde.plasma.core 2.0 as PlasmaCore\n"
        "\n"
        "PlasmaCore.Dialog {\n"
        "    id: root\n"
        "\n"
        "    location: true && PlasmaCore.Types.TopEdge\n"
        "    visible: true && true\n"
        "    type: true && PlasmaCore.Dialog.Notification\n"
        "\n"
        "    mainItem: Rectangle {\n"
        "        width: 200\n"
        "        height: 200\n"
        "    }\n"
        "}\n";

    // we use true && Value to force it to be a complex binding, which won't be evaluated in
    // component.beginCreate
    // the bug still appears without this, but we need to delay it in this test
    // so we can connect to the visibleChanged signal

    QQmlComponent component(&engine);

    QSignalSpy spy(&component, SIGNAL(statusChanged(QQmlComponent::Status)));
    component.setData(dialogQml, QUrl(QStringLiteral("test://dialogTest")));
    spy.wait();

    PlasmaQuick::Dialog *dialog = qobject_cast<PlasmaQuick::Dialog *>(component.beginCreate(engine.rootContext()));
    qDebug() << component.errorString();
    Q_ASSERT(dialog);

    m_dialogShown = false;

    // this will be called during component.completeCreate
    auto c = connect(dialog, &QWindow::visibleChanged, [=]() {
        m_dialogShown = true;
        QCOMPARE(dialog->type(), PlasmaQuick::Dialog::Notification);
        QCOMPARE(dialog->location(), Plasma::Types::TopEdge);
    });

    component.completeCreate();
    QCOMPARE(m_dialogShown, true);

    // disconnect on visible changed before we delete the dialog
    disconnect(c);

    delete dialog;
}

QTEST_MAIN(DialogQmlTest)
