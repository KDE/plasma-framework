/*

Conversation widget for kdm greeter

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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

#include "kgreet_winbind.h"
#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kuser.h>
#include <kprocess.h>

#include <QRegExp>
#include <QLayout>
#include <QLabel>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QTextStream>

#include <stdlib.h>
#include <stdio.h>

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

static char separator;
static QStringList staticDomains;
static QString defaultDomain;

static void
splitEntity( const QString &ent, QString &dom, QString &usr )
{
	int pos = ent.indexOf( separator );
	if (pos < 0)
		dom = "<local>", usr = ent;
	else
		dom = ent.left( pos ), usr = ent.mid( pos + 1 );
}

KWinbindGreeter::KWinbindGreeter( KGreeterPluginHandler *_handler,
                                  QWidget *parent,
                                  const QString &_fixedEntity,
                                  Function _func, Context _ctx ) :
	QObject(),
	KGreeterPlugin( _handler ),
	func( _func ),
	ctx( _ctx ),
	exp( -1 ),
	pExp( -1 ),
	running( false )
{
	QGridLayout *grid = 0;

	int line = 0;

	if (!_handler->gplugHasNode( "domain-entry" ) ||
	    !_handler->gplugHasNode( "user-entry" ) ||
	    !_handler->gplugHasNode( "pw-entry" ))
	{
		parent = new QWidget( parent );
		parent->setObjectName( "talker" );
		widgetList << parent;
		grid = new QGridLayout( parent );
		grid->setMargin( 0 );
	}

	domainLabel = loginLabel = passwdLabel = passwd1Label = passwd2Label = 0;
	domainCombo = 0;
	loginEdit = 0;
	passwdEdit = passwd1Edit = passwd2Edit = 0;
	if (ctx == ExUnlock || ctx == ExChangeTok)
		splitEntity( KUser().loginName(), fixedDomain, fixedUser );
	else
		splitEntity( _fixedEntity, fixedDomain, fixedUser );
	if (func != ChAuthTok) {
		if (fixedUser.isEmpty()) {
			domainCombo = new KComboBox( parent );
			connect( domainCombo, SIGNAL(activated( const QString & )),
			         SLOT(slotChangedDomain( const QString & )) );
			connect( domainCombo, SIGNAL(activated( const QString & )),
			         SLOT(slotLoginLostFocus()) );
			connect( domainCombo, SIGNAL(activated( const QString & )),
			         SLOT(slotChanged()) );
			// should handle loss of focus
			loginEdit = new KLineEdit( parent );
			loginEdit->setContextMenuPolicy( Qt::NoContextMenu );

			if (!grid) {
				loginEdit->setObjectName( "user-entry" );
				domainCombo->setObjectName( "domain-entry" );
				widgetList << domainCombo << loginEdit;
			} else {
				domainLabel = new QLabel( i18n("&Domain:"), parent );
				domainLabel->setBuddy( domainCombo );
				loginLabel = new QLabel( i18n("&Username:"), parent );
				loginLabel->setBuddy( loginEdit );
				grid->addWidget( domainLabel, line, 0 );
				grid->addWidget( domainCombo, line++, 1 );
				grid->addWidget( loginLabel, line, 0 );
				grid->addWidget( loginEdit, line++, 1 );
			}
			connect( loginEdit, SIGNAL(editingFinished()), SLOT(slotLoginLostFocus()) );
			connect( loginEdit, SIGNAL(editingFinished()), SLOT(slotChanged()) );
			connect( loginEdit, SIGNAL(textChanged( const QString & )), SLOT(slotChanged()) );
			connect( loginEdit, SIGNAL(selectionChanged()), SLOT(slotChanged()) );
			domainCombo->addItems( staticDomains );
			QTimer::singleShot( 0, this, SLOT(slotStartDomainList()) );
		} else if (ctx != Login && ctx != Shutdown && grid) {
			domainLabel = new QLabel( i18n("Domain:"), parent );
			grid->addWidget( domainLabel, line, 0 );
			grid->addWidget( new QLabel( fixedDomain, parent ), line++, 1 );
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
KWinbindGreeter::~KWinbindGreeter()
{
	abort();
	qDeleteAll( widgetList );
}

void
KWinbindGreeter::slotChangedDomain( const QString &dom )
{
	if (!loginEdit->completionObject())
		return;
	QStringList users;
	if (dom == "<local>") {
		for (QStringList::ConstIterator it = allUsers.begin(); it != allUsers.end(); ++it)
			if ((*it).indexOf( separator ) < 0)
				users << *it;
	} else {
		QString st( dom + separator );
		for (QStringList::ConstIterator it = allUsers.begin(); it != allUsers.end(); ++it)
			if ((*it).startsWith( st ))
				users << (*it).mid( st.length() );
	}
	loginEdit->completionObject()->setItems( users );
}

void // virtual
KWinbindGreeter::loadUsers( const QStringList &users )
{
	allUsers = users;
	KCompletion *userNamesCompletion = new KCompletion;
	loginEdit->setCompletionObject( userNamesCompletion );
	loginEdit->setAutoDeleteCompletionObject( true );
	loginEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
	slotChangedDomain( defaultDomain );
}

void // virtual
KWinbindGreeter::presetEntity( const QString &entity, int field )
{
	QString dom, usr;
	splitEntity( entity, dom, usr );
	domainCombo->setCurrentItem( dom, true );
	slotChangedDomain( dom );
	loginEdit->setText( usr );
	if (field > 1)
		passwdEdit->setFocus();
	else if (field == 1 || field == -1) {
		if (field == -1) {
			passwdEdit->setText( "     " );
			passwdEdit->setEnabled( false );
			authTok = false;
		}
		loginEdit->setFocus();
		loginEdit->selectAll();
	}
	curUser = entity;
}

QString // virtual
KWinbindGreeter::getEntity() const
{
	QString dom, usr;
	if (fixedUser.isEmpty())
		dom = domainCombo->currentText(), usr = loginEdit->text();
	else
		dom = fixedDomain, usr = fixedUser;
	return dom == "<local>" ? usr : dom + separator + usr;
}

void // virtual
KWinbindGreeter::setUser( const QString &user )
{
	// assert (fixedUser.isEmpty());
	curUser = user;
	QString dom, usr;
	splitEntity( user, dom, usr );
	domainCombo->setCurrentItem( dom, true );
	slotChangedDomain( dom );
	loginEdit->setText( usr );
	passwdEdit->setFocus();
	passwdEdit->selectAll();
}

void // virtual
KWinbindGreeter::setEnabled( bool enable )
{
	// assert( !passwd1Label );
	// assert( func == Authenticate && ctx == Shutdown );
//	if (domainCombo)
//		domainCombo->setEnabled( enable );
//	if (loginLabel)
//		loginLabel->setEnabled( enable );
	passwdLabel->setEnabled( enable );
	setActive( enable );
	if (enable)
		passwdEdit->setFocus();
}

void // private
KWinbindGreeter::returnData()
{
	switch (exp) {
	case 0:
		handler->gplugReturnText( getEntity().toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		break;
	case 1:
		handler->gplugReturnText( passwdEdit->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	case 2:
		handler->gplugReturnText( passwd1Edit->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsSecret );
		break;
	default: // case 3:
		handler->gplugReturnText( passwd2Edit->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsNewPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	}
}

bool // virtual
KWinbindGreeter::textMessage( const char *text, bool err )
{
	if (!err &&
	    QString( text ).indexOf( QRegExp( "^Changing password for [^ ]+$" ) ) >= 0)
		return true;
	return false;
}

void // virtual
KWinbindGreeter::textPrompt( const char *prompt, bool echo, bool nonBlocking )
{
	pExp = exp;
	if (echo)
		exp = 0;
	else if (!authTok)
		exp = 1;
	else {
		QString pr( prompt );
		if (pr.indexOf( QRegExp( "\\b(old|current)\\b", Qt::CaseInsensitive ) ) >= 0) {
			handler->gplugReturnText( "",
			                          KGreeterPluginHandler::IsOldPassword |
			                          KGreeterPluginHandler::IsSecret );
			return;
		} else if (pr.indexOf( QRegExp( "\\b(re-?(enter|type)|again|confirm|repeat)\\b",
		                                Qt::CaseInsensitive ) ) >= 0)
			exp = 3;
		else if (pr.indexOf( QRegExp( "\\bnew\\b", Qt::CaseInsensitive ) ) >= 0)
			exp = 2;
		else {
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
KWinbindGreeter::binaryPrompt( const char *, bool )
{
	// this simply cannot happen ... :}
	return true;
}

void // virtual
KWinbindGreeter::start()
{
	authTok = !(passwdEdit && passwdEdit->isEnabled());
	exp = has = -1;
	running = true;
}

void // virtual
KWinbindGreeter::suspend()
{
}

void // virtual
KWinbindGreeter::resume()
{
}

void // virtual
KWinbindGreeter::next()
{
	// assert( running );
	if (domainCombo && domainCombo->hasFocus())
		loginEdit->setFocus();
	else if (loginEdit && loginEdit->hasFocus()) {
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
KWinbindGreeter::abort()
{
	running = false;
	if (exp >= 0) {
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
}

void // virtual
KWinbindGreeter::succeeded()
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
KWinbindGreeter::failed()
{
	// assert( running || timed_login );
	setActive( false );
	setActive2( false );
	exp = -1;
	running = false;
}

void // virtual
KWinbindGreeter::revive()
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
KWinbindGreeter::clear()
{
	// assert( !running && !passwd1Edit );
	passwdEdit->clear();
	if (loginEdit) {
		domainCombo->setCurrentItem( defaultDomain );
		slotChangedDomain( defaultDomain );
		loginEdit->clear();
		loginEdit->setFocus();
		curUser.clear();
	} else
		passwdEdit->setFocus();
}


// private

void
KWinbindGreeter::setActive( bool enable )
{
	if (domainCombo)
		domainCombo->setEnabled( enable );
	if (loginEdit)
		loginEdit->setEnabled( enable );
	if (passwdEdit)
		passwdEdit->setEnabled( enable );
}

void
KWinbindGreeter::setActive2( bool enable )
{
	if (passwd1Edit) {
		passwd1Edit->setEnabled( enable );
		passwd2Edit->setEnabled( enable );
	}
}

void
KWinbindGreeter::slotLoginLostFocus()
{
	if (!running)
		return;
	QString ent( getEntity() );
	if (exp > 0) {
		if (curUser == ent)
			return;
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
	curUser = ent;
	handler->gplugSetUser( curUser );
}

void
KWinbindGreeter::slotChanged()
{
	if (running)
		handler->gplugChanged();
}

void
KWinbindGreeter::slotStartDomainList()
{
	m_domainLister = new KProcess( this );
	(*m_domainLister) << "wbinfo" << "--own-domain" << "--trusted-domains";
	m_domainLister->setOutputChannelMode( KProcess::OnlyStdoutChannel );
	connect( m_domainLister, SIGNAL(finished( int, QProcess::ExitStatus )),
	         SLOT(slotEndDomainList()) );
	m_domainLister->start();
}

void
KWinbindGreeter::slotEndDomainList()
{
	QStringList domainList;

	while (!m_domainLister->atEnd()) {
		QString dom = m_domainLister->readLine();
		dom.chop( 1 );
		if (!staticDomains.contains( dom ))
			domainList.append( dom );
	}

	delete m_domainLister;

	for (int i = domainCombo->count(), min = staticDomains.count(); --i >= min; ) {
		int dli = domainList.indexOf( domainCombo->itemText( i ) );
		if (dli < 0) {
			if (i == domainCombo->currentIndex())
				domainCombo->setCurrentItem( defaultDomain );
			domainCombo->removeItem( i );
		} else
			domainList.removeAt( dli );
	}
	domainCombo->addItems( domainList );

	QTimer::singleShot( 5 * 1000, this, SLOT(slotStartDomainList()) );
}

// factory

static bool init( const QString &,
                  QVariant (*getConf)( void *, const char *, const QVariant & ),
                  void *ctx )
{
	echoMode = getConf( ctx, "EchoPasswd", QVariant( -1 ) ).toInt();

	staticDomains = getConf( ctx, "winbind.Domains", QVariant( "" ) ).toString().split( ':', QString::SkipEmptyParts );
	if (!staticDomains.size())
		staticDomains << "<local>";
	defaultDomain = getConf( ctx, "winbind.DefaultDomain", QVariant( staticDomains.first() ) ).toString();
	QString sepstr = getConf( ctx, "winbind.Separator", QVariant( QString() ) ).toString();
	if (sepstr.isNull()) {
		FILE *sepfile = popen( "wbinfo --separator 2>/dev/null", "r" );
		if (sepfile) {
			QTextStream( sepfile ) >> sepstr;
			if (pclose( sepfile ))
				sepstr = "\\";
		} else
			sepstr = "\\";
	}
	separator = sepstr[0].toLatin1();

	KGlobal::locale()->insertCatalog( "kgreet_winbind" );
	return true;
}

static void done( void )
{
	KGlobal::locale()->removeCatalog( "kgreet_winbind" );
	// avoid static deletion problems ... hopefully
	staticDomains.clear();
	defaultDomain.clear();
}

static KGreeterPlugin *
create( KGreeterPluginHandler *handler,
        QWidget *parent,
        const QString &fixedEntity,
        KGreeterPlugin::Function func,
        KGreeterPlugin::Context ctx )
{
	return new KWinbindGreeter( handler, parent, fixedEntity, func, ctx );
}

KDE_EXPORT KGreeterPluginInfo kgreeterplugin_info = {
	I18N_NOOP2("@item:inmenu authentication method", "Winbind / Samba"), "classic",
	KGreeterPluginInfo::Local | KGreeterPluginInfo::Fielded | KGreeterPluginInfo::Presettable,
	init, done, create
};

#include "kgreet_winbind.moc"
