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
    class Dialog;
}

class DeclarativeItemContainer;

class DialogMargins : public QObject
{
    Q_OBJECT

    /**
     * Width in pixels of the left margin
     */
    Q_PROPERTY(int left READ left NOTIFY leftChanged)

    /**
     * Height in pixels of the top margin
     */
    Q_PROPERTY(int top READ top NOTIFY topChanged)

    /**
     * Width in pixels of the right margin
     */
    Q_PROPERTY(int right READ right NOTIFY rightChanged)

    /**
     * Height in pixels of the bottom margin
     */
    Q_PROPERTY(int bottom READ bottom NOTIFY bottomChanged)

public:
    DialogMargins(QQuickWindow *dialog, QObject *parent = 0);

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
    QQuickWindow *m_dialog;
    friend class DialogProxy;
};

/**
 * QML wrapper for kdelibs Plasma::Dialog
 *
 * Exposed as `Dialog` in QML.
 */
class DialogProxy : public QQuickWindow
{
    Q_OBJECT

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

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

    bool isVisible() const;
    void setVisible(const bool visible);

    bool isActiveWindow() const;

    /**
     * Ask the window manager to activate the window.
     * The window manager may or may not accept the activiation request
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
    //FIXME: alignment should be Qt::AlignmentFlag
    Q_INVOKABLE QPoint popupPosition(QQuickItem *item, int alignment=Qt::AlignLeft) ;

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

private Q_SLOTS:
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
    DialogMargins *m_margins;
    bool m_activeWindow;
    Plasma::Location m_location;
};

#endif
