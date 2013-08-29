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

#ifndef VIEW_H
#define VIEW_H

#include <QtQuick/QQuickView>

#include <plasmaview/plasmaview_export.h>
#include "plasma/corona.h"
#include "plasma/containment.h"

class ViewPrivate;

class PLASMAVIEW_EXPORT View : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(int location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    /**
     * @param corona the corona of this view
     * @param parent the QWindow this View is parented to
     **/
    explicit View(Plasma::Corona *corona, QWindow *parent = 0);
    virtual ~View();

    /**
     * @return the corona of this view
     **/
    Plasma::Corona *corona() const;

    /**
     * @return the KConfigGroup of this view
     **/
    virtual KConfigGroup config() const;

    /**
     * sets the containment for this view
     * @param cont the containment of this view
     **/
    virtual void setContainment(Plasma::Containment *cont);

    /**
     * @return the containment of this View
     **/
    Plasma::Containment *containment() const;

    /**
     * @return the location of this View
     **/

    //FIXME: Plasma::Types::Location should be something qml can understand
    int location() const;

    /**
     * Sets the location of the View
     * @param location the location of the View
     **/
    void setLocation(int location);

    /**
     * @return the formfactor of the View
     **/
    Plasma::Types::FormFactor formFactor() const;

    /**
     * @return the screenGeometry of the View
     **/
    QRectF screenGeometry();

protected Q_SLOTS:
    /**
     * It will be called when the configuration is requested
     */
    virtual void showConfigurationInterface(Plasma::Applet *applet);

Q_SIGNALS:
    /**
     * emitted when the location is changed
     **/
    void locationChanged(Plasma::Types::Location location);

    /**
     * emitted when the formfactor is changed
     **/
    void formFactorChanged(Plasma::Types::FormFactor formFactor);

    /**
     * emitted when the containment is changed
     **/
    void containmentChanged();

    /**
     * emitted when the screenGeometry is changed
     **/
    void screenGeometryChanged();

private:
    ViewPrivate *const d;
    friend class ViewPrivate;
};

#endif // VIEW_H
