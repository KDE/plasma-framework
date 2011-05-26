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

#ifndef SIGNINGTEST_H

#include <qtest_kde.h>

#include "plasma/package.h"

namespace Plasma
{
    class Signing;
}


class SigningTest : public QObject
{
    Q_OBJECT
public:
    explicit SigningTest(QObject *parent = 0);

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void confirmCtorPerformance();
    void missingFiles();
    void invalidSignature();
    void validSignature();
    void validSignatureWithoutDefinedSigFile();
    void validPackage();
    void trustLevel();
    void confirmDtorPerformance();

private:
    Plasma::Signing *m_signing;
    Plasma::Package m_package;
    QString m_path;
    QString m_sig;
    QString m_invalidSig;
};

#endif

