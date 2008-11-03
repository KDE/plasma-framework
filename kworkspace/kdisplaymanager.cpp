/*
   Copyright (C) 2004 Oswald Buddenhagen <ossi@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdisplaymanager.h"

#ifdef Q_WS_X11

#include <kapplication.h>
#include <klocale.h>
#include <QtDBus/QtDBus>

#include <QRegExp>

#include <X11/Xauth.h>
#include <X11/Xlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

static enum { Dunno, NoDM, NewKDM, OldKDM, GDM } DMType = Dunno;
static const char *ctl, *dpy;

class KDisplayManager::Private
{
public:
	Private() : fd(-1) {}
	~Private() {
		if (fd >= 0)
			close( fd );
	}

	int fd;
};

KDisplayManager::KDisplayManager() : d(new Private)
{
	const char *ptr;
	struct sockaddr_un sa;

	if (DMType == Dunno) {
		if (!(dpy = ::getenv( "DISPLAY" )))
			DMType = NoDM;
		else if ((ctl = ::getenv( "DM_CONTROL" )))
			DMType = NewKDM;
		else if ((ctl = ::getenv( "XDM_MANAGED" )) && ctl[0] == '/')
			DMType = OldKDM;
		else if (::getenv( "GDMSESSION" ))
			DMType = GDM;
		else
			DMType = NoDM;
	}
	switch (DMType) {
	default:
		return;
	case NewKDM:
	case GDM:
		if ((d->fd = ::socket( PF_UNIX, SOCK_STREAM, 0 )) < 0)
			return;
		sa.sun_family = AF_UNIX;
		if (DMType == GDM) {
			strcpy( sa.sun_path, "/var/run/gdm_socket" );
			if (::connect( d->fd, (struct sockaddr *)&sa, sizeof(sa) )) {
				strcpy( sa.sun_path, "/tmp/.gdm_socket" );
				if (::connect( d->fd, (struct sockaddr *)&sa, sizeof(sa) )) {
					::close( d->fd );
					d->fd = -1;
					break;
				}
			}
			GDMAuthenticate();
		} else {
			if ((ptr = strchr( dpy, ':' )))
				ptr = strchr( ptr, '.' );
			snprintf( sa.sun_path, sizeof(sa.sun_path),
			          "%s/dmctl-%.*s/socket",
			          ctl, ptr ? int(ptr - dpy) : 512, dpy );
			if (::connect( d->fd, (struct sockaddr *)&sa, sizeof(sa) )) {
				::close( d->fd );
				d->fd = -1;
			}
		}
		break;
	case OldKDM:
		{
			QString tf( ctl );
			tf.truncate( tf.indexOf( ',' ) );
			d->fd = ::open( tf.toLatin1(), O_WRONLY );
		}
		break;
	}
}

KDisplayManager::~KDisplayManager()
{
	delete d;
}

bool
KDisplayManager::exec( const char *cmd )
{
	QByteArray buf;

	return exec( cmd, buf );
}

/**
 * Execute a KDM/GDM remote control command.
 * @param cmd the command to execute. FIXME: undocumented yet.
 * @param buf the result buffer.
 * @return result:
 *  @li If true, the command was successfully executed.
 *   @p ret might contain addional results.
 *  @li If false and @p ret is empty, a communication error occurred
 *   (most probably KDM is not running).
 *  @li If false and @p ret is non-empty, it contains the error message
 *   from KDM.
 */
bool
KDisplayManager::exec( const char *cmd, QByteArray &buf )
{
	bool ret = false;
	int tl;
	int len = 0;

	if (d->fd < 0)
		goto busted;

	tl = strlen( cmd );
	if (::write( d->fd, cmd, tl ) != tl) {
	    bust:
		::close( d->fd );
		d->fd = -1;
	    busted:
		buf.resize( 0 );
		return false;
	}
	if (DMType == OldKDM) {
		buf.resize( 0 );
		return true;
	}
	for (;;) {
		if (buf.size() < 128)
			buf.resize( 128 );
		else if (buf.size() < len * 2)
			buf.resize( len * 2 );
		if ((tl = ::read( d->fd, buf.data() + len, buf.size() - len)) <= 0) {
			if (tl < 0 && errno == EINTR)
				continue;
			goto bust;
		}
		len += tl;
		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;
			if (len > 2 && (buf[0] == 'o' || buf[0] == 'O') &&
			    (buf[1] == 'k' || buf[1] == 'K') && buf[2] <= ' ')
				ret = true;
			break;
		}
	}
	return ret;
}

bool
KDisplayManager::canShutdown()
{
	if (DMType == OldKDM)
		return strstr( ctl, ",maysd" ) != 0;

	QByteArray re;

	if (DMType == GDM)
		return exec( "QUERY_LOGOUT_ACTION\n", re ) && re.indexOf( "HALT" ) >= 0;

	return exec( "caps\n", re ) && re.indexOf( "\tshutdown" ) >= 0;
}

void
KDisplayManager::shutdown( KWorkSpace::ShutdownType shutdownType,
              KWorkSpace::ShutdownMode shutdownMode, /* NOT Default */
              const QString &bootOption )
{
	if (shutdownType == KWorkSpace::ShutdownTypeNone)
		return;

	bool cap_ask;
	if (DMType == NewKDM) {
		QByteArray re;
		cap_ask = exec( "caps\n", re ) && re.indexOf( "\tshutdown ask" ) >= 0;
	} else {
		if (!bootOption.isEmpty())
			return;
		cap_ask = false;
	}
	if (!cap_ask && shutdownMode == KWorkSpace::ShutdownModeInteractive)
		shutdownMode = KWorkSpace::ShutdownModeForceNow;

	QByteArray cmd;
	if (DMType == GDM) {
		cmd.append( shutdownMode == KWorkSpace::ShutdownModeForceNow ?
		            "SET_LOGOUT_ACTION " : "SET_SAFE_LOGOUT_ACTION " );
		cmd.append( shutdownType == KWorkSpace::ShutdownTypeReboot ?
		            "REBOOT\n" : "HALT\n" );
	} else {
		cmd.append( "shutdown\t" );
		cmd.append( shutdownType == KWorkSpace::ShutdownTypeReboot ?
		            "reboot\t" : "halt\t" );
		if (!bootOption.isEmpty())
			cmd.append( "=" ).append( bootOption.toLocal8Bit() ).append( "\t" );
		cmd.append( shutdownMode == KWorkSpace::ShutdownModeInteractive ?
		            "ask\n" :
		            shutdownMode == KWorkSpace::ShutdownModeForceNow ?
		            "forcenow\n" :
		            shutdownMode == KWorkSpace::ShutdownModeTryNow ?
		            "trynow\n" : "schedule\n" );
	}
	exec( cmd.data() );
}

bool
KDisplayManager::bootOptions( QStringList &opts, int &defopt, int &current )
{
	if (DMType != NewKDM)
		return false;

	QByteArray re;
	if (!exec( "listbootoptions\n", re ))
		return false;

	opts = QString::fromLocal8Bit( re.data() ).split( '\t', QString::SkipEmptyParts );
	if (opts.size() < 4)
		return false;

	bool ok;
	defopt = opts[2].toInt( &ok );
	if (!ok)
		return false;
	current = opts[3].toInt( &ok );
	if (!ok)
		return false;

	opts = opts[1].split( ' ', QString::SkipEmptyParts );
	for (QStringList::Iterator it = opts.begin(); it != opts.end(); ++it)
		(*it).replace( "\\s", " " );

	return true;
}

void
KDisplayManager::setLock( bool on )
{
	if (DMType != GDM)
		exec( on ? "lock\n" : "unlock\n" );
}

bool
KDisplayManager::isSwitchable()
{
	if (DMType == OldKDM)
		return dpy[0] == ':';

	if (DMType == GDM)
		return exec( "QUERY_VT\n" );

	QByteArray re;

	return exec( "caps\n", re ) && re.indexOf( "\tlocal" ) >= 0;
}

int
KDisplayManager::numReserve()
{
	if (DMType == GDM)
		return 1; /* Bleh */

	if (DMType == OldKDM)
		return strstr( ctl, ",rsvd" ) ? 1 : -1;

	QByteArray re;
	int p;

	if (!(exec( "caps\n", re ) && (p = re.indexOf( "\treserve " )) >= 0))
		return -1;
	return atoi( re.data() + p + 9 );
}

void
KDisplayManager::startReserve()
{
	if (DMType == GDM)
		exec("FLEXI_XSERVER\n");
	else
		exec("reserve\n");
}

bool
KDisplayManager::localSessions( SessList &list )
{
	if (DMType == OldKDM)
		return false;

	QByteArray re;

	if (DMType == GDM) {
		if (!exec( "CONSOLE_SERVERS\n", re ))
			return false;
		QStringList sess = QString(re.data() +3).split( QChar(';'), QString::SkipEmptyParts);
		for (QStringList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
			QStringList ts = (*it).split( QChar(',') );
			SessEnt se;
			se.display = ts[0];
			se.user = ts[1];
			se.vt = ts[2].toInt();
			se.session = "<unknown>";
			se.self = ts[0] == ::getenv( "DISPLAY" ); /* Bleh */
			se.tty = false;
			list.append( se );
		}
	} else {
		if (!exec( "list\talllocal\n", re ))
			return false;
		QStringList sess = QString(re.data() + 3).split(QChar('\t'), QString::SkipEmptyParts );
		for (QStringList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
			QStringList ts = (*it).split( QChar(',') );
			SessEnt se;
			se.display = ts[0];
			if (ts[1][0] == '@')
				se.from = ts[1].mid( 1 ), se.vt = 0;
			else
				se.vt = ts[1].mid( 2 ).toInt();
			se.user = ts[2];
			se.session = ts[3];
			se.self = (ts[4].indexOf( '*' ) >= 0);
			se.tty = (ts[4].indexOf( 't' ) >= 0);
			list.append( se );
		}
	}
	return true;
}

void
KDisplayManager::sess2Str2( const SessEnt &se, QString &user, QString &loc )
{
	if (se.tty) {
		user = i18nc("user: ...", "%1: TTY login", se.user );
		loc = se.vt ? QString("vt%1").arg( se.vt ) : se.display ;
	} else {
		user =
			se.user.isEmpty() ?
				se.session.isEmpty() ?
					i18n("Unused") :
					se.session == "<remote>" ?
						i18n("X login on remote host") :
						i18nc("... host", "X login on %1", se.session ) :
				se.session == "<unknown>" ?
					se.user :
					i18nc("user: session type", "%1: %2",
						  se.user, se.session );
		loc =
			se.vt ?
				QString("%1, vt%2").arg( se.display ).arg( se.vt ) :
				se.display;
	}
}

QString
KDisplayManager::sess2Str( const SessEnt &se )
{
	QString user, loc;

	sess2Str2( se, user, loc );
	return i18nc("session (location)", "%1 (%2)", user, loc );
}

bool
KDisplayManager::switchVT( int vt )
{
	if (DMType == GDM)
		return exec( QString("SET_VT %1\n").arg(vt).toLatin1() );

	return exec( QString("activate\tvt%1\n").arg(vt).toLatin1() );
}

void
KDisplayManager::lockSwitchVT( int vt )
{
	if (switchVT( vt ))
        {
            QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
            screensaver.call( "Lock" );
        }
}

void
KDisplayManager::GDMAuthenticate()
{
	FILE *fp;
	const char *dpy, *dnum, *dne;
	int dnl;
	Xauth *xau;

	dpy = DisplayString( QX11Info::display() );
	if (!dpy) {
		dpy = ::getenv( "DISPLAY" );
		if (!dpy)
			return;
	}
	dnum = strchr( dpy, ':' ) + 1;
	dne = strchr( dpy, '.' );
	dnl = dne ? dne - dnum : strlen( dnum );

	/* XXX should do locking */
	if (!(fp = fopen( XauFileName(), "r" )))
		return;

	while ((xau = XauReadAuth( fp ))) {
		if (xau->family == FamilyLocal &&
		    xau->number_length == dnl && !memcmp( xau->number, dnum, dnl ) &&
		    xau->data_length == 16 &&
		    xau->name_length == 18 && !memcmp( xau->name, "MIT-MAGIC-COOKIE-1", 18 ))
		{
			QString cmd( "AUTH_LOCAL " );
			for (int i = 0; i < 16; i++)
				cmd += QString::number( (uchar)xau->data[i], 16 ).rightJustified( 2, '0');
			cmd += '\n';
			if (exec( cmd.toLatin1() )) {
				XauDisposeAuth( xau );
				break;
			}
		}
		XauDisposeAuth( xau );
	}

	fclose (fp);
}

#endif // Q_WS_X11
