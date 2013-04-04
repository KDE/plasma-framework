/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
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

#ifndef TOOLTIP_WINDOW_P
#define TOOLTIP_WINDOW_P

#include <QTimer>
#include <QQuickWindow>
#include <QWeakPointer>
#include <QtCore/QVariant>

class QQuickItem;
class QGraphicsWidget;

/**
 * QML wrapper for kdelibs Plasma::ToolTip
 *
 * Exposed as `ToolTip` in QML.
 */
class ToolTipWindow : public QQuickWindow
{
    Q_OBJECT

    /**
     * The item that will display this tooltip on mouse over 
     */
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)

    /**
     * The title of the tooltip, not more that 2-3 words
     */
    Q_PROPERTY(QString mainText READ mainText WRITE setMainText NOTIFY mainTextChanged)

    /**
     * subtitle of the tooltip. needed if a longer description is needed
     */
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)

    /**
     * Image to display in the tooltip, can be an image full path or a Freedesktop icon name or QIcon or QPixmap
     */
    Q_PROPERTY(QVariant image READ image WRITE setImage NOTIFY imageChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    /**
     * Visibility of the Dialog window. Doesn't have anything to do with the visibility of the mainItem.
     */
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)


public:
    ToolTipWindow(QWindow *parent = 0);
    ~ToolTipWindow();

    QQuickItem *target() const;
    void setTarget(QQuickItem *target);

    QString mainText() const;
    void setMainText(const QString &text);

    QString subText() const;
    void setSubText(const QString &text);

    QVariant image() const;
    void setImage(QVariant name);

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    bool isVisible() const;
    void setVisible(const bool visible);

    QPoint popupPosition(QQuickItem *item = 0, Qt::AlignmentFlag alignment=Qt::AlignCenter) ;

Q_SIGNALS:
    void targetChanged();
    void mainTextChanged();
    void subTextChanged();
    void imageChanged();
    void mainItemChanged();
    void visualParentChanged();
    void visibleChanged();

protected Q_SLOTS:
    void syncGeometry();
    void updateToolTip();

private:
    QTimer *m_syncTimer;
    QString m_mainText;
    QString m_subText;
    QVariant m_image;
    QGraphicsWidget *m_widget;
    QWeakPointer<QQuickItem> m_mainItem;
    QWeakPointer<QQuickItem> m_visualParent;
    QWeakPointer<QQuickItem> m_declarativeItemContainer;
    QWeakPointer<QQuickItem> m_target;
};

#endif
