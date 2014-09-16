/********************************************************************************
*   Copyright 2014 Marco Martin <mart@kde.org>                                  *
*                                                                               *
*   This library is free software; you can redistribute it and/or               *
*   modify it under the terms of the GNU Library General Public                 *
*   License as published by the Free Software Foundation; either                *
*   version 2 of the License, or (at your option) any later version.            *
*                                                                               *
*   This library is distributed in the hope that it will be useful,             *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of              *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            *
*   Library General Public License for more details.                            *
*                                                                               *
*   You should have received a copy of the GNU Library General Public License   *
*   along with this library; see the file COPYING.LIB.  If not, write to        *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,        *
*   Boston, MA 02110-1301, USA.                                                 *
*********************************************************************************/

#include "dialogtest.h"
#include <KWindowSystem>


void DialogTest::initTestCase()
{
    m_dialog = new PlasmaQuick::Dialog;

    m_panel = new QQuickView;
    m_panel->setGeometry(0, 0, 50, 50);
    m_panel->setFlags(Qt::FramelessWindowHint|Qt::WindowDoesNotAcceptFocus);

    m_content = new QQuickItem;
    m_content->setWidth(100);
    m_content->setHeight(100);
    m_dialog->setMainItem(m_content);

    m_panel->show();
    KWindowSystem::setType(m_panel->winId(), NET::Dock);
    m_dialog->setVisualParent(m_panel->contentItem());
    m_dialog->show();
}

void DialogTest::cleanupTestCase()
{
    delete m_dialog;
    delete m_panel;
}

void DialogTest::position()
{
    QTest::qWaitForWindowExposed(m_dialog);

    QCOMPARE(m_dialog->x(), 0);
    QCOMPARE(m_dialog->y(), 49);
}

QTEST_MAIN(DialogTest)
