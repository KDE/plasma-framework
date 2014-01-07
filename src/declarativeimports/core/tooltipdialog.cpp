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

#include "tooltipdialog.h"

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>

#include "framesvgitem.h"

#include <kdeclarative/qmlobject.h>

Q_GLOBAL_STATIC(ToolTipDialog, toolTipDialogInstance)

ToolTipDialog::ToolTipDialog(QQuickItem  *parent)
    : DialogProxy(parent),
      m_qmlObject(0)
{
    setFlags(Qt::ToolTip);
    setLocation(Plasma::Types::Floating);
    m_frameSvgItem->setImagePath("widgets/tooltip");

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, [=]() {
        setVisible(false);
    });
}

ToolTipDialog::~ToolTipDialog()
{
}

QQuickItem *ToolTipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new QmlObject(this);
    }

    if (!m_qmlObject->rootObject()) {
        //HACK: search our own import
        foreach (const QString &path, m_qmlObject->engine()->importPathList()) {
            if (QFile::exists(path + "/org/kde/plasma/core")) {
                m_qmlObject->setSource(QUrl::fromLocalFile(path + "/org/kde/plasma/core/private/DefaultToolTip.qml"));
                break;
            }
        }
    }

    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

void ToolTipDialog::showEvent(QShowEvent *event)
{
    m_showTimer->start(4000);

    DialogProxy::showEvent(event);
}

ToolTipDialog* ToolTipDialog::instance()
{
    return toolTipDialogInstance();
}



