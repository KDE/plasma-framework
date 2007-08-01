/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kde.org
 *                         and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QLayoutItem>

#include <plasma/dataengine.h>
#include <plasma/widgets/widget.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * Class that emulates a QPushButton inside Plasma
 *
 * @author Siraj Razick and Matias Valdenegro.
 *
 *
 */
class PLASMA_EXPORT PushButton : public QObject,
                                 public Plasma::Widget
{
    Q_OBJECT

    public:
        enum ButtonShape
        {
            Rectangle = 0,
            Round,
            Custom
        };

        enum ButtonState
        {
            None,
            Hover,
            Pressed,
            Released
        };

    public:

        /**
         * Constructor.
         */
        PushButton(Widget *parent = 0);

        /**
         * Virtual Destructor.
         */
        virtual ~PushButton();

        /**
         * Returns the text of this Button.
         */
        QString text() const;

        /**
         * Sets the text of this Button.
         */
        void setText(const QString &text);

        /**
         * Sets the icon of this Button.
         * @param path Path to the icon file. TODO : WTF is path?
         */
        void setIcon(const QString& path);

        /**
         * Paint function.
         */
        virtual void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        /**
         * Reimplemented from Plasma::Widget.
         */
        virtual QSizeF sizeHint() const ;

        /**
         * Reimplemented from Plasma::Widget.
         */
        virtual QSizeF minimumSize() const;

        /**
         * Reimplemented from Plasma::Widget.
         */
        virtual QSizeF maximumSize() const ;

        /**
         * Buttons prefer to expand in Horizontal direction.
         */
        virtual Qt::Orientations expandingDirections() const;

        /**
         * TODO: What does this function do?
         */
        virtual bool isEmpty() const;

    Q_SIGNALS:

        /**
         * Triggers whatever this button is clicked.
         */
        void clicked();

    public Q_SLOTS:
        void updated(const QString&, const DataEngine::Data &);

    protected:
        bool isDown();
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    private:
        class Private ;
        Private *  const d;

};

} // namespace Plasma

#endif
