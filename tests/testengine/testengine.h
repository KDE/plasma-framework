/*
    SPDX-FileCopyrightText: 2008 Gilles CHAUVIN <gcnweb+kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
 *  A data engine meant to test the Plasma data engine explorer.
 */

#ifndef __TESTENGINE_H__
#define __TESTENGINE_H__

#include "plasma/dataengine.h"

class TestEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    struct MyUserType {
        int     a;
        QString b;
    };

    TestEngine(QObject *parent, const QVariantList &args);
    ~TestEngine();

protected:
    void init();
    bool sourceRequestEvent(const QString &source) override;
};

#endif // __TESTENGINE_H__
