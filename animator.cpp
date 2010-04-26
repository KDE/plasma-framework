/*
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "animator.h"
#include "private/animator_p.h"

#include <kdebug.h>

#include "animations/animation.h"
#include "animations/fade_p.h"
#include "animations/grow_p.h"
#include "animations/pulser_p.h"
#include "animations/rotation_p.h"
#include "animations/slide_p.h"
#include "animations/rotationstacked_p.h"
#include "animations/geometry_p.h"
#include "animations/zoom_p.h"
#include "animations/pixmaptransition_p.h"
#include "animations/water_p.h"
#include "animations/pendulumcurve_p.h"
#include "animations/javascriptanimation_p.h"
#include "animations/animationscriptengine_p.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

static QSet<QString> s_paths;

namespace Plasma
{

Plasma::Animation* Animator::create(Animator::Animation type, QObject *parent)
{
    Plasma::Animation *result = 0;

    switch (type) {
    case FadeAnimation:
        result = new Plasma::FadeAnimation(parent);
        break;

    case GrowAnimation:
        result = new Plasma::GrowAnimation(parent);
        break;

    case PulseAnimation:
        result = new Plasma::PulseAnimation(parent);
        break;

    case RotationAnimation:
        result = new Plasma::RotationAnimation(parent);
        break;

    case RotationStackedAnimation:
        result = new Plasma::RotationStackedAnimation(parent);
        break;

    case SlideAnimation:
        result = new Plasma::SlideAnimation(parent);
        break;

    case GeometryAnimation:
        result = new Plasma::GeometryAnimation(parent);
        break;

    case ZoomAnimation:
        result = new Plasma::ZoomAnimation(parent);
        break;

    case PixmapTransitionAnimation:
        result = new Plasma::PixmapTransition(parent);
        break;

    case WaterAnimation:
        result = new Plasma::WaterAnimation(parent);
        break;

    default:
        kDebug() << "Unsupported animation type.";
    }

    return result;
}

QEasingCurve Animator::create(Animator::CurveShape type)
{
    QEasingCurve result;

    switch (type) {
    case EaseInCurve:
        result.setType(QEasingCurve::InQuad);
        break;

    case EaseOutCurve:
        result.setType(QEasingCurve::OutQuad);
        break;

    case EaseInOutCurve:
        result.setType(QEasingCurve::InOutQuad);
        break;

    case LinearCurve:
        result.setType(QEasingCurve::Linear);
        break;

    case PendularCurve:
        result = PendulumCurve();
        break;

    default:
        kDebug() << "Unsupported easing curve type.";
        break;
    }

    return result;
}

static Plasma::Animation *create(QString &path, QObject *parent = 0)
{
    Plasma::Animation *result = 0;
    /* Here this will be executed in the Animator::factory() and
     * the path is retrieved from the Theme class
     */
    //qDebug() << path;

    /**
     * FIXME: in future, the path and the animation name will not match 1:1
     * so we need a way to map animations to path names. trivial would be to map
     * "Zoom" to "zoom.js" (e.g. anim.toLower() + 'js') but that's also a bit lame.
     * I think we may need a mapping, much as we use KService to do for applet
     * names to libraries. Maybe we should, in fact, use KSycoca for this?
     * In any case, the method used below even allows for all anims to be defined in
     * one file!
     */
    if (!s_paths.contains(path)) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            kError() << "failed to open js......";
            return result;
        }

        QTextStream buffer(&file);
        QString tmp(buffer.readAll());
        s_paths.insert(path);

        QScriptEngine *engine = AnimationScriptEngine::globalEngine();
        QScriptValue def(engine->evaluate(tmp, path));
        if (engine->hasUncaughtException()) {
            const QScriptValue error = engine->uncaughtException();
            QString file = error.property("fileName").toString();
            const QString failureMsg = QString("Error in %1 on line %2.<br><br>%3").arg(
                    file).arg(error.property("lineNumber").toString()).arg(error.toString());
            kError() << "fail!" << failureMsg;
        }
    }

    result = new Plasma::JavascriptAnimation(path);
    result->setParent(parent);

    return result;
}

} // namespace Plasma

#include <animator.moc>

