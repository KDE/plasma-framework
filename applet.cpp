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

#include <QEvent>
#include <QList>
#include <QSize>
#include <QTimer>

#include <kstandarddirs.h>

#include "applet.h"

namespace Plasma
{

class Applet::Private
{
    public:
        Private(KService::Ptr appletDescription, int uniqueID,
                const AppletChain::Ptr appletChain)
            : id(uniqueID),
              globalConfig(0),
              appletConfig(0),
              appletDescription(appletDescription),
              chain(appletChain)
        { }

//         ~Private()
//         {
//         }

        int id;
        KSharedConfig::Ptr globalConfig;
        KSharedConfig::Ptr appletConfig;
        KService::Ptr appletDescription;
        QList<QObject*> watchedForFocus;
        AppletChain::Ptr chain;
};

Applet::Applet(QWidget* parent,
               KService::Ptr appletDescription,
               const AppletChain::Ptr chain,
               int id)
    : QWidget(parent),
      d(new Private(appletDescription, id, chain))
{
}

Applet::~Applet()
{
    needsFocus(false);
    delete d;
}

KSharedConfig::Ptr Applet::globalAppletConfig() const
{
    if (!d->globalConfig)
    {
        QString file = locateLocal("config",
                                   "plasma_" + globalName() + "rc",
                                   true);
        d->globalConfig = KSharedConfig::openConfig(file, false, true);
    }

    return d->globalConfig;
}

KSharedConfig::Ptr Applet::appletConfig() const
{
    if (!d->appletConfig)
    {
        QString file = locateLocal("appdata",
                                   "applets/" + instanceName() + "rc",
                                   true);
        d->appletConfig = KSharedConfig::openConfig(file, false, true);
    }

    return d->appletConfig;
}

const AppletChain::Ptr Applet::chain() const
{
    return d->chain;
}

void Applet::setChain(const AppletChain::Ptr appletChain)
{
    d->chain = appletChain;
}

void Applet::constraintsUpdated()
{
}

QString Applet::globalName() const
{
    return d->appletDescription->library();
}

QString Applet::instanceName() const
{
    return d->appletDescription->library() + QString::number(d->id);
}

void Applet::watchForFocus(QWidget* widget, bool watch)
{
    if (!widget)
    {
        return;
    }

    int index = d->watchedForFocus.indexOf(widget);
    if (watch)
    {
        if (index == -1)
        {
            d->watchedForFocus.append(widget);
            widget->installEventFilter(this);
        }
    }
    else if (index != -1)
    {
        d->watchedForFocus.removeAt(index);
        widget->removeEventFilter(this);
    }
}

void Applet::needsFocus(bool focus)
{
    if (focus == hasFocus())
    {
        return;
    }

    emit requestFocus(focus);
}

bool Applet::eventFilter(QObject *o, QEvent * e)
{
    if (!d->watchedForFocus.contains(o))
    {
        if (e->type() == QEvent::MouseButtonRelease ||
            e->type() == QEvent::FocusIn)
        {
            needsFocus(true);
        }
        else if (e->type() == QEvent::FocusOut)
        {
            needsFocus(false);
        }
    }

    return QWidget::eventFilter(o, e);
}

} // Plasma namespace

#include "applet.moc"
