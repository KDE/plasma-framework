/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>
                   Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef KWINDOWLISTMENU_H
#define KWINDOWLISTMENU_H

#include <kdemacros.h>
#include <kmenu.h>

/**
 * This class provides a menu which contains actions
 * to manage a window. It is used by the window manager
 * for example an accessable there via the menu button of
 * the window decoration.
 */
class KDE_EXPORT KWindowListMenu : public KMenu
{
    Q_OBJECT

  public:
    /**
     * Creates a new window list menu.
     *
     * @param parent The parent widget.
     */
    explicit KWindowListMenu( QWidget *parent = 0 );

    /**
     * Destroys the window list menu.
     */
    virtual ~KWindowListMenu();

    /**
     * Initializes the menu by filling it with actions
     * for managing a window.
     */
    void init();

  public Q_SLOTS:
    /**
     * Pre-selects the active window in the popup menu, for faster
     * keyboard navigation. Needs to be called after popup().
     * Should not be used when the popup is invoked using the mouse. 
     */
    void selectActiveWindow();

  protected Q_SLOTS:
    void slotForceActiveWindow();
    void slotSetCurrentDesktop();
    void slotUnclutterWindows();
    void slotCascadeWindows();

  private:
    class Private;
    Private* const d;
};

#endif
