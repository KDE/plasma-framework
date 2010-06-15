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
    if (AnimatorPrivate::s_stockAnimMappings.contains(type)) {
        return create(AnimatorPrivate::s_stockAnimMappings.value(type));
    } else if (AnimatorPrivate::s_loadableAnimMappings.contains(type)) {
        const QString anim = AnimatorPrivate::s_loadableAnimMappings.value(type);
        return create(anim, parent);
    }

    Plasma::Animation *result = 0;

    switch (type) {
    case FadeAnimation:
        result = create("FadeAnimation", parent);
        if (!result) {
            result = new Plasma::FadeAnimation(parent);
        }
        break;

    case GrowAnimation:
        result = create("GrowAnimation", parent);
        if (!result) {
            result = new Plasma::GrowAnimation(parent);
        }
        break;

    case PulseAnimation:
        result = create("PulseAnimation", parent);
        if (!result) {
            result = new Plasma::PulseAnimation(parent);
        }
        break;

    case RotationAnimation:
        result = create("RotationAnimation", parent);
        if (!result) {
            result = new Plasma::RotationAnimation(parent);
        }
        break;

    case RotationStackedAnimation:
        result = create("RotationStackedAnimation", parent);
        if (!result) {
            result = new Plasma::RotationStackedAnimation(parent);
        }
        break;

    case SlideAnimation:
        result = create("SlideAnimation", parent);
        if (!result) {
            result = new Plasma::SlideAnimation(parent);
        }
        break;

    case GeometryAnimation:
        result = create("GeometryAnimation", parent);
        if (!result) {
            result = new Plasma::GeometryAnimation(parent);
        }
        break;

    case ZoomAnimation:
        result = create("ZoomAnimation", parent);
        if (!result) {
            result = new Plasma::ZoomAnimation(parent);
        }
        break;

    case PixmapTransitionAnimation:
        result = create("PixmapTransitionAnimation", parent);
        if (!result) {
            result = new Plasma::PixmapTransition(parent);
        }
        break;

    case WaterAnimation:
        result = create("WaterAnimation", parent);
        if (!result) {
            result = new Plasma::WaterAnimation(parent);
        }
        break;

    default:
        //kDebug() << "Unsupported animation type.";
        break;
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

Plasma::Animation *Animator::create(const QString &anim, QObject *parent)
{
    if (AnimationScriptEngine::animationFailedToLoad(anim)) {
        return 0;
    }

    if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
        const QString path = Theme::defaultTheme()->animationPath(anim);
        if (path.isEmpty()) {
            AnimationScriptEngine::addToLoadFailures(anim);
            //kError() << "************ failed to find script file for animation" << anim;
            return 0;
        }

        if (!AnimationScriptEngine::loadScript(path)) {
            AnimationScriptEngine::addToLoadFailures(anim);
            return 0;
        }

        if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
            //kError() << "successfully loaded script file" << path << ", but did not get animation object for" << anim;
            AnimationScriptEngine::addToLoadFailures(anim);
            return 0;
        }
    }

    return new Plasma::JavascriptAnimation(anim, parent);
}

} // namespace Plasma

#include <animator.moc>

