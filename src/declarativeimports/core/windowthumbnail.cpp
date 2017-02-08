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
#include <QQuickWindow>
#include <QRunnable>

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

namespace Plasma
{

#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
class DiscardGlxPixmapRunnable : public QRunnable {
public:
    DiscardGlxPixmapRunnable(
        uint,
        QFunctionPointer,
        xcb_pixmap_t
    );
    void run() Q_DECL_OVERRIDE;
private:
    uint m_texture;
    QFunctionPointer m_releaseTexImage;
    xcb_pixmap_t m_glxPixmap;
};

DiscardGlxPixmapRunnable::DiscardGlxPixmapRunnable(uint texture, QFunctionPointer deleteFunction, xcb_pixmap_t pixmap)
    : QRunnable(),
    m_texture(texture),
    m_releaseTexImage(deleteFunction),
    m_glxPixmap(pixmap)
{}

void DiscardGlxPixmapRunnable::run()
{
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        Display *d = QX11Info::display();
        ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(d, m_glxPixmap);
        glDeleteTextures(1, &m_texture);
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
    // NoStage is supported since Qt >= 5.6.x
    #if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QQuickWindow::RenderStage m_renderStage = QQuickWindow::NoStage;
    #else
        QQuickWindow::RenderStage m_renderStage = QQuickWindow::BeforeSynchronizingStage;
    #endif
#endif

    //data is deleted in the render thread (with relevant GLX calls)
    //note runnable may be called *after* this is deleted
    //but the pointer is held by the WindowThumbnail which is in the main thread
#if HAVE_GLX
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        window()->scheduleRenderJob(new DiscardGlxPixmapRunnable(m_texture,
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
    textureNode->reset(window()->createTextureFromImage(image));
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
        if (m_glxPixmap == XCB_PIXMAP_NONE) {
            xcb_connection_t *c = QX11Info::connection();
            auto geometryCookie = xcb_get_geometry_unchecked(c, m_pixmap);
            QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter> geo(xcb_get_geometry_reply(c, geometryCookie, nullptr));

            if (geo.isNull()) {
                return false;
            }
            m_depth = geo->depth;

            if (!loadGLXTexture()) {
                return false;
            }

            textureNode->reset(window()->createTextureFromId(m_texture, QSize(geo->width, geo->height)));
        }
        textureNode->texture()->bind();
        bindGLXTexture();
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
            textureNode->reset(window()->createTextureFromId(m_texture, size));
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

GLXFBConfig *getConfig(int depth, int *index)
{
    const int attribs[] = {
        GLX_RENDER_TYPE,              GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE,            GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
        GLX_X_VISUAL_TYPE,            GLX_TRUE_COLOR,
        GLX_X_RENDERABLE,             1,
        GLX_CONFIG_CAVEAT,            int(GLX_DONT_CARE),
        GLX_RED_SIZE,                 5,
        GLX_GREEN_SIZE,               5,
        GLX_BLUE_SIZE,                5,
        GLX_ALPHA_SIZE,               0,
        GLX_STENCIL_SIZE,             0,
        GLX_DEPTH_SIZE,               0, // note: this is depth buffer and has nothing to do with the X depth
        GLX_BIND_TO_TEXTURE_RGB_EXT,  (depth == 32) ? int(GLX_DONT_CARE) : 1,
        GLX_BIND_TO_TEXTURE_RGBA_EXT, (depth == 32) ? 1 : int(GLX_DONT_CARE),
        0
    };

    if (QByteArray((char *)glGetString(GL_RENDERER)).contains("llvmpipe")) {
        return nullptr;
    }

    int count = 0;
    GLXFBConfig *fbConfigs = glXChooseFBConfig(QX11Info::display(), QX11Info::appScreen(), attribs, &count);
    if (count < 1) {
        return nullptr;
    }

    for (int i = 0; i < count; ++i) {
        int alphaSize = 0;
        int bufferSize = 0;
        glXGetFBConfigAttrib(QX11Info::display(), fbConfigs[i], GLX_BUFFER_SIZE, &bufferSize);
        glXGetFBConfigAttrib(QX11Info::display(), fbConfigs[i], GLX_ALPHA_SIZE, &alphaSize);
        if (bufferSize != depth && (bufferSize - alphaSize) != depth) {
            continue;
        }
        XVisualInfo *vi = glXGetVisualFromFBConfig(QX11Info::display(), fbConfigs[i]);
        if (!vi) {
            // no visual for the fb config - skip
            continue;
        }
        const int visualDepth = vi->depth;
        XFree(vi);

        if (visualDepth != depth) {
            // depth of the visual doesn't match our wanted depth - skip
            continue;
        }
        *index = i;
        break;
    }

    return fbConfigs;
}

bool WindowThumbnail::loadGLXTexture()
{
    GLXContext glxContext = glXGetCurrentContext();
    if (!glxContext) {
        return false;
    }

    // this is a cache of the GLXFBConfig per depth
    // it's kept as a method static to have it shared between multiple
    // window thumbnails.
    // As the GLXFBConfig might be context specific and we cannot be sure
    // that the code might be entered from different contexts, the cache
    // also maps the cached configs against the context.
    static QHash<GLXContext, QMap<int, GLXFBConfig> > s_fbConfigs;
    auto it = s_fbConfigs.find(glxContext);
    if (it == s_fbConfigs.end()) {
        // create a map entry for the current context
        s_fbConfigs.insert(glxContext, QMap<int, GLXFBConfig>());
        it = s_fbConfigs.find(glxContext);
        if (it == s_fbConfigs.end()) {
            // just for safety, should never ever happen
            return false;
        }
    }
    auto &configMap = it.value();
    auto configIt = configMap.constFind(m_depth);
    if (configIt == configMap.constEnd()) {
        // try getting a new fbconfig for the current depth
        int index = 0;
        GLXFBConfig *fbConfigs = getConfig(m_depth, &index);
        if (!fbConfigs) {
            return false;
        }
        configMap.insert(m_depth, fbConfigs[index]);
        XFree(fbConfigs);

        configIt = configMap.constFind(m_depth);
        if (configIt == configMap.constEnd()) {
            // just for safety, should never ever happen
            return false;
        }
    }

    glGenTextures(1, &m_texture);

    // we assume that Texture_2D is supported as we have a QtQuick OpenGL context
    int attrs[] = {
        GLX_TEXTURE_FORMAT_EXT, (m_depth == 32) ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT,
        GLX_MIPMAP_TEXTURE_EXT, false,
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT, XCB_NONE
    };
    m_glxPixmap = glXCreatePixmap(QX11Info::display(), configIt.value(), m_pixmap, attrs);
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
