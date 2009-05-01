/*
 *   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef PLASMA_PANELBACKGROUNDPROVIDER_H
#define PLASMA_PANELBACKGROUNDPROVIDER_H

#include <plasma/plasma_export.h>
#include <QtCore/QPoint>

class QPainter;
class QRegion;

namespace Plasma {

/**
 * Abstract class to provide additional panel backgrounds behind translucent panels.
 */
class PLASMA_EXPORT FrameBackgroundProvider
{
public:
    virtual ~FrameBackgroundProvider();
    /**
     * Returns an identity that can be used for caching the result of the background rendering.
     * @return The identity string
     */
    virtual QString identity() = 0;

    /**
     * Applies the background to the given target. The target must have correct alpha-values,
     * so the background can be painted under it. Also the clip-region must be already set correctly
     * to restrict the area where the background is painted.
     * @param target The target where the background should be painted
     * @param offset Additional offset for the rendering: The render-source is translated by this offset
     */
    virtual void apply(QPainter& target, QPoint offset = QPoint()) = 0;
};

}

#endif // PLASMA_PANELBACKGROUNDPROVIDER_H
