/***************************************************************************
 *   Copyright 2012 Marco Martin <mart@kde.org>                            *
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
#ifndef FULLSCREENSHEET_P
#define FULLSCREENSHEET_P

#include "fullscreenwindow.h"

class FullScreenSheet : public FullScreenWindow
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsObject *acceptButton READ acceptButton WRITE setAcceptButton NOTIFY acceptButtonChanged)
    Q_PROPERTY(QGraphicsObject *rejectButton READ rejectButton WRITE setRejectButton NOTIFY rejectButtonChanged)

    Q_PROPERTY(QString acceptButtonText READ acceptButtonText WRITE setAcceptButtonText NOTIFY acceptButtonTextChanged)
    Q_PROPERTY(QString rejectButtonText READ rejectButtonText WRITE setRejectButtonText NOTIFY rejectButtonTextChanged)

public:
    FullScreenSheet(QDeclarativeItem *parent = 0);
    ~FullScreenSheet();

    QGraphicsObject *acceptButton() const;
    void setAcceptButton(QGraphicsObject *button);

    QGraphicsObject *rejectButton() const;
    void setRejectButton(QGraphicsObject *button);

    QString acceptButtonText() const;
    void setAcceptButtonText(const QString &text);

    QString rejectButtonText() const;
    void setRejectButtonText(const QString &text);

Q_SIGNALS:
    void acceptButtonChanged();
    void rejectButtonChanged();
    void acceptButtonTextChanged();
    void rejectButtonTextChanged();
};

#endif
