/*
 * Copyright 2010 by Marco MArtin <mart@kde.org>
 * Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>
 * Copyright 2007 by Alexis MÃ©nard <darktears31@gmail.com>
 * Copyright 2007 Sebastian Kuegler <sebas@kde.org>
 * Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef LIBS_PLASMA_DIALOG_P_H
#define LIBS_PLASMA_DIALOG_P_H

#include "plasma/dialog.h"

namespace Plasma {

class Applet;
class FrameSvg;

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
            : q(dialog),
              background(0),
              view(0),
              resizeCorners(Dialog::NoCorner),
              resizeStartCorner(Dialog::NoCorner),
              moveTimer(0),
              aspectRatioMode(Plasma::IgnoreAspectRatio),
              resizeChecksWithBorderCheck(false)
    {
    }

    ~DialogPrivate()
    {
    }

    void scheduleBorderCheck(bool triggeredByResize = false);
    void themeChanged();
    void updateMask();
    void checkBorders();
    void checkBorders(bool updateMaskIfNeeded);
    void updateResizeCorners();
    int calculateWidthForHeightAndRatio(int height, qreal ratio);
    void delayedAdjustSize();

    Plasma::Dialog *q;

    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::FrameSvg *background;
    QGraphicsView *view;
    QWeakPointer<QGraphicsWidget> graphicsWidgetPtr;
    QWeakPointer<Applet> appletPtr;
    Dialog::ResizeCorners resizeCorners;
    QMap<Dialog::ResizeCorner, QRect> resizeAreas;
    int resizeStartCorner;
    QTimer *moveTimer;
    QTimer *adjustViewTimer;
    QTimer *adjustSizeTimer;
    QSize oldGraphicsWidgetMinimumSize;
    QSize oldGraphicsWidgetMaximumSize;
    Plasma::AspectRatioMode aspectRatioMode;
    bool resizeChecksWithBorderCheck;
};

}

#endif
