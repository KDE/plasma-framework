/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *   Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>                    *
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

#include "dialog.h"
#include "framesvgitem.h"
#include "dialogshadows_p.h"

#include <QApplication>
#include <QQuickItem>
#include <QDesktopWidget>
#include <QTimer>
#include <QLayout>
#include <QScreen>

#include <kwindowsystem.h>
#include <KWindowSystem/KWindowInfo>

#include <kwindoweffects.h>
#include <Plasma/Plasma>
#include <Plasma/Corona>

#include <QDebug>

#include <config-plasma.h>
#if HAVE_XCB_SHAPE
#include <QX11Info>
#include <xcb/shape.h>
#endif

//Unfortunately QWINDOWSIZE_MAX is not exported
#define DIALOGSIZE_MAX ((1<<24)-1)

class DialogPrivate {
public:
    DialogPrivate(Dialog *dialog)
        : location(Plasma::Types::BottomEdge),
          type(Dialog::Normal),
          hideOnWindowDeactivate(false),
          outputOnly(false),
          componentComplete(dialog->parent() == 0)
    {
    }

    QScreen* screenForItem(QQuickItem *item) const;

    Dialog *q;
    QTimer *syncTimer;
    Plasma::Types::Location location;
    Plasma::FrameSvgItem *frameSvgItem;
    QWeakPointer<QQuickItem> mainItem;
    QWeakPointer<QQuickItem> visualParent;

    QRect cachedGeometry;
    Dialog::WindowType type;
    bool hideOnWindowDeactivate;
    bool outputOnly;
    Plasma::Theme theme;
    bool componentComplete;

    //Attached Layout property of mainItem, if any
    QWeakPointer <QObject> mainItemLayout;
};

//find the screen which contains the item
QScreen* DialogPrivate::screenForItem(QQuickItem* item) const
{
    const QPoint globalPosition = item->window()->mapToGlobal(item->position().toPoint());
    foreach(QScreen *screen, QGuiApplication::screens()) {
        if (screen->geometry().contains(globalPosition)) {
            return screen;
        }
    }
    return QGuiApplication::primaryScreen();
}



Dialog::Dialog(QQuickItem *parent)
    : QQuickWindow(parent ? parent->window() : 0),
      d(new DialogPrivate(this))
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);

    d->syncTimer = new QTimer(this);
    d->syncTimer->setSingleShot(true);
    d->syncTimer->setInterval(0);
    connect(d->syncTimer, &QTimer::timeout, this,  &Dialog::syncToMainItemSize);

    connect(this, &QWindow::xChanged, [=]() {
        requestSyncToMainItemSize(true);
    });
    connect(this, &QWindow::yChanged, [=]() {
        requestSyncToMainItemSize(true);
    });
    connect(this, &QWindow::visibleChanged, this, &Dialog::updateInputShape);
    connect(this, &Dialog::outputOnlyChanged, this, &Dialog::updateInputShape);
//    connect(this, &QWindow::visibleChanged, this, &Dialog::onVisibleChanged);
    //HACK: this property is invoked due to the initialization that gets done to contentItem() in the getter
    property("data");
    //Create the FrameSvg background.
    d->frameSvgItem = new Plasma::FrameSvgItem(contentItem());
    d->frameSvgItem->setImagePath("dialogs/background");

    connect(&d->theme, &Plasma::Theme::themeChanged, this, &Dialog::updateContrast);

    //d->frameSvgItem->setImagePath("widgets/background"); // larger borders, for testing those
}

Dialog::~Dialog()
{
    if (!qApp->closingDown()) {
        DialogShadows::self()->removeWindow(this);
    }
}

QQuickItem *Dialog::mainItem() const
{
    return d->mainItem.data();
}

void Dialog::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem.data() != mainItem) {
        if (d->mainItem) {
            d->mainItem.data()->setParent(parent());
        }

        d->mainItem = mainItem;

        if (mainItem) {
            mainItem->setParent(contentItem());
            mainItem->setProperty("parent", QVariant::fromValue(contentItem()));

            if (mainItem->metaObject()->indexOfSignal("widthChanged")) {
                connect(mainItem, &QQuickItem::widthChanged, [=]() {
                    d->syncTimer->start(0);
                });
            }
            if (mainItem->metaObject()->indexOfSignal("heightChanged")) {
                connect(mainItem, &QQuickItem::heightChanged, [=]() {
                    d->syncTimer->start(0);
                });
            }
            requestSyncToMainItemSize();

            //Extract the representation's Layout, if any
            QObject *layout = 0;

            //Search a child that has the needed Layout properties
            //HACK: here we are not type safe, but is the only way to access to a pointer of Layout
            foreach (QObject *child, mainItem->children()) {
                //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
                if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
                    child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
                    child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
                    child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
                ) {
                    layout = child;
                }
            }
            d->mainItemLayout = layout;

            if (layout) {
                connect(layout, SIGNAL(minimumWidthChanged()), this, SLOT(updateMinimumWidth()));
                connect(layout, SIGNAL(minimumHeightChanged()), this, SLOT(updateMinimumHeight()));
                connect(layout, SIGNAL(maximumWidthChanged()), this, SLOT(updateMaximumWidth()));
                connect(layout, SIGNAL(maximumHeightChanged()), this, SLOT(updateMaximumHeight()));

                updateMinimumWidth();
                updateMinimumHeight();
                updateMaximumWidth();
                updateMaximumHeight();
            }

        }

        //if this is called in Component.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

QQuickItem *Dialog::visualParent() const
{
    return d->visualParent.data();
}

void Dialog::setVisualParent(QQuickItem *visualParent)
{
    if (d->visualParent.data() == visualParent) {
        return;
    }

    d->visualParent = visualParent;
    emit visualParentChanged();
    if (visualParent) {
        requestSyncToMainItemSize();
    }
}

void Dialog::updateVisibility(bool visible)
{
    if (visible) {
        if (location() == Plasma::Types::FullScreen) {
            d->frameSvgItem->setEnabledBorders(Plasma::FrameSvg::NoBorder);

            // We cache the original size of the item, to retrieve it
            // when the dialog is switched back from fullscreen.
            if (geometry() != screen()->availableGeometry()) {
                d->cachedGeometry = geometry();
            }
            setGeometry(screen()->availableGeometry());
        } else {
            if (!d->cachedGeometry.isNull()) {
                resize(d->cachedGeometry.size());
                syncMainItemToSize();
                d->cachedGeometry = QRect();
            }
            syncToMainItemSize();
        }
    }

    if (!(flags() & Qt::ToolTip)) {
        KWindowEffects::SlideFromLocation slideLocation = KWindowEffects::NoEdge;

        switch (d->location) {
        case Plasma::Types::TopEdge:
            slideLocation = KWindowEffects::TopEdge;
            break;
        case Plasma::Types::LeftEdge:
            slideLocation = KWindowEffects::LeftEdge;
            break;
        case Plasma::Types::RightEdge:
            slideLocation = KWindowEffects::RightEdge;
            break;
        case Plasma::Types::BottomEdge:
            slideLocation = KWindowEffects::BottomEdge;
            break;
            //no edge, no slide
        default:
            break;
        }

        KWindowEffects::slideWindow(winId(), slideLocation, -1);
    }

    if (visible) {
        raise();

        if (d->type != Normal) {
            KWindowSystem::setType(winId(), (NET::WindowType)d->type);
        } else {
            setFlags(Qt::FramelessWindowHint|flags());
        }
        if (d->type == Dock) {
            KWindowSystem::setOnAllDesktops(winId(), true);
        } else {
            KWindowSystem::setOnAllDesktops(winId(), false);
        }
    }
}

QPoint Dialog::popupPosition(QQuickItem *item, const QSize &size, Qt::AlignmentFlag alignment)
{
    if (!item) {
        //If no item was specified try to align at the center of the parent view
        QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem) {
            QScreen *screen = d->screenForItem(parentItem);

            switch (d->location) {
            case Plasma::Types::TopEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width()/2, screen->availableGeometry().y());
                break;
            case Plasma::Types::LeftEdge:
                return QPoint(screen->availableGeometry().x(), screen->availableGeometry().center().y() - size.height()/2);
                break;
            case Plasma::Types::RightEdge:
                return QPoint(screen->availableGeometry().right() - size.width(), screen->availableGeometry().center().y() - size.height()/2);
                break;
            case Plasma::Types::BottomEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width()/2, screen->availableGeometry().bottom() -size.height());
                break;
                //Default center in the screen
            default:
                return screen->geometry().center() - QPoint(size.width()/2, size.height()/2);
            }
        } else {
            return QPoint();
        }
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));

    if (item->window()) {
        pos = item->window()->mapToGlobal(pos.toPoint());
    } else {
        return QPoint();
    }

    //swap direction if necessary
    if (QApplication::isRightToLeft() && alignment != Qt::AlignCenter) {
        if (alignment == Qt::AlignRight) {
            alignment = Qt::AlignLeft;
        } else {
            alignment = Qt::AlignRight;
        }
    }

    //if the item is in a dock or in a window that ignores WM we want to position the popups outside of the dock
    const KWindowInfo winInfo = KWindowSystem::windowInfo(item->window()->winId(), NET::WMWindowType);
    const bool outsideParentWindow = (winInfo.windowType(NET::AllTypesMask) == NET::Dock) || (item->window()->flags() & Qt::X11BypassWindowManagerHint);

    //flag shows if the popup should be placed inside or outside the parent item
    //i.e if the parent item is the desktop we want to position the dialog to the left edge of
    //the parent, not just outside the parent
    const bool locateInsideParent = winInfo.windowType(NET::AllTypesMask) == NET::Desktop;

    QRect parentGeometryBounds;
    if (outsideParentWindow) {
        parentGeometryBounds = item->window()->geometry();
    } else {
        parentGeometryBounds = QRect(pos.toPoint(), QSize(item->width(), item->height()));
    }

    if (locateInsideParent) {
        //pretend the parent is smaller so that positioning to the outside edge of the parent is
        //aligned on the inside edge of the real parent
        parentGeometryBounds.adjust(size.width(), size.height(), -size.width(), -size.height());
    }

    const QPoint topPoint(pos.x() + (item->boundingRect().width() - size.width())/2,
                                   parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(pos.x() + (item->boundingRect().width() - size.width())/2,
                             parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(),
                           pos.y() + (item->boundingRect().height() - size.height())/2);

    const QPoint rightPoint(parentGeometryBounds.right(),
                            pos.y() + (item->boundingRect().height() - size.height())/2);

    QPoint dialogPos;
    if (d->location == Plasma::Types::BottomEdge) {
        dialogPos = bottomPoint;
    } else if (d->location == Plasma::Types::LeftEdge) {
        dialogPos = leftPoint;
    } else if (d->location == Plasma::Types::RightEdge) {
        dialogPos = rightPoint;
    } else { // Types::TopEdge
        dialogPos = topPoint;
    }

    //find the correct screen for the item
    //we do not rely on item->window()->screen() because
    //QWindow::screen() is always only the screen where the window gets first created
    //not actually the current window. See QWindow::screen() documentation
    QRect avail = d->screenForItem(item)->availableGeometry();

    if (outsideParentWindow && d->frameSvgItem->enabledBorders() != Plasma::FrameSvg::AllBorders) {
        //make the panel look it's inside the panel, in order to not make it look cutted
        switch (d->location) {
        case Plasma::Types::LeftEdge:
        case Plasma::Types::RightEdge:
            avail.setTop(qMax(avail.top(), parentGeometryBounds.top()));
            avail.setBottom(qMin(avail.bottom(), parentGeometryBounds.bottom()));
            break;
        default:
            avail.setLeft(qMax(avail.left(), parentGeometryBounds.left()));
            avail.setRight(qMin(avail.right(), parentGeometryBounds.right()));
            break;
        }
    }

    if (dialogPos.x() < avail.left()) {
        // popup hits lhs
        if (d->location == Plasma::Types::TopEdge || d->location == Plasma::Types::BottomEdge) {
            // move it
            dialogPos.setX(avail.left());
        } else {
            // swap edge
            dialogPos.setX(rightPoint.x());
        }
    }
    if (dialogPos.x() + size.width() > avail.right()) {
        // popup hits rhs
        if (d->location == Plasma::Types::TopEdge || d->location == Plasma::Types::BottomEdge) {
            dialogPos.setX(avail.right() - size.width());
        } else {
            dialogPos.setX(leftPoint.x());
        }
    }
    if (dialogPos.y() < avail.top()) {
        // hitting top
        if (d->location == Plasma::Types::LeftEdge || d->location == Plasma::Types::RightEdge) {
            dialogPos.setY(avail.top());
        } else {
            dialogPos.setY(bottomPoint.y());
        }
    }

    if (dialogPos.y() + size.height() > avail.bottom()) {
        // hitting bottom
        if (d->location == Plasma::Types::TopEdge || d->location == Plasma::Types::BottomEdge) {
            dialogPos.setY(topPoint.y());
        } else {
            dialogPos.setY(avail.bottom() - size.height());
        }
    }

    return dialogPos;
}

Plasma::Types::Location Dialog::location() const
{
    return d->location;
}

void Dialog::setLocation(Plasma::Types::Location location)
{
    if (d->location == location) {
        return;
    }
    d->location = location;
    emit locationChanged();
    requestSyncToMainItemSize();
}


QObject *Dialog::margins() const
{
    return d->frameSvgItem->margins();
}

void Dialog::setFramelessFlags(Qt::WindowFlags flags)
{
    setFlags(Qt::FramelessWindowHint|flags);
    emit flagsChanged();
}

void Dialog::adjustGeometry(const QRect &geom)
{
    setGeometry(geom);
}

void Dialog::resizeEvent(QResizeEvent *re)
{
    syncMainItemToSize();
    QQuickWindow::resizeEvent(re);
}

void Dialog::syncMainItemToSize()
{
    d->frameSvgItem->setX(0);
    d->frameSvgItem->setY(0);
    d->frameSvgItem->setWidth(width());
    d->frameSvgItem->setHeight(height());

    KWindowEffects::enableBlurBehind(winId(), true, d->frameSvgItem->frameSvg()->mask());
    updateContrast();

    if (d->mainItem) {
        d->mainItem.data()->setX(d->frameSvgItem->margins()->left());
        d->mainItem.data()->setY(d->frameSvgItem->margins()->top());
        d->mainItem.data()->setWidth(width() - d->frameSvgItem->margins()->left() - d->frameSvgItem->margins()->right());
        d->mainItem.data()->setHeight(height() - d->frameSvgItem->margins()->top() - d->frameSvgItem->margins()->bottom());
    }
}

void Dialog::syncToMainItemSize()
{
    //if manually sync a sync timer was running cancel it so we don't get called twice
    d->syncTimer->stop();

    if (!d->mainItem) {
        return;
    }
    syncBorders();
    const QSize s = QSize(d->mainItem.data()->width(), d->mainItem.data()->height()) +
                    QSize(d->frameSvgItem->margins()->left() + d->frameSvgItem->margins()->right(),
                          d->frameSvgItem->margins()->top() + d->frameSvgItem->margins()->bottom());

    if (visualParent()) {
        const QRect geom(popupPosition(visualParent(), s, Qt::AlignCenter), s);

        if (geom == geometry()) {
            return;
        }
        adjustGeometry(geom);
    } else {
        resize(s);
    }
}

void Dialog::requestSyncToMainItemSize(bool delayed)
{
    if (!d->componentComplete) {
        return;
    }

    if (delayed && !d->syncTimer->isActive()) {
        d->syncTimer->start(150);
    } else {
        d->syncTimer->start(0);
    }
}

void Dialog::updateContrast()
{
    KWindowEffects::enableBackgroundContrast(winId(), d->theme.backgroundContrastEnabled(),
                                                      d->theme.backgroundContrast(),
                                                      d->theme.backgroundIntensity(),
                                                      d->theme.backgroundSaturation(),
                                                      d->frameSvgItem->frameSvg()->mask());
}

void Dialog::setType(WindowType type)
{
    if (type == d->type) {
        return;
    }

    d->type = type;
    if (d->type != Normal) {
        KWindowSystem::setType(winId(), (NET::WindowType)type);
    } else {
        setFlags(Qt::FramelessWindowHint|flags());
    }

    if (type == Tooltip) {
        d->frameSvgItem->setImagePath("widgets/tooltip");
    } else {
        d->frameSvgItem->setImagePath("dialogs/background");
    }

    if (type == Dock) {
        KWindowSystem::setOnAllDesktops(winId(), true);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), false);
    }

    emit typeChanged();
}

Dialog::WindowType Dialog::type() const
{
    return d->type;
}

void Dialog::focusInEvent(QFocusEvent *ev)
{
    QQuickWindow::focusInEvent(ev);
}

void Dialog::focusOutEvent(QFocusEvent *ev)
{
    if (d->hideOnWindowDeactivate) {
        qDebug( ) << "DIALOG:  hiding dialog.";
        setVisible(false);
    }
    QQuickWindow::focusOutEvent(ev);
}

void Dialog::showEvent(QShowEvent *event)
{
    DialogShadows::self()->addWindow(this, d->frameSvgItem->enabledBorders());
    QQuickWindow::showEvent(event);
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        updateVisibility(true);
    } else if (event->type() == QEvent::Hide) {
        updateVisibility(false);
    }

    const bool retval = QQuickWindow::event(event);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    return retval;
}

void Dialog::hideEvent(QHideEvent *event)
{
    QQuickWindow::hideEvent(event);
}

void Dialog::classBegin()
{

}

void Dialog::componentComplete()
{
    d->componentComplete = true;
    syncToMainItemSize();
}

void Dialog::syncBorders()
{
    // FIXME: QWindow::screen() never ever changes if the window is moved across
    //        virtual screens (normal two screens with X), this seems to be intentional
    //        as it's explicitly mentioned in the docs. Until that's changed or some
    //        more proper way of howto get the current QScreen for given QWindow is found,
    //        we simply iterate over the virtual screens and pick the one our QWindow
    //        says it's at.
    QRect avail;
    QPoint pos = position();
    Q_FOREACH(QScreen *screen, screen()->virtualSiblings()) {
        if (screen->availableGeometry().contains(pos)) {
            avail = screen->availableGeometry();
            break;
        }
    }

    int borders = Plasma::FrameSvg::AllBorders;

    //Tooltips always have all the borders
    if (!(flags() & Qt::ToolTip)) {
        if (x() <= avail.x() || d->location == Plasma::Types::LeftEdge) {
            borders = borders & ~Plasma::FrameSvg::LeftBorder;
        }
        if (y() <= avail.y() || d->location == Plasma::Types::TopEdge) {
            borders = borders & ~Plasma::FrameSvg::TopBorder;
        }
        if (avail.right() <= x() + width() || d->location == Plasma::Types::RightEdge) {
            borders = borders & ~Plasma::FrameSvg::RightBorder;
        }
        if (avail.bottom() <= y() + height() || d->location == Plasma::Types::BottomEdge) {
            borders = borders & ~Plasma::FrameSvg::BottomBorder;
        }
    }

    d->frameSvgItem->setEnabledBorders((Plasma::FrameSvg::EnabledBorder)borders);

    if (isVisible()) {
        DialogShadows::self()->addWindow(this, d->frameSvgItem->enabledBorders());
    }
}

bool Dialog::hideOnWindowDeactivate() const
{
    return d->hideOnWindowDeactivate;
}

void Dialog::setHideOnWindowDeactivate(bool hide)
{
    if (flags() & Qt::X11BypassWindowManagerHint) {
        // doesn't get keyboard focus, so let's just ignore it
        return;
    }
    if (d->hideOnWindowDeactivate == hide) {
        return;
    }
    d->hideOnWindowDeactivate = hide;
    emit hideOnWindowDeactivateChanged();
}

bool Dialog::isOutputOnly() const
{
    return d->outputOnly;
}

void Dialog::setOutputOnly(bool outputOnly)
{
    if (d->outputOnly == outputOnly) {
        return;
    }
    d->outputOnly = outputOnly;
    emit outputOnlyChanged();
}

void Dialog::updateInputShape()
{
    if (!isVisible()) {
        return;
    }
#if HAVE_XCB_SHAPE
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        xcb_connection_t *c = QX11Info::connection();
        static bool s_shapeExtensionChecked = false;
        static bool s_shapeAvailable = false;
        if (!s_shapeExtensionChecked) {
            xcb_prefetch_extension_data(c, &xcb_shape_id);
            const xcb_query_extension_reply_t *extension = xcb_get_extension_data(c, &xcb_shape_id);
            if (extension->present) {
                // query version
                auto cookie = xcb_shape_query_version(c);
                QScopedPointer<xcb_shape_query_version_reply_t, QScopedPointerPodDeleter> version(xcb_shape_query_version_reply(c, cookie, Q_NULLPTR));
                if (!version.isNull()) {
                    s_shapeAvailable = (version->major_version * 0x10 + version->minor_version) >= 0x11;
                }
            }
            s_shapeExtensionChecked = true;
        }
        if (!s_shapeAvailable) {
            return;
        }
        if (d->outputOnly) {
            // set input shape, so that it doesn't accept any input events
            xcb_shape_rectangles(c, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                                 XCB_CLIP_ORDERING_UNSORTED, winId(), 0, 0, 0, NULL);
        } else {
            // delete the shape
            xcb_shape_mask(c, XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_INPUT,
                           winId(), 0, 0, XCB_PIXMAP_NONE);
        }
    }
#endif
}

void Dialog::setTransientParentAndNotify(QWindow *parent)
{
    if (parent == transientParent()) {
        return;
    }

    setTransientParent(parent);
    emit transientParentChanged();
}


void Dialog::updateMinimumWidth()
{
    if (d->mainItemLayout) {
        setMinimumWidth(d->mainItemLayout.data()->property("minimumWidth").toInt() + d->frameSvgItem->margins()->left() + d->frameSvgItem->margins()->right());
    } else {
        setMinimumWidth(-1);
    }
}

void Dialog::updateMinimumHeight()
{
    if (d->mainItemLayout) {
        setMinimumHeight(d->mainItemLayout.data()->property("minimumHeight").toInt() + d->frameSvgItem->margins()->top() + d->frameSvgItem->margins()->bottom());
    } else {
        setMinimumHeight(-1);
    }
}

void Dialog::updateMaximumWidth()
{
    if (d->mainItemLayout) {
        const int hint = d->mainItemLayout.data()->property("maximumWidth").toInt();
        if (hint > 0) {
            setMaximumWidth(hint + d->frameSvgItem->margins()->left() + d->frameSvgItem->margins()->right());
        } else {
            setMaximumWidth(DIALOGSIZE_MAX);
        }
    } else {
        setMaximumWidth(DIALOGSIZE_MAX);
    }
}

void Dialog::updateMaximumHeight()
{
    if (d->mainItemLayout) {
        const int hint = d->mainItemLayout.data()->property("maximumHeight").toInt();
        if (hint > 0) {
            setMaximumHeight(hint + d->frameSvgItem->margins()->top() + d->frameSvgItem->margins()->bottom());
        } else {
            setMaximumHeight(DIALOGSIZE_MAX);
        }
    } else {
        setMaximumHeight(DIALOGSIZE_MAX);
    }
}

#include "dialog.moc"

