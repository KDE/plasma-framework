/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "dropmenu.h"
#include "containmentinterface.h"

#include <QApplication>
#include <QUrl>
#include <QPoint>
#include <QMenu>
#include <QJSValue>
#include <QAction>
#include <QList>

#include <KIO/DropJob>
#include <KLocalizedString>

DropMenu::DropMenu(KIO::DropJob *dropJob, const QPoint &dropPoint, ContainmentInterface *parent)
    : QObject(parent),
    m_dropPoint(dropPoint),
    m_dropJob(dropJob)
{
    if (!dropJob) {
        m_menu = new QMenu(i18n("Content dropped"));
        if (m_menu->winId()) {
            m_menu->windowHandle()->setTransientParent(parent->window());
        }
        connect(m_menu, &QMenu::aboutToHide, this, &QObject::deleteLater);
    } else {
        connect(m_dropJob, &QObject::destroyed, this, &QObject::deleteLater);
    }
}

DropMenu::~DropMenu()
{
    if (m_menu) {
        delete m_menu;
    }
}

QList<QUrl> DropMenu::urls() const
{
    return m_urls;
}

void DropMenu::setUrls(const QList<QUrl> &urls)
{
    m_urls = urls;
}

QPoint DropMenu::dropPoint() const
{
    return m_dropPoint;
}

void DropMenu::show()
{
    if (m_dropJob) {
        m_dropJob->setApplicationActions(m_dropActions);
        m_dropJob->showMenu(m_dropPoint);
    } else if (m_menu) {
        m_menu->addActions(m_dropActions);
        m_menu->popup(m_dropPoint);
    }
}

void DropMenu::addAction(QAction *action)
{
    m_dropActions << action;
}

bool DropMenu::isDropjobMenu() const
{
    return (m_dropJob? true : false);
}

void DropMenu::setMultipleMimetypes(bool multipleMimetypes)
{
    if (m_multipleMimetypes != multipleMimetypes) {
        m_multipleMimetypes = multipleMimetypes;
    }
}

bool DropMenu::isMultipleMimetypes() const
{
    return m_multipleMimetypes;
}


#include "moc_dropmenu.cpp"
