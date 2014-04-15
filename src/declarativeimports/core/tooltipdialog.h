/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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

#ifndef TOOLTIPDIALOG_H
#define TOOLTIPDIALOG_H

#include "dialog.h"

#include <QTimer>
#include <QQuickWindow>
#include <QWeakPointer>
#include <QtCore/QVariant>

class QQuickItem;
class QGraphicsWidget;
class QPropertyAnimation;

namespace KDeclarative
{
    class QmlObject;
}

/**
 * QML wrapper for kdelibs Plasma::ToolTipDialog
 *
 * Exposed as `ToolTipDialog` in QML.
 */
class ToolTipDialog : public QQuickWindow
{
    Q_OBJECT

public:
    ToolTipDialog(QWindow *parent = 0);
    ~ToolTipDialog();

    QQuickItem *loadDefaultItem();

    Plasma::Types::Direction direction() const;
    void setDirection(Plasma::Types::Direction loc);

    /**
     * animate the position change if visible
     */
    virtual void adjustGeometry(const QRect &geom);

    void dismiss();
    void keepalive();

    bool interactive();
    void setInteractive(bool interactive);

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    Plasma::Types::Location location() const;
    void setLocation(Plasma::Types::Location location);

    virtual QPoint popupPosition(QQuickItem *item, const QSize &size);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *re);
    bool event(QEvent *e);

private Q_SLOTS:
    void valueChanged(const QVariant &value);

Q_SIGNALS:
    void locationChanged();
    void visualParentChanged();

private:
    QWeakPointer <QQuickItem> m_mainItem;
    QWeakPointer <QQuickItem> m_visualParent;
    KDeclarative::QmlObject *m_qmlObject;
    QTimer *m_showTimer;
    QPropertyAnimation *m_animation;
    int m_hideTimeout;
    Plasma::Types::Direction m_direction;
    Plasma::Types::Location m_location;
    bool m_interactive;
};

#endif
