/*
 * Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*
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

#include "animator.h"
#include "private/animator_p.h"

#include <kdebug.h>

#include "animations/abstractanimation.h"
#include "animations/animation.h"
#include "animations/expand.h"
#include "animations/fade.h"
#include "animations/grow.h"
#include "animations/pulser.h"
#include "animations/rotation.h"
#include "animations/slide.h"

namespace Plasma
{

AbstractAnimation *Animator::create(Animation type, QObject *parent)
{
    AbstractAnimation *result = 0;

    switch (type) {

    case FadeAnimation:
        result = new Plasma::FadeAnimation;
        break;

    case GrowAnimation:
        result = new Plasma::GrowAnimation;
        break;

    case ExpandAnimation:
        result = new Plasma::ExpandAnimation;
        break;

    case PulseAnimation:
        result = new Plasma::PulseAnimation;
        break;

    case RotationAnimation:
        result = new Plasma::RotationAnimation;
        break;

    case RotationStackedAnimation:
        //TODO: implement stacked rotation
        //result = new Plasma::Plasma::RotationStackedAnimation;
        break;

    case SlideAnimation:
        result = new Plasma::SlideAnimation;
        break;

    default:
        kDebug() << "Unsupported animation type.";

    }

    if (result) {
        result->setParent(parent);
    }

    return result;
}

} // namespace Plasma

#include <animator.moc>

