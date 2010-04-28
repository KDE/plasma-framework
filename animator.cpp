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

#include <QFile>
#include <QTextStream>

#include <KDebug>

#include "animations/animation.h"
#include "animations/animationscriptengine_p.h"
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
#include "theme.h"

namespace Plasma
{

QHash<Animator::Animation, Animator::Animation> AnimatorPrivate::s_stockAnimMappings;
QHash<Animator::Animation, QString> AnimatorPrivate::s_loadableAnimMappings;

void AnimatorPrivate::mapAnimation(Animator::Animation from, Animator::Animation to)
{
    if (from == to) {
        return;
    }

    s_loadableAnimMappings.remove(from);
    s_stockAnimMappings.insert(from, to);
}

void AnimatorPrivate::mapAnimation(Animator::Animation from, const QString &to)
{
    s_stockAnimMappings.remove(from);
    s_loadableAnimMappings.insert(from, to);
}

Plasma::Animation* Animator::create(Animator::Animation type, QObject *parent)
{
    Plasma::Animation *result = 0;

    if (AnimatorPrivate::s_stockAnimMappings.contains(type)) {
        return create(AnimatorPrivate::s_stockAnimMappings.value(type));
    } else if (AnimatorPrivate::s_loadableAnimMappings.contains(type)) {
        const QString anim = AnimatorPrivate::s_loadableAnimMappings.value(type);
        if (AnimationScriptEngine::isAnimationRegistered(anim)) {
            result = new JavascriptAnimation(anim, parent);
        }

        return result;
    }

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

Plasma::Animation *Animator::create(QString &anim, QObject *parent)
{
    if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
        const QString path = Theme::defaultTheme()->animationPath(anim);
        if (path.isEmpty()) {
            kError() << "failed to find script file for animation" << anim;
            return 0;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            kError() << "failed to open script file" << path;
            return 0;
        }

        QTextStream buffer(&file);
        QString tmp(buffer.readAll());

        QScriptEngine *engine = AnimationScriptEngine::globalEngine();
        QScriptValue def(engine->evaluate(tmp, path));
        if (engine->hasUncaughtException()) {
            const QScriptValue error = engine->uncaughtException();
            QString file = error.property("fileName").toString();
            const QString failureMsg = QString("Error in %1 on line %2.\n%3")
                                              .arg(file)
                                              .arg(error.property("lineNumber").toString())
                                              .arg(error.toString());
            kError() << failureMsg;
            return 0;
        }

        if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
            kError() << "successfully loaded script file" << path << ", but did not get animation object for" << anim;
        }
    }

    return new Plasma::JavascriptAnimation(anim, parent);
}

} // namespace Plasma

#include <animator.moc>

