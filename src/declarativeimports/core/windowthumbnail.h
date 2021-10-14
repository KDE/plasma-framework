/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef PLASMA_WINDOWTHUMBNAIL_H
#define PLASMA_WINDOWTHUMBNAIL_H
#include <config-plasma.h>
#include <config-x11.h>

#include <cstdint>

// Qt
#include <QAbstractNativeEventFilter>
#include <QPointer>
#include <QQuickItem>
#include <QSGSimpleTextureNode>
#include <QSGTextureProvider>
#include <QWindow>
// xcb
#if HAVE_XCB_COMPOSITE
#include <xcb/damage.h>

#if HAVE_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <fixx11h.h> // egl.h could include XLib.h

#endif // HAVE_EGL

#endif // HAVE_XCB_COMPOSITE
class KWindowInfo;

namespace Plasma
{
class WindowTextureProvider;

/**
 * @brief Renders a thumbnail for the window specified by the @c winId property.
 *
 * This declarative item is able to render a live updating thumbnail for the
 * window specified by the given @c winId property. If it is not possible to get
 * the thumbnail, the window's icon is rendered instead or in case that the window
 * Id is invalid a generic fallback icon is used.
 *
 * The thumbnail does not necessarily fill out the complete geometry as the
 * thumbnail gets scaled keeping the aspect ratio. This means the thumbnail gets
 * rendered into the center of the item's geometry.
 *
 * Note: live updating thumbnails are only implemented on the X11 platform. On X11
 * a running compositor is not required as this item takes care of redirecting the
 * window. For technical reasons the window's frame is not included on X11.
 *
 * If the window closes, the thumbnail does not get destroyed, which allows to have
 * a window close animation.
 *
 * Example usage:
 * @code
 * WindowThumbnail {
 *     winId: 102760466
 * }
 * @endcode
 *
 */
class WindowThumbnail : public QQuickItem, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_PROPERTY(uint winId READ winId WRITE setWinId NOTIFY winIdChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    Q_PROPERTY(bool thumbnailAvailable READ thumbnailAvailable NOTIFY thumbnailAvailableChanged)

public:
    explicit WindowThumbnail(QQuickItem *parent = nullptr);
    ~WindowThumbnail() override;
    bool nativeEventFilter(const QByteArray &eventType, void *message, long int *result) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;

    uint32_t winId() const;
    void setWinId(uint32_t winId);

    qreal paintedWidth() const;
    qreal paintedHeight() const;
    bool thumbnailAvailable() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;

Q_SIGNALS:
    void winIdChanged();
    void paintedSizeChanged();
    void thumbnailAvailableChanged();

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void releaseResources() override;

private:
    void iconToTexture(WindowTextureProvider *textureProvider);
    void windowToTexture(WindowTextureProvider *textureProvider);
    bool startRedirecting();
    void stopRedirecting();
    void resetDamaged();
    void setThumbnailAvailable(bool thumbnailAvailable);
    void sceneVisibilityChanged(bool visible);

    bool m_xcb;
    bool m_composite;
    QPointer<QWindow> m_scene;
    uint32_t m_winId;
    QSizeF m_paintedSize;
    bool m_thumbnailAvailable;
    bool m_redirecting;
    bool m_damaged;
    int m_depth;
    mutable WindowTextureProvider *m_textureProvider = nullptr;
#if HAVE_XCB_COMPOSITE
    xcb_pixmap_t pixmapForWindow();
    bool m_openGLFunctionsResolved;
    uint8_t m_damageEventBase;
    xcb_damage_damage_t m_damage;
    xcb_pixmap_t m_pixmap;

    /*The following must *only* be used from the render thread*/
    uint m_texture;
#if HAVE_GLX
    bool windowToTextureGLX(WindowTextureProvider *textureProvider);
    void resolveGLXFunctions();
    bool loadGLXTexture();
    void bindGLXTexture();
    xcb_pixmap_t m_glxPixmap;
    xcb_visualid_t m_visualid;
    QFunctionPointer m_bindTexImage;
    QFunctionPointer m_releaseTexImage;
#endif // HAVE_GLX
#if HAVE_EGL
    bool xcbWindowToTextureEGL(WindowTextureProvider *textureProvider);
    void resolveEGLFunctions();
    void bindEGLTexture();
    bool m_eglFunctionsResolved;
    EGLImageKHR m_image;
    QFunctionPointer m_eglCreateImageKHR;
    QFunctionPointer m_eglDestroyImageKHR;
    QFunctionPointer m_glEGLImageTargetTexture2DOES;
#endif // HAVE_EGL
#endif
};

class WindowTextureProvider : public QSGTextureProvider
{
    Q_OBJECT

public:
    QSGTexture *texture() const override;
    void setTexture(QSGTexture *texture);

private:
    QScopedPointer<QSGTexture> m_texture;
};

}

#endif //  PLASMA_WINDOWTHUMBNAIL_H
