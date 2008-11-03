/*

Conversation widget for kdm greeter

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kgreet_classic.h"
#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"

#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kuser.h>

#include <QRegExp>
#include <QLayout>
#include <QLabel>

static int echoMode;

class KDMPasswordEdit : public KLineEdit {
public:
	KDMPasswordEdit( QWidget *parent ) : KLineEdit( parent )
	{
		if (::echoMode == -1)
			setPasswordMode(true);
		else
			setEchoMode( ::echoMode ? Password : NoEcho );
		setContextMenuPolicy( Qt::NoContextMenu );
	}
};

KClassicGreeter::KClassicGreeter( KGreeterPluginHandler *_handler,
                                  QWidget *parent,
                                  const QString &_fixedEntity,
                                  Function _func, Context _ctx ) :
	QObject(),
	KGreeterPlugin( _handler ),
	fixedUser( _fixedEntity ),
	func( _func ),
	ctx( _ctx ),
	exp( -1 ),
	pExp( -1 ),
	running( false )
{
	QGridLayout *grid = 0;
	int line = 0;

	if (!_handler->gplugHasNode( "user-entry" ) ||
	    !_handler->gplugHasNode( "pw-entry" ))
	{
		parent = new QWidget( parent );
		parent->setObjectName( "talker" );
		widgetList << parent;
		grid = new QGridLayout( parent );
		grid->setMargin( 0 );
	}

	loginLabel = passwdLabel = passwd1Label = passwd2Label = 0;
	loginEdit = 0;
	passwdEdit = passwd1Edit = passwd2Edit = 0;
	if (ctx == ExUnlock || ctx == ExChangeTok)
		fixedUser = KUser().loginName();
	if (func != ChAuthTok) {
		if (fixedUser.isEmpty()) {
			loginEdit = new KLineEdit( parent );
			loginEdit->setContextMenuPolicy( Qt::NoContextMenu );
			connect( loginEdit, SIGNAL(editingFinished()), SLOT(slotLoginLostFocus()) );
			connect( loginEdit, SIGNAL(editingFinished()), SLOT(slotChanged()) );
			connect( loginEdit, SIGNAL(textChanged( const QString & )), SLOT(slotChanged()) );
			connect( loginEdit, SIGNAL(selectionChanged()), SLOT(slotChanged()) );
			if (!grid) {
				loginEdit->setObjectName( "user-entry" );
				widgetList << loginEdit;
			} else {
				loginLabel = new QLabel( i18n("&Username:"), parent );
				loginLabel->setBuddy( loginEdit );
				grid->addWidget( loginLabel, line, 0 );
				grid->addWidget( loginEdit, line++, 1 );
			}
		} else if (ctx != Login && ctx != Shutdown && grid) {
			loginLabel = new QLabel( i18n("Username:"), parent );
			grid->addWidget( loginLabel, line, 0 );
			grid->addWidget( new QLabel( fixedUser, parent ), line++, 1 );
		}
		passwdEdit = new KDMPasswordEdit( parent );
		connect( passwdEdit, SIGNAL(textChanged( const QString & )),
		         SLOT(slotChanged()) );
		connect( passwdEdit, SIGNAL(editingFinished()), SLOT(slotChanged()) );
		if (!grid) {
			passwdEdit->setObjectName( "pw-entry" );
			widgetList << passwdEdit;
		} else {
			passwdLabel = new QLabel( func == Authenticate ?
			                             i18n("&Password:") :
			                             i18n("Current &password:"),
			                          parent );
			passwdLabel->setBuddy( passwdEdit );
			grid->addWidget( passwdLabel, line, 0 );
			grid->addWidget( passwdEdit, line++, 1 );
		}
		if (loginEdit)
			loginEdit->setFocus();
		else
			passwdEdit->setFocus();
	}
	if (func != Authenticate) {
		passwd1Edit = new KDMPasswordEdit( parent );
		passwd1Label = new QLabel( i18n("&New password:"), parent );
		passwd1Label->setBuddy( passwd1Edit );
		passwd2Edit = new KDMPasswordEdit( parent );
		passwd2Label = new QLabel( i18n("Con&firm password:"), parent );
		passwd2Label->setBuddy( passwd2Edit );
		if (grid) {
			grid->addWidget( passwd1Label, line, 0 );
			grid->addWidget( passwd1Edit, line++, 1 );
			grid->addWidget( passwd2Label, line, 0 );
			grid->addWidget( passwd2Edit, line, 1 );
		}
		if (!passwdEdit)
			passwd1Edit->setFocus();
	}
}

// virtual
KClassicGreeter::~KClassicGreeter()
{
	abort();
	qDeleteAll( widgetList );
}

void // virtual
KClassicGreeter::loadUsers( const QStringList &users )
{
	KCompletion *userNamesCompletion = new KCompletion;
	userNamesCompletion->setItems( users );
	loginEdit->setCompletionObject( userNamesCompletion );
	loginEdit->setAutoDeleteCompletionObject( true );
	loginEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
}

void // virtual
KClassicGreeter::presetEntity( const QString &entity, int field )
{
	loginEdit->setText( entity );
	if (field == 1)
		passwdEdit->setFocus();
	else {
		loginEdit->setFocus();
		loginEdit->selectAll();
		if (field == -1) {
			passwdEdit->setText( "     " );
			passwdEdit->setEnabled( false );
			authTok = false;
		}
	}
	curUser = entity;
}

QString // virtual
KClassicGreeter::getEntity() const
{
	return fixedUser.isEmpty() ? loginEdit->text() : fixedUser;
}

void // virtual
KClassicGreeter::setUser( const QString &user )
{
	// assert( fixedUser.isEmpty() );
	curUser = user;
	loginEdit->setText( user );
	passwdEdit->setFocus();
	passwdEdit->selectAll();
}

void // virtual
KClassicGreeter::setEnabled( bool enable )
{
	// assert( !passwd1Label );
	// assert( func == Authenticate && ctx == Shutdown );
//	if (loginLabel)
//		loginLabel->setEnabled( enable );
	passwdLabel->setEnabled( enable );
	setActive( enable );
	if (enable)
		passwdEdit->setFocus();
}

void // private
KClassicGreeter::returnData()
{
	switch (exp) {
	case 0:
		handler->gplugReturnText( (loginEdit ? loginEdit->text() :
		                                       fixedUser).toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		break;
	case 1:
		Q_ASSERT(passwdEdit);
		handler->gplugReturnText( passwdEdit->text().toLocal8Bit() ,
		                          KGreeterPluginHandler::IsPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	case 2:
		Q_ASSERT(passwd1Edit);
		handler->gplugReturnText( passwd1Edit->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsSecret );
		break;
	default: // case 3:
		Q_ASSERT(passwd2Edit);
		handler->gplugReturnText( passwd2Edit->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsNewPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	}
}

bool // virtual
KClassicGreeter::textMessage( const char *text, bool err )
{
	if (!err &&
	    QString( text ).indexOf( QRegExp( "^Changing password for [^ ]+$" ) ) >= 0)
		return true;
	return false;
}

void // virtual
KClassicGreeter::textPrompt( const char *prompt, bool echo, bool nonBlocking )
{
	pExp = exp;
	if (echo)
		exp = 0;
	else if (!authTok)
		exp = 1;
	else {
		QString pr( prompt );
		if (pr.indexOf( QRegExp( "\\bpassword\\b", Qt::CaseInsensitive ) ) >= 0) {
			if (pr.indexOf( QRegExp( "\\b(re-?(enter|type)|again|confirm|repeat)\\b",
			                      Qt::CaseInsensitive ) ) >= 0)
				exp = 3;
			else if (pr.indexOf( QRegExp( "\\bnew\\b", Qt::CaseInsensitive ) ) >= 0)
				exp = 2;
			else { // QRegExp( "\\b(old|current)\\b", Qt::CaseInsensitive ) is too strict
				handler->gplugReturnText( "",
				                          KGreeterPluginHandler::IsOldPassword |
				                          KGreeterPluginHandler::IsSecret );
				return;
			}
		} else {
			handler->gplugMsgBox( QMessageBox::Critical,
			                      i18n("Unrecognized prompt \"%1\"",
			                        prompt ) );
			handler->gplugReturnText( 0, 0 );
			exp = -1;
			return;
		}
	}

	if (pExp >= 0 && pExp >= exp) {
		revive();
		has = -1;
	}

	if (has >= exp || nonBlocking)
		returnData();
}

bool // virtual
KClassicGreeter::binaryPrompt( const char *, bool )
{
	// this simply cannot happen ... :}
	return true;
}

void // virtual
KClassicGreeter::start()
{
	authTok = !(passwdEdit && passwdEdit->isEnabled());
	exp = has = -1;
	running = true;
}

void // virtual
KClassicGreeter::suspend()
{
}

void // virtual
KClassicGreeter::resume()
{
}

void // virtual
KClassicGreeter::next()
{
	// assert( running );
	if (loginEdit && loginEdit->hasFocus()) {
		passwdEdit->setFocus(); // will cancel running login if necessary
		has = 0;
	} else if (passwdEdit && passwdEdit->hasFocus()) {
		if (passwd1Edit)
			passwd1Edit->setFocus();
		has = 1;
	} else if (passwd1Edit) {
		if (passwd1Edit->hasFocus()) {
			passwd2Edit->setFocus();
			has = 1; // sic!
		} else
			has = 3;
	} else
		has = 1;
	if (exp < 0)
		handler->gplugStart();
	else if (has >= exp)
		returnData();
}

void // virtual
KClassicGreeter::abort()
{
	running = false;
	if (exp >= 0) {
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
}

void // virtual
KClassicGreeter::succeeded()
{
	// assert( running || timed_login );
	if (!authTok) {
		setActive( false );
		if (passwd1Edit) {
			authTok = true;
			return;
		}
	} else
		setActive2( false );
	exp = -1;
	running = false;
}

void // virtual
KClassicGreeter::failed()
{
	// assert( running || timed_login );
	setActive( false );
	setActive2( false );
	exp = -1;
	running = false;
}

void // virtual
KClassicGreeter::revive()
{
	// assert( !running );
	setActive2( true );
	if (authTok) {
		passwd1Edit->clear();
		passwd2Edit->clear();
		passwd1Edit->setFocus();
	} else {
		passwdEdit->clear();
		if (loginEdit && loginEdit->isEnabled())
			passwdEdit->setEnabled( true );
		else {
			setActive( true );
			if (loginEdit && loginEdit->text().isEmpty())
				loginEdit->setFocus();
			else
				passwdEdit->setFocus();
		}
	}
}

void // virtual
KClassicGreeter::clear()
{
	// assert( !running && !passwd1Edit );
	passwdEdit->clear();
	if (loginEdit) {
		loginEdit->clear();
		loginEdit->setFocus();
		curUser.clear();
	} else
		passwdEdit->setFocus();
}


// private

void
KClassicGreeter::setActive( bool enable )
{
	if (loginEdit)
		loginEdit->setEnabled( enable );
	if (passwdEdit)
		passwdEdit->setEnabled( enable );
}

void
KClassicGreeter::setActive2( bool enable )
{
	if (passwd1Edit) {
		passwd1Edit->setEnabled( enable );
		passwd2Edit->setEnabled( enable );
	}
}

void
KClassicGreeter::slotLoginLostFocus()
{
	if (!running)
		return;
	if (exp > 0) {
		if (curUser == loginEdit->text())
			return;
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
	curUser = loginEdit->text();
	handler->gplugSetUser( curUser );
}

void
KClassicGreeter::slotChanged()
{
	if (running)
		handler->gplugChanged();
}

// factory

static bool init( const QString &,
                  QVariant (*getConf)( void *, const char *, const QVariant & ),
                  void *ctx )
{
	echoMode = getConf( ctx, "EchoPasswd", QVariant( -1 ) ).toInt();
	KGlobal::locale()->insertCatalog( "kgreet_classic" );
	return true;
}

static void done( void )
{
	KGlobal::locale()->removeCatalog( "kgreet_classic" );
}

static KGreeterPlugin *
create( KGreeterPluginHandler *handler,
        QWidget *parent,
        const QString &fixedEntity,
        KGreeterPlugin::Function func,
        KGreeterPlugin::Context ctx )
{
	return new KClassicGreeter( handler, parent, fixedEntity, func, ctx );
}

KDE_EXPORT KGreeterPluginInfo kgreeterplugin_info = {
	I18N_NOOP2("@item:inmenu authentication method", "Username + password (classic)"), "classic",
	KGreeterPluginInfo::Local | KGreeterPluginInfo::Presettable,
	init, done, create
};

#include "kgreet_classic.moc"
