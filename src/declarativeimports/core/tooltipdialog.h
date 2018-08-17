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
#include <QtCore/QVariant>

class QQuickItem;
class QGraphicsWidget;

namespace KDeclarative
{
class QmlObject;
}

/**
 * Internally used by Tooltip
 */
class ToolTipDialog : public PlasmaQuick::Dialog
{
    Q_OBJECT

public:
    explicit ToolTipDialog(QQuickItem *parent = nullptr);
    ~ToolTipDialog() override;

    QQuickItem *loadDefaultItem();

    Plasma::Types::Direction direction() const;
    void setDirection(Plasma::Types::Direction loc);

    void dismiss();
    void keepalive();

    bool interactive();
    void setInteractive(bool interactive);

    /**
     * Basically the last one who has shown the dialog
     */
    QObject *owner() const;
    void setOwner(QObject *owner);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *re) override;
    bool event(QEvent *e) override;

private Q_SLOTS:
    void valueChanged(const QVariant &value);

private:
    KDeclarative::QmlObject *m_qmlObject;
    QTimer *m_showTimer;
    int m_hideTimeout;
    bool m_interactive;
    QObject *m_owner;
};

#endif
