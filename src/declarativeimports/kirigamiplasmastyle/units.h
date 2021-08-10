/*
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIRIGAMIPLASMADESKTOPUNITS_H
#define KIRIGAMIPLASMADESKTOPUNITS_H

#include <QObject>

#include <Kirigami/Units>

#include <KConfigWatcher>

class Units : public Kirigami::Units
{
    Q_OBJECT

public:
    explicit Units(QObject *parent = nullptr);

    void updateAnimationSpeed();

private:
    KConfigWatcher::Ptr m_animationSpeedWatcher;
};

#endif
