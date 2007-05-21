/*
 *   Copyright (C) 2007 by Rafael Fernández López <ereslibre@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

// KDE includes
#include <kdemacros.h>

// Plasma includes
#include "datavisualization.h"


namespace Plasma
{


/**
 * This class emulates a QRadioButton.
 *
 * You will be able to add radio buttons as childrens of a QGraphicsItem, so
 * only one radio button will be checked at a time. If you don't add them as
 * siblings (adding all them as childrens of the same QGraphicsItem), then
 * you can add them directly to a scene, where they will be siblings too. This
 * way you can have groups of radio buttons.
 *
 * @note Please before working with radio buttons, add them to a scene or as
 *       childrens of another QGraphicsItem.
 *
 * @author Rafael Fernández López
 */


class KDE_EXPORT RadioButton : public DataVisualization
                             , public QGraphicsItem
{
    Q_OBJECT

public:
    RadioButton(QGraphicsItem *parent = 0);
    virtual ~RadioButton();

    // QGraphicsItem overriden virtual methods
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    // Getters
    bool isChecked() const;
    const QString &text() const;

    // Setters
    void setChecked(bool checked);
    void setText(const QString &text);

public Q_SLOTS:
    // DataVisualization overridden virtual slots
    void updated(const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void clicked();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    class Private;
    Private *const d;
};


} // Plasma namespace

#endif // RADIOBUTTON_H
