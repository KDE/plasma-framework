/*****************************************************************
Copyright (c) 2000-2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>
Copyright (c) 2001-2005 Aaron Seigo <aseigo@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
******************************************************************/

#ifndef appletinfo_h_
#define appletinfo_h_

#include <QList>
#include <QVector>

#include <kdemacros.h>

namespace Plasma
{

class KDE_EXPORT AppletInfo
{
    public:
        typedef QList<AppletInfo> List;
        typedef QMap<QObject*, AppletInfo*> Dict;

        /**
         * AppletInfo constructor
         * Each AppletInfo describes a specific applet
         * @arg desktopFile the name of the desktop file describing this applet
         */
        AppletInfo(const QString& desktopFile = QString::null);

        /**
         * Destructor
         */
        virtual ~AppletInfo();

        /**
         * Copy constructor
         */
        AppletInfo(const AppletInfo& copy);

        /**
         * Returns the name of the applet, suitable for use in user interfaces.
         */
        QString name() const;

        /**
         * Returns a descriptive comment for the applet, suitable for use in
         * user interfaces.
         */
        QString comment() const;

        /**
         * Returns the type of the applet, allowing one to distinguish between
         * applets, buttons
         */
        QString icon() const;

        /**
         * Returns the name of the library containing the applet
         */
        QString library() const;

        /**
         * Returns the language this applet is written in. "native" means it is
         * a compiled plugin that can be loaded via dlopen. anything else is
         * taken to be the name of the language (and therefore the bindings)
         * required for this applet, e.g. "javascript"
         */
        QString languageBindings() const;

        /**
         * Returns the full path to the desktop file being used for this applet
         */
        QString desktopFilePath() const;

        /**
         * Returns the name of the desktop file that describes this applet
         */
        QString desktopFile() const;

        /**
         * Returns a configuration file name that can be used for an instance
         * of this applet. If it is a unique applet, this will return the same
         * name each time, otherwise each time this is called the name may be
         * different.
         */
        QString generateConfigFileName() const;

        /**
         * Some applets only allow for one instance of the applet to be
         * instantiated at a time. This method returns true if this is the case
         * for this applet.
         */
        bool unique() const;

        /**
         * Not all applets are meant to be visible to the user, though they may
         * be available for use by the application internally. This method
         * returns true if this is the case for this applet.
         */
        bool hidden() const;

        /**
         * Assignment operator
         */
        AppletInfo &operator=(const AppletInfo& rhs);

        /**
         * Less than operator, for sorting by name in lists
         */
        bool operator<(const AppletInfo& rhs) const;

        /**
         * Greater than operator, for sorting by name in lists
         */
        bool operator>(const AppletInfo& rhs) const;

        /**
         * Less than or equals to operator, for sorting by name in lists
         */
        bool operator<=(const AppletInfo& rhs) const;

        /**
         * Inequality operator. Compares the library used by each.
         */
        bool operator!=(const AppletInfo& rhs) const;

        /**
         * Less than operator. Compares the library used by each.
         */
        bool operator==(const AppletInfo& rhs) const;

    private:
        void setName(const QString& name);
        void setComment(const QString& comment);
        void setIcon(const QString& icon);
        void setLibrary(const QString& lib);
        void setLanguageBindings(const QString& language);
        void setUnique(bool u);

        class Private;
        Private *d;
};

} // Plasma namespace

#endif
