/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef TASKRMBMENU_H
#define TASKRMBMENU_H

// Qt
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtGui/QMenu>

// Own
#include <taskmanager/task.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

class TASKMANAGER_EXPORT TaskRMBMenu : public QMenu
{
	Q_OBJECT

public:
	explicit TaskRMBMenu(const TaskList&, bool showAll = true, QWidget *parent = 0);
	explicit TaskRMBMenu(TaskPtr, bool showAll = true, QWidget *parent = 0);

private:
	void fillMenu(TaskPtr);
	void fillMenu();
    QMenu* makeAdvancedMenu(TaskPtr);
	QMenu* makeDesktopsMenu(TaskPtr);
	QMenu* makeDesktopsMenu();

private Q_SLOTS:
	void slotMinimizeAll();
	void slotMaximizeAll();
	void slotRestoreAll();
	void slotShadeAll();
	void slotCloseAll();
	void slotAllToDesktop();
	void slotAllToCurrentDesktop();
	void slotToDesktop();

private:
	TaskList tasks;
	bool showAll;
	QList< QPair<TaskPtr, int> > toDesktopMap;
};

} // TaskManager namespace


#endif
