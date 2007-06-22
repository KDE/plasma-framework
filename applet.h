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

#include <QtGui/QGraphicsItem>
#include <QtGui/QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>
#include <kgenericfactory.h>

#include <plasma/plasma.h>

namespace Plasma
{

class DataEngine;

/**
 * @short The base Applet (Plasmoid) class
 *
 *
 */
class PLASMA_EXPORT Applet : public QObject, public QGraphicsItem
{
    Q_OBJECT

    public:
        typedef QList<Applet*> List;
        typedef QHash<QString, Applet*> Dict;

        /**
         * @arg parent the QGraphicsItem this applet is parented to
         * @arg servideId the name of the .desktop file containing the
         *      information about the widget
         * @arg appletId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        Applet(QGraphicsItem* parent,
               const QString& serviceId,
               int appletId);

        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have two elements: the KService service ID for the desktop entry
         * and an applet ID which must be a base 10 number.
         *
         * @arg parent a QObject parent; you probably want to pass in 0
         * @arg args a list of strings containing two entries: the service id
         *      and the applet id
         */
        Applet(QObject* parent, const QStringList& args);

        ~Applet();

        /**
        * Returns the KConfigGroup to access the applets configuration.
        *
        * This config object will write to an instance
        * specific config file named \<appletname\>\<instanceid\>rc
        * in the Plasma appdata directory.
        **/
        KConfigGroup appletConfig() const;

        /**
         * Returns a KConfigGroup object to be shared by all applets of this
         * type.
         *
         * This config object will write to an applet-specific config object
         * named plasma_\<appletname\>rc in the local config directory.
         */
        KConfigGroup globalAppletConfig() const;

        /**
         * Ensures that the DataEngine named name is loaded and ready to be used
         *
         * @return returns true on success, false on failure
         */
        DataEngine* dataEngine(const QString& name);

        /**
         * called when any of the geometry constraints have been updated
         * this is always called prior to painting and should be used as an
         * opportunity to layout the widget, calculate sizings, etc.
         *
         * Do not call update() from this method; an update() will be triggered
         * at the appropriate time for the applet.
         *
         * @property constraint
         */
        virtual void constraintsUpdated();

        /**
         * Returns the current form factor the applet is being displayed in.
         * @see Plasma::FormFactor
         */
        FormFactor formFactor() const;

        /**
         * Returns the location of the scene which is displaying applet.
         * @see Plasma::Location
         */
        Location location() const;

        /**
         * Returns a maximum size hint based on the Corona's space availability
         * An applet may choose to violate this size hint, but should try and
         * respect it as much as possible.
         */
        QRectF maxSizeHint() const;

        /**
         * Returns a list of all known applets in a hash keyed by a unique
         * identifier for each applet
         *
         * @return list of applets
         **/
        static KPluginInfo::List knownApplets();

        /**
         * Reimplement this slot to show a configuration dialog and let the user
         * play with the plasmoid options. Called when the user selects the configure entry
         * from the contextual menu.
         */
        virtual void configureDialog(){} //default implementation is empty

        /**
         * Attempts to load an applet, returning a pointer to the applet if
         * successful. The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param applet unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @param args to send the applet extra arguments
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet* loadApplet(const QString &name, uint appletId = 0,
                                  const QStringList& args = QStringList());

        /**
         * Attempts to load an applet, returning a pointer to the applet if
         * successful. The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired applet
         * @param applet unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet* loadApplet(const KPluginInfo* info, uint appletId = 0,
                                  const QStringList& args = QStringList());

        /**
         * This method is called when the interface should be painted.
         * @see QGraphicsItem::paint
         **/
        virtual void paintInterface(QPainter *painter,
                                    const QStyleOptionGraphicsItem *option,
                                    QWidget *widget = 0);

        /**
         * Returns the user-visible name for the applet, as specified in the
         * .desktop file.
         * @return the user-visible name for the applet.
         **/
        QString name() const;

        /**
         * @return true if this applet is immutable
         **/
        bool immutable() const;

        /** 
         * Sets whether or not this applet is immutable or not
         *
         * @arg immutable true if this applet should not be changable
         **/
        void setImmutable(bool immutable);

        /**
         * @return returns whether or not the applet is using the standard
         *         background
         **/
        bool drawStandardBackground();

        /**
         * Sets whether or not the applet should automatically draw the standard 
         * background or not. Defaults to false
         **/
        void setDrawStandardBackground(bool drawBackground);

        /**
         * Reimplented from QGraphicsItem
         **/
        enum { Type = Plasma::AppletType };
        int type() const { return Type; }

    Q_SIGNALS:
        /**
         * Emit this signal when your applet needs to take (or lose) keyboard
         * focus. This ensures that autohiding elements stay unhidden and other
         * bits of bookkeeping are performed to ensure proper function.
         *
         * If you call watchForFocus on your applet, then this is handled for
         * the applet and it is not necessary to emit the signal directly.
         *
         * @param focus true if the applet is taking keyboard focus, false if
         *        it is giving it up
         **/
        void requestFocus( bool focus );

    protected:
        /**
         * Returns the name of the applet. This will be the same for all
         * instances of this applet.
         **/
        QString globalName() const;

        /**
         * Returns a name unique to the insane of this applet. Useful for
         * being able to refer directly to a particular applet. Combines the
         * global name with the applet id
         **/
        QString instanceName() const;

        /**
        * Register widgets that can receive keyboard focus with this method
        * This call results in an eventFilter being places on the widget.
        * @param widget the widget to watch for keyboard focus
        * @param watch whether to start watching the widget, or to stop doing so
        */
        void watchForFocus( QObject *widget, bool watch = true );

        /**
        * Call this whenever focus is needed or not needed. You do not have to
        * call this method for widgets that have been registered with
        * watchForFocus
        * @see watchForFocus
        * @param focus whether to or not to request focus
        */
        void needsFocus(bool focus);

        /**
         * @internal event filter; used for focus watching
         **/
        bool eventFilter( QObject *o, QEvent *e );

    private:
        /**
         * Reimplemented from QGraphicsItem
         **/
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        void init();

        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_APPLET(libname, classname) \
        K_EXPORT_COMPONENT_FACTORY(                \
                        plasma_applet_##libname,    \
                        KGenericFactory<classname>("plasma_applet_" #libname))

#endif // multiple inclusion guard

