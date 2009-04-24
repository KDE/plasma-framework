/******************************************************************************
*   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                            *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef ACTIONWIDGETINTERFACE_P_H
#define ACTIONWIDGETINTERFACE_P_H

#include <QAction>

namespace Plasma
{

template <class T>
class ActionWidgetInterface
{
public:
    T *t;
    QAction *action;

    ActionWidgetInterface(T *publicClass)
        : t(publicClass),
          action(0)
    {
    }

    virtual ~ActionWidgetInterface()
    {
    }

    virtual void changed()
    {
    }

    void clearAction()
    {
        action = 0;
        syncToAction();
        changed();
    }

    void syncToAction()
    {
        if (!action) {
            t->setIcon(QIcon());
            t->setText(QString());
            t->setEnabled(false);
            return;
        }
        //we don't get told *what* changed, just that something changed
        //so we update everything we care about
        t->setIcon(action->icon());
        t->setText(action->iconText());
        t->setEnabled(action->isEnabled());
        t->setVisible(action->isVisible());

        if (!t->toolTip().isEmpty()) {
            t->setToolTip(action->text());
        }

        changed();
    }

    void setAction(QAction *a)
    {
        if (action) {
            QObject::disconnect(action, 0, t, 0);
            QObject::disconnect(t, 0, action, 0);
        }

        action = a;

        if (action) {
            QObject::connect(action, SIGNAL(changed()), t, SLOT(syncToAction()));
            QObject::connect(action, SIGNAL(destroyed(QObject*)), t, SLOT(clearAction()));
            QObject::connect(t, SIGNAL(clicked()), action, SLOT(trigger()));
            syncToAction();
        }
    }
};

} // namespace Plasma
#endif

