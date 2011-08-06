/***************************************************************************
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef QMENU_PROXY_H
#define QMENU_PROXY_H

#include <QAction>
#include <QDeclarativeListProperty>

class QMenuItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

public:
    QMenuItem(QObject *parent = 0) : QObject(parent) { m_action = new QAction(0); }
    ~QMenuItem() { }

    bool enabled() const { return m_action->isEnabled(); }
    void setEnabled(const bool enabled) { m_action->setEnabled(enabled); }

    bool checkable() const { return m_action->isCheckable(); }
    void setCheckable(const bool checkable) { m_action->setCheckable(checkable); }

    bool checked() const { return m_action->isChecked(); }
    void setChecked(const bool checked) { m_action->setChecked(checked); }

    QString text() const { return m_action->text(); }
    void setText(const QString &text) { m_action->setText(text); }

    QIcon icon() const { return m_action->icon(); }
    void setIcon(const QIcon &icon) { m_action->setIcon(icon); }

    QAction *nativeAction() { return m_action; }

private:
    QAction *m_action;
};

typedef QList<QMenuItem*> QMenuItems;

class QMenuProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QMenuItem> actions READ actions)
    Q_CLASSINFO("DefaultProperty", "actions")

public:
    QMenuProxy(QObject *parent = 0);
    ~QMenuProxy();

    QDeclarativeListProperty<QMenuItem> actions();
    int actionCount() const;
    QMenuItem *action(int) const;

    Q_INVOKABLE void showMenu(int x, int y);

Q_SIGNALS:
    void actionTriggered(QString itemName);

private:
    QMenuItems m_actions;
};

#endif //QMENU_PROXY_H

