/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dialog.h"
#include "../declarativeimports/core/framesvgitem.h"
#include "config-plasma.h"
#include "configview.h"
#include "dialogshadows_p.h"
#include "view.h"

#include <QLayout>
#include <QMenu>
#include <QPointer>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>

#include <QPlatformSurfaceEvent>

#include <KWindowSystem/KWindowInfo>
#include <KWindowSystem>

#include <kquickaddons/quickviewsharedengine.h>

#include <KWindowEffects>
#include <Plasma/Corona>

#include <QDebug>

#if HAVE_KWAYLAND
#include "waylandintegration_p.h"
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#endif

#if HAVE_XCB_SHAPE
#include <QX11Info>
#include <xcb/shape.h>
#endif

#if HAVE_X11
#include <QtPlatformHeaders/QXcbWindowFunctions>
#endif

// Unfortunately QWINDOWSIZE_MAX is not exported
#define DIALOGSIZE_MAX ((1 << 24) - 1)

namespace PlasmaQuick
{
class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
        : q(dialog)
        , location(Plasma::Types::BottomEdge)
        , frameSvgItem(nullptr)
        , hasMask(false)
        , type(Dialog::Normal)
        , hideOnWindowDeactivate(false)
        , outputOnly(false)
        , visible(false)
        , componentComplete(dialog->parent() == nullptr)
        , backgroundHints(Dialog::StandardBackground)
    {
        hintsCommitTimer.setSingleShot(true);
        hintsCommitTimer.setInterval(0);
        QObject::connect(&hintsCommitTimer, SIGNAL(timeout()), q, SLOT(updateLayoutParameters()));
    }

    void updateInputShape();

    // SLOTS
    /**
     * Sync Borders updates the enabled borders of the frameSvgItem depending
     * on the geometry of the window.
     *
     * \param windowGeometry The window geometry which should be taken into
     * consideration when activating/deactivating certain borders
     */
    void syncBorders(const QRect &windowGeometry);

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
     * Gets the maximum and minimum size hints for the window based on the contents. it doesn't actually resize anything
     */
    void getSizeHints(QSize &min, QSize &max) const;

    /**
     * This function is an optimized version of updateMaximumHeight,
     * updateMaximumWidth,updateMinimumWidth and updateMinimumHeight.
     * It should be called when you need to call all 4 of these functions
     * AND you have called syncToMainItemSize before.
     */
    void updateLayoutParameters();

    QRect availableScreenGeometryForPosition(const QPoint &pos) const;

    /**
     * This function checks the current position of the dialog and repositions
     * it so that no part of it is not on the screen
     */
    void repositionIfOffScreen();

    void slotMainItemSizeChanged();
    void slotWindowPositionChanged();

    void syncToMainItemSize();

    bool mainItemContainsPosition(const QPointF &point) const;
    QPointF positionAdjustedForMainItem(const QPointF &point) const;

    void setupWaylandIntegration();

    void applyType();

    Dialog *q;
    Plasma::Types::Location location;
    Plasma::FrameSvgItem *frameSvgItem;
    QPointer<QQuickItem> mainItem;
    QPointer<QQuickItem> visualParent;
    QTimer hintsCommitTimer;
#if HAVE_KWAYLAND
    QPointer<KWayland::Client::PlasmaShellSurface> shellSurface;
#endif

    QRect cachedGeometry;
    bool hasMask;
    Dialog::WindowType type;
    bool hideOnWindowDeactivate;
    bool outputOnly;
    bool visible;
    Plasma::Theme theme;
    bool componentComplete;
    Dialog::BackgroundHints backgroundHints;

    // Attached Layout property of mainItem, if any
    QPointer<QObject> mainItemLayout;
};

QRect DialogPrivate::availableScreenGeometryForPosition(const QPoint &pos) const
{
    // FIXME: QWindow::screen() never ever changes if the window is moved across
    //        virtual screens (normal two screens with X), this seems to be intentional
    //        as it's explicitly mentioned in the docs. Until that's changed or some
    //        more proper way of howto get the current QScreen for given QWindow is found,
    //        we simply iterate over the virtual screens and pick the one our QWindow
    //        says it's at.
    QRect avail;
    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        // we check geometry() but then take availableGeometry()
        // to reliably check in which screen a position is, we need the full
        // geometry, including areas for panels
        if (screen->geometry().contains(pos)) {
            avail = screen->availableGeometry();
            break;
        }
    }

    /*
     * if the heuristic fails (because the topleft of the dialog is offscreen)
     * use at least our screen()
     * the screen should be correctly updated now on Qt 5.3+ so should be
     * more reliable anyways (could be tried to remove the whole Q_FOREACH
     * at this point)
     *
     * important: screen can be a nullptr... see bug 345173
     */
    if (avail.isEmpty() && q->screen()) {
        avail = q->screen()->availableGeometry();
    }

    return avail;
}

void DialogPrivate::syncBorders(const QRect &geom)
{
    QRect avail = availableScreenGeometryForPosition(geom.topLeft());
    int borders = Plasma::FrameSvg::AllBorders;

    // Tooltips always have all the borders
    // floating windows have all borders
    if (!q->flags().testFlag(Qt::ToolTip) && location != Plasma::Types::Floating) {
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
        KWindowEffects::enableBlurBehind(q, false);
        KWindowEffects::enableBackgroundContrast(q, false);
        q->setMask(QRegion());
        DialogShadows::self()->removeWindow(q);
    } else {
        auto prefix = QStringLiteral("");
        if ((backgroundHints & Dialog::SolidBackground) == Dialog::SolidBackground) {
            prefix = QStringLiteral("solid/");
        }
        if (type == Dialog::Tooltip) {
            frameSvgItem->setImagePath(prefix + QStringLiteral("widgets/tooltip"));
        } else {
            frameSvgItem->setImagePath(prefix + QStringLiteral("dialogs/background"));
        }

        KWindowEffects::enableBlurBehind(q, theme.blurBehindEnabled(), frameSvgItem->mask());

        KWindowEffects::enableBackgroundContrast(q,
                                                 theme.backgroundContrastEnabled(),
                                                 theme.backgroundContrast(),
                                                 theme.backgroundIntensity(),
                                                 theme.backgroundSaturation(),
                                                 frameSvgItem->mask());

        if (KWindowSystem::compositingActive()) {
            if (hasMask) {
                hasMask = false;
                q->setMask(QRegion());
            }
        } else {
            hasMask = true;
            q->setMask(frameSvgItem->mask());
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

            if (mainItem) {
                syncToMainItemSize();
            }
            if (mainItemLayout) {
                updateLayoutParameters();
            }

#if HAVE_KWAYLAND
            // if is a wayland window that was hidden, we need
            // to set its position again as there won't be any move event to sync QWindow::position and shellsurface::position
            if (shellSurface && type != Dialog::OnScreenDisplay) {
                shellSurface->setPosition(q->position());
            }
#endif
        }
    }

    if (!q->flags().testFlag(Qt::ToolTip) && type != Dialog::Notification && type != Dialog::CriticalNotification) {
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
        // no edge, no slide
        default:
            break;
        }

        KWindowEffects::slideWindow(q, slideLocation, -1);
    }

    if (visible) {
        q->raise();

        applyType();
    }
}

void DialogPrivate::updateMinimumWidth()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMinimumWidth(0);

    // this is to try to get the internal item resized a tad before, but
    // the flicker almost always happen anyways, so is *probably* useless
    // this other kind of flicker is the view not being always focused exactly
    // on the scene
    auto margin = frameSvgItem->fixedMargins();
    int minimumWidth = mainItemLayout->property("minimumWidth").toInt() + margin->left() + margin->right();
    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
    }
    q->contentItem()->setWidth(qMax(q->width(), minimumWidth));
    q->setWidth(qMax(q->width(), minimumWidth));

    hintsCommitTimer.start();
}

void DialogPrivate::updateMinimumHeight()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMinimumHeight(0);

    // this is to try to get the internal item resized a tad before, but
    // the flicker almost always happen anyways, so is *probably* useless
    // this other kind of flicker is the view not being always focused exactly
    // on the scene
    auto margin = frameSvgItem->fixedMargins();
    int minimumHeight = mainItemLayout->property("minimumHeight").toInt() + margin->top() + margin->bottom();
    if (q->screen()) {
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
    }
    q->contentItem()->setHeight(qMax(q->height(), minimumHeight));
    q->setHeight(qMax(q->height(), minimumHeight));

    hintsCommitTimer.start();
}

void DialogPrivate::updateMaximumWidth()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMaximumWidth(DIALOGSIZE_MAX);

    auto margin = frameSvgItem->fixedMargins();
    int maximumWidth = mainItemLayout->property("maximumWidth").toInt() + margin->left() + margin->right();
    if (q->screen()) {
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
    }
    q->contentItem()->setWidth(qMax(q->width(), maximumWidth));
    q->setWidth(qMax(q->width(), maximumWidth));

    hintsCommitTimer.start();
}

void DialogPrivate::updateMaximumHeight()
{
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    if (!componentComplete) {
        return;
    }

    q->setMaximumHeight(DIALOGSIZE_MAX);

    auto margin = frameSvgItem->fixedMargins();
    int maximumHeight = mainItemLayout->property("maximumHeight").toInt() + margin->top() + margin->bottom();
    if (q->screen()) {
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }
    q->contentItem()->setHeight(qMax(q->height(), maximumHeight));
    q->setHeight(qMin(q->height(), maximumHeight));

    hintsCommitTimer.start();
}

void DialogPrivate::getSizeHints(QSize &min, QSize &max) const
{
    if (!componentComplete || !mainItem || !mainItemLayout) {
        return;
    }
    Q_ASSERT(mainItem);
    Q_ASSERT(mainItemLayout);

    auto margin = frameSvgItem->fixedMargins();

    int minimumHeight = mainItemLayout->property("minimumHeight").toInt();
    int maximumHeight = mainItemLayout->property("maximumHeight").toInt();
    maximumHeight = maximumHeight ? maximumHeight : DIALOGSIZE_MAX;

    int minimumWidth = mainItemLayout->property("minimumWidth").toInt();
    int maximumWidth = mainItemLayout->property("maximumWidth").toInt();
    maximumWidth = maximumWidth ? maximumWidth : DIALOGSIZE_MAX;

    minimumHeight += margin->top() + margin->bottom();
    maximumHeight += margin->top() + margin->bottom();
    minimumWidth += margin->left() + margin->right();
    maximumWidth += margin->left() + margin->right();

    if (q->screen()) {
        minimumWidth = qMin(q->screen()->availableGeometry().width(), minimumWidth);
        minimumHeight = qMin(q->screen()->availableGeometry().height(), minimumHeight);
        maximumWidth = qMin(q->screen()->availableGeometry().width(), maximumWidth);
        maximumHeight = qMin(q->screen()->availableGeometry().height(), maximumHeight);
    }

    min = QSize(minimumWidth, minimumHeight);
    max = QSize(maximumWidth, maximumHeight);
}

void DialogPrivate::updateLayoutParameters()
{
    if (!componentComplete || !mainItem || !mainItemLayout) {
        return;
    }

    mainItem->disconnect(q);
    auto margin = frameSvgItem->fixedMargins();

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);

    const QSize finalSize(qBound(min.width(), q->width(), max.width()), qBound(min.height(), q->height(), max.height()));

    if (visualParent) {
        // it's important here that we're using re->size() as size, we don't want to do recursive resizeEvents
        const QRect geom(q->popupPosition(visualParent, finalSize), finalSize);
        q->adjustGeometry(geom);
    } else {
        q->resize(finalSize);
    }

    mainItem->setPosition(QPointF(margin->left(), margin->top()));
    mainItem->setSize(QSizeF(q->width() - margin->left() - margin->right(), q->height() - margin->top() - margin->bottom()));

    frameSvgItem->setSize(QSizeF(q->width(), q->height()));

    repositionIfOffScreen();
    updateTheme();

    // FIXME: this seems to interfere with the geometry change that
    // sometimes is still going on, causing flicker (this one is two repositions happening in quick succession).
    // it may have to be delayed further
    q->setMinimumSize(min);
    q->setMaximumSize(max);

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
    if (KWindowSystem::isPlatformX11()) {
        xcb_connection_t *c = QX11Info::connection();
        static bool s_shapeExtensionChecked = false;
        static bool s_shapeAvailable = false;
        if (!s_shapeExtensionChecked) {
            xcb_prefetch_extension_data(c, &xcb_shape_id);
            const xcb_query_extension_reply_t *extension = xcb_get_extension_data(c, &xcb_shape_id);
            if (extension->present) {
                // query version
                auto cookie = xcb_shape_query_version(c);
                QScopedPointer<xcb_shape_query_version_reply_t, QScopedPointerPodDeleter> version(xcb_shape_query_version_reply(c, cookie, nullptr));
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
            xcb_shape_rectangles(c, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED, q->winId(), 0, 0, 0, nullptr);
        } else {
            // delete the shape
            xcb_shape_mask(c, XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_INPUT, q->winId(), 0, 0, XCB_PIXMAP_NONE);
        }
    }
#endif
}

void DialogPrivate::syncToMainItemSize()
{
    Q_ASSERT(mainItem);

    if (!componentComplete) {
        return;
    }
    if (mainItem->width() <= 0 || mainItem->height() <= 0) {
        qWarning() << "trying to show an empty dialog";
    }

    updateTheme();
    if (visualParent) {
        // fixedMargins will get all the borders, no matter if they are enabled
        auto margins = frameSvgItem->fixedMargins();

        const QSize fullSize = QSize(mainItem->width(), mainItem->height()) + QSize(margins->left() + margins->right(), margins->top() + margins->bottom());

        // We get the popup position with the fullsize as we need the popup
        // position in order to determine our actual size, as the position
        // determines which borders will be shown.
        const QRect geom(q->popupPosition(visualParent, fullSize), fullSize);

        // We're then moving the window to where we think we would be with all
        // the borders. This way when syncBorders is called, it has a geometry
        // to work with.
        syncBorders(geom);
    } else {
        syncBorders(q->geometry());
    }

    QSize s = QSize(mainItem->width(), mainItem->height())
        + QSize(frameSvgItem->fixedMargins()->left() + frameSvgItem->fixedMargins()->right(),
                frameSvgItem->fixedMargins()->top() + frameSvgItem->fixedMargins()->bottom());

    QSize min;
    QSize max(DIALOGSIZE_MAX, DIALOGSIZE_MAX);
    getSizeHints(min, max);
    s = QSize(qBound(min.width(), s.width(), max.width()), qBound(min.height(), s.height(), max.height()));

    q->contentItem()->setSize(s);

    frameSvgItem->setSize(s);

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

    mainItem->setPosition(QPointF(frameSvgItem->fixedMargins()->left(), frameSvgItem->fixedMargins()->top()));

    updateTheme();
}

void DialogPrivate::slotWindowPositionChanged()
{
    // Tooltips always have all the borders
    // floating windows have all borders
    if (!q->isVisible() || q->flags().testFlag(Qt::ToolTip) || location == Plasma::Types::Floating) {
        return;
    }

    syncBorders(q->geometry());
    updateTheme();

    if (mainItem) {
        auto margin = frameSvgItem->fixedMargins();
        mainItem->setPosition(QPoint(margin->left(), margin->top()));
        mainItem->setSize(QSize(q->width() - margin->left() - margin->right(), q->height() - margin->top() - margin->bottom()));
    }
}

bool DialogPrivate::mainItemContainsPosition(const QPointF &point) const
{
    if (!mainItem) {
        return false;
    }

    return QRectF(mainItem->mapToScene(QPoint(0, 0)), QSizeF(mainItem->width(), mainItem->height())).contains(point);
}

QPointF DialogPrivate::positionAdjustedForMainItem(const QPointF &point) const
{
    if (!mainItem) {
        return point;
    }

    QRectF itemRect(mainItem->mapToScene(QPoint(0, 0)), QSizeF(mainItem->width(), mainItem->height()));

    return QPointF(qBound(itemRect.left(), point.x(), itemRect.right()), qBound(itemRect.top(), point.y(), itemRect.bottom()));
}

void DialogPrivate::setupWaylandIntegration()
{
#if HAVE_KWAYLAND
    if (shellSurface) {
        // already setup
        return;
    }

    using namespace KWayland::Client;
    PlasmaShell *interface = WaylandIntegration::self()->waylandPlasmaShell();
    if (!interface) {
        return;
    }
    Surface *s = Surface::fromWindow(q);
    if (!s) {
        return;
    }
    shellSurface = interface->createSurface(s, q);
#endif
}

void DialogPrivate::applyType()
{
    if (type != Dialog::Normal) {
        /*QXcbWindowFunctions::WmWindowType*/ int wmType = 0;

#if HAVE_X11
        if (KWindowSystem::isPlatformX11()) {
            switch (type) {
            case Dialog::Normal:
                Q_UNREACHABLE();
                break;
            case Dialog::Dock:
                wmType = QXcbWindowFunctions::WmWindowType::Dock;
                break;
            case Dialog::DialogWindow:
                wmType = QXcbWindowFunctions::WmWindowType::Dialog;
                break;
            case Dialog::PopupMenu:
                wmType = QXcbWindowFunctions::WmWindowType::PopupMenu;
                break;
            case Dialog::Tooltip:
                wmType = QXcbWindowFunctions::WmWindowType::Tooltip;
                break;
            case Dialog::Notification:
                wmType = QXcbWindowFunctions::WmWindowType::Notification;
                break;
            case Dialog::OnScreenDisplay:
            case Dialog::CriticalNotification:
                // Not supported by Qt
                break;
            }

            if (wmType) {
                QXcbWindowFunctions::setWmWindowType(q, static_cast<QXcbWindowFunctions::WmWindowType>(wmType));
            }
        }
#endif

        if (!wmType) {
            KWindowSystem::setType(q->winId(), static_cast<NET::WindowType>(type));
        }
    } else {
        q->setFlags(Qt::FramelessWindowHint | q->flags());

#if HAVE_KWAYLAND
        // Only possible after setup
        if (shellSurface) {
            if (q->flags() & Qt::WindowStaysOnTopHint) {
                shellSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Panel);
                shellSurface->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
            } else {
                shellSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Normal);
                shellSurface->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AlwaysVisible);
            }
        }
#endif
    }

    // an OSD can't be a Dialog, as qt xcb would attempt to set a transient parent for it
    // see bug 370433
    if (type == Dialog::OnScreenDisplay) {
        q->setFlags((q->flags() & ~Qt::Dialog) | Qt::Window);
    }

    if (backgroundHints == Dialog::NoBackground) {
        frameSvgItem->setImagePath(QString());
    } else {
        auto prefix = QStringLiteral("");
        if ((backgroundHints & Dialog::SolidBackground) == Dialog::SolidBackground) {
            prefix = QStringLiteral("solid/");
        }
        if (type == Dialog::Tooltip) {
            frameSvgItem->setImagePath(prefix + QStringLiteral("widgets/tooltip"));
        } else {
            frameSvgItem->setImagePath(prefix + QStringLiteral("dialogs/background"));
        }
    }

    if (type == Dialog::Dock || type == Dialog::Notification || type == Dialog::OnScreenDisplay || type == Dialog::CriticalNotification) {
        KWindowSystem::setOnAllDesktops(q->winId(), true);
    } else {
        KWindowSystem::setOnAllDesktops(q->winId(), false);
    }

#if HAVE_KWAYLAND
    if (shellSurface) {
        shellSurface->setPanelTakesFocus(!q->flags().testFlag(Qt::WindowDoesNotAcceptFocus));
    }
#endif
}

Dialog::Dialog(QQuickItem *parent)
    : QQuickWindow(parent ? parent->window() : nullptr)
    , d(new DialogPrivate(this))
{
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint | Qt::Dialog);

    connect(this, &QWindow::xChanged, [=]() {
        d->slotWindowPositionChanged();
    });
    connect(this, &QWindow::yChanged, [=]() {
        d->slotWindowPositionChanged();
    });

    // Given dialogs are skip task bar and don't have a decoration
    // minimizing them using e.g. "minimize all" should just close them
    connect(this, &QWindow::windowStateChanged, this, [this](Qt::WindowState newState) {
        if (newState == Qt::WindowMinimized) {
            setVisible(false);
        }
    });

    connect(this, &QWindow::visibleChanged, this, &Dialog::visibleChangedProxy);
    connect(this, SIGNAL(visibleChanged(bool)), this, SLOT(updateInputShape()));
    connect(this, SIGNAL(outputOnlyChanged()), this, SLOT(updateInputShape()));

    // HACK: this property is invoked due to the initialization that gets done to contentItem() in the getter
    property("data");
    // Create the FrameSvg background.
    d->frameSvgItem = new Plasma::FrameSvgItem(contentItem());
    // This is needed as a transition thing for KWayland
    setProperty("__plasma_frameSvg", QVariant::fromValue(d->frameSvgItem->frameSvg()));

    connect(&d->theme, SIGNAL(themeChanged()), this, SLOT(updateTheme()));
}

Dialog::~Dialog()
{
    if (!QCoreApplication::instance()->closingDown()) {
        DialogShadows::self()->removeWindow(this);
    }

    // Prevent signals from super-class destructor invoking our now-destroyed slots
    disconnect(this, nullptr, this, nullptr);
}

QQuickItem *Dialog::mainItem() const
{
    return d->mainItem;
}

void Dialog::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem != mainItem) {
        d->hintsCommitTimer.stop();

        if (d->mainItem) {
            disconnect(d->mainItem, nullptr, this, nullptr);
            d->mainItem->setParentItem(nullptr);
        }

        if (d->mainItemLayout) {
            disconnect(d->mainItemLayout, nullptr, this, nullptr);
        }

        d->mainItem = mainItem;

        if (mainItem) {
            mainItem->setParentItem(contentItem());

            connect(mainItem, SIGNAL(widthChanged()), this, SLOT(slotMainItemSizeChanged()));
            connect(mainItem, SIGNAL(heightChanged()), this, SLOT(slotMainItemSizeChanged()));
            d->slotMainItemSizeChanged();

            // Extract the representation's Layout, if any
            QObject *layout = nullptr;
            setMinimumSize(QSize(0, 0));
            setMaximumSize(QSize(DIALOGSIZE_MAX, DIALOGSIZE_MAX));

            // Search a child that has the needed Layout properties
            // HACK: here we are not type safe, but is the only way to access to a pointer of Layout
            const auto lstChild = mainItem->children();
            for (QObject *child : lstChild) {
                // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
                if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() && child->property("preferredWidth").isValid()
                    && child->property("preferredHeight").isValid() && child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid()
                    && child->property("fillWidth").isValid() && child->property("fillHeight").isValid()) {
                    layout = child;
                    break;
                }
            }

            d->mainItemLayout = layout;

            if (layout) {
                // Why queued connections?
                // we need to be sure that the properties are
                // already *all* updated when we call the management code
                connect(layout, SIGNAL(minimumWidthChanged()), this, SLOT(updateMinimumWidth()));
                connect(layout, SIGNAL(minimumHeightChanged()), this, SLOT(updateMinimumHeight()));
                connect(layout, SIGNAL(maximumWidthChanged()), this, SLOT(updateMaximumWidth()));
                connect(layout, SIGNAL(maximumHeightChanged()), this, SLOT(updateMaximumHeight()));

                d->updateLayoutParameters();
            }
        }

        // if this is called in Component.onCompleted we have to wait a loop the item is added to a scene
        Q_EMIT mainItemChanged();
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
    Q_EMIT visualParentChanged();
    if (visualParent) {
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
        if (d->mainItem) {
            d->syncToMainItemSize();
        }
    }
}

QPoint Dialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item) {
        // If no item was specified try to align at the center of the parent view
        QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem) {
            QScreen *screen = parentItem->window()->screen();

            switch (d->location) {
            case Plasma::Types::TopEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().y());
            case Plasma::Types::LeftEdge:
                return QPoint(screen->availableGeometry().x(), screen->availableGeometry().center().y() - size.height() / 2);
            case Plasma::Types::RightEdge:
                return QPoint(screen->availableGeometry().right() - size.width(), screen->availableGeometry().center().y() - size.height() / 2);
            case Plasma::Types::BottomEdge:
                return QPoint(screen->availableGeometry().center().x() - size.width() / 2, screen->availableGeometry().bottom() - size.height());
            // Default center in the screen
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

    // if the item is in a dock or in a window that ignores WM we want to position the popups outside of the dock
    const KWindowInfo winInfo(item->window()->winId(), NET::WMWindowType);
    const bool outsideParentWindow =
        ((winInfo.windowType(NET::AllTypesMask) == NET::Dock) || (item->window()->flags() & Qt::X11BypassWindowManagerHint)) && item->window()->mask().isNull();

    QRect parentGeometryBounds;
    if (outsideParentWindow) {
        parentGeometryBounds = item->window()->geometry();
    } else {
        parentGeometryBounds = item->mapRectToScene(item->boundingRect()).toRect();
        if (item->window()) {
            parentGeometryBounds.moveTopLeft(item->window()->mapToGlobal(parentGeometryBounds.topLeft()));
            pos = parentGeometryBounds.topLeft();
        }
    }

    const QPoint topPoint(pos.x() + (item->mapRectToScene(item->boundingRect()).width() - size.width()) / 2, parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(pos.x() + (item->mapRectToScene(item->boundingRect()).width() - size.width()) / 2, parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(), pos.y() + (item->mapRectToScene(item->boundingRect()).height() - size.height()) / 2);

    const QPoint rightPoint(parentGeometryBounds.right(), pos.y() + (item->mapRectToScene(item->boundingRect()).height() - size.height()) / 2);

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

    // find the correct screen for the item
    // we do not rely on item->window()->screen() because
    // QWindow::screen() is always only the screen where the window gets first created
    // not actually the current window. See QWindow::screen() documentation
    QRect avail = item->window()->screen()->availableGeometry();

    if (outsideParentWindow && d->frameSvgItem->enabledBorders() != Plasma::FrameSvg::AllBorders) {
        // make the panel look it's inside the panel, in order to not make it look cut
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
            dialogPos.setX(qMax(avail.left(), (avail.right() - size.width() + 1)));
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
            dialogPos.setY(qMax(avail.top(), (avail.bottom() - size.height() + 1)));
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
    Q_EMIT locationChanged();

    if (d->mainItem) {
        d->syncToMainItemSize();
    }
}

QObject *Dialog::margins() const
{
    return d->frameSvgItem->fixedMargins();
}

QObject *Dialog::inset() const
{
    return d->frameSvgItem->inset();
}

void Dialog::setFramelessFlags(Qt::WindowFlags flags)
{
    if (d->type == Dialog::Normal)
        flags |= Qt::Dialog;
    setFlags(Qt::FramelessWindowHint | flags);
    d->applyType();
    Q_EMIT flagsChanged();
}

void Dialog::adjustGeometry(const QRect &geom)
{
    setGeometry(geom);
}

void Dialog::resizeEvent(QResizeEvent *re)
{
    QQuickWindow::resizeEvent(re);

    // it's a spontaneous event generated in qguiapplication.cpp QGuiApplicationPrivate::processWindowScreenChangedEvent
    // QWindowSystemInterfacePrivate::GeometryChangeEvent gce(window, QHighDpi::fromNativePixels(window->handle()->geometry(), window), QRect());
    // This happens before the first show event when there is more than one screen,
    // right after the window has been created, the window is still 0x0,
    // but the resize event gets delivered with 0x0 again and executed with all the bad side effects
    // this seems to happen for every window when there are multiple screens, so something we have probably to watch out for in the future
    if (re->size().isEmpty() || re->size() == re->oldSize()) {
        return;
    }

    // A dialog can be resized even if no mainItem has ever been set
    if (!d->mainItem) {
        return;
    }

    d->mainItem->disconnect(this);

    d->frameSvgItem->setSize(QSizeF(re->size().width(), re->size().height()));
    auto margin = d->frameSvgItem->fixedMargins();
    d->mainItem->setPosition(QPointF(margin->left(), margin->top()));

    d->mainItem->setSize(QSize(re->size().width() - margin->left() - margin->right(), re->size().height() - margin->top() - margin->bottom()));

    QObject::connect(d->mainItem, SIGNAL(widthChanged()), this, SLOT(slotMainItemSizeChanged()));
    QObject::connect(d->mainItem, SIGNAL(heightChanged()), this, SLOT(slotMainItemSizeChanged()));
}

void Dialog::setType(WindowType type)
{
    if (type == d->type) {
        return;
    }

    d->type = type;
    d->applyType();
    Q_EMIT typeChanged();
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
            if (parentWindow->isActive() && !(parentWindow->flags() & Qt::WindowDoesNotAcceptFocus)) {
                parentHasFocus = true;

                break;
            }

            parentWindow = parentWindow->transientParent();
        }

        const QWindow *focusWindow = QGuiApplication::focusWindow();
        bool childHasFocus = focusWindow && ((focusWindow->isActive() && isAncestorOf(focusWindow)) || (focusWindow->type() & Qt::Popup) == Qt::Popup);

        const bool viewClicked = qobject_cast<const KQuickAddons::QuickViewSharedEngine *>(focusWindow)
#if PLASMAQUICK_BUILD_DEPRECATED_SINCE(5, 83)
            || qobject_cast<const View *>(focusWindow)
#endif
            || qobject_cast<const ConfigView *>(focusWindow);

        if (viewClicked || (!parentHasFocus && !childHasFocus)) {
            setVisible(false);
            Q_EMIT windowDeactivated();
        }
    }

    QQuickWindow::focusOutEvent(ev);
}

void Dialog::showEvent(QShowEvent *event)
{
    if (d->backgroundHints != Dialog::NoBackground) {
        DialogShadows::self()->addWindow(this, d->frameSvgItem->enabledBorders());
    }

    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);

    QQuickWindow::showEvent(event);
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        auto ee = static_cast<QExposeEvent *>(event);

        if (!KWindowSystem::isPlatformWayland() || ee->region().isNull()) {
            return QQuickWindow::event(event);
        }

        /*
         * expose event is the only place where to correctly
         * register our wayland extensions, as showevent is a bit too
         * soon and the platform window isn't shown yet
         * create a shell surface every time the window gets exposed
         * (only the first expose event, guarded by shelldurface existence)
         * and tear it down when the window gets hidden
         * see https://phabricator.kde.org/T6064
         */
#if HAVE_KWAYLAND
        // sometimes non null regions arrive even for non visible windows
        // for which surface creation would fail
        if (!d->shellSurface && isVisible()) {
            KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
            d->setupWaylandIntegration();
            d->updateVisibility(true);
            d->updateTheme();
        }
#endif
    } else if (event->type() == QEvent::PlatformSurface) {
        const QPlatformSurfaceEvent *pSEvent = static_cast<QPlatformSurfaceEvent *>(event);

        if (pSEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        }
    } else if (event->type() == QEvent::Show) {
        d->updateVisibility(true);
    } else if (event->type() == QEvent::Hide) {
        d->updateVisibility(false);
#if HAVE_KWAYLAND
        delete d->shellSurface;
        d->shellSurface = nullptr;
#endif

    } else if (event->type() == QEvent::Move) {
#if HAVE_KWAYLAND
        if (d->shellSurface) {
            QMoveEvent *me = static_cast<QMoveEvent *>(event);
            d->shellSurface->setPosition(me->pos());
        }
#endif
    }

    /*Fitt's law: if the containment has margins, and the mouse cursor clicked
     * on the mouse edge, forward the click in the containment boundaries
     */
    if (d->mainItem && !d->mainItem->size().isEmpty()) {
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);

            // don't mess with position if the cursor is actually outside the view:
            // somebody is doing a click and drag that must not break when the cursor i outside
            if (geometry().contains(me->screenPos().toPoint()) && !d->mainItemContainsPosition(me->windowPos())) {
                QMouseEvent me2(me->type(),
                                d->positionAdjustedForMainItem(me->windowPos()),
                                d->positionAdjustedForMainItem(me->windowPos()),
                                d->positionAdjustedForMainItem(me->windowPos()) + position(),
                                me->button(),
                                me->buttons(),
                                me->modifiers());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &me2);
                }
                return true;
            }
            break;
        }

        case QEvent::Wheel: {
            QWheelEvent *we = static_cast<QWheelEvent *>(event);

            const QPoint pos = we->position().toPoint();

            if (!d->mainItemContainsPosition(pos)) {
                QWheelEvent we2(d->positionAdjustedForMainItem(pos),
                                d->positionAdjustedForMainItem(pos) + position(),
                                we->pixelDelta(),
                                we->angleDelta(),
                                we->buttons(),
                                we->modifiers(),
                                we->phase(),
                                false /*inverted*/);

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &we2);
                }
                return true;
            }
            break;
        }

        case QEvent::DragEnter: {
            QDragEnterEvent *de = static_cast<QDragEnterEvent *>(event);
            if (!d->mainItemContainsPosition(de->pos())) {
                QDragEnterEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                                    de->possibleActions(),
                                    de->mimeData(),
                                    de->mouseButtons(),
                                    de->keyboardModifiers());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &de2);
                }
                return true;
            }
            break;
        }
        // DragLeave just works
        case QEvent::DragLeave:
            break;
        case QEvent::DragMove: {
            QDragMoveEvent *de = static_cast<QDragMoveEvent *>(event);
            if (!d->mainItemContainsPosition(de->pos())) {
                QDragMoveEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                                   de->possibleActions(),
                                   de->mimeData(),
                                   de->mouseButtons(),
                                   de->keyboardModifiers());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &de2);
                }
                return true;
            }
            break;
        }
        case QEvent::Drop: {
            QDropEvent *de = static_cast<QDropEvent *>(event);
            if (!d->mainItemContainsPosition(de->pos())) {
                QDropEvent de2(d->positionAdjustedForMainItem(de->pos()).toPoint(),
                               de->possibleActions(),
                               de->mimeData(),
                               de->mouseButtons(),
                               de->keyboardModifiers());

                if (isVisible()) {
                    QCoreApplication::sendEvent(this, &de2);
                }
                return true;
            }
            break;
        }

        default:
            break;
        }
    }

    return QQuickWindow::event(event);
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
    QQuickWindow::setVisible(d->visible);
    d->updateTheme();
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
    Q_EMIT hideOnWindowDeactivateChanged();
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
    Q_EMIT outputOnlyChanged();
}

void Dialog::setVisible(bool visible)
{
    // only update real visibility when we have finished component completion
    // and all flags have been set

    d->visible = visible;
    if (d->componentComplete) {
        if (visible && d->visualParent) {
            setPosition(popupPosition(d->visualParent, size()));
        }

        // Bug 381242: Qt remembers minimize state and re-applies it when showing
        setWindowStates(windowStates() & ~Qt::WindowMinimized);
        QQuickWindow::setVisible(visible);
        // signal will be emitted and proxied from the QQuickWindow code
    } else {
        Q_EMIT visibleChangedProxy();
    }
}

bool Dialog::isVisible() const
{
    if (d->componentComplete) {
        return QQuickWindow::isVisible();
    }
    return d->visible;
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
    Q_EMIT backgroundHintsChanged();
}

}

#include "moc_dialog.cpp"
