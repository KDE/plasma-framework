/*
    SPDX-FileCopyrightText: 2008-2013 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010-2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

class ContainmentItem;

class DropMenu : public QObject
{
    Q_OBJECT

public:
    DropMenu(KIO::DropJob *dropJob, const QPoint &dropPoint, ContainmentItem *parent = nullptr);
    ~DropMenu() override;

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
