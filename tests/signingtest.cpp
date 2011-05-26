/********************************************************************************
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>                              *
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

#include "signingtest.h"

#include <QTime>

#include <kdebug.h>

#include "plasma/remote/signing.h"

SigningTest::SigningTest(QObject *parent)
    : QObject(parent),
      m_signing(0)
{
    const QString prefix = QString::fromLatin1(KDESRCDIR);
    m_package = prefix + "signed.plasmoid";
    m_packageSig = prefix + "signed.plasmoid.asc";
}

void SigningTest::init()
{
}

void SigningTest::cleanup()
{
}

void SigningTest::confirmCtorPerformance()
{
    QTime t;
    t.start();
    m_signing = new Plasma::Signing;
    QVERIFY(t.elapsed() < 50);
}

void SigningTest::missingFiles()
{
    QVERIFY(m_signing->signerOf(KUrl("/nonexistantpackage"), KUrl("/noneexistantsignature")).isEmpty());
    QVERIFY(m_signing->signerOf(KUrl(m_package), KUrl("/noneexistantsignature")).isEmpty());
    QVERIFY(m_signing->signerOf(KUrl("/nonexistantpackage"), KUrl(m_packageSig)).isEmpty());
}

void SigningTest::validSignature()
{
    QVERIFY(!m_signing->signerOf(m_package, m_packageSig).isEmpty());
}

void SigningTest::validSignatureWithoutDefinedSigFile()
{
    QVERIFY(!m_signing->signerOf(m_package).isEmpty());
}

void SigningTest::confirmDtorPerformance()
{
    QTime t;
    t.start();
    delete m_signing;
    m_signing = 0;
    QVERIFY(t.elapsed() < 50);
}

QTEST_KDEMAIN(SigningTest, NoGUI)

