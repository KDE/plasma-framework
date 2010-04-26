/////////////////////////////////////////////////////////////////////////
// jsanim.cpp                                             //
//                                                                     //
// Copyright (C)  2010  Adenilson Cavalcanti <cavalcantii@gmail.com>   //
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "jsanim_p.h"
#include "engine.h"
#include <QDebug>

/* TODO:
 * - support passing more parameters to the js animation object
 * - support more properties: angle, direction, etc
 * - support calling a 'resetAnimation' in js class when animation is stopped
 */

namespace Plasma
{

JavascriptAnimation::JavascriptAnimation(const QString &name, QObject *parent)
    : Animation(parent), m_fps(0), m_name(name), engine(0), m_instance(0), m_method(0)
{

}

JavascriptAnimation::~JavascriptAnimation()
{
    delete m_instance;
    delete m_method;
}

void JavascriptAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    qDebug() << ".................. state: " << newState;
    if (oldState == Stopped && newState == Running) {
        if (!engine) {
            engine = AnimationEngine::globalEngine();

            //Define the class and create an instance
            if (!m_instance) {
                m_instance = new QScriptValue;
                *m_instance = AnimationEngine::animation(m_name).construct(QScriptValueList()
                                               << engine->newQObject(targetWidget())
                                               << duration());
                qDebug( )<< "trying for" << m_name << m_instance->isFunction();
            }

            //Get the method of the object
            if (!m_method) {
                m_method = new QScriptValue;
                *m_method = m_instance->property(QString("updateCurrentTime"));
                if (!m_method->isFunction()) {
                    qDebug() << "**************** ERROR! ************";
                } else {
                    qDebug() << ".................. success js instance creation!";
                }

            }

            //Center the widget for transformation
            qreal x = targetWidget()->geometry().height()/2;
            qreal y = targetWidget()->geometry().width()/2;
            targetWidget()->setTransformOriginPoint(x, y);

        }

        m_fps = 0;

    } else if (oldState == Running && newState == Stopped) {

        qDebug() << ".........." << m_name << " fps: " << m_fps * 1000/duration();
    }
}

void JavascriptAnimation::updateCurrentTime(int currentTime)
{
    if (m_method->isFunction()) {
        ++m_fps;
        QScriptValueList args;
        args << currentTime;

        m_method->call(*m_instance, args);
    }
}

} //namespace Plasma

#include <../jsanim_p.moc>
