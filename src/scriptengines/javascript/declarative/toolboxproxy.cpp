/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>
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

#include "toolboxproxy.h"
#include "../plasmoid/appletinterface.h"

#include <QAction>
#include <KIcon>
#include <KDebug>

#include <Plasma/Corona>

class ToolBoxProxyPrivate {
public:
    bool showing;
    Plasma::Containment *containment;
    QList<QAction*> actions;
    AppletInterface* appletInterface;
    QAction* addPanelAction;
    QAction* addWidgetsAction;
    QAction* configureAction;
};

ToolBoxProxy::ToolBoxProxy(Plasma::Containment *parent, AppletInterface *appletInterface)
    : AbstractToolBox(parent)
{
    d = new ToolBoxProxyPrivate;
    d->containment = parent;
    d->appletInterface = appletInterface;
    init();
}

ToolBoxProxy::ToolBoxProxy(QObject *parent, const QVariantList &args)
    : AbstractToolBox(parent, args)
{
    d = new ToolBoxProxyPrivate;
    d->containment = qobject_cast<Plasma::Containment *>(parent);
    d->appletInterface = 0;
    init();
}

ToolBoxProxy::~ToolBoxProxy()
{
    delete d;
}

void ToolBoxProxy::init()
{
    d->showing = false;
    d->addPanelAction = 0;
    d->addWidgetsAction = 0;
    d->configureAction = 0;

    if (d->containment) {
        connect(d->containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
                this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
        connect(this, SIGNAL(configureRequested(Plasma::Containment*)),
                d->containment, SIGNAL(configureRequested(Plasma::Containment*)));
        connect(this, SIGNAL(showAddWidgetsInterface(const QPointF&)),
                d->containment, SIGNAL(showAddWidgetsInterface(const QPointF&)));
    }
    loadActions();
}

void ToolBoxProxy::loadActions()
{
    d->actions.clear();
    if (d->containment) {
        if (!d->configureAction) {
            d->configureAction = new QAction(this);
            d->configureAction->setText(i18n("%1 Settings", d->containment->name()));
            d->configureAction->setIcon(KIcon("configure"));
            d->configureAction->setObjectName("configure");
            connect(d->configureAction, SIGNAL(triggered()), this, SLOT(configureRequested()));
        }
        addTool(d->configureAction);

        if (d->appletInterface) {
            foreach (QAction *action, d->appletInterface->contextualActions()) {
                addTool(action);
            }
        }
        foreach (QAction *action, d->containment->actions()) {
            addTool(action);
        }
        foreach (QAction *action, d->containment->corona()->actions()) {
            addTool(action);
        }
        if (!d->addWidgetsAction) {
            d->addWidgetsAction = new QAction(this);
            d->addWidgetsAction->setObjectName("add widgets");
            d->addWidgetsAction->setText(i18n("Add Widgets"));
            d->addWidgetsAction->setIcon(KIcon("list-add"));
            connect(d->addWidgetsAction, SIGNAL(triggered()), this, SLOT(addWidgetsRequested()));
        }
        if (d->appletInterface && !d->appletInterface->immutable()) {
            addTool(d->addWidgetsAction);
        }
    }
    emit actionsChanged();
}

QDeclarativeListProperty<QAction> ToolBoxProxy::actions()
{
    return QDeclarativeListProperty<QAction>(this, d->actions);
}

void ToolBoxProxy::addTool(QAction *action)
{
    if (!action || d->actions.contains(action)) {
        return;
    }
    if (d->appletInterface && d->appletInterface->immutable() && action->objectName() == "add panel") {
        d->addPanelAction = action;
        return;
    }
    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)), Qt::UniqueConnection);
    d->actions.append(action);
}

void ToolBoxProxy::removeTool(QAction *action)
{
    disconnect(action, 0, this, 0);
    d->actions.removeAll(action);
    emit actionsChanged();
}

void ToolBoxProxy::actionDestroyed(QObject *object)
{
    d->actions.removeAll(static_cast<QAction*>(object));
}

void ToolBoxProxy::configureRequested()
{
    emit configureRequested(d->containment);
}

void ToolBoxProxy::addWidgetsRequested()
{
    emit showAddWidgetsInterface(QPointF(0, 0));
}

bool ToolBoxProxy::isShowing() const
{
    return d->showing;
}

void ToolBoxProxy::setShowing(const bool show)
{
    if (d->showing == show) {
        return;
    }
    d->showing = show;
}

void ToolBoxProxy::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    Q_UNUSED(immutability);
    loadActions();
}

#include "toolboxproxy.moc"
