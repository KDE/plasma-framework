/*
    SPDX-FileCopyrightText: 2008 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2008, 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dropmenu.h"
#include "containmentinterface.h"

#include <QAction>
#include <QApplication>
#include <QJSValue>
#include <QList>
#include <QMenu>
#include <QUrl>

#include <KIO/DropJob>
#include <KLocalizedString>

DropMenu::DropMenu(KIO::DropJob *dropJob, const QPoint &dropPoint, ContainmentInterface *parent)
    : QObject(parent)
    , m_dropPoint(dropPoint)
    , m_dropJob(dropJob)
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
    return (m_dropJob ? true : false);
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
