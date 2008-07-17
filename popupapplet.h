/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef PLASMAAPPLETDIALOG_H
#define PLASMAAPPLETDIALOG_H

#include <plasma/applet.h>
#include <plasma/plasma_export.h>

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

namespace Plasma
{

class Dialog;
class Icon;
class PopupAppletPrivate;

class PLASMA_EXPORT PopupApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    PopupApplet(QObject *parent, const QVariantList &args);
    ~PopupApplet();

    void setIcon(const QIcon &icon);
    void setIcon(const QString &iconName);
    QIcon icon() const;

    virtual QWidget *widget() = 0;

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void showPopup();
    void hidePopup();

private:
    Q_PRIVATE_SLOT(d, void togglePopup())
    PopupAppletPrivate * const d;
};

} // Plasma namespace

#endif /* PLASMAAPPLETDIALOG_H */

