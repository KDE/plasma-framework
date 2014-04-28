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
    , m_winId(0)
    , m_damaged(false)
#if HAVE_XCB_COMPOSITE
    , m_openGLFunctionsResolved(false)
    , m_damageEventBase(0)
    , m_damage(XCB_NONE)
    , m_pixmap(XCB_PIXMAP_NONE)
    , m_texture(0)
#if HAVE_GLX
    , m_glxPixmap(XCB_PIXMAP_NONE)
#endif // HAVE_GLX
#if HAVE_EGL
    , m_eglFunctionsResolved(false)
    , m_image(EGL_NO_IMAGE_KHR)
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
    });
    if (QGuiApplication *gui = dynamic_cast<QGuiApplication *>(QCoreApplication::instance())) {
        m_xcb = (gui->platformName() == QStringLiteral("xcb"));
        if (m_xcb) {
            gui->installNativeEventFilter(this);
#if HAVE_XCB_COMPOSITE
            xcb_connection_t *c = QX11Info::connection();
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
        discardPixmap();
    }
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
    startRedirecting();
    emit winIdChanged();
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
    const QSize size(node->texture()->textureSize().scaled(boundingRect().size().toSize(), Qt::KeepAspectRatio));
    const qreal x = boundingRect().x() + (boundingRect().width() - size.width()) / 2;
    const qreal y = boundingRect().y() + (boundingRect().height() - size.height()) / 2;
    node->setRect(QRectF(QPointF(x, y), size));
    return node;
}

bool WindowThumbnail::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
{
    Q_UNUSED(result)
    if (!m_xcb || eventType != QByteArrayLiteral("xcb_generic_event_t")) {
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
            // TODO: only discard if size changes
            discardPixmap();
        }
    }
#endif
    // do not filter out any events, there might be further WindowThumbnails for the same window
    return false;
}

void WindowThumbnail::iconToTexture(WindowTextureNode *textureNode)
{
    QIcon icon;
    if (KWindowSystem::self()->hasWId(m_winId)) {
        icon = KWindowSystem::self()->icon(m_winId);
    } else {
        // fallback to plasma icon
        icon = QIcon::fromTheme("plasma");
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

            if (!loadGLXTexture()) {
                return false;
            }

            QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter> geo(xcb_get_geometry_reply(c, geometryCookie, Q_NULLPTR));
            QSize size;
            if (!geo.isNull()) {
                size.setWidth(geo->width);
                size.setHeight(geo->height);
            }
            textureNode->reset(window()->createTextureFromId(m_texture, size, QQuickWindow::TextureOwnsGLTexture));
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
            QScopedPointer<xcb_get_geometry_reply_t, QScopedPointerPodDeleter> geo(xcb_get_geometry_reply(c, geometryCookie, Q_NULLPTR));
            QSize size;
            if (!geo.isNull()) {
                size.setWidth(geo->width);
                size.setHeight(geo->height);
            }
            textureNode->reset(window()->createTextureFromId(m_texture, size, QQuickWindow::TextureOwnsGLTexture));
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
#if HAVE_GLX
        if (m_glxPixmap != XCB_PIXMAP_NONE) {
            discardPixmap();
        }
#endif
#if HAVE_EGL
        if (m_image != EGL_NO_IMAGE_KHR) {
            discardPixmap();
        }
#endif
    }
    if (m_pixmap == XCB_PIXMAP_NONE) {
        m_pixmap = pixmapForWindow();
    }
    if (m_pixmap == XCB_PIXMAP_NONE) {
        // create above failed
        iconToTexture(textureNode);
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
    textureNode->markDirty(QSGNode::DirtyForceUpdate);
#else
    iconToTexture(textureNode);
#endif
}

#if HAVE_XCB_COMPOSITE
xcb_pixmap_t WindowThumbnail::pixmapForWindow()
{
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
        qDebug() << "Have texture from pixmap";
        m_bindTexImage = context->getProcAddress(QByteArrayLiteral("glXBindTexImageEXT"));
        m_releaseTexImage = context->getProcAddress(QByteArrayLiteral("glXReleaseTexImageEXT"));
    }
    m_openGLFunctionsResolved = true;
}

void WindowThumbnail::bindGLXTexture()
{
    Display *d = QX11Info::display();
    ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
    ((glXBindTexImageEXT_func)(m_bindTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT, NULL);
    resetDamaged();
}

bool WindowThumbnail::loadGLXTexture()
{
    GLXContext glxContext = glXGetCurrentContext();
    if (!glxContext) {
        return false;
    }
    Display *d = QX11Info::display();
    glGenTextures(1, &m_texture);
    int fbConfig = 0;
    glXQueryContext(d, glxContext, GLX_FBCONFIG_ID, &fbConfig);

    int fbAttribs[] = {
        GLX_FBCONFIG_ID, fbConfig, XCB_NONE
    };
    int count = 0;
    GLXFBConfig *fbConfigs = glXChooseFBConfig(d, QX11Info::appScreen(), fbAttribs, &count);
    if (count == 0) {
        qDebug() << "Didn't get our FBConfig";
        return false;
    }

    int bindRgb, bindRgba;
    glXGetFBConfigAttrib(d, fbConfigs[0], GLX_BIND_TO_TEXTURE_RGBA_EXT, &bindRgba);
    glXGetFBConfigAttrib(d, fbConfigs[0], GLX_BIND_TO_TEXTURE_RGB_EXT,  &bindRgb);

    XVisualInfo *vi = glXGetVisualFromFBConfig(d, fbConfigs[0]);
    int textureFormat;
    if (window()->openglContext()->format().hasAlpha()) {
        textureFormat = bindRgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
    } else {
        textureFormat = bindRgb ? GLX_TEXTURE_FORMAT_RGB_EXT : GLX_TEXTURE_FORMAT_RGBA_EXT;
    }
    XFree(vi);

    // we assume that Texture_2D is supported as we have a QtQuick OpenGL context
    int attrs[] = {
        GLX_TEXTURE_FORMAT_EXT, textureFormat,
        GLX_MIPMAP_TEXTURE_EXT, false,
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT, XCB_NONE
    };
    m_glxPixmap = glXCreatePixmap(d, fbConfigs[0], m_pixmap, attrs);
    return true;
}
#endif

#endif

void WindowThumbnail::resetDamaged()
{
    m_damaged = false;
#if HAVE_XCB_COMPOSITE
    xcb_damage_subtract(QX11Info::connection(), m_damage, XCB_NONE, XCB_NONE);
#endif
}

void WindowThumbnail::stopRedirecting()
{
    if (!m_xcb) {
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

    const uint32_t values[] = {XCB_EVENT_MASK_NO_EVENT};
    xcb_change_window_attributes(c, m_winId, XCB_CW_EVENT_MASK, values);
#endif
}

void WindowThumbnail::startRedirecting()
{
    if (!m_xcb || !window() || window()->winId() == m_winId) {
        return;
    }
#if HAVE_XCB_COMPOSITE
    xcb_connection_t *c = QX11Info::connection();

    // redirect the window
    xcb_composite_redirect_window(c, m_winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    // generate the damage handle
    m_damage = xcb_generate_id(c);
    xcb_damage_create(c, m_damage, m_winId, XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

    const uint32_t values[] = {XCB_EVENT_MASK_STRUCTURE_NOTIFY};
    xcb_change_window_attributes(c, m_winId, XCB_CW_EVENT_MASK, values);
    // force to update the texture
    m_damaged = true;
#endif
}

void WindowThumbnail::discardPixmap()
{
    if (!m_xcb) {
        return;
    }
#if HAVE_XCB_COMPOSITE
#if HAVE_GLX
    if (m_glxPixmap != XCB_PIXMAP_NONE) {
        Display *d = QX11Info::display();
        ((glXReleaseTexImageEXT_func)(m_releaseTexImage))(d, m_glxPixmap, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(d, m_glxPixmap);
        m_glxPixmap = XCB_PIXMAP_NONE;
        glDeleteTextures(1, &m_texture);
    }
#endif
#if HAVE_EGL
    if (m_image != EGL_NO_IMAGE_KHR) {
        ((eglDestroyImageKHR_func)(m_eglDestroyImageKHR))(eglGetCurrentDisplay(), m_image);
        m_image = EGL_NO_IMAGE_KHR;
        glDeleteTextures(1, &m_texture);
    }
#endif
    if (m_pixmap != XCB_WINDOW_NONE) {
        xcb_free_pixmap(QX11Info::connection(), m_pixmap);
        m_pixmap = XCB_PIXMAP_NONE;
    }
    m_damaged = true;
    update();
#endif
}

} // namespace
