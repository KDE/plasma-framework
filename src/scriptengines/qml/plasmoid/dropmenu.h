/*
 *   Copyright 2008-2013 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010-2013 Marco Martin <mart@kde.org>
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

#ifndef DROPMENU_H 
#define DROPMENU_H 

#include <QObject>
#include <QPoint>

class QJSValue;
class QMenu;
class QAction;

namespace KIO
{
class DropJob;
}

class ContainmentInterface;

class DropMenu : public QObject
{
    Q_OBJECT

public:
    DropMenu(KIO::DropJob *dropJob, const QPoint &dropPoint, ContainmentInterface *parent = nullptr);
    ~DropMenu();

    QList<QUrl> urls() const;
    QPoint dropPoint() const;
    void setUrls(const QList<QUrl> &urls);
    void setMultipleMimetypes(bool multipleMimetypes);

    void addAction(QAction *action);
    bool isDropjobMenu() const;
    bool isMultipleMimetypes() const;
    void show();

private:
    QPoint m_dropPoint;
    QMenu *m_menu = nullptr;
    KIO::DropJob *m_dropJob = nullptr;
    QList<QAction *> m_dropActions = QList<QAction *>();
    QList<QUrl> m_urls = QList<QUrl>();
    bool m_multipleMimetypes = false;
};

#endif
