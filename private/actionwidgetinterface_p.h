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

#include "private/themedwidgetinterface_p.h"

namespace Plasma
{

template <class T>
class ActionWidgetInterface : public ThemedWidgetInterface<T>
{
public:
    QAction *action;

    ActionWidgetInterface(T *parent)
        : ThemedWidgetInterface<T>(parent),
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
            ThemedWidgetInterface<T>::q->setIcon(QIcon());
            ThemedWidgetInterface<T>::q->setText(QString());
            ThemedWidgetInterface<T>::q->setEnabled(false);
            return;
        }
        //we don't get told *what* changed, just that something changed
        //so we update everything we care about
        ThemedWidgetInterface<T>::q->setIcon(action->icon());
        ThemedWidgetInterface<T>::q->setText(action->iconText());
        ThemedWidgetInterface<T>::q->setEnabled(action->isEnabled());
        ThemedWidgetInterface<T>::q->setVisible(action->isVisible());

        if (!ThemedWidgetInterface<T>::q->toolTip().isEmpty()) {
            ThemedWidgetInterface<T>::q->setToolTip(action->text());
        }

        changed();
    }

    void setAction(QAction *a)
    {
        if (action) {
            QObject::disconnect(action, 0, ThemedWidgetInterface<T>::q, 0);
            QObject::disconnect(ThemedWidgetInterface<T>::q, 0, action, 0);
        }

        action = a;

        if (action) {
            QObject::connect(action, SIGNAL(changed()), ThemedWidgetInterface<T>::q, SLOT(syncToAction()));
            QObject::connect(action, SIGNAL(destroyed(QObject*)), ThemedWidgetInterface<T>::q, SLOT(clearAction()));
            QObject::connect(ThemedWidgetInterface<T>::q, SIGNAL(clicked()), action, SLOT(trigger()));
            syncToAction();
        }
    }
};

} // namespace Plasma
#endif

