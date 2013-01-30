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

#include "fullscreensheet.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>

#include <KDebug>

FullScreenSheet::FullScreenSheet(QDeclarativeItem *parent)
    : FullScreenWindow(parent)
{
    init("Sheet");

    view()->setGeometry(QApplication::desktop()->availableGeometry());

    if (mainItem()) {
        connect(mainItem(), SIGNAL(titleChanged()),
                this, SIGNAL(titleChanged()));
        connect(mainItem(), SIGNAL(acceptButtonChanged()),
                this, SIGNAL(acceptButtonChanged()));
        connect(mainItem(), SIGNAL(rejectButtonChanged()),
                this, SIGNAL(rejectButtonChanged()));
        connect(mainItem(), SIGNAL(acceptButtonTextChanged()),
                this, SIGNAL(acceptButtonTextChanged()));
        connect(mainItem(), SIGNAL(rejectButtonTextChanged()),
                this, SIGNAL(rejectButtonTextChanged()));
    }
}

FullScreenSheet::~FullScreenSheet()
{
}

QString FullScreenSheet::title() const
{
    if (mainItem()) {
        return mainItem()->property("title").toString();
    } else {
        return 0;
    }
}

void FullScreenSheet::setTitle(const QString &text)
{
    if (mainItem()) {
        mainItem()->setProperty("title", QVariant::fromValue(text));
    }
}

QDeclarativeItem *FullScreenSheet::acceptButton() const
{
    if (mainItem()) {
        return mainItem()->property("acceptButton").value<QDeclarativeItem *>();
    } else {
        return 0;
    }
}

void FullScreenSheet::setAcceptButton(QDeclarativeItem *button)
{
    if (mainItem()) {
        mainItem()->setProperty("acceptButton", QVariant::fromValue(button));
    }
}

QDeclarativeItem *FullScreenSheet::rejectButton() const
{
    if (mainItem()) {
        return mainItem()->property("rejectButton").value<QDeclarativeItem *>();
    } else {
        return 0;
    }
}

void FullScreenSheet::setRejectButton(QDeclarativeItem *button)
{
    if (mainItem()) {
        mainItem()->setProperty("rejectButton", QVariant::fromValue(button));
    }
}

QString FullScreenSheet::acceptButtonText() const
{
    if (mainItem()) {
        return mainItem()->property("acceptButtonText").toString();
    } else {
        return 0;
    }
}

void FullScreenSheet::setAcceptButtonText(const QString &text)
{
    if (mainItem()) {
        mainItem()->setProperty("acceptButtonText", QVariant::fromValue(text));
    }
}

QString FullScreenSheet::rejectButtonText() const
{
    if (mainItem()) {
        return mainItem()->property("rejectButtonText").toString();
    } else {
        return 0;
    }
}

void FullScreenSheet::setRejectButtonText(const QString &text)
{
    if (mainItem()) {
        mainItem()->setProperty("rejectButtonText", QVariant::fromValue(text));
    }
}

void FullScreenSheet::open()
{
    if (mainItem()) {
        view()->setGeometry(QApplication::desktop()->availableGeometry());
        QMetaObject::invokeMethod(mainItem(), "open");
    }
}


#include "fullscreensheet.moc"

