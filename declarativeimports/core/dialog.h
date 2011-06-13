/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
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
#ifndef DIALOG_PROXY_P
#define DIALOG_PROXY_P

#include <QObject>
#include <QWeakPointer>
#include <QPoint>

class QGraphicsObject;

namespace Plasma
{
    class Dialog;
}

class DeclarativeItemContainer;

class DialogMargins : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int left READ left NOTIFY leftChanged)
    Q_PROPERTY(int top READ top NOTIFY topChanged)
    Q_PROPERTY(int right READ right NOTIFY rightChanged)
    Q_PROPERTY(int bottom READ bottom NOTIFY bottomChanged)

public:
    DialogMargins(Plasma::Dialog *dialog, QObject *parent = 0);

    int left() const;
    int top() const;
    int right() const;
    int bottom() const;

Q_SIGNALS:
    void leftChanged();
    void rightChanged();
    void topChanged();
    void bottomChanged();

protected:
    void checkMargins();

private:
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;
    Plasma::Dialog *m_dialog;
    friend class DialogProxy;
};

class DialogProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsObject *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
    //to set the size try to force doing so from the inner item
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int windowFlags READ windowFlags WRITE setWindowFlags)
    Q_PROPERTY(QObject *margins READ margins CONSTANT)

public:
    enum WidgetAttribute {
        WA_X11NetWmWindowTypeDock = Qt::WA_X11NetWmWindowTypeDock
    };

    DialogProxy(QObject *parent = 0);
    ~DialogProxy();

    QGraphicsObject *mainItem() const;
    void setMainItem(QGraphicsObject *mainItem);

    bool isVisible() const;
    void setVisible(const bool visible);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int width() const;
    int height() const;

    //FIXME: passing an int is ugly
    int windowFlags() const;
    void setWindowFlags(const int);

    QObject *margins() const;

    //FIXME: alignment should be Qt::AlignmentFlag
    Q_INVOKABLE QPoint popupPosition(QGraphicsObject *item, int alignment=Qt::AlignLeft) const;
    //FIXME:: Qt::WidgetAttribute should be already 
    Q_INVOKABLE void setAttribute(int attribute, bool on);

Q_SIGNALS:
    void mainItemChanged();
    void visibleChanged();
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();

protected Q_SLOTS:
    void syncMainItem();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Plasma::Dialog *m_dialog;
    Qt::WindowFlags m_flags;
    DeclarativeItemContainer *m_declarativeItemContainer;
    QWeakPointer<QGraphicsObject> m_mainItem;
    DialogMargins *m_margins;
};

#endif
