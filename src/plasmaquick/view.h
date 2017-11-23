/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMAQUICKVIEW_H
#define PLASMAQUICKVIEW_H

#include <QtQuick/QQuickView>

#include <plasmaquick/plasmaquick_export.h>
#include "plasma/corona.h"
#include "plasma/containment.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace PlasmaQuick
{

class ViewPrivate;

/**
 * Deprecated, use ContainmentView instead.
 */
class PLASMAQUICK_EXPORT View : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(Plasma::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(Plasma::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    /**
     * @param corona the corona of this view
     * @param parent the QWindow this View is parented to
     **/
    PLASMA_DEPRECATED explicit View(Plasma::Corona *corona, QWindow *parent = 0);
    PLASMA_DEPRECATED virtual ~View();

    /**
     * @return the corona of this view
     **/
    PLASMA_DEPRECATED Plasma::Corona *corona() const;

    /**
     * @return the KConfigGroup of this view
     **/
    PLASMA_DEPRECATED virtual KConfigGroup config() const;

    /**
     * sets the containment for this view
     * @param cont the containment of this view
     **/
    PLASMA_DEPRECATED void setContainment(Plasma::Containment *cont);

    /**
     * @return the containment of this View
     **/
    PLASMA_DEPRECATED Plasma::Containment *containment() const;

    /**
     * @return the location of this View
     **/
    PLASMA_DEPRECATED Plasma::Types::Location location() const;

    /**
     * Sets the location of the View
     * @param location the location of the View
     **/
    PLASMA_DEPRECATED void setLocation(Plasma::Types::Location location);

    /**
     * @return the formfactor of the View
     **/
    PLASMA_DEPRECATED Plasma::Types::FormFactor formFactor() const;

    /**
     * @return the screenGeometry of the View
     **/
    PLASMA_DEPRECATED QRectF screenGeometry();

protected Q_SLOTS:
    /**
     * It will be called when the configuration is requested
     */
    PLASMA_DEPRECATED virtual void showConfigurationInterface(Plasma::Applet *applet);

Q_SIGNALS:
    /**
     * emitted when the location is changed
     **/
    PLASMA_DEPRECATED void locationChanged(Plasma::Types::Location location);

    /**
     * emitted when the formfactor is changed
     **/
    PLASMA_DEPRECATED void formFactorChanged(Plasma::Types::FormFactor formFactor);

    /**
     * emitted when the containment is changed
     **/
    PLASMA_DEPRECATED void containmentChanged();

    /**
     * emitted when the screenGeometry is changed
     **/
    PLASMA_DEPRECATED void screenGeometryChanged();

private:
    ViewPrivate *const d;
    Q_PRIVATE_SLOT(d, void updateDestroyed(bool))
    friend class ViewPrivate;
};

}

#endif // View_H
