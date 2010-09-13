/********************************************************************************
*   Copyright 2010 by Martin Blumenstingl <darklight.xdarklight@googlemail.com> *
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

#ifndef CONFIGLOADERTEST_H

#include <qtest_kde.h>

namespace Plasma
{
    class ConfigLoader;
}

class QFile;

class ConfigLoaderTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void boolDefaultValue();
    void colorDefaultValue();
    void dateTimeDefaultValue();
    void enumDefaultValue();
    void fontDefaultValue();
    void intDefaultValue();
    void passwordDefaultValue();
    void pathDefaultValue();
    void stringDefaultValue();
    void stringListDefaultValue();
    void uintDefaultValue();
    void urlDefaultValue();
    void doubleDefaultValue();
    void intListDefaultValue();
    void longLongDefaultValue();
    void pointDefaultValue();
    void rectDefaultValue();
    void sizeDefaultValue();
    void ulongLongDefaultValue();

private:
    Plasma::ConfigLoader* cl;
    QFile* configFile;
};

#endif

