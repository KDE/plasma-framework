/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CUSTOMCORONA_H
#define CUSTOMCORONA_H

#include <Plasma/Corona>
#include <PlasmaQuick/ContainmentView>

class CustomCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit CustomCorona(QObject *parent = nullptr);
    QRect screenGeometry(int id) const override;

public Q_SLOTS:
    void load();

private:
    PlasmaQuick::ContainmentView *m_view;
};

#endif
