/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "windowthumbnail.h"
// KF5
#include <KX11Extras>
// Qt
#include <QGuiApplication>
#include <QIcon>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QRunnable>
#include <QSGImageNode>

// X11
#if HAVE_XCB_COMPOSITE
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <xcb/composite.h>
#if HAVE_GLX
#include <GL/glx.h>
typedef void (*glXBindTexImageEXT_func)(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
typedef void (*glXReleaseTexImageEXT_func)(Display *dpy, GLXDrawable drawable, int buffer);
#endif
#if HAVE_EGL
typedef EGLImageKHR (*eglCreateImageKHR_func)(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLint *);
typedef EGLBoolean (*eglDestroyImageKHR_func)(EGLDisplay, EGLImageKHR);
typedef GLvoid (*glEGLImageTargetTexture2DOES_func)(GLenum, GLeglImageOES);
#endif // HAVE_EGL
#endif

#include <cstdlib>

namespace Plasma
{
class DiscardTextureProviderRunnable : public QRunnable
{
public:
    explicit DiscardTextureProviderRunnable(WindowTextureProvider *provider)
        : m_provider(provider)
    {
    }

    void run() override
    {
        delete m_provider;
    }

private:
    WindowTextureProvider *m_provider;
};

#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
class DiscardGlxPixmapRunnable : public QRunnable
{
public:
    DiscardGlxPixmapRunnable(uint, QFunctionPointer, xcb_pixmap_t);
    void run() override;

private:
    uint m_texture;
    QFunctionPointer m_releaseTexImage;
    xcb_pixmap_t m_glxPixmap;
};

DiscardGlxPixmapRunnable::DiscardGlxPixmapRunnable(uint texture, QFunctionPointer deleteFunction, xcb_pixmap_t pixmap)
    : QRunnable()
    , m_texture(texture)
    , m_releaseTexImage(deleteFunction)
    , m_glxPixmap(pixmap)
{
}

void DiscardGlxPixmapRunnable::run()
{
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        Display *d = QX11Info::display();
        ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(d, m_glxPixmap);
        glDeleteTextures(1, &m_texture);
    }
}
#endif // HAVE_GLX

#if HAVE_EGL
class DiscardEglPixmapRunnable : public QRunnable
{
public:
    DiscardEglPixmapRunnable(uint, QFunctionPointer, EGLImageKHR);
    void run() override;

private:
    uint m_texture;
    QFunctionPointer m_eglDestroyImageKHR;
    EGLImageKHR m_image;
};

DiscardEglPixmapRunnable::DiscardEglPixmapRunnable(uint texture, QFunctionPointer deleteFunction, EGLImageKHR image)
    : QRunnable()
    , m_texture(texture)
    , m_eglDestroyImageKHR(deleteFunction)
    , m_image(image)
{
}

void DiscardEglPixmapRunnable::run()
{
    if (m_image != EGL_NO_IMAGE_KHR) {
        ((eglDestroyImageKHR_func)(m_eglDestroyImageKHR))(eglGetCurrentDisplay(), m_image);
        glDeleteTextures(1, &m_texture);
    }
}
#endif // HAVE_EGL
#endif // HAVE_XCB_COMPOSITE

QSGTexture *WindowTextureProvider::texture() const
{
    return m_texture.get();
}

void WindowTextureProvider::setTexture(QSGTexture *texture)
{
    m_texture.reset(texture);
    Q_EMIT textureChanged();
}

WindowThumbnail::WindowThumbnail(QQuickItem *parent)
    : QQuickItem(parent)
    , QAbstractNativeEventFilter()
    , m_xcb(false)
    , m_composite(false)
    , m_winId(0)
    , m_paintedSize(QSizeF())
    , m_thumbnailAvailable(false)
    , m_redirecting(false)
    , m_damaged(false)
    , m_depth(0)
#if HAVE_XCB_COMPOSITE
    , m_openGLFunctionsResolved(false)
    , m_damageEventBase(0)
    , m_damage(XCB_NONE)
    , m_pixmap(XCB_PIXMAP_NONE)
    , m_texture(0)
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

    if (QGuiApplication *gui = dynamic_cast<QGuiApplication *>(QCoreApplication::instance())) {
        m_xcb = (gui->platformName() == QLatin1String("xcb"));
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

void WindowThumbnail::itemChange(ItemChange change, const ItemChangeData &data)
{
    switch (change) {
    case ItemSceneChange:
        if (m_scene) {
            disconnect(m_scene.data(), &QWindow::visibleChanged, this, &WindowThumbnail::sceneVisibilityChanged);
        }
        m_scene = data.window;
        if (m_scene) {
            connect(m_scene.data(), &QWindow::visibleChanged, this, &WindowThumbnail::sceneVisibilityChanged);
            // restart the redirection, it might not have been active yet
            stopRedirecting();
            if (startRedirecting()) {
                update();
            }
        }
        break;

    case ItemEnabledHasChanged:
        Q_FALLTHROUGH();
    case ItemVisibleHasChanged:
        if (data.boolValue) {
            if (startRedirecting()) {
                update();
            }
        } else {
            stopRedirecting();
            releaseResources();
        }
        break;

    default:
        break;
    }
}

void WindowThumbnail::releaseResources()
{
    QQuickWindow::RenderStage m_renderStage = QQuickWindow::NoStage;
    if (m_textureProvider) {
        window()->scheduleRenderJob(new DiscardTextureProviderRunnable(m_textureProvider), m_renderStage);
        m_textureProvider = nullptr;
    }

#if HAVE_XCB_COMPOSITE

#if HAVE_GLX && HAVE_EGL
    // only one (or none) should be set, but never both
    Q_ASSERT(m_glxPixmap == XCB_PIXMAP_NONE || m_image == EGL_NO_IMAGE_KHR);
#endif

    // data is deleted in the render thread (with relevant GLX calls)
    // note runnable may be called *after* this is deleted
    // but the pointer is held by the WindowThumbnail which is in the main thread
#if HAVE_GLX
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        window()->scheduleRenderJob(new DiscardGlxPixmapRunnable(m_texture, m_releaseTexImage, m_glxPixmap), m_renderStage);

        m_glxPixmap = XCB_PIXMAP_NONE;
        m_texture = 0;
    }
#endif
#if HAVE_EGL
    if (m_image != EGL_NO_IMAGE_KHR) {
        window()->scheduleRenderJob(new DiscardEglPixmapRunnable(m_texture, m_eglDestroyImageKHR, m_image), m_renderStage);
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
    if (!KX11Extras::self()->hasWId(winId)) {
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

    Q_EMIT winIdChanged();
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

    if (!m_textureProvider) {
        m_textureProvider = new WindowTextureProvider();
    }

    if (!m_xcb || m_winId == 0 || (window() && window()->winId() == m_winId)) {
        iconToTexture(m_textureProvider);
    } else {
        windowToTexture(m_textureProvider);
    }

    QSGImageNode *node = static_cast<QSGImageNode *>(oldNode);
    if (!node) {
        node = window()->createImageNode();
        qsgnode_set_description(node, QStringLiteral("windowthumbnail"));
        node->setFiltering(QSGTexture::Linear);
    }

    node->setTexture(m_textureProvider->texture());
    const QSizeF size(node->texture()->textureSize().scaled(boundingRect().size().toSize(), Qt::KeepAspectRatio));
    if (size != m_paintedSize) {
        m_paintedSize = size;
        Q_EMIT paintedSizeChanged();
    }
    const qreal x = boundingRect().x() + (boundingRect().width() - size.width()) / 2;
    const qreal y = boundingRect().y() + (boundingRect().height() - size.height()) / 2;
    node->setRect(QRectF(QPointF(x, y), size));
    return node;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool WindowThumbnail::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
#else
bool WindowThumbnail::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#endif
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

void WindowThumbnail::iconToTexture(WindowTextureProvider *textureProvider)
{
    QIcon icon;
    if (KX11Extras::self()->hasWId(m_winId)) {
        icon = KX11Extras::self()->icon(m_winId, boundingRect().width(), boundingRect().height());
    } else {
        // fallback to plasma icon
        icon = QIcon::fromTheme(QStringLiteral("plasma"));
    }
    QImage image = icon.pixmap(window(), boundingRect().size().toSize()).toImage();
    textureProvider->setTexture(window()->createTextureFromImage(image, QQuickWindow::TextureCanUseAtlas));
}

#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
bool WindowThumbnail::windowToTextureGLX(WindowTextureProvider *textureProvider)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (window()->openglContext()) {
#else
    const auto openglContext = static_cast<QOpenGLContext *>(window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource));
    if (openglContext) {
#endif
        if (!m_openGLFunctionsResolved) {
            resolveGLXFunctions();
        }
        if (!m_bindTexImage || !m_releaseTexImage) {
            return false;
        }
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            textureProvider->setTexture(window()->createTextureFromId(m_texture, QSize(geo->width, geo->height), QQuickWindow::TextureCanUseAtlas));
        }
        textureProvider->texture()->bind();
#else
            textureProvider->setTexture(
                QNativeInterface::QSGOpenGLTexture::fromNative(m_texture, window(), QSize(geo->width, geo->height), QQuickWindow::TextureCanUseAtlas));
        }
        openglContext->functions()->glBindTexture(GL_TEXTURE_2D, m_texture);
#endif
        bindGLXTexture();
        return true;
    }
    return false;
}
#endif // HAVE_GLX

#if HAVE_EGL
bool WindowThumbnail::xcbWindowToTextureEGL(WindowTextureProvider *textureProvider)
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

            const EGLint attribs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
            m_image = ((eglCreateImageKHR_func)(
                m_eglCreateImageKHR))(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)(uintptr_t)m_pixmap, attribs);

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            textureProvider->setTexture(window()->createTextureFromId(m_texture, size, QQuickWindow::TextureCanUseAtlas));
        }
        textureProvider->texture()->bind();
#else
            textureProvider->setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(m_texture, window(), size, QQuickWindow::TextureCanUseAtlas));
        }
        auto *openglContext = static_cast<QOpenGLContext *>(window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource));
        openglContext->functions()->glBindTexture(GL_TEXTURE_2D, m_texture);
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto *context = window()->openglContext();
#else
    auto *context = static_cast<QOpenGLContext *>(window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource));
#endif
    QList<QByteArray> extensions = QByteArray(eglQueryString(display, EGL_EXTENSIONS)).split(' ');
    if (extensions.contains(QByteArrayLiteral("EGL_KHR_image")) //
        || (extensions.contains(QByteArrayLiteral("EGL_KHR_image_base")) //
            && extensions.contains(QByteArrayLiteral("EGL_KHR_image_pixmap")))) {
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

void WindowThumbnail::windowToTexture(WindowTextureProvider *textureProvider)
{
    if (!m_damaged && textureProvider->texture()) {
        return;
    }
#if HAVE_XCB_COMPOSITE
    if (m_pixmap == XCB_PIXMAP_NONE) {
        m_pixmap = pixmapForWindow();
    }
    if (m_pixmap == XCB_PIXMAP_NONE) {
        // create above failed
        iconToTexture(textureProvider);
        setThumbnailAvailable(false);
        return;
    }
    bool fallbackToIcon = true;
#if HAVE_GLX
    fallbackToIcon = !windowToTextureGLX(textureProvider);
#endif // HAVE_GLX
#if HAVE_EGL
    if (fallbackToIcon) {
        // if glx succeeded fallbackToIcon is false, thus we shouldn't try egl
        fallbackToIcon = !xcbWindowToTextureEGL(textureProvider);
    }
#endif // HAVE_EGL
    if (fallbackToIcon) {
        // just for safety to not crash
        iconToTexture(textureProvider);
    }
    setThumbnailAvailable(!fallbackToIcon);
#else
    iconToTexture(textureProvider);
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto *context = window()->openglContext();
#else
    auto *context = static_cast<QOpenGLContext *>(window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource));
#endif
    QList<QByteArray> extensions = QByteArray(glXQueryExtensionsString(QX11Info::display(), QX11Info::appScreen())).split(' ');
    if (extensions.contains(QByteArrayLiteral("GLX_EXT_texture_from_pixmap"))) {
        m_bindTexImage = context->getProcAddress(QByteArrayLiteral("glXBindTexImageEXT"));
        m_releaseTexImage = context->getProcAddress(QByteArrayLiteral("glXReleaseTexImageEXT"));
    } else {
        qWarning() << "couldn't resolve GLX_EXT_texture_from_pixmap functions";
    }
    m_openGLFunctionsResolved = true;
}

void WindowThumbnail::bindGLXTexture()
{
    Display *d = QX11Info::display();
    ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
    ((glXBindTexImageEXT_func)(m_bindTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT, nullptr);
    resetDamaged();
}

struct FbConfigInfo {
    GLXFBConfig fbConfig;
    int textureFormat;
};

struct GlxGlobalData {
    GlxGlobalData()
    {
        xcb_connection_t *const conn = QX11Info::connection();

        // Fetch the render pict formats
        reply = xcb_render_query_pict_formats_reply(conn, xcb_render_query_pict_formats_unchecked(conn), nullptr);

        // Init the visual ID -> format ID hash table
        for (auto screens = xcb_render_query_pict_formats_screens_iterator(reply); screens.rem; xcb_render_pictscreen_next(&screens)) {
            for (auto depths = xcb_render_pictscreen_depths_iterator(screens.data); depths.rem; xcb_render_pictdepth_next(&depths)) {
                const xcb_render_pictvisual_t *visuals = xcb_render_pictdepth_visuals(depths.data);
                const int len = xcb_render_pictdepth_visuals_length(depths.data);

                for (int i = 0; i < len; i++) {
                    visualPictFormatHash.insert(visuals[i].visual, visuals[i].format);
                }
            }
        }

        // Init the format ID -> xcb_render_directformat_t* hash table
        const xcb_render_pictforminfo_t *formats = xcb_render_query_pict_formats_formats(reply);
        const int len = xcb_render_query_pict_formats_formats_length(reply);

        for (int i = 0; i < len; i++) {
            if (formats[i].type == XCB_RENDER_PICT_TYPE_DIRECT) {
                formatInfoHash.insert(formats[i].id, &formats[i].direct);
            }
        }

        // Init the visual ID -> depth hash table
        const xcb_setup_t *setup = xcb_get_setup(conn);

        for (auto screen = xcb_setup_roots_iterator(setup); screen.rem; xcb_screen_next(&screen)) {
            for (auto depth = xcb_screen_allowed_depths_iterator(screen.data); depth.rem; xcb_depth_next(&depth)) {
                const int len = xcb_depth_visuals_length(depth.data);
                const xcb_visualtype_t *visuals = xcb_depth_visuals(depth.data);

                for (int i = 0; i < len; i++) {
                    visualDepthHash.insert(visuals[i].visual_id, depth.data->depth);
                }
            }
        }
    }

    ~GlxGlobalData()
    {
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

    const int red_bits = qPopulationCount(direct->red_mask);
    const int green_bits = qPopulationCount(direct->green_mask);
    const int blue_bits = qPopulationCount(direct->blue_mask);
    const int alpha_bits = qPopulationCount(direct->alpha_mask);

    const int depth = visualDepth(visual);

    const auto rgb_sizes = std::tie(red_bits, green_bits, blue_bits);

    const int attribs[] = {GLX_RENDER_TYPE,
                           GLX_RGBA_BIT,
                           GLX_DRAWABLE_TYPE,
                           GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
                           GLX_X_VISUAL_TYPE,
                           GLX_TRUE_COLOR,
                           GLX_X_RENDERABLE,
                           True,
                           GLX_CONFIG_CAVEAT,
                           int(GLX_DONT_CARE), // The ARGB32 visual is marked non-conformant in Catalyst
                           GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT,
                           int(GLX_DONT_CARE),
                           GLX_BUFFER_SIZE,
                           red_bits + green_bits + blue_bits + alpha_bits,
                           GLX_RED_SIZE,
                           red_bits,
                           GLX_GREEN_SIZE,
                           green_bits,
                           GLX_BLUE_SIZE,
                           blue_bits,
                           GLX_ALPHA_SIZE,
                           alpha_bits,
                           GLX_STENCIL_SIZE,
                           0,
                           GLX_DEPTH_SIZE,
                           0,
                           0};

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
        int red;
        int green;
        int blue;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_RED_SIZE, &red);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_GREEN_SIZE, &green);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BLUE_SIZE, &blue);

        if (std::tie(red, green, blue) != rgb_sizes) {
            continue;
        }

        xcb_visualid_t visual;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_VISUAL_ID, (int *)&visual);

        if (visualDepth(visual) != depth) {
            continue;
        }

        int bind_rgb;
        int bind_rgba;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &bind_rgba);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &bind_rgb);

        if (!bind_rgb && !bind_rgba) {
            continue;
        }

        int texture_targets;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &texture_targets);

        if ((texture_targets & GLX_TEXTURE_2D_BIT_EXT) == 0) {
            continue;
        }

        int depth;
        int stencil;
        glXGetFBConfigAttrib(dpy, configs[i], GLX_DEPTH_SIZE, &depth);
        glXGetFBConfigAttrib(dpy, configs[i], GLX_STENCIL_SIZE, &stencil);

        int texture_format;
        if (alpha_bits) {
            texture_format = bind_rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
        } else {
            texture_format = bind_rgb ? GLX_TEXTURE_FORMAT_RGB_EXT : GLX_TEXTURE_FORMAT_RGBA_EXT;
        }

        candidates.append(FBConfig{configs[i], depth, stencil, texture_format});
    }

    XFree(configs);

    std::stable_sort(candidates.begin(), candidates.end(), [](const FBConfig &left, const FBConfig &right) {
        if (left.depth < right.depth) {
            return true;
        }

        if (left.stencil < right.stencil) {
            return true;
        }

        return false;
    });

    FbConfigInfo *info = nullptr;

    if (!candidates.isEmpty()) {
        const FBConfig &candidate = candidates.front();

        info = new FbConfigInfo;
        info->fbConfig = candidate.config;
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

    glGenTextures(1, &m_texture);

    /* clang-format off */
    const int attrs[] = {
        GLX_TEXTURE_FORMAT_EXT,
        info->textureFormat,
        GLX_MIPMAP_TEXTURE_EXT,
        false,
        GLX_TEXTURE_TARGET_EXT,
        GLX_TEXTURE_2D_EXT,
        XCB_NONE};
    /* clang-format on */

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
    if (m_redirecting) {
        xcb_composite_unredirect_window(c, m_winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    }
    m_redirecting = false;
    if (m_damage == XCB_NONE) {
        return;
    }
    xcb_damage_destroy(c, m_damage);
    m_damage = XCB_NONE;
#endif
}

bool WindowThumbnail::startRedirecting()
{
    if (!m_xcb || !m_composite || !window() || !window()->isVisible() || window()->winId() == m_winId || !isEnabled() || !isVisible()) {
        return false;
    }
#if HAVE_XCB_COMPOSITE
    if (m_winId == XCB_WINDOW_NONE) {
        return false;
    }
    xcb_connection_t *c = QX11Info::connection();

    // need to get the window attributes for the existing event mask
    const auto attribsCookie = xcb_get_window_attributes_unchecked(c, m_winId);

    // redirect the window
    xcb_composite_redirect_window(c, m_winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    m_redirecting = true;

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
    return true;
#else
    return false;
#endif
}

void WindowThumbnail::setThumbnailAvailable(bool thumbnailAvailable)
{
    if (m_thumbnailAvailable != thumbnailAvailable) {
        m_thumbnailAvailable = thumbnailAvailable;
        Q_EMIT thumbnailAvailableChanged();
    }
}

void WindowThumbnail::sceneVisibilityChanged(bool visible)
{
    if (visible) {
        if (startRedirecting()) {
            update();
        }
    } else {
        stopRedirecting();
        releaseResources();
    }
}

bool WindowThumbnail::isTextureProvider() const
{
    return true;
}

QSGTextureProvider *WindowThumbnail::textureProvider() const
{
    // When Item::layer::enabled == true, QQuickItem will be a texture
    // provider. In this case we should prefer to return the layer rather
    // than our texture
    if (QQuickItem::isTextureProvider()) {
        return QQuickItem::textureProvider();
    }

    if (!m_textureProvider) {
        m_textureProvider = new WindowTextureProvider();
    }

    return m_textureProvider;
}

} // namespace
