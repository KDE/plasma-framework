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
     * Visibility of the Dialog window. Doesn't have anything to do with the visibility of the mainItem.
     */
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

    /**
     * Window flags of the Dialog window
     */
    Q_PROPERTY(int windowFlags READ windowFlags WRITE setWindowFlags)

    /**
     * Margins of the dialog around the mainItem.
     * @see DialogMargins
     */
    Q_PROPERTY(QObject *margins READ margins CONSTANT)

    /**
     * True if the dialog window is the active one in the window manager.
     */
    Q_PROPERTY(bool activeWindow READ isActiveWindow NOTIFY activeWindowChanged)

    /**
     * Plasma Location of the dialog window. Useful if this dialog is apopup for a panel
     */
    Q_PROPERTY(int location READ location WRITE setLocation NOTIFY locationChanged)
//This won't be available on windows, but should be used only by kwin and never by applets anyways
#ifndef Q_WS_WIN
    /**
     * Window ID of the dialog window.
     **/
    Q_PROPERTY(qulonglong windowId READ windowId CONSTANT)
#endif

    Q_CLASSINFO("DefaultProperty", "mainItem")

public:
    enum WidgetAttribute {
        WA_X11NetWmWindowTypeDock = Qt::WA_X11NetWmWindowTypeDock
    };

    DialogProxy(QQuickItem *parent = 0);
    ~DialogProxy();

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    bool isVisible() const;
    void setVisible(const bool visible);

    bool isActiveWindow() const;

    /**
     * Ask the window manager to activate the window.
     * The window manager may or may not accept the activation request
     */
    Q_INVOKABLE void activateWindow();

    //FIXME: passing an int is ugly
    int windowFlags() const;
    void setWindowFlags(const int);

    int location() const;
    void setLocation(int location);

    QObject *margins() const;

#ifndef Q_WS_WIN
    qulonglong windowId() const;
#endif

    /**
     * @returns The suggested screen position for the popup
     * @arg item the item the popup has to be positioned relatively to. if null, the popup will be positioned in the center of the window
     * @arg alignment alignment of the popup compared to the item
     */
    QPoint popupPosition(QQuickItem *item, Qt::AlignmentFlag alignment=Qt::AlignCenter) ;

    /**
     * Set a Qt.WidgetAttribute to the dialog window
     *
     * @arg int attribute see Qt.WidgetAttribute
     * @arg bool on activate or deactivate the atrtibute
     */
    //FIXME:: Qt::WidgetAttribute should be already 
    Q_INVOKABLE void setAttribute(int attribute, bool on);

Q_SIGNALS:
    void mainItemChanged();
    void visibleChanged();
    void activeWindowChanged();
    void locationChanged();
    void visualParentChanged();

private Q_SLOTS:
    void syncMainItemToSize();
    void syncToMainItemSize();

protected:
   // bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *re);
    void focusInEvent(QFocusEvent *ev);
    void focusOutEvent(QFocusEvent *ev);

private:
    Qt::WindowFlags m_flags;
    QTimer *m_syncTimer;
    QWeakPointer<QQuickItem> m_mainItem;
    QWeakPointer<QQuickItem> m_visualParent;
    bool m_activeWindow;
    Plasma::Location m_location;
    Plasma::FrameSvgItem *m_frameSvgItem;
    QRect m_cachedGeometry;
};

#endif
