/*
	Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
	Original author: Gregory Schlomoff <greg@betterinbox.com>

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
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#ifndef DECLARATIVEDRAGDROPEVENT_H
#define DECLARATIVEDRAGDROPEVENT_H

#include <QObject>
#include <QGraphicsSceneDragDropEvent>
#include "DeclarativeMimeData.h"

class DeclarativeDragDropEvent : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int x READ x)
	Q_PROPERTY(int y READ y)
	Q_PROPERTY(int buttons READ buttons)
	Q_PROPERTY(int modifiers READ modifiers)
	Q_PROPERTY(DeclarativeMimeData* data READ data)
	Q_PROPERTY(Qt::DropActions possibleActions READ possibleActions)
	Q_PROPERTY(Qt::DropAction proposedAction READ proposedAction)

public:

	DeclarativeDragDropEvent(QGraphicsSceneDragDropEvent* e, QObject* parent = 0);

	int x() const { return m_x; }
	int y() const { return m_y; }
	int buttons() const { return m_buttons; }
	int modifiers() const { return m_modifiers; }
	DeclarativeMimeData* data() { return &m_data; }
	Qt::DropAction proposedAction() const { return m_event->proposedAction(); }
	Qt::DropActions possibleActions() const { return m_event->possibleActions(); }

public slots:
	void accept(int action);

private:
	int m_x;
	int m_y;
	Qt::MouseButtons m_buttons;
	Qt::KeyboardModifiers m_modifiers;
	DeclarativeMimeData m_data;
	QGraphicsSceneDragDropEvent* m_event;
};

#endif // DECLARATIVEDRAGDROPEVENT_H
