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

#ifndef DECLARATIVEDRAGAREA_H
#define DECLARATIVEDRAGAREA_H

#include <QDeclarativeItem>

class QDeclarativeComponent;
class DeclarativeMimeData;

class DeclarativeDragArea : public QDeclarativeItem
{
	Q_OBJECT
	Q_PROPERTY(QDeclarativeComponent* delegate READ delegate WRITE setDelegate NOTIFY delegateChanged RESET resetDelegate)
	Q_PROPERTY(QDeclarativeItem* source READ source WRITE setSource NOTIFY sourceChanged RESET resetSource)
	Q_PROPERTY(QDeclarativeItem* target READ source NOTIFY targetChanged)
	Q_PROPERTY(DeclarativeMimeData* data READ data CONSTANT)
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged) //TODO: Should call setAcceptDrops()
	Q_PROPERTY(Qt::DropActions supportedActions READ supportedActions WRITE setSupportedActions NOTIFY supportedActionsChanged)
	Q_PROPERTY(Qt::DropAction defaultAction READ defaultAction WRITE setDefaultAction NOTIFY defaultActionChanged)


public:
	DeclarativeDragArea(QDeclarativeItem *parent=0);
	~DeclarativeDragArea();

	QDeclarativeComponent *delegate() const;
	void setDelegate(QDeclarativeComponent* delegate);
	void resetDelegate();

	QDeclarativeItem* target() const;
	QDeclarativeItem* source() const;
	void setSource(QDeclarativeItem* source);
	void resetSource();

	bool isEnabled() const;
	void setEnabled(bool enabled);

	//supported actions
	Qt::DropActions supportedActions() const;
	void setSupportedActions(Qt::DropActions actions);

	//default action
	Qt::DropAction defaultAction() const;
	void setDefaultAction(Qt::DropAction action);

	DeclarativeMimeData* data() const;

signals:
	void delegateChanged();
	void sourceChanged();
	void targetChanged();
	void dataChanged();
	void enabledChanged();
	void drop(int action);
	void supportedActionsChanged();
	void defaultActionChanged();

protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *) {}
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *) {}

private:
	QDeclarativeComponent* m_delegate;
	QDeclarativeItem* m_source;
	QDeclarativeItem* m_target;
	bool m_enabled;
	Qt::DropActions m_supportedActions;
	Qt::DropAction m_defaultAction;
	DeclarativeMimeData* const m_data;
};

#endif // DECLARATIVEDRAGAREA_H
