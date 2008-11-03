/*

Conversation widget for kdm greeter

Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
Copyright (C) 2008 Oswald Buddenhagen <ossi@kde.org>

based on classic kdm greeter:

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

#include "kgreet_generic.h"

#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kuser.h>

#include <QLayout>
#include <QLabel>
#include <QTextDocument>

extern KDE_EXPORT KGreeterPluginInfo kgreeterplugin_info; // defined at bottom

static int echoMode;

class KDMPasswordEdit : public KLineEdit {
public:
	KDMPasswordEdit( QWidget *parent = 0 ) : KLineEdit( parent )
	{
		if (::echoMode == -1)
			setPasswordMode( true );
		else
			setEchoMode( ::echoMode ? Password : NoEcho );
		setContextMenuPolicy( Qt::NoContextMenu );
	}
};

KGenericGreeter::KGenericGreeter( KGreeterPluginHandler *_handler,
                                  QWidget *parent,
                                  const QString &_fixedEntity,
                                  Function _func, Context _ctx ) :
	QObject(),
	KGreeterPlugin( _handler ),
	m_lineEdit( 0 ),
	fixedUser( _fixedEntity ),
	func( _func ),
	ctx( _ctx ),
	exp( -1 ),
	running( false )
{
	m_parentWidget = new QWidget( parent );
	m_parentWidget->setObjectName( "talker" );
	// XXX set some minimal size
	widgetList << m_parentWidget;
	m_grid = new QGridLayout( m_parentWidget );
	m_grid->setMargin( 0 );

	if (ctx == ExUnlock || ctx == ExChangeTok)
		fixedUser = KUser().loginName();
}

// virtual
KGenericGreeter::~KGenericGreeter()
{
	abort();
	delete m_parentWidget;
}

void // virtual
KGenericGreeter::loadUsers( const QStringList &users )
{
	m_users = users;
}

void // virtual
KGenericGreeter::presetEntity( const QString &entity, int /* field */ )
{
	// assert( !running );
	curUser = entity;
}

QString // virtual
KGenericGreeter::getEntity() const
{
	return fixedUser.isEmpty() ? curUser : fixedUser;
}

void // virtual
KGenericGreeter::setUser( const QString &user )
{
	// assert( running );
	// assert( fixedUser.isEmpty() );
	if (!(kgreeterplugin_info.flags & KGreeterPluginInfo::Presettable))
		return; // Not interested in PAM telling us who logged in
	if (exp) {
		abort();
		start();
	}
	curUser = user;
	if (m_lineEdit) { // could be null if plugin is misconfigured
		m_lineEdit->setText( user );
		m_lineEdit->selectAll();
		m_lineEdit->setFocus();
	}
}

void // virtual
KGenericGreeter::setEnabled( bool enable )
{
	// assert( func == Authenticate && ctx == Shutdown );
	// XXX this is likely to bear some bogosity
	foreach (QWidget *w, m_children)
		w->setEnabled( enable );
	if (enable && m_lineEdit)
		m_lineEdit->setFocus();
}

bool // virtual
KGenericGreeter::textMessage( const char *text, bool err )
{
	if (err)
		return false;

	if (m_infoMsgs.isEmpty())
		revive();
	QString qtext = QString::fromUtf8( text );
	m_infoMsgs.append( qtext );
	QLabel *label = new QLabel( qtext, m_parentWidget );
	m_grid->addWidget( label, m_line++, 0, 1, 2 );
	m_children.append( label );

	return true;
}

void // virtual
KGenericGreeter::textPrompt( const char *prompt, bool echo, bool /* nonBlocking */ )
{
	exp =
		exp >= 0 ||
		func != Authenticate ||
		!(kgreeterplugin_info.flags & KGreeterPluginInfo::Presettable);

	if (!exp && !fixedUser.isEmpty()) {
		handler->gplugReturnText( fixedUser.toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		return;
	}

	if (m_infoMsgs.isEmpty())
		revive();
	else
		m_infoMsgs.clear();

	QLabel *label = new QLabel( QString::fromUtf8( prompt ).trimmed() );
	m_grid->addWidget( label, m_line, 0 );
	m_children.append( label );
	m_echo = echo;
	if (echo) {
		m_lineEdit = new KLineEdit;
		m_lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
		if (!exp) {
			if (!m_users.isEmpty()) {
				KCompletion *userNamesCompletion = new KCompletion;
				userNamesCompletion->setItems( m_users );
				m_lineEdit->setCompletionObject( userNamesCompletion );
				m_lineEdit->setAutoDeleteCompletionObject( true );
				m_lineEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
			}
			if (!curUser.isEmpty()) {
				m_lineEdit->setText( curUser );
				m_lineEdit->selectAll();
				connect( m_lineEdit, SIGNAL(selectionChanged()), SLOT(slotChanged()) );
			}
			connect( m_lineEdit, SIGNAL(editingFinished()), SLOT(slotLoginLostFocus()) );
		}
		connect( m_lineEdit, SIGNAL(editingFinished()), SLOT(slotChanged()) );
		connect( m_lineEdit, SIGNAL(textChanged( const QString & )), SLOT(slotChanged()) );
	} else {
		m_lineEdit = new KDMPasswordEdit;
	}
	m_lineEdit->setMinimumWidth(
		m_lineEdit->fontMetrics().width( "This is a long password" ) );
	m_grid->addWidget( m_lineEdit, m_line, 1 );
	m_children.append( m_lineEdit );
	m_lineEdit->show();
	m_lineEdit->setFocus();
}

bool // virtual
KGenericGreeter::binaryPrompt( const char *, bool )
{
	// FIXME
	return true;
}

void // virtual
KGenericGreeter::start()
{
	exp = -1;
	running = true;
	handler->gplugStart();
}

void // virtual
KGenericGreeter::suspend()
{
}

void // virtual
KGenericGreeter::resume()
{
}

void // virtual
KGenericGreeter::next()
{
	if (m_lineEdit) {
		m_lineEdit->setEnabled( false );
		QString text = m_lineEdit->text();
		m_lineEdit = 0;
		handler->gplugReturnText( text.toLocal8Bit(),
		                          !m_echo ?
		                             KGreeterPluginHandler::IsSecret :
		                             !exp ?
		                                KGreeterPluginHandler::IsUser : 0 );
	}
}

void // virtual
KGenericGreeter::abort()
{
	running = false;
	if (exp >= 0) {
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
}

void // virtual
KGenericGreeter::succeeded()
{
	failed(); // redefining terms :-D
}

void // virtual
KGenericGreeter::failed()
{
	// assert( running || timed_login );
	if (!m_infoMsgs.isEmpty()) {
		QString text = "<qt>";
		foreach (const QString &msg, m_infoMsgs)
			text += "<p>" + Qt::escape( msg ) + "</p>";
		text += "</qt>";
		revive();
		handler->gplugMsgBox( QMessageBox::Information, text );
	} else {
		foreach (QWidget *w, m_children)
			w->setEnabled( false );
	}
	exp = -1;
	running = false;
}

void // virtual
KGenericGreeter::revive()
{
	// assert( !running );
	foreach (QWidget *w, m_children)
		w->deleteLater();
	m_children.clear();
	m_infoMsgs.clear();
	m_lineEdit = 0;
	m_line = 0;
}

void // virtual
KGenericGreeter::clear()
{
	// assert( !running && !passwd1Edit );
	revive();
	curUser = QString::null;
}


// private
void
KGenericGreeter::slotLoginLostFocus()
{
	if (curUser != m_lineEdit->text()) {
		curUser = m_lineEdit->text();
		handler->gplugSetUser( curUser );
	}
}

void
KGenericGreeter::slotChanged()
{
	handler->gplugChanged();
}

// factory
static bool init( const QString &,
                  QVariant (*getConf)( void *, const char *, const QVariant & ),
                  void *ctx )
{
	echoMode = getConf( ctx, "EchoMode", QVariant( -1 ) ).toInt();
	// Fielded entities are not supported per se.
	// This implies that the first field is the presettable entity, if any.
	if (getConf( ctx, "generic.Presettable", QVariant( false ) ).toBool())
		kgreeterplugin_info.flags |= KGreeterPluginInfo::Presettable;
	KGlobal::locale()->insertCatalog( "kgreet_generic" );
	return true;
}

static void done( void )
{
	KGlobal::locale()->removeCatalog( "kgreet_generic" );
}

static KGreeterPlugin *
create( KGreeterPluginHandler *handler,
        QWidget *parent,
        const QString &fixedEntity,
        KGreeterPlugin::Function func,
        KGreeterPlugin::Context ctx )
{
	return new KGenericGreeter( handler, parent, fixedEntity, func, ctx );
}

KDE_EXPORT KGreeterPluginInfo kgreeterplugin_info = {
	I18N_NOOP2("@item:inmenu authentication method", "Generic"), "generic",
	KGreeterPluginInfo::Local,
	init, done, create
};

#include "kgreet_generic.moc"
