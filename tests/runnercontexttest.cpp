/******************************************************************************
*   Copyright 2010 Aaron Seigo <aseigo@kde.org>                               *
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

#include "runnercontexttest.h"

#include "plasma/runnercontext.h"

Q_DECLARE_METATYPE(Plasma::RunnerContext::Type)

void RunnerContextTest::typeDetection_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Plasma::RunnerContext::Type>("type");

    QTest::newRow("man page listing") << "man:/" << Plasma::RunnerContext::NetworkLocation;
    QTest::newRow("ls man page listing") << "man://ls" << Plasma::RunnerContext::NetworkLocation;
    QTest::newRow("http without host") << "http://" << Plasma::RunnerContext::UnknownType;
    QTest::newRow("http with host") << "http://kde.org" << Plasma::RunnerContext::NetworkLocation;
    QTest::newRow("file double slash") << "file://home" << Plasma::RunnerContext::Directory;
    QTest::newRow("file triple slash") << "file:///home" << Plasma::RunnerContext::Directory;
    QTest::newRow("file single slash") << "file:/home" << Plasma::RunnerContext::Directory;
    QTest::newRow("file multiple path") << "file://usr/bin" << Plasma::RunnerContext::Directory;
    QTest::newRow("invalid file path") << "file://bad/path" << Plasma::RunnerContext::UnknownType;
    QTest::newRow("executable") << "ls" << Plasma::RunnerContext::Executable;
    QTest::newRow("executable with params") << "ls -R" << Plasma::RunnerContext::ShellCommand;
    QTest::newRow("full path executable") << "ls -R" << Plasma::RunnerContext::ShellCommand;
    QTest::newRow("full path executable with params") << "/bin/ls -R" << Plasma::RunnerContext::ShellCommand;
    QTest::newRow("protocol-less path") << "/home" << Plasma::RunnerContext::Directory;
    QTest::newRow("invalid protocol-less path") << "/bad/path" << Plasma::RunnerContext::UnknownType;
}

void RunnerContextTest::typeDetection()
{
    QFETCH(QString, url);
    QFETCH(Plasma::RunnerContext::Type, type);

    m_context.setQuery(url);
    QCOMPARE(m_context.type(), type);
}

QTEST_KDEMAIN(RunnerContextTest, NoGUI)
