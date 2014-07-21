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

#include "framesvgtest.h"
#include <QStandardPaths>



void FrameSvgTest::initTestCase()
{
    m_frameSvg = new Plasma::FrameSvg;
    m_frameSvg->setImagePath(QFINDTESTDATA("data/background.svgz"));
    QVERIFY(m_frameSvg->isValid());
}

void FrameSvgTest::cleanupTestCase()
{
    delete m_frameSvg;
}

void FrameSvgTest::margins()
{
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::LeftMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::TopMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::RightMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::BottomMargin), (qreal)26);
}

void FrameSvgTest::contentsRect()
{
    m_frameSvg->resizeFrame(QSize(100,100));
    QEXPECT_FAIL("", "contentsRect() is broken in master right now.", Continue);
    QCOMPARE(m_frameSvg->contentsRect(), QRectF(26, 26, 48, 48));
}

QTEST_MAIN(FrameSvgTest)

