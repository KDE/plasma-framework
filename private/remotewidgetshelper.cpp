/*
    Copyright (C) 2009 Dario Freddi <drf@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/
#include "remotewidgetshelper_p.h"

#include <QFile>
#include <QTextStream>

ActionReply RemoteWidgetsHelper::save(const QVariantMap &args)
{
    QString filename = args["filename"].toString();
    QString source = args["source"].toString();  
  
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ActionReply reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(file.error());

        return reply;
    }
    
    QFile sfile(source);
    if (!sfile.open(QIODevice::ReadOnly)) {
        ActionReply reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(sfile.error());

        return reply;
    }

    QTextStream stream(&file);
    stream << sfile.readAll();

    return ActionReply::SuccessReply;
}

KDE4_AUTH_HELPER_MAIN("org.kde.kcontrol.kcmremotewidgets", RemoteWidgetsHelper)
