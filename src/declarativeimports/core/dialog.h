/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
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
#ifndef DIALOG_PROXY_P
#define DIALOG_PROXY_P

#include <QQuickItem>
#include <QQuickWindow>
#include <QWeakPointer>
#include <QPoint>

#include <Plasma/Plasma>

#include <netwm_def.h>

class QQuickItem;

namespace Plasma
{
    class FrameSvgItem;
}


/**
 * QML wrapper for dialogs
 *
 * Exposed as `PlasmaCore.Dialog` in QML.
 */
class DialogProxy : public QQuickWindow
{
    Q_OBJECT

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    /**
     * Margins of the dialog around the mainItem.
     * @see DialogMargins
     */
    Q_PROPERTY(QObject *margins READ margins CONSTANT)

    /**
     * Plasma Location of the dialog window. Useful if this dialog is a popup for a panel
     */
    Q_PROPERTY(Plasma::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)

    /**
     * Type of the window
     */
    Q_PROPERTY(WindowType type READ type WRITE setType NOTIFY typeChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * Whether the dialog is output only. Default value is @c false. If it is @c true
     * the dialog does not accept input and all pointer events are not accepted, thus the dialog
     * is click through.
     *
     * This property is currently only supported on the X11 platform. On any other platform the
     * property has no effect.
     **/
    Q_PROPERTY(bool outputOnly READ isOutputOnly WRITE setOutputOnly NOTIFY outputOnlyChanged)

    Q_CLASSINFO("DefaultProperty", "mainItem")

public:
    enum WindowType {
        Normal = NET::Normal,
        Dock = NET::Dock,
        Dialog = NET::Dialog,
        PopupMenu = NET::PopupMenu,
        Tooltip = NET::Tooltip,
        Notification = NET::Notification
    };
    Q_ENUMS(WindowType)

    DialogProxy(QQuickItem *parent = 0);
    ~DialogProxy();

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    Plasma::Types::Location location() const;
    void setLocation(Plasma::Types::Location location);

    QObject *margins() const;

    /*
     * set the dialog position. subclasses may change it. ToolTipDialog adjusts the position in an animated way
     */
    virtual void adjustGeometry(const QRect &geom);

    /**
     * @returns The suggested screen position for the popup
     * @arg item the item the popup has to be positioned relatively to. if null, the popup will be positioned in the center of the window
     * @arg alignment alignment of the popup compared to the item
     */
    QPoint popupPosition(QQuickItem *item, const QSize &size, Qt::AlignmentFlag alignment=Qt::AlignCenter) ;

    void setType(WindowType type);
    WindowType type() const;
    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    void setOutputOnly(bool outputOnly);
    bool isOutputOnly() const;

    void updateInputShape();

Q_SIGNALS:
    void mainItemChanged();
    void locationChanged();
    void visualParentChanged();
    void typeChanged();
    void hideOnWindowDeactivateChanged();
    void outputOnlyChanged();

public Q_SLOTS:
    void syncMainItemToSize();
    void syncToMainItemSize();

protected:
   // bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *re);
    void focusInEvent(QFocusEvent *ev);
    void focusOutEvent(QFocusEvent *ev);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool event(QEvent *event);

    QTimer *m_syncTimer;
    Plasma::Types::Location m_location;
    Plasma::FrameSvgItem *m_frameSvgItem;
    QWeakPointer<QQuickItem> m_mainItem;
    QWeakPointer<QQuickItem> m_visualParent;

private Q_SLOTS:
    void syncBorders();

    void updateVisibility(bool visible);

private:
    QRect m_cachedGeometry;
    WindowType m_type;
    bool m_hideOnWindowDeactivate;
    bool m_outputOnly;
};

#endif
