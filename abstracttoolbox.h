/*
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_ABSTRACTTOOLBOX_H
#define PLASMA_ABSTRACTTOOLBOX_H

#include <QGraphicsWidget>
#include <QGraphicsItem>

#include "plasma/plasma_export.h"

class QAction;

class KConfigGroup;

namespace Plasma
{

class AbstractToolBoxPrivate;
class Containment;

class PLASMA_EXPORT AbstractToolBox : public QGraphicsWidget
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(bool showing READ isShowing WRITE setShowing)

public:
    enum ToolType {
        AddTool = 0,
        ConfigureTool = 100,
        ControlTool = 200,
        MiscTool = 300,
        DestructiveTool = 400,
        UserToolType = DestructiveTool + 1000
    };
    Q_ENUMS(ToolType)

    explicit AbstractToolBox(Containment *parent);
    explicit AbstractToolBox(QObject *parent = 0,
                    const QVariantList &args = QVariantList());
    ~AbstractToolBox();

    /**
     * Create a new AbstractToolBox, loading the proper plugin
     * @param name the plugin name
     * @param args the plugin arguments
     * @param containment the containment parent of the toolbox
     * @since 4.6
     */
    static AbstractToolBox *load(const QString &name, const QVariantList &args=QVariantList(), Plasma::Containment *containment=0);

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate the tool with
     */
    virtual void addTool(QAction *action) = 0;

    /**
     * remove the tool associated with this action
     */
    virtual void removeTool(QAction *action) = 0;
    virtual bool isShowing() const = 0;
    virtual void setShowing(const bool show) = 0;

public Q_SLOTS:
    //FIXME for KDE5: those should become virtuals
    /**
     * Restore the ToolBox settings
     * It has to be reimplemented in toolboxes that need it
     * @since 4.6
     */
    void restore(const KConfigGroup &group);

    /**
     * Save the ToolBox settings
     * It has to be reimplemented in toolboxes that need it
     * @since 4.6
     */
    void save(const KConfigGroup &group);

    /**
     * Inform the ToolBox it has to reposition itlself
     * It has to be reimplemented in toolboxes that need it
     * @since 4.6
     */
    void reposition();

Q_SIGNALS:
    /**
     * Toolbox opened or closed
     */
    void toggled();

    /**
     * Toolbox opened or closed
     * @param open tells if the toolbox opened or closed
     */
    void visibilityChanged(bool open);

protected:
    Containment *containment() const;

private:
    AbstractToolBoxPrivate * const d;

};

} // Plasma namespace

/**
 * Register an applet when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_TOOLBOX(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_toolbox_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard

