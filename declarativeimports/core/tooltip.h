/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
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

#ifndef TOOLTIP_PROXY_P
#define TOOLTIP_PROXY_P

#include <QObject>
#include <QWeakPointer>

class QGraphicsObject;
class QGraphicsWidget;
class DeclarativeItemContainer;

class ToolTipProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsObject *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QString mainText READ mainText WRITE setMainText NOTIFY mainTextChanged)
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)

public:
    ToolTipProxy(QObject *parent = 0);
    ~ToolTipProxy();

    QGraphicsObject *target() const;
    void setTarget(QGraphicsObject *target);

    QString mainText() const;
    void setMainText(const QString &text);

    QString subText() const;
    void setSubText(const QString &text);

    // SHOULD BE PIXMAP OR QICON
    QString image() const;
    void setImage(const QString &name);

Q_SIGNALS:
    void targetChanged();
    void mainTextChanged();
    void subTextChanged();
    void imageChanged();

protected Q_SLOTS:
    void syncTarget();
    void updateToolTip();

private:
    QString m_mainText;
    QString m_subText;
    QString m_image;
    DeclarativeItemContainer *m_declarativeItemContainer;
    QGraphicsWidget *m_widget;
    QWeakPointer<QGraphicsObject> m_target;
};

#endif
