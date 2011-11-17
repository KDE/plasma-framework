/*
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef JAVASCRIPTANIMATIONBINDINGS_P_H
#define JAVASCRIPTANIMATIONBINDINGS_P_H

#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

namespace Plasma
{

class ParallelAnimationGroup : public QParallelAnimationGroup
{
    Q_OBJECT

public:
    ParallelAnimationGroup(QObject *parent);

public Q_SLOTS:
    void addAnimation(QAbstractAnimation * animation);
    QAbstractAnimation *animationAt(int index) const;
    int animationCount() const;
    void clear();
    int indexOfAnimation(QAbstractAnimation *animation) const;
    void insertAnimation(int index, QAbstractAnimation * animation);
    void removeAnimation(QAbstractAnimation * animation);
};

class SequentialAnimationGroup : public QSequentialAnimationGroup
{
    Q_OBJECT

public:
    SequentialAnimationGroup(QObject *parent);

public Q_SLOTS:
    void addAnimation(QAbstractAnimation * animation);
    QAbstractAnimation *animationAt(int index) const;
    int animationCount() const;
    void clear();
    int indexOfAnimation(QAbstractAnimation *animation) const;
    void insertAnimation(int index, QAbstractAnimation * animation);
    void removeAnimation(QAbstractAnimation * animation);
};

} // namespace Plasma

#endif

