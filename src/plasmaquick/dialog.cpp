/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *   Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>                    *
 *   Copyright 2014 Vishesh Handa <vhanda@kde.org>                         *
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
#include <QPointer>

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
          backgroundHints(Dialog::StandardBackground)
    {
    }

    void updateInputShape();

    //SLOTS
    /**
     * Sync Borders updates the enabled borders of the frameSvgItem depending
     * on the geometry of the window.
     *
     * \param windowGeometry The window geometry which should be taken into
     * consideration when activating/deactivating certain borders
     */
    void syncBorders(const QRect& windowGeometry);

    /**
     * This function sets the blurBehind, background contrast and shadows. It
     * does so wrt the frameSvgItem. So make sure the frameSvgItem is the
     * correct size before calling this function.
     */
    void updateTheme();
    void updateVisibility(bool visible);

    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();

    /**
     * This function is an optimized version of updateMaximumHeight,
     * updateMaximumWidth,updateMinimumWidth and updateMinimumHeight.
     * It should be called when you need to call all 4 of these functions
     * AND you have called syncToMainItemSize before.
     */
    void updateLayoutParameters();

    QRect availableScreenGeometryForPosition(const QPoint& pos) const;

    /**
     * This function checks the current position of the dialog and repositions
     * it so that no part of it is not on the screen
     */
    void repositionIfOffScreen();

    void slotMainItemSizeChanged();
    void slotWindowPositionChanged();

    void syncToMainItemSize();

    Dialog *q;
    QTimer *syncTimer;
    Plasma::Types::Location location;
    Plasma::FrameSvgItem *frameSvgItem;
    QPointer<QQuickItem> mainItem;
    QPointer<QQuickItem> visualParent;

    QRect cachedGeometry;
    Dialog::WindowType type;
    bool hideOnWindowDeactivate;
    bool outputOnly;
    Plasma::Theme theme;
    bool componentComplete;
    Dialog::BackgroundHints backgroundHints;

    //Attached Layout property of mainItem, if any
    QPointer <QObject> mainItemLayout;
};

QRect DialogPrivate::availableScreenGeometryForPosition(const QPoint& pos) const
{
    // FIXME: QWindow::screen() never ever changes if the window is moved across
    //        virtual screens (normal two screens with X), this seems to be intentional
    //        as it's explicitly mentioned in the docs. Until that's changed or some
    //        more proper way of howto get the current QScreen for given QWindow is found,
    //        we simply iterate over the virtual screens and pick the one our QWindow
    //        says it's at.
    QRect avail;
    Q_FOREACH (QScreen *screen, q->screen()->virtualSiblings()) {
        if (screen->availableGeometry().contains(pos)) {
            avail = screen->availableGeometry();
            break;
        }
    }

    return avail;
}

void DialogPrivate::syncBorders(const QRect& geom)
{
    QRect avail = availableScreenGeometryForPosition(geom.topLeft());
    int borders = Plasma::FrameSvg::AllBorders;

    //Tooltips always have all the borders
    // floating windows have all borders
    if ((q->flags() & Qt::ToolTip) != Qt::ToolTip && location != Plasma::Types::Floating) {
        if (geom.x() <= avail.x() || location == Plasma::Types::LeftEdge) {
            borders = borders & ~Plasma::FrameSvg::LeftBorder;
        }
        if (geom.y() <= avail.y() || location == Plasma::Types::TopEdge) {
            borders = borders & ~Plasma::FrameSvg::TopBorder;
        }
        if (avail.right() <= geom.x() + geom.width() || location == Plasma::Types::RightEdge) {
            borders = borders & ~Plasma::FrameSvg::RightBorder;
        }
        if (avail.bottom() <= geom.y() + geom.height() || location == Plasma::Types::BottomEdge) {
            borders = borders & ~Plasma::FrameSvg::BottomBorder;
        }
    }

    if (frameSvgItem->enabledBorders() != (Plasma::FrameSvg::EnabledBorder)borders) {
        frameSvgItem->setEnabledBorders((Plasma::FrameSvg::EnabledBorder)borders);
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
        if (visualParent && visualParent->window()) {
            q->setTransientParent(visualParent->window());
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
                slotWindowPositionChanged();
                if (visualParent) {
                    q->setPosition(q->popupPosition(visualParent, q->size()));
                }
                cachedGeometry = QRect();
            }
            syncToMainItemSize();
            if (mainItemLayout) {
                updateLayoutParameters();
            }
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
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    mainItem->disconnect(q);

    syncBorders(q->geometry());

    int minimumWidth = mainItemLayout->property("minimumWidth").toInt();
    auto margin = frameSvgItem->margins();

    int oldWidth = q->width();

    q->setMinimumWidth(minimumWidth + margin->left() + margin->right());
    q->setWidth(qMax(q->width(), q->minimumWidth()));

    mainItem->setWidth(q->width() - margin->left() - margin->right());
    frameSvgItem->setWidth(q->width());

    if (location == Plasma::Types::RightEdge) {
        q->setX(q->x() + (oldWidth - q->size().width()));
    }
    repositionIfOffScreen();
    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, q->size()), q->size());
        q->setGeometry(geom);
    }

    updateTheme();

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::updateMinimumHeight()
{
    if (!componentComplete) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    mainItem->disconnect(q);

    syncBorders(q->geometry());

    int minimumHeight = mainItemLayout->property("minimumHeight").toInt();
    auto margin = frameSvgItem->margins();

    int oldHeight = mainItem->height();

    q->setMinimumHeight(minimumHeight + margin->top() + margin->bottom());
    q->setHeight(qMax(q->height(), q->minimumHeight()));

    mainItem->setHeight(q->height() - margin->top() - margin->bottom());
    frameSvgItem->setHeight(q->height());

    if (location == Plasma::Types::BottomEdge) {
        q->setY(q->y() + (oldHeight - q->size().height()));
    }
    repositionIfOffScreen();
    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, q->size()), q->size());
        q->setGeometry(geom);
    }

    updateTheme();

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::updateMaximumWidth()
{
    if (!componentComplete) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    mainItem->disconnect(q);

    syncBorders(q->geometry());

    int maximumWidth = mainItemLayout->property("maximumWidth").toInt();
    maximumWidth = maximumWidth ? maximumWidth : DIALOGSIZE_MAX;
    auto margin = frameSvgItem->margins();

    q->setMaximumWidth(maximumWidth + margin->left() + margin->right());
    q->setWidth(qBound(q->minimumWidth(), q->width(), q->maximumWidth()));
    mainItem->setWidth(q->width() - margin->left() - margin->right());
    frameSvgItem->setWidth(q->width());

    repositionIfOffScreen();
    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, q->size()), q->size());
        q->setGeometry(geom);
    }

    updateTheme();

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::updateMaximumHeight()
{
    if (!componentComplete) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    mainItem->disconnect(q);

    syncBorders(q->geometry());

    int maximumHeight = mainItemLayout->property("maximumHeight").toInt();
    maximumHeight = maximumHeight ? maximumHeight : DIALOGSIZE_MAX;
    auto margin = frameSvgItem->margins();

    q->setMaximumHeight(maximumHeight + margin->top() + margin->bottom());
    q->setHeight(qBound(q->minimumHeight(), q->height(), q->maximumHeight()));

    mainItem->setHeight(q->height() - margin->top() - margin->bottom());
    frameSvgItem->setHeight(q->height());

    repositionIfOffScreen();
    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, q->size()), q->size());
        q->setGeometry(geom);
    }

    updateTheme();

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::updateLayoutParameters()
{
    if (!componentComplete || !mainItem || !q->isVisible()) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    mainItem->disconnect(q);

    int minimumHeight = mainItemLayout->property("minimumHeight").toInt();
    int maximumHeight = mainItemLayout->property("maximumHeight").toInt();
    maximumHeight = maximumHeight ? maximumHeight : DIALOGSIZE_MAX;

    int minimumWidth = mainItemLayout->property("minimumWidth").toInt();
    int maximumWidth = mainItemLayout->property("maximumWidth").toInt();
    maximumWidth = maximumWidth ? maximumWidth : DIALOGSIZE_MAX;

    auto margin = frameSvgItem->margins();

    q->setMinimumHeight(minimumHeight + margin->top() + margin->bottom());
    q->setMaximumHeight(maximumHeight + margin->top() + margin->bottom());
    q->setHeight(qBound(q->minimumHeight(), q->height(), q->maximumHeight()));

    q->setMinimumWidth(minimumWidth + margin->left() + margin->right());
    q->setMaximumWidth(maximumWidth + margin->left() + margin->right());
    q->setWidth(qBound(q->minimumWidth(), q->width(), q->maximumWidth()));

    mainItem->setX(margin->left());
    mainItem->setY(margin->top());
    mainItem->setWidth(q->width() - margin->left() - margin->right());
    mainItem->setHeight(q->height() - margin->top() - margin->bottom());

    frameSvgItem->setWidth(q->width());
    frameSvgItem->setHeight(q->height());

    repositionIfOffScreen();
    updateTheme();

    QObject::connect(mainItem, SIGNAL(widthChanged()), q, SLOT(slotMainItemSizeChanged()));
    QObject::connect(mainItem, SIGNAL(heightChanged()), q, SLOT(slotMainItemSizeChanged()));
}

void DialogPrivate::repositionIfOffScreen()
{
    if (!componentComplete) {
        return;
    }
    const QRect avail = availableScreenGeometryForPosition(q->position());

    int x = q->x();
    int y = q->y();

    if (x < avail.left()) {
        x = avail.left();
    } else if (x + q->width() > avail.right()) {
        x = avail.right() - q->width() + 1;
    }

    if (y < avail.top()) {
        y = avail.top();
    } else if (y + q->height() > avail.bottom()) {
        y = avail.bottom() - q->height() + 1;
    }

    q->setX(x);
    q->setY(y);
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

void DialogPrivate::syncToMainItemSize()
{
    if (!componentComplete || !mainItem || !q->isVisible()) {
        return;
    }

    if (visualParent) {
        // Get the full size with ALL the borders
        frameSvgItem->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        auto margins = frameSvgItem->margins();

        const QSize fullSize = QSize(mainItem->width(), mainItem->height()) +
                               QSize(margins->left() + margins->right(),
                                     margins->top() + margins->bottom());

        // We get the popup position with the fullsize as we need the popup
        // position in order to determine our actual size, as the position
        // determines which borders will be shown.
        const QRect geom(q->popupPosition(visualParent, fullSize), fullSize);

        // We're then moving the window to where we think we would be with all
        // the borders. This way when syncBorders is called, it has a geometry
        // to work with.
        syncBorders(geom);
    }
    else {
        syncBorders(q->geometry());
    }

    const QSize s = QSize(mainItem->width(), mainItem->height()) +
                    QSize(frameSvgItem->margins()->left() + frameSvgItem->margins()->right(),
                          frameSvgItem->margins()->top() + frameSvgItem->margins()->bottom());

    frameSvgItem->setX(0);
    frameSvgItem->setY(0);
    frameSvgItem->setWidth(s.width());
    frameSvgItem->setHeight(s.height());

    if (visualParent) {
        const QRect geom(q->popupPosition(visualParent, s), s);

        if (geom == q->geometry()) {
            return;
        }
        q->adjustGeometry(geom);
        // The borders will instantly be updated but the geometry might take a
        // while as sub-classes can reimplement adjustGeometry and animate it.
        syncBorders(geom);
    } else {
        q->resize(s);
    }

    mainItem->setX(frameSvgItem->margins()->left());
    mainItem->setY(frameSvgItem->margins()->top());

    updateTheme();
}

void DialogPrivate::slotWindowPositionChanged()
{
    // Tooltips always have all the borders
    // floating windows have all borders
    if ((q->flags() & Qt::ToolTip) || location == Plasma::Types::Floating) {
        return;
    }

    syncBorders(q->geometry());
    updateTheme();

    if (mainItem) {
        auto margin = frameSvgItem->margins();
        mainItem->setX(margin->left());
        mainItem->setY(margin->top());
        mainItem->setWidth(q->width() - margin->left() - margin->right());
        mainItem->setHeight(q->height() - margin->top() - margin->bottom());
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

    connect(this, &QWindow::xChanged, [=]() { d->slotWindowPositionChanged(); });
    connect(this, &QWindow::yChanged, [=]() { d->slotWindowPositionChanged(); });

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
    return d->mainItem;
}

void Dialog::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem != mainItem) {
        if (d->mainItem) {
            d->mainItem->setParent(parent());
        }

        d->mainItem = mainItem;

        if (mainItem) {
            mainItem->setParent(contentItem());
            mainItem->setProperty("parent", QVariant::fromValue(contentItem()));

            connect(mainItem, SIGNAL(widthChanged()), this, SLOT(slotMainItemSizeChanged()));
            connect(mainItem, SIGNAL(heightChanged()), this, SLOT(slotMainItemSizeChanged()));
            d->slotMainItemSizeChanged();

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
                disconnect(d->mainItemLayout, 0, this, 0);
            }
            d->mainItemLayout = layout;

            if (layout) {
                connect(layout, SIGNAL(minimumWidthChanged()), this, SLOT(updateMinimumWidth()));
                connect(layout, SIGNAL(minimumHeightChanged()), this, SLOT(updateMinimumHeight()));
                connect(layout, SIGNAL(maximumWidthChanged()), this, SLOT(updateMaximumWidth()));
                connect(layout, SIGNAL(maximumHeightChanged()), this, SLOT(updateMaximumHeight()));

                d->updateLayoutParameters();
            }

        }

        //if this is called in Component.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

void DialogPrivate::slotMainItemSizeChanged()
{
    syncToMainItemSize();
}

QQuickItem *Dialog::visualParent() const
{
    return d->visualParent;
}

void Dialog::setVisualParent(QQuickItem *visualParent)
{
    if (d->visualParent == visualParent) {
        return;
    }

    d->visualParent = visualParent;
    emit visualParentChanged();
    if (visualParent) {
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
        d->syncToMainItemSize();
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

    d->syncToMainItemSize();
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

void Dialog::resizeEvent(QResizeEvent* re)
{
    QQuickWindow::resizeEvent(re);
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
    d->componentComplete = false;
}

void Dialog::componentComplete()
{
    d->componentComplete = true;
    d->syncToMainItemSize();

    if (d->mainItemLayout) {
        d->updateLayoutParameters();
    }
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

