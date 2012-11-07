/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.

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

#ifndef TOOLBOXPROXY_H
#define TOOLBOXPROXY_H

#include <Plasma/AbstractToolBox>
#include <Plasma/Plasma>

#include <QDeclarativeListProperty>

class QAction;

class ToolBoxProxyPrivate;
class AppletInterface;

class ToolBoxProxy : public Plasma::AbstractToolBox
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeListProperty<QAction> actions READ actions NOTIFY actionsChanged)

public:
    explicit ToolBoxProxy(Plasma::Containment *parent, AppletInterface *appletInterface);
    explicit ToolBoxProxy(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~ToolBoxProxy();

    bool isShowing() const; // satisfy badly named API
    void setShowing(const bool show);

    QDeclarativeListProperty<QAction> actions();

public Q_SLOTS:
    void configureRequested();
    void addWidgetsRequested();

Q_SIGNALS:
    void actionsChanged();
    void immutableChanged();

    void configureRequested(Plasma::Containment* containment);
    void showAddWidgetsInterface(const QPointF& pos);

private Q_SLOTS:
    void actionDestroyed(QObject *object);
    void immutabilityChanged(Plasma::ImmutabilityType immutability);

private:
    void init();
    void loadActions();
    /**
     * create a toolbox tool from the given action
     * @p action the action to associate hte tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);

    ToolBoxProxyPrivate* d;
};

#endif // TOOLBOXPROXY_H
