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

#include "javascriptanimation_p.h"

#include <kdebug.h>

#include "animationscriptengine_p.h"
/* TODO:
 * - support passing more parameters to the js animation object
 * - support more properties: angle, direction, etc
 * - support calling a 'resetAnimation' in js class when animation is stopped
 */
#define ADD_ENUM_VALUE(__c__, __ns__, __v__) \
    __c__.setProperty(#__v__, QScriptValue(__c__.engine(), __ns__::__v__))

namespace Plasma
{

JavascriptAnimation::JavascriptAnimation(const QString &name, QObject *parent)
    : EasingAnimation(parent),
#ifdef PLASMA_JSANIM_FPS
      m_fps(0),
#endif
      m_name(name)
{

}

JavascriptAnimation::~JavascriptAnimation()
{
}

void JavascriptAnimation::prepInstance()
{
    QScriptEngine *engine = AnimationScriptEngine::globalEngine();
    m_instance.setProperty("__plasma_javascriptanimation", engine->newQObject(this),
                          QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, FadeAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, AppearAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, DisappearAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, ActivateAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, FadeAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, GrowAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, PulseAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, RotationAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, RotationStackedAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, SlideAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, GeometryAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, ZoomAnimation);
    ADD_ENUM_VALUE(m_instance, Plasma::Animator, PixmapTransitionAnimation);
    ADD_ENUM_VALUE(m_instance, JavascriptAnimation, PauseAnimation);
    ADD_ENUM_VALUE(m_instance, JavascriptAnimation, PropertyAnimation);
}

void JavascriptAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    //kDebug() << ".................. state: " << newState;
    if (oldState == Stopped && newState == Running) {
        if (!m_method.isFunction()) {
            //Define the class and create an instance
            m_instance = AnimationScriptEngine::animation(m_name).construct();
            kDebug() << "trying for" << m_name << m_instance.isFunction();

            //Get the method of the object
            m_method = m_instance.property(QString("updateCurrentTime"));
            if (!m_method.isFunction()) {
                qDebug() << "**************** ERROR! Name: " << m_name << " ************";
                m_instance = m_method = QScriptValue();
            } else {
                prepInstance();

                //TODO: this really should be done in the bindings provided
                //Center the widget for transformation
                qreal x = targetWidget()->geometry().height()/2;
                qreal y = targetWidget()->geometry().width()/2;
                targetWidget()->setTransformOriginPoint(x, y);
            }
        }

        if (m_method.isFunction()) {
            m_instance.setProperty("duration", duration(), QScriptValue::ReadOnly);
            m_instance.setProperty("target", m_instance.engine()->newQObject(targetWidget()), QScriptValue::ReadOnly);
        }
#ifdef PLASMA_JSANIM_FPS
        m_fps = 0;
    } else if (oldState == Running && newState == Stopped) {
        kDebug() << ".........." << m_name << " fps: " << m_fps * 1000/duration();
#endif
    }
}

void JavascriptAnimation::updateEffectiveTime(int currentTime)
{
    if (m_method.isFunction()) {
#ifdef PLASMA_JSANIM_FPS
        ++m_fps;
#endif
        QScriptValueList args;
        args << currentTime;

        m_method.call(m_instance, args);
    }
}

} //namespace Plasma

#include <javascriptanimation_p.moc>
