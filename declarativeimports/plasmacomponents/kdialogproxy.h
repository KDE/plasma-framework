/*
*   Copyright (C) 2011 by Marco MArtin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef KDIALOG_PROXY_H
#define KDIALOG_PROXY_H

#include <QObject>
#include <KDialog>
#include "kdialogproxy.h"
#include "enums.h"

class QLabel;

class KDialogProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString titleText READ titleText WRITE setTitleText NOTIFY titleTextChanged)
    Q_PROPERTY(QString titleIcon READ titleIcon WRITE setTitleIcon NOTIFY titleIconChanged)
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString acceptButtonText READ acceptButtonText WRITE setAcceptButtonText NOTIFY acceptButtonTextChanged)
    Q_PROPERTY(QString rejectButtonText READ rejectButtonText WRITE setRejectButtonText NOTIFY rejectButtonTextChanged)
    Q_PROPERTY(DialogStatus::Status status READ status NOTIFY statusChanged)

public:
    KDialogProxy(QObject *parent = 0);
    ~KDialogProxy();

    void setTitleText(const QString &text);
    QString titleText() const;

    void setTitleIcon(const QString &icon);
    QString titleIcon() const;

    void setMessage(const QString &message);
    QString message() const;

    void setAcceptButtonText(const QString &text);
    QString acceptButtonText() const;

    void setRejectButtonText(const QString &text);
    QString rejectButtonText() const;

    void setStatus(DialogStatus::Status status);
    DialogStatus::Status status() const;

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

Q_SIGNALS:
    void titleTextChanged();
    void titleIconChanged();
    void messageChanged();
    void acceptButtonTextChanged();
    void rejectButtonTextChanged();
    void statusChanged();
    void accepted();
    void rejected();

private:
    KDialog *m_dialog;
    QLabel *m_label;
    QString m_titleText;
    QString m_titleIcon;
    QString m_message;
    QString m_acceptButtonText;
    QString m_rejectButtonText;
    DialogStatus::Status m_status;
};

#endif //KDIALOG_PROXY_H

