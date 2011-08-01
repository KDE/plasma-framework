/*
 *   Copyright 2011 Kevin Kofler <kevin.kofler@chello.at>
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

#ifndef PLASMA_COMPONENTINSTALLER_H
#define PLASMA_COMPONENTINSTALLER_H

class QString;
class QWidget;

namespace Plasma
{

class ComponentInstallerPrivate;

/**
 * @class ComponentInstaller plasma/private/componentinstaller_p.h
 *
 * @short This class provides a generic API for installation of components.
 *
 * @internal
 *
 * Plasma::ComponentInstaller allows searching for a missing data or script
 * engine by name, and allowing the user to install the missing service.
 * Currently, PackageKit is supported as the mechanism to install components,
 * but more mechanisms could be supported in the future through the same API.
 *
 * @since 4.8
 */
class ComponentInstaller
{
    public:
        /**
         * Singleton pattern accessor.
         */
        static ComponentInstaller *self();

        /**
         * Installs a missing component asynchronously.
         *
         * By default, this method will cache requested components and not
         * prompt again for the same engine in the same session. The force
         * parameter can be used to disable this mechanism, e.g. when the user
         * just installed a new widget written in a scripting language, and so
         * is likely to want the script engine installed after all.
         *
         * In the case of on-demand installation, this will unfortunately not
         * allow the call which triggered the missing component lookup to
         * succeed, but we cannot afford to block all of Plasma until the
         * mechanism is done installing the service.
         *
         * This function does nothing if PackageKit integration was disabled at
         * compile time.
         *
         * @param type the type of the component, should be "scriptengine" or
         *             "dataengine"
         * @param name the name of the component
         * @param parent a parent widget, used to set the wid for PackageKit
         * @param force whether to always prompt, even if recently prompted
         */
        void installMissingComponent(const QString &type, const QString &name,
                                     QWidget *parent = 0, bool force = false);

    private:
        /**
         * Default constructor. The singleton method self() is the
         * preferred access mechanism.
         */
        ComponentInstaller();
        ~ComponentInstaller();

        ComponentInstallerPrivate *const d;

        friend class ComponentInstallerSingleton;
};

} // namespace Plasma

#endif // multiple inclusion guard
