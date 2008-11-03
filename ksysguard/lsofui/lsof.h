/*  This file is part of the KDE project
    
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

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

#ifndef LSOFWIDGET_H_
#define LSOFWIDGET_H_

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtGui/QTreeWidget>
#include <kapplication.h>

struct KLsofWidgetPrivate;

class KDE_EXPORT KLsofWidget : public QTreeWidget {
  Q_OBJECT
  Q_PROPERTY( qlonglong pid READ pid WRITE setPid )
	public:
		KLsofWidget(QWidget *parent = NULL);
                ~KLsofWidget();
		bool update();

	private Q_SLOTS:
		/* For QProcess *process */
		//void error ( QProcess::ProcessError error );
		void finished ( int exitCode, QProcess::ExitStatus exitStatus );
		//void readyReadStandardError ();
		//void readyReadStandardOutput ();
		//void started ();
		qlonglong pid() const;
		void setPid(qlonglong pid);
	private:
		KLsofWidgetPrivate* const d;
};

/*  class LsofProcessInfo {
    public:
	pid_t tpid;
	int pidst;
	pid_t pid;
	pid_t ppid;
	pid_t pgrp;
	int uid;
	QString cmd;
	QString login;
  };
  class LsofFileInfo {
	QString file_descriptor;
	char access;
	int file_struct_share_count;
	char device_character_code;
	long major_minor;
	long file_struct_address;
	long file_flags;
	long inode;
	long link_count;
	char lock;
	long file_struct_node_id;
	long file_offset;
	QString protocol_name;
	QString stream_module;
	QString file_type;
	QString tcp_info;
  };
*/
#endif 
