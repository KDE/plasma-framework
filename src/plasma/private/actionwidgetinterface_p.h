/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ACTIONWIDGETINTERFACE_P_H
#define ACTIONWIDGETINTERFACE_P_H

#include <QAction>

#include "private/themedwidgetinterface_p.h"

namespace Plasma
{
template<class T>
class ActionWidgetInterface : public ThemedWidgetInterface<T>
{
public:
    ActionWidgetInterface(T *parent)
        : ThemedWidgetInterface<T>(parent)
        , action(nullptr)
    {
    }

    virtual ~ActionWidgetInterface()
    {
        setAction(nullptr);
    }

    virtual void changed()
    {
    }

    void clearAction()
    {
        action = nullptr;
        syncToAction();
        changed();
    }

    void syncToAction()
    {
        if (!action) {
            this->q->setIcon(QIcon());
            this->q->setText(QString());
            this->q->setEnabled(false);
            return;
        }
        // we don't get told *what* changed, just that something changed
        // so we update everything we care about
        this->q->setIcon(action->icon());
        this->q->setText(action->iconText());
        this->q->setEnabled(action->isEnabled());
        this->q->setVisible(action->isVisible());

        if (!this->q->toolTip().isEmpty()) {
            this->q->setToolTip(action->text());
        }

        changed();
    }

    void setAction(QAction *a)
    {
        if (action) {
            QObject::disconnect(action, 0, this->q, 0);
            QObject::disconnect(this->q, 0, action, 0);
        }

        action = a;

        if (action) {
            QObject::connect(action, SIGNAL(changed()), this->q, SLOT(syncToAction()));
            QObject::connect(action, SIGNAL(destroyed(QObject *)), this->q, SLOT(clearAction()));
            QObject::connect(this->q, SIGNAL(clicked()), action, SLOT(trigger()));
            syncToAction();
        }
    }

    QAction *action = nullptr;
};

} // namespace Plasma
#endif
