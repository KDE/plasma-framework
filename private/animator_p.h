/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *                 2007 Alexis Ménard <darktears31@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef ANIMATOR_P_H
#define ANIMATOR_P_H

#include <QHash>
#include <QPixmap>
#include <QSet>
#include <QTime>
#include <QTimeLine>

class QGraphicsItem;

namespace Plasma
{

class  AnimatorPrivate
{
public:
    static void mapAnimation(Animator::Animation from, Animator::Animation to);
    static void mapAnimation(Animator::Animation from, const QString &to);
    static QHash<Animator::Animation, Animator::Animation> s_stockAnimMappings;
    static QHash<Animator::Animation, QString> s_loadableAnimMappings;
};

}

#endif

