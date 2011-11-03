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

#include "kdialogproxy.h"

#include <QLabel>

#include <KIcon>

KDialogProxy::KDialogProxy (QObject *parent)
    : QObject(parent),
      m_status(DialogStatus::Closed)
{
    m_dialog = new KDialog(0);
    m_label = new QLabel(m_dialog);
    m_label->setWordWrap(true);
    m_dialog->setMainWidget(m_label);
    m_dialog->setButtons( KDialog::Ok | KDialog::Cancel);
    connect(m_dialog, SIGNAL(okClicked()), this, SIGNAL(accepted()));
    connect(m_dialog, SIGNAL(cancelClicked()), this, SIGNAL(rejected()));
    connect(m_dialog, SIGNAL(closeClicked()), this, SIGNAL(rejected()));
}

KDialogProxy::~KDialogProxy()
{
    delete m_dialog;
}


void KDialogProxy::setTitleText(const QString &text)
{
    if (text == m_titleText) {
        return;
    }

    m_dialog->setCaption(text);
    m_titleText = text;
    emit titleTextChanged();
}

QString KDialogProxy::titleText() const
{
    return m_titleText;
}


void KDialogProxy::setTitleIcon(const QString &icon)
{
    if (icon == m_titleIcon) {
        return;
    }

    m_dialog->setWindowIcon(KIcon(icon));
    m_titleIcon = icon;
    emit titleIconChanged();
}

QString KDialogProxy::titleIcon() const
{
    return m_titleIcon;
}


void KDialogProxy::setMessage(const QString &message)
{
    if (message == m_message) {
        return;
    }

    m_label->setText(message);
    m_message = message;
    emit messageChanged();
}

QString KDialogProxy::message() const
{
    return m_message;
}


void KDialogProxy::setAcceptButtonText(const QString &text)
{
    if (text == m_acceptButtonText) {
        return;
    }

    m_dialog->setButtonText(KDialog::Ok, text);
    m_acceptButtonText = text;
    emit acceptButtonTextChanged();
}

QString KDialogProxy::acceptButtonText() const
{
    return m_acceptButtonText;
}


void KDialogProxy::setRejectButtonText(const QString &text)
{
    if (text == m_rejectButtonText) {
        return;
    }

    m_dialog->setButtonText(KDialog::Cancel, text);
    m_rejectButtonText = text;
    emit rejectButtonTextChanged();
}

QString KDialogProxy::rejectButtonText() const
{
    return m_rejectButtonText;
}


DialogStatus::Status KDialogProxy::status() const
{
    return m_status;
}

void KDialogProxy::open()
{
    m_dialog->show();
    m_status = DialogStatus::Open;
    emit statusChanged();
}

void KDialogProxy::close()
{
    m_dialog->hide();
    m_status = DialogStatus::Closed;
    emit statusChanged();
}

#include "kdialogproxy.moc"

