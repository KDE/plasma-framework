/*****************************************************************

Copyright (c) 2000-2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 Richard Moore <rich@kde.org>

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

#ifndef TASK_H
#define TASK_H

// Qt
#include <QtGui/QDrag>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>
#include <QtGui/QIcon>

// KDE
#include <KDE/KSharedPtr>
#include <KDE/KWindowSystem>
#include <KDE/NETWinInfo>

#include <taskmanager/taskmanager.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

/**
 * A dynamic interface to a task (main window).
 *
 * @see TaskManager
 */
class TASKMANAGER_EXPORT Task : public QObject, public KShared
{
    Q_OBJECT
    Q_PROPERTY( QString visibleName READ visibleName )
    Q_PROPERTY( QString name READ name )
    Q_PROPERTY( QString className READ className )
    Q_PROPERTY( QString visibleNameWithState READ visibleNameWithState )
    Q_PROPERTY( QPixmap pixmap READ pixmap )
    Q_PROPERTY( bool maximized READ isMaximized )
    Q_PROPERTY( bool minimized READ isMinimized )
    // KDE4 deprecated
    Q_PROPERTY( bool iconified READ isIconified )
    Q_PROPERTY( bool shaded READ isShaded WRITE setShaded )
    Q_PROPERTY( bool active READ isActive )
    Q_PROPERTY( bool onCurrentDesktop READ isOnCurrentDesktop )
    Q_PROPERTY( bool onAllDesktops READ isOnAllDesktops )
    Q_PROPERTY( bool alwaysOnTop READ isAlwaysOnTop WRITE setAlwaysOnTop )
    Q_PROPERTY( bool modified READ isModified )
    Q_PROPERTY( bool demandsAttention READ demandsAttention )
    Q_PROPERTY( int desktop READ desktop )
    Q_PROPERTY( double thumbnailSize READ thumbnailSize WRITE setThumbnailSize )
    Q_PROPERTY( bool hasThumbnail READ hasThumbnail )
    Q_PROPERTY( QPixmap thumbnail READ thumbnail )

public:
    Task(WId win, QObject *parent, const char *name = 0);
    virtual ~Task();

    WId window() const;
    KWindowInfo info() const;

    QString visibleName() const;
    QString visibleNameWithState() const;
    QString name() const;
    QString className() const;
    QString classClass() const;

    /**
     * A list of the window ids of all transient windows (dialogs) associated
     * with this task.
     */
    WindowList transients() const;

    /**
     * Returns a 16x16 (KIconLoader::Small) icon for the task. This method will
     * only fall back to a static icon if there is no icon of any size in
     * the WM hints.
     */
    QPixmap pixmap() const;

    /**
     * Returns the best icon for any of the KIconLoader::StdSizes. If there is no
     * icon of the specified size specified in the WM hints, it will try to
     * get one using KIconLoader.
     *
     * <pre>
     *   bool gotStaticIcon;
     *   QPixmap icon = myTask->icon( KIconLoader::SizeMedium, gotStaticIcon );
     * </pre>
     *
     * @param size Any of the constants in KIconLoader::StdSizes.
     * @param isStaticIcon Set to true if KIconLoader was used, false otherwise.
     */
    QPixmap bestIcon( int size, bool &isStaticIcon );

    /**
     * Tries to find an icon for the task with the specified size. If there
     * is no icon that matches then it will either resize the closest available
     * icon or return a null pixmap depending on the value of allowResize.
     *
     * Note that the last icon is cached, so a sequence of calls with the same
     * parameters will only query the NET properties if the icon has changed or
     * none was found.
     */
    QPixmap icon( int width, int height, bool allowResize = false );

    /**
     * \return a QIcon for the task
     */
    QIcon icon();

    /**
     * Returns true iff the windows with the specified ids should be grouped
     * together in the task list.
     */
    static bool idMatch(const QString &, const QString &);

    // state

    /**
     * Returns true if the task's window is maximized.
     */
    bool isMaximized() const;

    /**
     * Returns true if the task's window is minimized.
     */
    bool isMinimized() const;

    /**
     * @deprecated
     * Returns true if the task's window is minimized(iconified).
     */
    bool isIconified() const;

    /**
     * Returns true if the task's window is shaded.
     */
    bool isShaded() const;

    /**
     * Returns true if the task's window is the active window.
     */
    bool isActive() const;

    /**
     * Returns true if the task's window is the topmost non-iconified,
     * non-always-on-top window.
     */
    bool isOnTop() const;

    /**
     * Returns true if the task's window is on the current virtual desktop.
     */
    bool isOnCurrentDesktop() const;

    /**
     * Returns true if the task's window is on all virtual desktops.
     */
    bool isOnAllDesktops() const;

    /**
     * Returns true if the task's window will remain at the top of the
     * stacking order.
     */
    bool isAlwaysOnTop() const;

    /**
     * Returns true if the task's window will remain at the bottom of the
     * stacking order.
     */
    bool isKeptBelowOthers() const;

    /**
     * Returns true if the task's window is in full screen mode
     */
    bool isFullScreen() const;

    /**
     * Returns true if the document the task is editing has been modified.
     * This is currently handled heuristically by looking for the string
     * '[i18n_modified]' in the window title where i18n_modified is the
     * word 'modified' in the current language.
     */
    bool isModified() const ;

    /**
     * Returns the desktop on which this task's window resides.
     */
    int desktop() const;

    /**
     * Returns true if the task is not active but demands user's attention.
     */
    bool demandsAttention() const;


    /**
    * Returns true if the window is on the specified screen of a multihead configuration
    */
    bool isOnScreen( int screen ) const;

    /**
     * Returns true if the task should be shown in taskbar-like apps
     */
    bool showInTaskbar() const;

    /**
     * Returns true if the task should be shown in pager-like apps
     */
    bool showInPager() const;

    /**
     * Returns the geometry for this window
     */
    QRect geometry() const;

    // internal

    //* @internal
    ::TaskManager::TaskChanges refresh(unsigned int dirty);
    //* @internal
    void refreshIcon();
    //* @internal
    void addTransient( WId w, const NETWinInfo& info );
    //* @internal
    void removeTransient( WId w );
    //* @internal
    bool hasTransient(WId w) const;
    //* @internal
    void updateDemandsAttentionState( WId w );
    //* @internal
    void setActive(bool a);

    // For thumbnails

    /**
     * Returns the current thumbnail size.
     */
    double thumbnailSize() const;

    /**
     * Sets the size for the window thumbnail. For example a size of
     * 0.2 indicates the thumbnail will be 20% of the original window
     * size.
     */
    void setThumbnailSize( double size );

    /**
     * Returns true if this task has a thumbnail. Note that this method
     * can only ever return true after a call to updateThumbnail().
     */
    bool hasThumbnail() const;

    /**
     * Returns the thumbnail for this task (or a null image if there is
     * none).
     */
    QPixmap thumbnail() const;

    QPixmap thumbnail(int maxDimension);

    void updateWindowPixmap();

public Q_SLOTS:
    // actions

    /**
     * Maximise the main window of this task.
     */
    void setMaximized(bool);
    void toggleMaximized();

    /**
     * Restore the main window of the task (if it was iconified).
     */
    void restore();

    /**
     * Move the window of this task.
     */
    void move();

    /**
     * Resize the window of this task.
     */
    void resize();

    /**
     * Iconify the task.
     */
    void setIconified(bool);
    void toggleIconified();

    /**
     * Close the task's window.
     */
    void close();

    /**
     * Raise the task's window.
     */
    void raise();

    /**
     * Lower the task's window.
     */
    void lower();

   /**
     * Activate the task's window.
     */
    void activate();

    /**
     * Perform the action that is most appropriate for this task. If it
     * is not active, activate it. Else if it is not the top window, raise
     * it. Otherwise, iconify it.
     */
    void activateRaiseOrIconify();

    /**
     * If true, the task's window will remain at the top of the stacking order.
     */
    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();

    /**
     * If true, the task's window will remain at the bottom of the stacking order.
     */
    void setKeptBelowOthers(bool);
    void toggleKeptBelowOthers();

    /**
     * If true, the task's window will enter full screen mode.
     */
    void setFullScreen(bool);
    void toggleFullScreen();

    /**
     * If true then the task's window will be shaded. Most window managers
     * represent this state by displaying on the window's title bar.
     */
    void setShaded(bool);
    void toggleShaded();

    /**
     * Moves the task's window to the specified virtual desktop.
     */
    void toDesktop(int);

    /**
     * Moves the task's window to the current virtual desktop.
     */
    void toCurrentDesktop();

    /**
     * This method informs the window manager of the location at which this
     * task will be displayed when iconised. It is used, for example by the
     * KWin inconify animation.
     */
    void publishIconGeometry(QRect);

Q_SIGNALS:
    /**
     * Indicates that this task has changed in some way.
     */
    void changed(::TaskManager::TaskChanges change);

    /**
     * Indicates that this task is now the active task.
     */
    void activated();

    /**
     * Indicates that this task is no longer the active task.
     */
    void deactivated();

protected:
    void findWindowFrameId();

private:
    class Private;
    Private * const d;
};


/**
 * Provids a drag object for tasks across desktops.
 * FIXME: should be folded into the Task class the same way it has been with
 *        AppletInfo and KUrl
 */
class TASKMANAGER_EXPORT TaskDrag : public QDrag
{
public:
    /**
     * Constructs a task drag object for a task list.
     */
    explicit TaskDrag(const TaskList& tasks, QWidget* source = 0);
    ~TaskDrag();

    /**
     * Returns true if the mime source can be decoded to a TaskDrag.
     */
    static bool canDecode( const QMimeData* e );

    /**
     * Decodes the tasks from the mime source and returns them if successful.
     * Otherwise an empty task list is returned.
     */
    static TaskList decode( const QMimeData* e );

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace

#endif
