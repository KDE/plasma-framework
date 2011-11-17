/*
 * Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com>
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

#ifndef PLASMA_ANIMATIONS_JS_P_H
#define PLASMA_ANIMATIONS_JS_P_H

#include <QScriptValue>

#include "animator.h"
#include "easinganimation_p.h"
#include "plasma_export.h"

class QString;
class QScriptEngine;

//#define PLASMA_JSANIM_FPS

namespace Plasma
{

class JavascriptAnimation: public EasingAnimation
{
    Q_OBJECT

public:
    enum { PauseAnimation = Animator::LastAnimation + 1,
           PropertyAnimation = Animator::LastAnimation + 2
    };

    explicit JavascriptAnimation(const QString &name, QObject *parent = 0);

    ~JavascriptAnimation();

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateEffectiveTime(int currentTime);

private:
    void prepInstance();

#ifdef PLASMA_JSANIM_FPS
    int m_fps;
#endif
    QString m_name;
    QScriptValue m_instance;
    QScriptValue m_method;
};

}

#endif

