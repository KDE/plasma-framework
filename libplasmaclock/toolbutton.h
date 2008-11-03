/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>

namespace Plasma
{
    class FrameSvg;
}

class ToolButton: public QToolButton
{
    Q_OBJECT

public:
    ToolButton();
    ToolButton(QWidget *parent);
    void setAction(QAction *action);

protected:
    void paintEvent(QPaintEvent *event);

protected slots:
    void actionDestroyed(QObject *);
    void syncToAction();

private:
    QAction *m_action;
    Plasma::FrameSvg *m_background;
};

#endif

