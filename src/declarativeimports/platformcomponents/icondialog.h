/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ICONDIALOG_H
#define ICONDIALOG_H

#include <QObject>
#include <QString>

#include "utils/d_ptr.h"

/**
 * Class which handles an icondialog execution.
 *
 * @deprecated Use IconDialog from org.kde.kquickcontrolsaddons
 *
 * Example:
 * @code
 * IconDialog {
 *   id: iconDialog
 * }
 *
 * ...
 *
 * icon = iconDialog.openDialog()
 * @endcode
 */
class IconDialog : public QObject
{
    Q_OBJECT

public:
    explicit IconDialog(QObject *parent = nullptr);
    ~IconDialog() override;

    Q_INVOKABLE QString openDialog();
    Q_INVOKABLE void closeDialog();

private:
    D_PTR;
};

#endif /* ICONDIALOG_H */
