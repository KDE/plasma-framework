/*

    Authentication method specific conversation plugin for KDE's greeter widgets

    Copyright (C) 2003 Oswald Buddenhagen <ossi@kde.org>
    Copyright (C) 2003 Fabian Kaiser <xfk@softpro.de>

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
*/

#ifndef KGREETERPLUGIN_H
#define KGREETERPLUGIN_H

#include <QtCore/QVariant>
#include <QtGui/QMessageBox>
#include <kdemacros.h>

class QWidget;

class KGreeterPluginHandler {
public:
    virtual ~KGreeterPluginHandler() {}
	/* keep in sync with V_IS_* */
    enum { IsSecret = 1, IsUser = 2, IsPassword = 4, IsOldPassword = 8,
           IsNewPassword = 16 };
    /**
     * Reply to textPrompt().
     * @param text text to return to core; null to abort auth cycle
     * @param tag zero or one of Is*
     */
    virtual void gplugReturnText( const char *text, int tag ) = 0;
    /**
     * Reply to binaryPrompt().
     * @param data data in pam_client format to return to the core;
     *  null to abort auth cycle
     */
    virtual void gplugReturnBinary( const char *data ) = 0;
    /**
     * Tell the greeter who is logging in.
     * Call this preferably before gplugStart, as otherwise the .dmrc
     * load will be delayed. Don't call at all if your plugin doesn't
     * have the Local flag set. Call only for internally generated
     * user changes.
     * @param user the user logging in
     */
    virtual void gplugSetUser( const QString &user ) = 0;
    /**
     * Start processing.
     */
    virtual void gplugStart() = 0;
    /**
     * This should be called each time the talker changes in any way from the
     * pristine state after an authentication cycle starts, so the greeter
     * knows it must reset the fields after some time of inactivity.
     */
    virtual void gplugChanged() = 0;
    /**
     * Plugins that expect user input from a different device than the mouse or
     * keyboard must call this when user activity is detected to prevent the
     * greeter from resetting/going away. Events should be compressed to no
     * more than ten per second; one every five seconds is actually enough.
     * Events should be actual changes to the input fields, not random motion.
     */
    virtual void gplugActivity() = 0;
    /**
     * Show a message box on behalf of the talker.
     * @param type message severity
     * @param text message text
     */
    virtual void gplugMsgBox( QMessageBox::Icon type, const QString &text ) = 0;
    /**
     * Determine if the named widget is welcomed.
     * @param id the widget name
     */
    virtual bool gplugHasNode( const QString &id ) = 0;
};

/**
 * Abstract base class for conversation plugins ("talkers") to be used with
 * KDM, kdesktop_lock, etc.
 * The authentication method used by a particular instance of a plugin
 * may be configurable, but the instance must handle exactly one method,
 * i.e., info->method must be determined at the latest at init() time.
 */
class KGreeterPlugin {
public:
    KGreeterPlugin( KGreeterPluginHandler *h ) : handler( h ) {}
    virtual ~KGreeterPlugin() {}

    /**
     * Variations of the talker:
     * - Authenticate: authentication
     * - AuthChAuthTok: authentication and password change
     * - ChAuthTok: password change
     */
    enum Function { Authenticate, AuthChAuthTok, ChAuthTok };

    /**
     * Contexts the talker can be used in:
     * - Login: kdm login dialog
     * - Shutdown: kdm shutdown dialog
     * - Unlock: kdm unlock dialog (TODO)
     * - ChangeTok: kdm password change dialog (TODO)
     * - ExUnlock: kdesktop_lock unlock dialog
     * - ExChangeTok: kdepasswd password change dialog (TODO)
     *
     * The Ex* contexts exist within a running session; the talker must know
     * how to obtain the currently logged in user (+ domain/realm, etc.)
     * itself (i.e., fixedEntity will be null). The non-Ex variants will have
     * a fixedEntity passed in.
     */
    enum Context { Login, Shutdown, Unlock, ChangeTok,
                   ExUnlock, ExChangeTok };

    /**
     * Provide the talker with a list of selectable users. This can be used
     * for autocompletion, etc.
     * Will be called only when not running.
     * @param users the users to load.
     */
    virtual void loadUsers( const QStringList &users ) = 0;

    /**
     * Preload the talker with an (opaque to the greeter) entity.
     * Will be called only when not running.
     * @param entity the entity to preload the talker with. That
     *  will usually be something like "user" or "user@domain".
     * @param field the sub-widget (probably line edit) to put the cursor into.
     *  If -1, preselect the user for timed login. This means pre-filling
     *  the password field with anything, disabling it, and placing the
     *  cursor in the user name field.
     */
    virtual void presetEntity( const QString &entity, int field ) = 0;

    /**
     * Obtain the actually logged in entity.
     * Will be called only after succeeded() was called.
     */
    virtual QString getEntity() const = 0;

    /**
     * "Push" a user into the talker. That can be a click into the user list
     * or successful authentication without the talker calling gplugSetUser.
     * Will be called only when running.
     * @param user the user to set. Note that this is a UNIX login, not a
     *  canonical entity
     */
    virtual void setUser( const QString &user ) = 0;

    /**
     * En-/disable any widgets contained in the talker.
     * Will be called only when not running.
     * @param on the state to set
     */
    virtual void setEnabled( bool on ) = 0;

    /**
     * Called when a message from the authentication backend arrives.
     * @param message the message received from the backend
     * @param error if true, @p message is an error message, otherwise it's
     *  an informational message
     * @return true means that the talker already handled the message, false
     *  that the greeter should display it in a message box
     *
     * FIXME: Filtering a message usually means that the backend issued a
     * prompt and obtains the authentication data itself. However, in that
     * state the backend is unresponsive, e.g., no shutdown is possible.
     * The frontend could send the backend a signal, but the "escape path"
     * within the backend is unclear (PAM won't like simply longjmp()ing
     * out of it).
     */
    virtual bool textMessage( const char *message, bool error ) = 0;

    /**
     * Prompt the user for data. Reply by calling handler->gplugReturnText().
     * @param propmt the prompt to display. It may be null, in which case
     *  "Username"/"Password" should be shown and the replies should be tagged
     *  with the respective Is* flag.
     * @param echo if true, a normal input widget can be used, otherwise one that
     *  visually obscures the user's input.
     * @param nonBlocking if true, report whatever is already available,
     *  otherwise wait for user input.
     */
    virtual void textPrompt( const char *prompt, bool echo, bool nonBlocking ) = 0;

    /**
     * Request binary authentication data from the talker. Reply by calling
     * handler->gplugReturnBinary().
     * @param prompt prompt in pam_client format
     * @param nonBlocking if true, report whatever is already available,
     *  otherwise wait for user input.
     * @return always true for now
     *
     * TODO:
     * @return if true, the prompt was handled by the talker, otherwise the
     *  handler has to use libpam_client to obtain the authentication data.
     *  In that state the talker still can abort the data fetch by
     *  gplugReturn()ing a null array. When the data was obtained, another
     *  binaryPrompt with a null prompt will be issued.
     */
    virtual bool binaryPrompt( const char *prompt, bool nonBlocking ) = 0;

    /**
     * This can either
     *  - Start a processing cycle. Will be called only when not running.
     *  - Restart authTok cycle - will be called while running and implies
     *    revive(). PAM is a bit too clever, so we need this.
     * In any case the talker is running afterwards.
     */
    virtual void start() = 0;

    /**
     * Request to suspend the auth. Make sure that a second talker of any
     * type will be able to operate while this one is suspended (no busy
     * device nodes, etc.).
     * Will be called only if running within Login context. (Actually it
     * won't be called at all, but be prepared.)
     */
    virtual void suspend() = 0;

    /**
     * Request to resume the auth from the point it was suspended at.
     * Will be called only when suspended.
     */
    virtual void resume() = 0;

    /**
     * The "login" button was pressed in the greeter.
     * This might call gplugReturn* or gplugStart.
     * Will be called only when running.
     */
    virtual void next() = 0;

    /**
     * Abort auth cycle. Note that this should _not_ clear out already
     * entered auth tokens if they are still on the screen.
     * Will be called only when running and stops it.
     */
    virtual void abort() = 0;

    /**
     * Indicate successful end of the current phase.
     * This is more or less a request to disable editable widgets
     * responsible for the that phase.
     * There will be no further attempt to enter that phase until the
     * widget is destroyed.
     * Will be called only when running and stops it.
     */
    virtual void succeeded() = 0;

    /**
     * Indicate unsuccessful end of the current phase.
     * This is mostly a request to disable all editable widgets.
     * The widget will be treated as dead until revive() is called.
     * Will be called only when running and stops it.
     */
    virtual void failed() = 0;

    /**
     * Prepare retrying the previously failed phase.
     * This is mostly a request to re-enable all editable widgets failed()
     * disabled previously, clear the probably incorrect authentication tokens
     * and to set the input focus appropriately.
     * Will be called only after failed() (possibly with clear() in between),
     * or after presetEntity() with field -1.
     */
    virtual void revive() = 0;

    /**
     * Clear any edit widgets, particularly anything set by setUser.
     * Will be called only when not running.
     */
    virtual void clear() = 0;

    typedef QList<QWidget *> WidgetList;

    /**
     * Obtain the QWidget to actually handle the conversation.
     */
    const WidgetList &getWidgets() const { return widgetList; }

protected:
    KGreeterPluginHandler *handler;
    WidgetList widgetList;
};

struct KDE_EXPORT KGreeterPluginInfo {
    /**
     * Human readable name of this plugin (should be a little more
     * informative than just the libary name). Must be I18N_NOOP()ed.
     */
    const char *name;

    /**
     * The authentication method to use - the meaning is up to the backend,
     * but will usually be related to the PAM service.
     */
    const char *method;

    /**
     * Capabilities.
     */
    enum {
        /**
         * All users exist on the local system permanently (will be listed
         * by getpwent()); an entity corresponds to a UNIX user.
         */
        Local = 1,
        /**
         * The entities consist of multiple fields.
         * PluginOptions/<plugin>.FocusField is used instead of FocusPasswd.
         */
        Fielded = 2,
        /**
         * An entity can be preset, the talker has a widget where a user can
         * be selected explicitly. If the method is "classic", timed login
         * is possible, too.
         * This also means that setUser/gplugSetUser can be used and a
         * userlist can be shown at all - provided Local is set as well.
         */
        Presettable = 4
    };

    /*
     * Capability flags.
     */
    int flags;

    /**
     * Call after loading the plugin.
     *
     * @param method if non-empty and the plugin is unable to handle that
     *  method, return false. If the plugin has a constant method defined
     *  above, it can ignore this parameter.
     * @param getConf can be used to obtain configuration items from the
     *  greeter; you have to pass it the @p ctx pointer.
     *   The only predefined key (in KDM) is "EchoMode", which is an int
     *   (in fact, QLineEdit::EchoModes).
     *   Other keys are obtained from the PluginOptions option; see kdmrc
     *   for details.
     *   If the key is unknown, dflt is returned.
     * @param ctx context pointer for @p getConf
     * @return if false, unload the plugin again (don't call done() first)
     */
    bool (*init)( const QString &method,
                  QVariant (*getConf)( void *ctx, const char *key,
                                       const QVariant &dflt ),
                  void *ctx );

    /**
     * Call before unloading the plugin.
     * This pointer can be null.
     */
    void (*done)( void );

    /**
     * Factory method to create an instance of the plugin.
     * Note that multiple instances can exist at one time, but only
     * one of them is active at any moment (the others would be suspended
     * or not running at all).
     * @param handler the object offering the necessary callbacks
     * @param parent parent widget
     * @param predecessor the focus widget before the conversation widget
     * @param fixedEntity see below
     * @param func see below
     * @param ctx see below
     * @return an instance of this conversation plugin
     *
     * Valid combinations of Function and Context:
     * - Authenticate:Login - init
     * - Authenticate:Shutdown - init, for now "root" is passed as fixedEntitiy
     *  and it is not supposed to be displayed. Plugins with Local not set
     *  might have to conjure something up to make getEntity() return a
     *  canonical entitiy. FIXME: don't restrict shutdown to root.
     * - AuthChAuthTok:Login, AuthChAuthTok:Shutdown - cont/cont,
     *  only relevant for classic method (as it is relevant only for password-
     *  less logins, which always use classic). The login should not be shown -
     *  it is known to the user already; the backend won't ask for it, either.
     * - ChAuthTok:Login & ChAuthTok:Shutdown - cont
     * - Authenticate:Unlock & Authenticate:ExUnlock - init,
     *   AuthChAuthTok:ChangeTok & AuthChAuthTok:ExChangeTok - init/cont,
     *  display fixedEntity as labels. The backend does not ask for the UNIX
     *  login, as it already knows it - but it will ask for all components of
     *  the entity if it is no UNIX login.
     *
     * "init" means that the plugin is supposed to call gplugStart, "cont"
     * that the backend is already in a cycle of the method the plugin was
     * initialized with (it does not hurt if gplugStart is still called).
     */
    KGreeterPlugin *(*create)( KGreeterPluginHandler *handler,
                               QWidget *parent,
                               const QString &fixedEntity,
                               KGreeterPlugin::Function func,
                               KGreeterPlugin::Context ctx );
};

#endif
