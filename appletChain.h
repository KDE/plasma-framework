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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef PLASMA_APPLETCHAIN_H
#define PLASMA_APPLETCHAIN_H

#include <ksharedptr.h>

#include "plasma.h"

namespace Plasma
{

class KDE_EXPORT AppletChain : public QObject, public KShared
{
    Q_OBJECT
    Q_PROPERTY(AppletConstraint constraint READ constraint WRITE setConstraint)

    public:
        typedef KSharedPtr<AppletChain> Ptr;

        AppletChain(QObject* parent);
        ~AppletChain();

        Plasma::AppletConstraint constraint();
        void setConstraint(Plasma::AppletConstraint constraint);
        Plasma::Direction popupDirection() const;

        void setScreenEdge(Plasma::ScreenEdge edge);
        Plasma::ScreenEdge screenEdge();


    private:
        class Private;
        Private* d;
};

} // Plasma namespace

#endif // multiple inclusion guard
