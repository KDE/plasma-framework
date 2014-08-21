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
#include "../declarativeimports/core/framesvgitem.h"
#include "dialogshadows_p.h"
#include "view.h"

#include <QQuickItem>
#include <QTimer>
#include <QLayout>
#include <QScreen>
#include <QMenu>

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

namespace PlasmaQuick
{

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
        : q(dialog),
          location(Plasma::Types::BottomEdge),
          type(Dialog::Normal),
          hideOnWindowDeactivate(false),
          outputOnly(false),
          componentComplete(dialog->parent() == 0),
          resizeOrigin(Undefined),
          backgroundHints(Dialog::StandardBackground)
    {
    }

    enum ResizeOrigin {
        Undefined,
        MainItem,
        Window
    };

    void updateInputShape();

    //SLOTS
    void syncBorders();
    void updateTheme();
    void updateVisibility(bool visible);

    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();

    void syncMainItemToSize();
    void syncToMainItemSize();
    void requestSizeSync(bool delayed = false);

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
    ResizeOrigin resizeOrigin;
    Dialog::BackgroundHints backgroundHints;

    //Attached Layout property of mainItem, if any
    QWeakPointer <QObject> mainItemLayout;
};

void DialogPrivate::syncBorders()
{
    // FIXME: QWindow::screen() never ever changes if the window is moved across
    //        virtual screens (normal two screens with X), this seems to be intentional
    //        as it's explicitly mentioned in the docs. Until that's changed or some
    //        more proper way of howto get the current QScreen for given QWindow is found,
    //        we simply iterate over the virtual screens and pick the one our QWindow
    //        says it's at.
    QRect avail;
    QPoint pos = q->position();
    Q_FOREACH (QScreen *screen, q->screen()->virtualSiblings()) {
        if (screen->availableGeometry().contains(pos)) {
            avail = screen->availableGeometry();
            break;
        }
    }

    int borders = Plasma::FrameSvg::AllBorders;

    //Tooltips always have all the borders
    // floating windows have all borders
    if ((q->flags() & Qt::ToolTip) != Qt::ToolTip && location != Plasma::Types::Floating) {
        if (q->x() <= avail.x() || location == Plasma::Types::LeftEdge) {
            borders = borders & ~Plasma::FrameSvg::LeftBorder;
        }
        if (q->y() <= avail.y() || location == Plasma::Types::TopEdge) {
            borders = borders & ~Plasma::FrameSvg::TopBorder;
        }
        if (avail.right() <= q->x() + q->width() || location == Plasma::Types::RightEdge) {
            borders = borders & ~Plasma::FrameSvg::RightBorder;
        }
        if (avail.bottom() <= q->y() + q->height() || location == Plasma::Types::BottomEdge) {
            borders = borders & ~Plasma::FrameSvg::BottomBorder;
        }
    }

    if (frameSvgItem->enabledBorders() != (Plasma::FrameSvg::EnabledBorder)borders) {
        frameSvgItem->setEnabledBorders((Plasma::FrameSvg::EnabledBorder)borders);

        if (mainItemLayout) {
            updateMinimumWidth();
            updateMinimumHeight();
            updateMaximumWidth();
            updateMaximumHeight();
        }
    }
}

void DialogPrivate::updateTheme()
{
    if (backgroundHints == Dialog::NoBackground) {
        frameSvgItem->setImagePath(QString());
        KWindowEffects::enableBlurBehind(q->winId(), false);
        KWindowEffects::enableBackgroundContrast(q->winId(), false);
        q->setMask(QRegion());
        DialogShadows::self()->removeWindow(q);
    } else {
        if (type == Dialog::Tooltip) {
            frameSvgItem->setImagePath("widgets/tooltip");
        } else {
            frameSvgItem->setImagePath("dialogs/background");
        }

        KWindowEffects::enableBlurBehind(q->winId(), true, frameSvgItem->frameSvg()->mask());

        KWindowEffects::enableBackgroundContrast(q->winId(), theme.backgroundContrastEnabled(),
                theme.backgroundContrast(),
                theme.backgroundIntensity(),
                theme.backgroundSaturation(),
                frameSvgItem->frameSvg()->mask());

        if (KWindowSystem::compositingActive()) {
            q->setMask(QRegion());
        } else {
            q->setMask(frameSvgItem->frameSvg()->mask());
        }
        if (q->isVisible()) {
            DialogShadows::self()->addWindow(q, frameSvgItem->enabledBorders());
        }
    }
    updateInputShape();
}

void DialogPrivate::updateVisibility(bool visible)
{
    if (visible) {
        if (visualParent && visualParent.data()->window()) {
            q->setTransientParent(visualParent.data()->window());
        }

        if (q->location() == Plasma::Types::FullScreen) {
            frameSvgItem->setEnabledBorders(Plasma::FrameSvg::NoBorder);

            // We cache the original size of the item, to retrieve it
            // when the dialog is switched back from fullscreen.
            if (q->geometry() != q->screen()->availableGeometry()) {
                cachedGeometry = q->geometry();
            }
            q->setGeometry(q->screen()->availableGeometry());
        } else {
            if (!cachedGeometry.isNull()) {
                q->resize(cachedGeometry.size());
                syncMainItemToSize();
                cachedGeometry = QRect();
            }
            syncToMainItemSize();
        }
    }

    if (!(q->flags() & Qt::ToolTip)) {
        KWindowEffects::SlideFromLocation slideLocation = KWindowEffects::NoEdge;

        switch (location) {
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

        KWindowEffects::slideWindow(q->winId(), slideLocation, -1);
    }

    if (visible) {
        q->raise();

        if (type != Dialog::Normal) {
            KWindowSystem::setType(q->winId(), (NET::WindowType)type);
        } else {
            q->setFlags(Qt::FramelessWindowHint | q->flags());
        }
        if (type == Dialog::Dock) {
            KWindowSystem::setOnAllDesktops(q->winId(), true);
        } else {
            KWindowSystem::setOnAllDesktops(q->winId(), false);
        }
    }
}

void DialogPrivate::updateMinimumWidth()
{
    if (mainItemLayout) {
        int oldWidth = q->width();
        q->setMinimumWidth(mainItemLayout.data()->property("minimumWidth").toInt() + frameSvgItem->margins()->left() + frameSvgItem->margins()->right());
        //Sometimes setMinimumWidth doesn't actually resize: Qt bug?
        resizeOrigin = DialogPrivate::Window;
        q->setWidth(qMax(q->width(), q->minimumWidth()));

        if (location == Plasma::Types::RightEdge) {
            q->setX(q->x() + (oldWidth - q->size().width()));
        }
    } else {
        q->setMinimumWidth(-1);
    }
}

void DialogPrivate::updateMinimumHeight()
{
    if (mainItemLayout) {
        int oldHeight = q->height();
        q->setMinimumHeight(mainItemLayout.data()->property("minimumHeight").toInt() + frameSvgItem->margins()->top() + frameSvgItem->margins()->bottom());
        //Sometimes setMinimumHeight doesn't actually resize: Qt bug?
        resizeOrigin = DialogPrivate::Window;
        q->setHeight(qMax(q->height(), q->minimumHeight()));

        if (location == Plasma::Types::BottomEdge) {
            q->setY(q->y() + (oldHeight - q->size().height()));
        }
    } else {
        q->setMinimumHeight(-1);
    }
}

void DialogPrivate::updateMaximumWidth()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumWidth").toInt();
        resizeOrigin = DialogPrivate::Window;
        if (hint > 0) {
            q->setMaximumWidth(hint + frameSvgItem->margins()->left() + frameSvgItem->margins()->right());
            q->setWidth(qMin(q->width(), q->maximumWidth()));
        } else {
            q->setMaximumWidth(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumWidth(DIALOGSIZE_MAX);
    }
}

void DialogPrivate::updateMaximumHeight()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumHeight").toInt();
        resizeOrigin = DialogPrivate::Window;
        if (hint > 0) {
            q->setMaximumHeight(hint + frameSvgItem->margins()->top() + frameSvgItem->margins()->bottom());
            q->setHeight(qMin(q->height(), q->maximumHeight()));
        } else {
            q->setMaximumHeight(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumHeight(DIALOGSIZE_MAX);
    }
}

void DialogPrivate::updateInputShape()
{
    if (!q->isVisible()) {
        return;
    }

#if HAVE_XCB_SHAPE
    if (backgroundHints == Dialog::NoBackground) {
        return;
    }

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
        if (outputOnly) {
            // set input shape, so that it doesn't accept any input events
            xcb_shape_rectangles(c, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                                 XCB_CLIP_ORDERING_UNSORTED, q->winId(), 0, 0, 0, NULL);
        } else {
            // delete the shape
            xcb_shape_mask(c, XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_INPUT,
                           q->winId(), 0, 0, XCB_PIXMAP_NONE);
        }
    }
#endif
}

void DialogPrivate::syncMainItemToSize()
{
    syncBorders();

    updateTheme();

    if (mainItem) {
        mainItem.data()->setX(frameSvgItem->margins()->left());
        mainItem.data()->setY(frameSvgItem->margins()->top());
        mainItem.data()->setWidth(q->width() - frameSvgItem->margins()->left() - frameSvgItem->margins()->right());
        mainItem.data()->setHeight(q->height() - frameSvgItem->margins()->top() - frameSvgItem->margins()->bottom());
    }

    if (q->visualParent()) {
        q->setPosition(q->popupPosition(q->visualParent(), q->size()));
    }
}

void DialogPrivate::syncToMainItemSize()
{
    //if manually sync a sync timer was running cancel it so we don't get called twice
    syncTimer->stop();

    if (!mainItem) {
        return;
    }

    const QSize s = QSize(mainItem.data()->width(), mainItem.data()->height()) +
                    QSize(frameSvgItem->margins()->left() + frameSvgItem->margins()->right(),
                          frameSvgItem->margins()->top() + frameSvgItem->margins()->bottom());

    frameSvgItem->setX(0);
    frameSvgItem->setY(0);
    frameSvgItem->setWidth(s.width());
    frameSvgItem->setHeight(s.height());

    if (q->visualParent()) {
        const QRect geom(q->popupPosition(q->visualParent(), s), s);

        if (geom == q->geometry()) {
            return;
        }
        q->adjustGeometry(geom);
    } else {
        q->resize(s);
    }

    syncBorders();
    mainItem.data()->setX(frameSvgItem->margins()->left());
    mainItem.data()->setY(frameSvgItem->margins()->top());

    updateTheme();
}

void DialogPrivate::requestSizeSync(bool delayed)
{
    if (!componentComplete) {
        return;
    }

    if (delayed && !syncTimer->isActive()) {
        syncTimer->start(150);
    } else {
        syncTimer->start(0);
    }
}

Dialog::Dialog(QQuickItem *parent)
    : QQuickWindow(parent ? parent->window() : 0),
      d(new DialogPrivate(this))
{
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);

    setIcon(QIcon::fromTheme("plasma"));

    d->syncTimer = new QTimer(this);
    d->syncTimer->setSingleShot(true);
    d->syncTimer->setInterval(0);
    connect(d->syncTimer, &QTimer::timeout,
    [ = ]() {
        if (d->resizeOrigin == DialogPrivate::MainItem) {
            d->syncToMainItemSize();
        } else {
            d->syncMainItemToSize();
        }
        d->resizeOrigin = DialogPrivate::Undefined;
    });

    connect(this, &QWindow::xChanged, [ = ]() {
        //Tooltips always have all the borders
        // floating windows have all borders
        if (!(flags() & Qt::ToolTip) && d->location != Plasma::Types::Floating) {
            d->resizeOrigin = DialogPrivate::Window;
            d->requestSizeSync(true);
        }
    });
    connect(this, &QWindow::yChanged, [ = ]() {
        //Tooltips always have all the borders
        // floating windows have all borders
        if (!(flags() & Qt::ToolTip) && d->location != Plasma::Types::Floating) {
            d->resizeOrigin = DialogPrivate::Window;
            d->requestSizeSync(true);
        }
    });
    connect(this, SIGNAL(visibleChanged(bool)),
            this, SLOT(updateInputShape()));
    connect(this, SIGNAL(outputOnlyChanged()),
            this, SLOT(updateInputShape()));

    //HACK: this property is invoked due to the initialization that gets done to contentItem() in the getter
    property("data");
    //Create the FrameSvg background.
    d->frameSvgItem = new Plasma::FrameSvgItem(contentItem());

    connect(&d->theme, SIGNAL(themeChanged()),
            this, SLOT(updateTheme()));

}

Dialog::~Dialog()
{
    if (!QCoreApplication::instance()->closingDown()) {
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
                connect(mainItem, &QQuickItem::widthChanged, [ = ]() {
                    d->resizeOrigin = DialogPrivate::MainItem;
                    d->syncTimer->start(0);
                });
            }
            if (mainItem->metaObject()->indexOfSignal("heightChanged")) {
                connect(mainItem, &QQuickItem::heightChanged, [ = ]() {
                    d->resizeOrigin = DialogPrivate::MainItem;
                    d->syncTimer->start(0);
                });
            }
            d->resizeOrigin = DialogPrivate::MainItem;
            d->requestSizeSync();

            //Extract the representation's Layout, if any
            QObject *layout = 0;
            setMinimumSize(QSize(0, 0));
            setMaximumSize(QSize(DIALOGSIZE_MAX, DIALOGSIZE_MAX));

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
            if (d->mainItemLayout) {
                disconnect(d->mainItemLayout.data(), 0, this, 0);
            }
            d->mainItemLayout = layout;

            if (layout) {
                connect(layout, SIGNAL(minimumWidthChanged()), this, SLOT(updateMinimumWidth()));
                connect(layout, SIGNAL(minimumHeightChanged()), this, SLOT(updateMinimumHeight()));
                connect(layout, SIGNAL(maximumWidthChanged()), this, SLOT(updateMaximumWidth()));
                connect(layout, SIGNAL(maximumHeightChanged()), this, SLOT(updateMaximumHeight()));

                d->updateMinimumWidth();
                d->updateMinimumHeight();
                d->updateMaximumWidth();
                d->updateMaximumHeight();
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
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
        d->resizeOrigin = DialogPrivate::MainItem;
        d->requestSizeSync(true);
    }
}

QPoint Dialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item) {
        //If no item was specified try to align at the center of the parent view
        QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem) {
            QScreen *screen = parentItem->window()->screen();

            switch (d->location) {
            case Plasma::Types::TopEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().y());
                break;
            case Plasma::Types::LeftEdge:
                return QPoint(screen->availableGeometry().x(), screen->availableGeometry().center().y() - size.height() / 2);
                break;
            case Plasma::Types::RightEdge:
                return QPoint(screen->availableGeometry().right() - size.width(), screen->availableGeometry().center().y() - size.height() / 2);
                break;
            case Plasma::Types::BottomEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().bottom() - size.height());
                break;
            //Default center in the screen
            default:
                return screen->geometry().center() - QPoint(size.width() / 2, size.height() / 2);
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

    //if the item is in a dock or in a window that ignores WM we want to position the popups outside of the dock
    const KWindowInfo winInfo = KWindowSystem::windowInfo(item->window()->winId(), NET::WMWindowType);
    const bool outsideParentWindow = (winInfo.windowType(NET::AllTypesMask) == NET::Dock) || (item->window()->flags() & Qt::X11BypassWindowManagerHint);

    QRect parentGeometryBounds;
    if (outsideParentWindow) {
        parentGeometryBounds = item->window()->geometry();
    } else {
        parentGeometryBounds = QRect(pos.toPoint(), QSize(item->width(), item->height()));
    }

    const QPoint topPoint(pos.x() + (item->boundingRect().width() - size.width()) / 2,
                          parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(pos.x() + (item->boundingRect().width() - size.width()) / 2,
                             parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(),
                           pos.y() + (item->boundingRect().height() - size.height()) / 2);

    const QPoint rightPoint(parentGeometryBounds.right(),
                            pos.y() + (item->boundingRect().height() - size.height()) / 2);

    QPoint dialogPos;
    if (d->location == Plasma::Types::TopEdge) {
        dialogPos = bottomPoint;
    } else if (d->location == Plasma::Types::LeftEdge) {
        dialogPos = rightPoint;
    } else if (d->location == Plasma::Types::RightEdge) {
        dialogPos = leftPoint;
    } else { // Types::BottomEdge
        dialogPos = topPoint;
    }

    //find the correct screen for the item
    //we do not rely on item->window()->screen() because
    //QWindow::screen() is always only the screen where the window gets first created
    //not actually the current window. See QWindow::screen() documentation
    QRect avail = item->window()->screen()->availableGeometry();

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
        if (d->location != Plasma::Types::LeftEdge || d->location == Plasma::Types::RightEdge) {
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
            dialogPos.setX(avail.right() - size.width() + 1);
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
            dialogPos.setY(avail.bottom() - size.height() + 1);
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
    d->resizeOrigin = DialogPrivate::MainItem;
    d->requestSizeSync();
}

QObject *Dialog::margins() const
{
    return d->frameSvgItem->margins();
}

void Dialog::setFramelessFlags(Qt::WindowFlags flags)
{
    setFlags(Qt::FramelessWindowHint | flags);
    emit flagsChanged();
}

void Dialog::adjustGeometry(const QRect &geom)
{
    setGeometry(geom);
}

void Dialog::resizeEvent(QResizeEvent *re)
{
    d->frameSvgItem->setX(0);
    d->frameSvgItem->setY(0);
    d->frameSvgItem->setWidth(re->size().width());
    d->frameSvgItem->setHeight(re->size().height());

    QQuickWindow::resizeEvent(re);

    if (d->resizeOrigin == DialogPrivate::Undefined) {
        d->resizeOrigin = DialogPrivate::Window;
    }
    d->requestSizeSync(true);
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
        setFlags(Qt::FramelessWindowHint | flags());
    }

    if (d->backgroundHints == Dialog::NoBackground) {
        d->frameSvgItem->setImagePath(QString());
    } else {
        if (d->type == Tooltip) {
            d->frameSvgItem->setImagePath("widgets/tooltip");
        } else {
            d->frameSvgItem->setImagePath("dialogs/background");
        }
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
        bool parentHasFocus = false;

        QWindow *parentWindow = transientParent();

        while (parentWindow) {
            if (parentWindow->isActive() &&
                !(parentWindow->flags() & Qt::WindowDoesNotAcceptFocus)) {
                parentHasFocus  = true;

                break;
            }

            parentWindow = parentWindow->transientParent();
        }

        const QWindow *focusWindow = QGuiApplication::focusWindow();
        bool childHasFocus = focusWindow && ((focusWindow->isActive() && isAncestorOf(focusWindow)) || focusWindow->type() & Qt::Popup);

        if (qobject_cast<const View *>(focusWindow) || (!parentHasFocus && !childHasFocus)) {
            qDebug() << "DIALOG:  hiding dialog.";
            setVisible(false);
            emit windowDeactivated();
        }
    }

    QQuickWindow::focusOutEvent(ev);
}

void Dialog::showEvent(QShowEvent *event)
{
    if (d->backgroundHints != Dialog::NoBackground) {
        DialogShadows::self()->addWindow(this, d->frameSvgItem->enabledBorders());
    }
    QQuickWindow::showEvent(event);
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        d->updateVisibility(true);
    } else if (event->type() == QEvent::Hide) {
        d->updateVisibility(false);
    }

    const bool retval = QQuickWindow::event(event);
    if (event->type() != QEvent::DeferredDelete) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }
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
    d->updateTheme();
    d->componentComplete = true;
    d->syncToMainItemSize();
}

bool Dialog::hideOnWindowDeactivate() const
{
    return d->hideOnWindowDeactivate;
}

void Dialog::setHideOnWindowDeactivate(bool hide)
{
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

Dialog::BackgroundHints Dialog::backgroundHints() const
{
    return d->backgroundHints;
}

void Dialog::setBackgroundHints(Dialog::BackgroundHints hints)
{
    if (d->backgroundHints == hints) {
        return;
    }

    d->backgroundHints = hints;
    d->updateTheme();
    emit backgroundHintsChanged();
}

}

#include "moc_dialog.cpp"

