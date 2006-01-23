/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_APPLET_H
#define PLASMA_APPLET_H

#include <QWidget>

#include <kconfig.h>
#include <kservice.h>

#include "plasma.h"
#include "appletChain.h"

namespace Plasma
{

class KDE_EXPORT Applet : public QWidget
{
    Q_OBJECT

    public:
        typedef QList<Applet*> List;

        Applet(QWidget* parent,
               KService::Ptr appletDescription,
               const AppletChain::Ptr chain,
               int id);
        ~Applet();

        /**
        * Returns the KConfig object to access the applets configuration.
        *
        * For unique applets this config object will write to a config file
        * named \<appletname\>rc in the user's local %KDE directory.
        *
        * For normal applets this config object will write to an instance
        * specific config file nameed \<appletname\>\<instanceid\>rc
        * in the user's local %KDE directory.
        **/
        KSharedConfig::Ptr globalAppletConfig() const;
        KSharedConfig::Ptr appletConfig() const;

        const AppletChain::Ptr chain() const;
        void setChain(const AppletChain::Ptr);

        /*
         * called when any of the geometry constraints have been updated
         * this is always called prior to painting and should be used as an
         * opportunity to layout the widget, calculate sizings, etc.
         * @property constraint
         */
        virtual void constraintsUpdated();

    Q_SIGNALS:
        void requestFocus(bool focus);

    protected:

        QString globalName() const;
        QString instanceName() const;

        /**
        * Register widgets that can receive keyboard focus with this this method
        * This call results in an eventFilter being places on the widget.
        * @param widget the widget to watch for keyboard focus
        * @param watch whether to start watching the widget, or to stop doing so
        */
        void watchForFocus(QWidget* widget, bool watch = true);

        /**
        * Call this whenever focus is needed or not needed. You do not have to
        * call this method for widgets that have been registered with
        * watchForFocus
        * @see watchForFocus
        * @param focus whether to or not to request focus
        */
        void needsFocus(bool focus);


        bool eventFilter(QObject *o, QEvent *e);

    private:
        class Private;
        Private* d;
};

} // Plasma namespace

#endif // multiple inclusion guard
