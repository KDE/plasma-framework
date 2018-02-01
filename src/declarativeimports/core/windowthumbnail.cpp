/*
 *   Copyright 2013 by Martin Gräßlin <mgraesslin@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "windowthumbnail.h"
// KF5
#include <kwindowsystem.h>
// Qt
#include <QGuiApplication>
#include <QIcon>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QQuickWindow>
#include <QRunnable>

#include <cmath>

// X11
#if HAVE_XCB_COMPOSITE
#include <QX11Info>
#include <xcb/composite.h>
#if HAVE_GLX
#include <GL/glx.h>
typedef void (*glXBindTexImageEXT_func)(Display *dpy, GLXDrawable drawable,
                                        int buffer, const int *attrib_list);
typedef void (*glXReleaseTexImageEXT_func)(Display *dpy, GLXDrawable drawable, int buffer);
#endif
#if HAVE_EGL
typedef EGLImageKHR(*eglCreateImageKHR_func)(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLint *);
typedef EGLBoolean(*eglDestroyImageKHR_func)(EGLDisplay, EGLImageKHR);
typedef GLvoid(*glEGLImageTargetTexture2DOES_func)(GLenum, GLeglImageOES);
#endif // HAVE_EGL
#endif

#include <cstdlib>

namespace Plasma
{

#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
class DiscardGlxPixmapRunnable : public QRunnable {
public:
    DiscardGlxPixmapRunnable(
        uint,
        uint,
        uint,
        uint,
        QFunctionPointer,
        xcb_pixmap_t
    );
    void run() Q_DECL_OVERRIDE;
private:
    uint m_texture;
    uint m_mipmaps;
    uint m_readFb;
    uint m_drawFb;
    QFunctionPointer m_releaseTexImage;
    xcb_pixmap_t m_glxPixmap;
};

DiscardGlxPixmapRunnable::DiscardGlxPixmapRunnable(uint texture, uint mipmaps, uint readFb, uint drawFb,
                                                   QFunctionPointer deleteFunction, xcb_pixmap_t pixmap)
    : QRunnable(),
    m_texture(texture),
    m_mipmaps(mipmaps),
    m_readFb(readFb),
    m_drawFb(drawFb),
    m_releaseTexImage(deleteFunction),
    m_glxPixmap(pixmap)
{}

void DiscardGlxPixmapRunnable::run()
{
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        Display *d = QX11Info::display();
        ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(d, m_glxPixmap);

        GLuint textures[] = { m_texture, m_mipmaps };
        GLuint framebuffers[] = { m_readFb, m_drawFb };

        glDeleteTextures(2, textures);
        QOpenGLContext::currentContext()->functions()->glDeleteFramebuffers(2, framebuffers);
    }
}
#endif //HAVE_GLX

#if HAVE_EGL
class DiscardEglPixmapRunnable : public QRunnable {
public:
    DiscardEglPixmapRunnable(
        uint,
        QFunctionPointer,
        EGLImageKHR
    );
    void run() Q_DECL_OVERRIDE;
private:
    uint m_texture;
    QFunctionPointer m_eglDestroyImageKHR;
    EGLImageKHR m_image;
};

DiscardEglPixmapRunnable::DiscardEglPixmapRunnable(uint texture, QFunctionPointer deleteFunction, EGLImageKHR image)
    : QRunnable(),
    m_texture(texture),
    m_eglDestroyImageKHR(deleteFunction),
    m_image(image)
{}

void DiscardEglPixmapRunnable::run()
{
    if (m_image != EGL_NO_IMAGE_KHR) {
        ((eglDestroyImageKHR_func)(m_eglDestroyImageKHR))(eglGetCurrentDisplay(), m_image);
        glDeleteTextures(1, &m_texture);
    }
}
#endif//HAVE_EGL
#endif //HAVE_XCB_COMPOSITE

// QSGSimpleTextureNode does not support mipmap filtering, so this is the
// only way to prevent it from setting TEXTURE_MIN_FILTER to LINEAR.
class ThumbnailTexture : public QSGTexture
{
public:
    ThumbnailTexture(int texture, const QSize &size) : m_texture(texture), m_size(size) {}
    void bind() override final { glBindTexture(GL_TEXTURE_2D, m_texture); }
    bool hasAlphaChannel() const override final { return true; }
    bool hasMipmaps() const override final { return true; }
    int textureId() const override final { return m_texture; }
    QSize textureSize() const override final { return m_size; }

private:
    int m_texture;
    QSize m_size;
};


// ------------------------------------------------------------------


WindowTextureNode::WindowTextureNode()
    : QSGSimpleTextureNode()
{
}

WindowTextureNode::~WindowTextureNode()
{
}

void WindowTextureNode::reset(QSGTexture *texture)
{
    setTexture(texture);
    m_texture.reset(texture);
}

WindowThumbnail::WindowThumbnail(QQuickItem *parent)
    : QQuickItem(parent)
    , QAbstractNativeEventFilter()
    , m_xcb(false)
    , m_composite(false)
    , m_winId(0)
    , m_paintedSize(QSizeF())
    , m_thumbnailAvailable(false)
    , m_damaged(false)
    , m_depth(0)
#if HAVE_XCB_COMPOSITE
    , m_openGLFunctionsResolved(false)
    , m_damageEventBase(0)
    , m_damage(XCB_NONE)
    , m_pixmap(XCB_PIXMAP_NONE)
    , m_texture(0)
    , m_mipmaps(0)
    , m_readFb(0)
    , m_drawFb(0)
#if HAVE_GLX
    , m_glxPixmap(XCB_PIXMAP_NONE)
    , m_bindTexImage(nullptr)
    , m_releaseTexImage(nullptr)
#endif // HAVE_GLX
#if HAVE_EGL
    , m_eglFunctionsResolved(false)
    , m_image(EGL_NO_IMAGE_KHR)
    , m_eglCreateImageKHR(nullptr)
    , m_eglDestroyImageKHR(nullptr)
    , m_glEGLImageTargetTexture2DOES(nullptr)
#endif // HAVE_EGL
#endif
{
    setFlag(ItemHasContents);
    connect(this, &QQuickItem::windowChanged, [this](QQuickWindow * window) {
        if (!window) {
            return;
        }
        // restart the redirection, it might not have been active yet
        stopRedirecting();
        startRedirecting();
        update();
    });
    connect(this, &QQuickItem::enabledChanged, [this]() {
        if (!isEnabled()) {
            stopRedirecting();
            releaseResources();
        } else if (isVisible()) {
            startRedirecting();
            update();
        }
    });
    connect(this, &QQuickItem::visibleChanged, [this]() {
        if (!isVisible()) {
            stopRedirecting();
            releaseResources();
        } else if (isEnabled()) {
            startRedirecting();
            update();
        }
    });
    if (QGuiApplication *gui = dynamic_cast<QGuiApplication *>(QCoreApplication::instance())) {
        m_xcb = (gui->platformName() == QStringLiteral("xcb"));
        if (m_xcb) {
            gui->installNativeEventFilter(this);
#if HAVE_XCB_COMPOSITE
            xcb_connection_t *c = QX11Info::connection();
            xcb_prefetch_extension_data(c, &xcb_composite_id);
            const auto *compositeReply = xcb_get_extension_data(c, &xcb_composite_id);
            m_composite = (compositeReply && compositeReply->present);

            xcb_prefetch_extension_data(c, &xcb_damage_id);
            const auto *reply = xcb_get_extension_data(c, &xcb_damage_id);
            m_damageEventBase = reply->first_event;
            if (reply->present) {
                xcb_damage_query_version_unchecked(c, XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);
            }
#endif
        }
    }
}

WindowThumbnail::~WindowThumbnail()
{
    if (m_xcb) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
        stopRedirecting();
    }
}

void WindowThumbnail::releaseResources()
{
#if HAVE_XCB_COMPOSITE

#if HAVE_GLX && HAVE_EGL
    //only one (or none) should be set, but never both
    Q_ASSERT(m_glxPixmap == XCB_PIXMAP_NONE || m_image == EGL_NO_IMAGE_KHR);
#endif
#if HAVE_GLX || HAVE_EGL
    QQuickWindow::RenderStage m_renderStage = QQuickWindow::NoStage;
#endif

    //data is deleted in the render thread (with relevant GLX calls)
    //note runnable may be called *after* this is deleted
    //but the pointer is held by the WindowThumbnail which is in the main thread
#if HAVE_GLX
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        window()->scheduleRenderJob(new DiscardGlxPixmapRunnable(m_texture,
                                                        m_mipmaps,
                                                        m_readFb,
                                                        m_drawFb,
                                                        m_releaseTexImage,
                                                        m_glxPixmap),
                                                        m_renderStage);

        m_glxPixmap = XCB_PIXMAP_NONE;
        m_texture = 0;
    }
#endif
#if HAVE_EGL
    if (m_image != EGL_NO_IMAGE_KHR) {
        window()->scheduleRenderJob(new DiscardEglPixmapRunnable(m_texture,
                                                        m_eglDestroyImageKHR,
                                                        m_image),
                                                        m_renderStage);

        m_image = EGL_NO_IMAGE_KHR;
        m_texture = 0;
    }
#endif
#endif
}



uint32_t WindowThumbnail::winId() const
{
    return m_winId;
}

void WindowThumbnail::setWinId(uint32_t winId)
{
    if (m_winId == winId) {
        return;
    }
    if (!KWindowSystem::self()->hasWId(winId)) {
        // invalid Id, don't updated
        return;
    }
    if (window() && winId == window()->winId()) {
        // don't redirect to yourself
        return;
    }
    stopRedirecting();
    m_winId = winId;

    if (isEnabled() && isVisible()) {
        startRedirecting();
    }

    emit winIdChanged();
}

qreal WindowThumbnail::paintedWidth() const
{
    return m_paintedSize.width();
}

qreal WindowThumbnail::paintedHeight() const
{
    return m_paintedSize.height();
}

bool WindowThumbnail::thumbnailAvailable() const
{
    return m_thumbnailAvailable;
}

QSGNode *WindowThumbnail::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)
    auto *node = static_cast<WindowTextureNode *>(oldNode);
    if (!node) {
        node = new WindowTextureNode();
        node->setFiltering(QSGTexture::Linear);
    }
    if (!m_xcb || m_winId == 0 || (window() && window()->winId() == m_winId)) {
        iconToTexture(node);
    } else {
        windowToTexture(node);
    }
    node->setRect(boundingRect());
    const QSizeF size(node->texture()->textureSize().scaled(boundingRect().size().toSize(), Qt::KeepAspectRatio));
    if (size != m_paintedSize) {
        m_paintedSize = size;
        emit paintedSizeChanged();
    }
    const qreal x = boundingRect().x() + (boundingRect().width() - size.width()) / 2;
    const qreal y = boundingRect().y() + (boundingRect().height() - size.height()) / 2;
    node->setRect(QRectF(QPointF(x, y), size));
    return node;
}

bool WindowThumbnail::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
{
    Q_UNUSED(result)
    if (!m_xcb || !m_composite || eventType != QByteArrayLiteral("xcb_generic_event_t")) {
        // currently we are only interested in XCB events
        return false;
    }
#if HAVE_XCB_COMPOSITE
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
    const uint8_t responseType = event->response_type & ~0x80;
    if (responseType == m_damageEventBase + XCB_DAMAGE_NOTIFY) {
        if (reinterpret_cast<xcb_damage_notify_event_t *>(event)->drawable == m_winId) {
            m_damaged = true;
            update();
        }
    } else if (responseType == XCB_CONFIGURE_NOTIFY) {
        if (reinterpret_cast<xcb_configure_notify_event_t *>(event)->window == m_winId) {
            releaseResources();
            m_damaged = true;
            update();
        }
    } else if (responseType == XCB_MAP_NOTIFY) {
        if (reinterpret_cast<xcb_configure_notify_event_t *>(event)->window == m_winId) {
            releaseResources();
            m_damaged = true;
            update();
        }
    }
#else
    Q_UNUSED(message)
#endif
    // do not filter out any events, there might be further WindowThumbnails for the same window
    return false;
}

void WindowThumbnail::iconToTexture(WindowTextureNode *textureNode)
{
    QIcon icon;
    if (KWindowSystem::self()->hasWId(m_winId)) {
        icon = KWindowSystem::self()->icon(m_winId, boundingRect().width(), boundingRect().height());
    } else {
        // fallback to plasma icon
        icon = QIcon::fromTheme(QStringLiteral("plasma"));
    }
    QImage image = icon.pixmap(boundingRect().size().toSize()).toImage();
    textureNode->reset(window()->createTextureFromImage(image, QQuickWindow::TextureCanUseAtlas));
}

#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
bool WindowThumbnail::windowToTextureGLX(WindowTextureNode *textureNode)
{
    if (glXGetCurrentContext()) {
        if (!m_openGLFunctionsResolved) {
            resolveGLXFunctions();
        }
        if (!m_bindTexImage || !m_releaseTexImage) {
            return false;
        }

        QOpenGLContext *ctx = QOpenGLContext::currentContext();

        QOpenGLFunctions *funcs = ctx->functions();
        QOpenGLExtraFunctions *extraFuncs = ctx->extraFunctions();

        static bool haveTextureStorage = !ctx->isOpenGLES() &&
                                          ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_storage"));

        static bool sRGB = !ctx->isOpenGLES() &&
                            ctx->hasExtension(QByteArrayLiteral("GL_ARB_framebuffer_sRGB")) &&
                            ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_sRGB_decode"));

        // Save the GL state
        GLuint prevReadFb = 0, prevDrawFb = 0, prevTex2D = 0, prevScissorTest = 0, prevFramebufferSrgb = 0;
        funcs->glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint *) &prevReadFb);
        funcs->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint *) &prevDrawFb);
        funcs->glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &prevTex2D);
        funcs->glGetIntegerv(GL_SCISSOR_TEST, (GLint *) &prevScissorTest);

        if (sRGB)
            funcs->glGetIntegerv(GL_FRAMEBUFFER_SRGB, (GLint *) &prevFramebufferSrgb);

        if (m_glxPixmap == XCB_PIXMAP_NONE) {
            xcb_connection_t *c = QX11Info::connection();
            auto attrCookie = xcb_get_window_attributes_unchecked(c, m_winId);
            auto geometryCookie = xcb_get_geometry_unchecked(c, m_pixmap);
            QScopedPointer<xcb_get_window_attributes_reply_t, QScopedPointerPodDeleter> attr(xcb_get_window_attributes_reply(c, attrCookie, nullptr));
            QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter> geo(xcb_get_geometry_reply(c, geometryCookie, nullptr));

            if (attr.isNull()) {
                return false;
            }

            if (geo.isNull()) {
                return false;
            }

            m_depth = geo->depth;
            m_visualid = attr->visual;

            if (!loadGLXTexture()) {
                return false;
            }

            const uint32_t width = geo->width;
            const uint32_t height = geo->height;
            const uint32_t levels = std::log2(std::min(width, height)) + 1;

            funcs->glBindTexture(GL_TEXTURE_2D, m_mipmaps);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1);

            if (haveTextureStorage)
                extraFuncs->glTexStorage2D(GL_TEXTURE_2D, levels, sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, width, height);
            else
                funcs->glTexImage2D(GL_TEXTURE_2D, 0, sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            GLuint framebuffers[2];
            funcs->glGenFramebuffers(2, framebuffers);

            m_readFb = framebuffers[0];
            m_drawFb = framebuffers[1];

            ThumbnailTexture *texture = new ThumbnailTexture(m_mipmaps, QSize(width, height));
            textureNode->reset(texture);
        }

        funcs->glBindTexture(GL_TEXTURE_2D, m_texture);
        bindGLXTexture();

        const QSize size = textureNode->texture()->textureSize();

        // Blit the window texture to the mipmap texture
        funcs->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_readFb);
        funcs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_drawFb);

        funcs->glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
        funcs->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mipmaps, 0);

        // Disable sRGB encoding so there is no conversion in the blit
        if (sRGB && prevFramebufferSrgb)
            glDisable(GL_FRAMEBUFFER_SRGB);

        if (prevScissorTest)
            glDisable(GL_SCISSOR_TEST);

        extraFuncs->glBlitFramebuffer(0, 0, size.width(), size.height(),
                                      0, 0, size.width(), size.height(),
                                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

        funcs->glBindTexture(GL_TEXTURE_2D, m_mipmaps);

        if (sRGB) {
            // Enable sRGB encoding and decoding when generating the mipmaps
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
            funcs->glEnable(GL_FRAMEBUFFER_SRGB);
        }

        // Regenerate the miplevels
        funcs->glGenerateMipmap(GL_TEXTURE_2D);

        if (sRGB) {
            // Disable sRGB decoding again, so the texture is rendered correctly in the QtQuick scene
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);

            if (!prevFramebufferSrgb)
                funcs->glDisable(GL_FRAMEBUFFER_SRGB);
        }

        // Restore the GL state
        funcs->glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFb);
        funcs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFb);

        funcs->glBindTexture(GL_TEXTURE_2D, prevTex2D);

        if (prevScissorTest)
            glEnable(GL_SCISSOR_TEST);

        return true;
    }
    return false;
}
#endif // HAVE_GLX

#if HAVE_EGL
bool WindowThumbnail::xcbWindowToTextureEGL(WindowTextureNode *textureNode)
{
    EGLContext context = eglGetCurrentContext();

    if (context != EGL_NO_CONTEXT) {
        if (!m_eglFunctionsResolved) {
            resolveEGLFunctions();
        }
        if (QByteArray((char *)glGetString(GL_RENDERER)).contains("llvmpipe")) {
            return false;
        }
        if (!m_eglCreateImageKHR || !m_eglDestroyImageKHR || !m_glEGLImageTargetTexture2DOES) {
            return false;
        }
        if (m_image == EGL_NO_IMAGE_KHR) {
            xcb_connection_t *c = QX11Info::connection();
            auto geometryCookie = xcb_get_geometry_unchecked(c, m_pixmap);

            const EGLint attribs[] = {
                EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                EGL_NONE
            };
            m_image = ((eglCreateImageKHR_func)(m_eglCreateImageKHR))(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
                      EGL_NATIVE_PIXMAP_KHR,
                      (EGLClientBuffer)m_pixmap, attribs);

            if (m_image == EGL_NO_IMAGE_KHR) {
                qDebug() << "failed to create egl image";
                return false;
            }

            glGenTextures(1, &m_texture);
            QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter> geo(xcb_get_geometry_reply(c, geometryCookie, nullptr));
            QSize size;
            if (!geo.isNull()) {
                size.setWidth(geo->width);
                size.setHeight(geo->height);
            }
            textureNode->reset(window()->createTextureFromId(m_texture, size, QQuickWindow::TextureCanUseAtlas));
        }
        textureNode->texture()->bind();
        bindEGLTexture();
        return true;
    }
    return false;
}

void WindowThumbnail::resolveEGLFunctions()
{
    EGLDisplay display = eglGetCurrentDisplay();
    if (display == EGL_NO_DISPLAY) {
        return;
    }
    auto *context = window()->openglContext();
    QList<QByteArray> extensions = QByteArray(eglQueryString(display, EGL_EXTENSIONS)).split(' ');
    if (extensions.contains(QByteArrayLiteral("EGL_KHR_image")) ||
            (extensions.contains(QByteArrayLiteral("EGL_KHR_image_base")) &&
             extensions.contains(QByteArrayLiteral("EGL_KHR_image_pixmap")))) {
        if (context->hasExtension(QByteArrayLiteral("GL_OES_EGL_image"))) {
            qDebug() << "Have EGL texture from pixmap";
            m_eglCreateImageKHR = context->getProcAddress(QByteArrayLiteral("eglCreateImageKHR"));
            m_eglDestroyImageKHR = context->getProcAddress(QByteArrayLiteral("eglDestroyImageKHR"));
            m_glEGLImageTargetTexture2DOES = context->getProcAddress(QByteArrayLiteral("glEGLImageTargetTexture2DOES"));
        }
    }
    m_eglFunctionsResolved = true;
}

void WindowThumbnail::bindEGLTexture()
{
    ((glEGLImageTargetTexture2DOES_func)(m_glEGLImageTargetTexture2DOES))(GL_TEXTURE_2D, (GLeglImageOES)m_image);
    resetDamaged();
}
#endif // HAVE_EGL

#endif // HAVE_XCB_COMPOSITE

void WindowThumbnail::windowToTexture(WindowTextureNode *textureNode)
{
    if (!m_damaged && textureNode->texture()) {
        return;
    }
#if HAVE_XCB_COMPOSITE
    if (!textureNode->texture()) {
        // the texture got discarded by the scene graph, but our mapping is still valid
        // let's discard the pixmap to have a clean state again
        releaseResources();
    }
    if (m_pixmap == XCB_PIXMAP_NONE) {
        m_pixmap = pixmapForWindow();
    }
    if (m_pixmap == XCB_PIXMAP_NONE) {
        // create above failed
        iconToTexture(textureNode);
        setThumbnailAvailable(false);
        return;
    }
    bool fallbackToIcon = true;
#if HAVE_GLX
    fallbackToIcon = !windowToTextureGLX(textureNode);
#endif // HAVE_GLX
#if HAVE_EGL
    if (fallbackToIcon) {
        // if glx succeeded fallbackToIcon is false, thus we shouldn't try egl
        fallbackToIcon = !xcbWindowToTextureEGL(textureNode);
    }
#endif // HAVE_EGL
    if (fallbackToIcon) {
        // just for safety to not crash
        iconToTexture(textureNode);
    }
    setThumbnailAvailable(!fallbackToIcon);
    textureNode->markDirty(QSGNode::DirtyForceUpdate);
#else
    iconToTexture(textureNode);
#endif
}

#if HAVE_XCB_COMPOSITE
xcb_pixmap_t WindowThumbnail::pixmapForWindow()
{
    if (!m_composite) {
        return XCB_PIXMAP_NONE;
    }

    xcb_connection_t *c = QX11Info::connection();
    xcb_pixmap_t pix = xcb_generate_id(c);
    auto cookie = xcb_composite_name_window_pixmap_checked(c, m_winId, pix);
    QScopedPointer<xcb_generic_error_t, QScopedPointerPodDeleter> error(xcb_request_check(c, cookie));
    if (error) {
        return XCB_PIXMAP_NONE;
    }
    return pix;
}

#if HAVE_GLX
void WindowThumbnail::resolveGLXFunctions()
{
    auto *context = window()->openglContext();
    QList<QByteArray> extensions = QByteArray(glXQueryExtensionsString(QX11Info::display(), QX11Info::appScreen())).split(' ');
    if (extensions.contains(QByteArrayLiteral("GLX_EXT_texture_from_pixmap"))) {
        m_bindTexImage = context->getProcAddress(QByteArrayLiteral("glXBindTexImageEXT"));
        m_releaseTexImage = context->getProcAddress(QByteArrayLiteral("glXReleaseTexImageEXT"));
    } else
        qWarning() << "couldn't resolve GLX_EXT_texture_from_pixmap functions";
    m_openGLFunctionsResolved = true;
}

void WindowThumbnail::bindGLXTexture()
{
    Display *d = QX11Info::display();
    ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
    ((glXBindTexImageEXT_func)(m_bindTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT, NULL);
    resetDamaged();
}

struct FbConfigInfo
{
    GLXFBConfig fbConfig;
    int textureFormat;
};

struct GlxGlobalData
{
    GlxGlobalData() {
	xcb_connection_t * const conn = QX11Info::connection();

        // Fetch the render pict formats
        reply = xcb_render_query_pict_formats_reply(conn,
                        xcb_render_query_pict_formats_unchecked(conn), nullptr);

        // Init the visual ID -> format ID hash table
        for (auto screens = xcb_render_query_pict_formats_screens_iterator(reply); screens.rem; xcb_render_pictscreen_next(&screens)) {
            for (auto depths = xcb_render_pictscreen_depths_iterator(screens.data); depths.rem; xcb_render_pictdepth_next(&depths)) {
                const xcb_render_pictvisual_t *visuals = xcb_render_pictdepth_visuals(depths.data);
                const int len = xcb_render_pictdepth_visuals_length(depths.data);

                for (int i = 0; i < len; i++)
                    visualPictFormatHash.insert(visuals[i].visual, visuals[i].format);
            }
        }

        // Init the format ID -> xcb_render_directformat_t* hash table
        const xcb_render_pictforminfo_t *formats = xcb_render_query_pict_formats_formats(reply);
        const int len = xcb_render_query_pict_formats_formats_length(reply);

        for (int i = 0; i < len; i++) {
            if (formats[i].type == XCB_RENDER_PICT_TYPE_DIRECT)
                formatInfoHash.insert(formats[i].id, &formats[i].direct);
        }

        // Init the visual ID -> depth hash table
        const xcb_setup_t *setup = xcb_get_setup(conn);

        for (auto screen = xcb_setup_roots_iterator(setup); screen.rem; xcb_screen_next(&screen)) {
            for (auto depth = xcb_screen_allowed_depths_iterator(screen.data); depth.rem; xcb_depth_next(&depth)) {
                const int len = xcb_depth_visuals_length(depth.data);
                const xcb_visualtype_t *visuals = xcb_depth_visuals(depth.data);

                for (int i = 0; i < len; i++)
                    visualDepthHash.insert(visuals[i].visual_id, depth.data->depth);
            }
        }
    }

    ~GlxGlobalData() {
        qDeleteAll(visualFbConfigHash);
        std::free(reply);
    }

    xcb_render_query_pict_formats_reply_t *reply;
    QHash<xcb_visualid_t, xcb_render_pictformat_t> visualPictFormatHash;
    QHash<xcb_visualid_t, uint32_t> visualDepthHash;
    QHash<xcb_visualid_t, FbConfigInfo *> visualFbConfigHash;
    QHash<xcb_render_pictformat_t, const xcb_render_directformat_t *> formatInfoHash;
};

Q_GLOBAL_STATIC(GlxGlobalData, g_glxGlobalData)

static xcb_render_pictformat_t findPictFormat(xcb_visualid_t visual)
{
    GlxGlobalData *d = g_glxGlobalData;
    return d->visualPictFormatHash.value(visual);
}

static const xcb_render_directformat_t *findPictFormatInfo(xcb_render_pictformat_t format)
{
    GlxGlobalData *d = g_glxGlobalData;
    return d->formatInfoHash.value(format);
}

static int visualDepth(xcb_visualid_t visual)
{
    GlxGlobalData *d = g_glxGlobalData;
    return d->visualDepthHash.value(visual);
}

FbConfigInfo *getConfig(xcb_visualid_t visual)
{
    Display *dpy = QX11Info::display();
    const xcb_render_pictformat_t format = findPictFormat(visual);
    const xcb_render_directformat_t *direct = findPictFormatInfo(format);

    if (!direct) {
        return nullptr;
    }

    const int red_bits   = qPopulationCount(direct->red_mask);
    const int green_bits = qPopulationCount(direct->green_mask);
    const int blue_bits  = qPopulationCount(direct->blue_mask);
    const int alpha_bits = qPopulationCount(direct->alpha_mask);

    const int depth = visualDepth(visual);

    const auto rgb_sizes = std::tie(red_bits, green_bits, blue_bits);

    const int attribs[] = {
        GLX_RENDER_TYPE,                  GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE,                GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
        GLX_X_VISUAL_TYPE,                GLX_TRUE_COLOR,
        GLX_X_RENDERABLE,                 True,
        GLX_CONFIG_CAVEAT,                int(GLX_DONT_CARE), // The ARGB32 visual is marked non-conformant in Catalyst
        GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, int(GLX_DONT_CARE),
        GLX_BUFFER_SIZE,                  red_bits + green_bits + blue_bits + alpha_bits,
        GLX_RED_SIZE,                     red_bits,
        GLX_GREEN_SIZE,                   green_bits,
        GLX_BLUE_SIZE,                    blue_bits,
        GLX_ALPHA_SIZE,                   alpha_bits,
        GLX_STENCIL_SIZE,                 0,
        GLX_DEPTH_SIZE,                   0,
        0
    };

    if (QByteArray((char *)glGetString(GL_RENDERER)).contains("llvmpipe")) {
        return nullptr;
    }

    int count = 0;
    GLXFBConfig *configs = glXChooseFBConfig(dpy, QX11Info::appScreen(), attribs, &count);
    if (count < 1) {
        return nullptr;
    }

    struct FBConfig {
        GLXFBConfig config;
        int depth;
        int stencil;
        int format;
    };

    QList<FBConfig> candidates;

    for (int i = 0; i < count; i++) {
        int red, green, blue;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_RED_SIZE,   &red);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_GREEN_SIZE, &green);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BLUE_SIZE,  &blue);

        if (std::tie(red, green, blue) != rgb_sizes)
            continue;

        xcb_visualid_t visual;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_VISUAL_ID, (int *) &visual);

        if (visualDepth(visual) != depth)
            continue;

        int bind_rgb, bind_rgba;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &bind_rgba);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_RGB_EXT,  &bind_rgb);

        if (!bind_rgb && !bind_rgba)
            continue;

        int texture_targets;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &texture_targets);

        if ((texture_targets & GLX_TEXTURE_2D_BIT_EXT) == 0)
            continue;

        int depth, stencil;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_DEPTH_SIZE,   &depth);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_STENCIL_SIZE, &stencil);

        int texture_format;
        if (alpha_bits)
            texture_format = bind_rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
        else
            texture_format = bind_rgb ? GLX_TEXTURE_FORMAT_RGB_EXT : GLX_TEXTURE_FORMAT_RGBA_EXT;

        candidates.append(FBConfig{configs[i], depth, stencil, texture_format});
    }

    if (count > 0)
        XFree(configs);

    std::stable_sort(candidates.begin(), candidates.end(), [](const FBConfig &left, const FBConfig &right) {
        if (left.depth < right.depth)
            return true;

        if (left.stencil < right.stencil)
            return true;

        return false;
    });

    FbConfigInfo *info = nullptr;

    if (candidates.size() > 0) {
        const FBConfig &candidate = candidates.front();

        info = new FbConfigInfo;
        info->fbConfig      = candidate.config;
        info->textureFormat = candidate.format;
    }


    return info;
}

bool WindowThumbnail::loadGLXTexture()
{
    GLXContext glxContext = glXGetCurrentContext();
    if (!glxContext) {
        return false;
    }

    FbConfigInfo *info = nullptr;

    auto &hashTable = g_glxGlobalData->visualFbConfigHash;
    auto it = hashTable.constFind(m_visualid);

    if (it != hashTable.constEnd()) {
        info = *it;
    } else {
        info = getConfig(m_visualid);
        hashTable.insert(m_visualid, info);
    }

    if (!info) {
        return false;
    }

    GLuint textures[2];
    glGenTextures(2, textures);

    m_texture = textures[0];
    m_mipmaps = textures[1];

    const int attrs[] = {
        GLX_TEXTURE_FORMAT_EXT, info->textureFormat,
        GLX_MIPMAP_TEXTURE_EXT, false,
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        XCB_NONE
    };

    m_glxPixmap = glXCreatePixmap(QX11Info::display(), info->fbConfig, m_pixmap, attrs);

    return true;
}
#endif

#endif

void WindowThumbnail::resetDamaged()
{
    m_damaged = false;
#if HAVE_XCB_COMPOSITE
    if (m_damage == XCB_NONE) {
        return;
    }
    xcb_damage_subtract(QX11Info::connection(), m_damage, XCB_NONE, XCB_NONE);
#endif
}

void WindowThumbnail::stopRedirecting()
{
    if (!m_xcb || !m_composite) {
        return;
    }
#if HAVE_XCB_COMPOSITE
    xcb_connection_t *c = QX11Info::connection();
    if (m_pixmap != XCB_PIXMAP_NONE) {
        xcb_free_pixmap(c, m_pixmap);
        m_pixmap = XCB_PIXMAP_NONE;
    }
    if (m_winId == XCB_WINDOW_NONE) {
        return;
    }
    xcb_composite_unredirect_window(c, m_winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    if (m_damage == XCB_NONE) {
        return;
    }
    xcb_damage_destroy(c, m_damage);
    m_damage = XCB_NONE;
#endif
}

void WindowThumbnail::startRedirecting()
{
    if (!m_xcb || !m_composite || !window() || window()->winId() == m_winId) {
        return;
    }
#if HAVE_XCB_COMPOSITE
    if (m_winId == XCB_WINDOW_NONE) {
        return;
    }
    xcb_connection_t *c = QX11Info::connection();

    // need to get the window attributes for the existing event mask
    const auto attribsCookie = xcb_get_window_attributes_unchecked(c, m_winId);

    // redirect the window
    xcb_composite_redirect_window(c, m_winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    // generate the damage handle
    m_damage = xcb_generate_id(c);
    xcb_damage_create(c, m_damage, m_winId, XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

    QScopedPointer<xcb_get_window_attributes_reply_t, QScopedPointerPodDeleter> attr(xcb_get_window_attributes_reply(c, attribsCookie, nullptr));
    uint32_t events = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    if (!attr.isNull()) {
        events = events | attr->your_event_mask;
    }
    // the event mask will not be removed again. We cannot track whether another component also needs STRUCTURE_NOTIFY (e.g. KWindowSystem).
    // if we would remove the event mask again, other areas will break.
    xcb_change_window_attributes(c, m_winId, XCB_CW_EVENT_MASK, &events);
    // force to update the texture
    m_damaged = true;
#endif
}



void WindowThumbnail::setThumbnailAvailable(bool thumbnailAvailable)
{
    if (m_thumbnailAvailable != thumbnailAvailable) {
        m_thumbnailAvailable = thumbnailAvailable;
        emit thumbnailAvailableChanged();
    }
}

} // namespace
