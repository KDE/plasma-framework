/*
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
class IconDialog: public QObject
{
    Q_OBJECT

public:
    explicit IconDialog(QObject *parent = nullptr);
    ~IconDialog();

    Q_INVOKABLE QString openDialog();
    Q_INVOKABLE void closeDialog();

private:
    D_PTR;
};

#endif /* ICONDIALOG_H */

