/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOOLTIPDIALOG_H
#define TOOLTIPDIALOG_H

#include "popupplasmawindow.h"

#include <QQuickWindow>
#include <QTimer>
#include <QVariant>

class QQuickItem;

namespace PlasmaQuick
{
class SharedQmlEngine;
}

/**
 * Internally used by Tooltip
 */
class ToolTipDialog : public PlasmaQuick::PopupPlasmaWindow
{
    Q_OBJECT

public:
    explicit ToolTipDialog();
    ~ToolTipDialog() override;

    QQuickItem *loadDefaultItem();

    void dismiss();
    void keepalive();

    bool interactive();
    void setInteractive(bool interactive);

    int hideTimeout() const;
    void setHideTimeout(int timeout);

    /**
     * Basically the last one who has shown the dialog
     */
    QObject *owner() const;
    void setOwner(QObject *owner);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool event(QEvent *e) override;

private Q_SLOTS:
    void valueChanged(const QVariant &value);

private:
    void updateSize();

    QPointer<QQuickItem> m_lastMainItem;
    PlasmaQuick::SharedQmlEngine *m_qmlObject;
    QTimer *m_showTimer;
    int m_hideTimeout;
    bool m_interactive;
    QObject *m_owner;
};

#endif
