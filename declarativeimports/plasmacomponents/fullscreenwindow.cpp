/***************************************************************************
 *   Copyright 2012 Marco Martin <mart@kde.org>                            *
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

#include "fullscreenwindow.h"
#include "../core/declarativeitemcontainer_p.h"
#include "plasmacomponentsplugin.h"

#include <QApplication>
#include <QDeclarativeItem>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QLayout>
#include <QTimer>
#include <QDesktopWidget>
#include <QFile>

#include <KWindowSystem>
#include <KStandardDirs>

#include <kdeclarative.h>

#include <Plasma/Corona>
#include <Plasma/WindowEffects>


uint FullScreenWindow::s_numItems = 0;

class Background : public QWidget
{
public:
    Background(FullScreenWindow *dialog)
        : QWidget( 0L ),
          m_dialog(dialog)
    {
        setAttribute( Qt::WA_NoSystemBackground );
        setAttribute( Qt::WA_TranslucentBackground );

        setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
        KWindowSystem::setOnAllDesktops(winId(), true);
        unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager | NET::MaxVert | NET::MaxHoriz;
        KWindowSystem::setState(effectiveWinId(), state);
    }

    ~Background()
    {}

    void paintEvent( QPaintEvent *e )
    {
        QPainter painter( this );
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(e->rect(), QColor(0, 0, 0, 80));
    }

    void mousePressEvent(QMouseEvent *event)
    {
        event->accept();
        m_dialog->view()->winId();
        KWindowSystem::forceActiveWindow(m_dialog->view()->winId());
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        if (!m_dialog->view()->geometry().contains(event->globalPos())) {
            emit m_dialog->clickedOutside();
            m_dialog->close();
        }
    }

private:
    FullScreenWindow *m_dialog;
};

FullScreenWindow::FullScreenWindow(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_declarativeItemContainer(0)
{
    m_view = new QGraphicsView();
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->installEventFilter(this);
    m_view->setAutoFillBackground(false);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setAttribute(Qt::WA_TranslucentBackground);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    m_view->setCacheMode(QGraphicsView::CacheNone);
    m_view->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    m_view->setFrameShape(QFrame::NoFrame);
    KWindowSystem::setOnAllDesktops(m_view->winId(), true);
    unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager;
    KWindowSystem::setState(m_view->effectiveWinId(), state);

    m_background = new Background(this);
}

FullScreenWindow::~FullScreenWindow()
{
    delete m_view;
}

void FullScreenWindow::init(const QString &componentName)
{
    if (m_rootObject) {
        return;
    }

    //Try to figure out the path of the dialog component
    const QString target = KDeclarative::componentsTarget();
    QString filePath;
    if (target != KDeclarative::defaultComponentsTarget()) {
        const QString file = "platformimports/" % target % "/org/kde/plasma/components/" % componentName % ".qml";
        filePath = KStandardDirs::locate("module", file);
    }

    if (filePath.isEmpty()) {
        const QString file = "imports/org/kde/plasma/components/" % componentName % ".qml";
        filePath = KStandardDirs::locate("module", file);
    }

    if (filePath.isEmpty()) {
        kWarning() << "Component not found:" << componentName;
        return;
    }

    QDeclarativeEngine *engine = EngineBookKeeping::self()->engine();
    if (!engine) {
        kWarning() << "Warning, no QDeclarativeEngines available anymore, should never happen";
        Q_ASSERT(0);
    }
    QDeclarativeComponent *component = new QDeclarativeComponent(engine, filePath, this);

    QDeclarativeContext *creationContext = component->creationContext();
    m_rootObject = component->create(creationContext);
    if (component->status() == QDeclarativeComponent::Error) {
        kWarning()<<component->errors();
    }

    if (m_rootObject) {
        setMainItem(qobject_cast<QGraphicsObject *>(m_rootObject.data()));
        connect(m_rootObject.data(), SIGNAL(statusChanged()), this, SLOT(statusHasChanged()));
        connect(m_rootObject.data(), SIGNAL(accepted()), this, SIGNAL(accepted()));
        connect(m_rootObject.data(), SIGNAL(rejected()), this, SIGNAL(rejected()));
        connect(m_rootObject.data(), SIGNAL(clickedOutside()), this, SIGNAL(clickedOutside()));
    }
}

QGraphicsObject *FullScreenWindow::mainItem() const
{
    return m_mainItem.data();
}

void FullScreenWindow::setMainItem(QGraphicsObject *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        if (m_mainItem) {
            m_mainItem.data()->setParent(mainItem->parent());
            m_mainItem.data()->removeEventFilter(this);
            m_mainItem.data()->setY(0);
            m_scene = 0;
        }
        m_mainItem = mainItem;
        if (mainItem) {
            mainItem->setParentItem(0);
            mainItem->setParent(this);
            m_scene = mainItem->scene();
            m_view->resize(mainItem->boundingRect().size().toSize());
            mainItem->installEventFilter(this);
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        QTimer::singleShot(0, this, SLOT(syncViewToMainItem()));
    }
}

void FullScreenWindow::syncViewToMainItem()
{
    if (!m_mainItem) {
        return;
    }

    //not have a scene? go up in the hyerarchy until we find something with a scene
    QGraphicsScene *scene = m_mainItem.data()->scene();
    if (!scene) {
        QObject *parent = m_mainItem.data();
        while ((parent = parent->parent())) {
            QGraphicsObject *qo = qobject_cast<QGraphicsObject *>(parent);
            if (qo) {
                scene = qo->scene();

                if (scene) {
                    scene->addItem(m_mainItem.data());
                    ++s_numItems;
                    Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(scene);
                    QDeclarativeItem *di = qobject_cast<QDeclarativeItem *>(m_mainItem.data());

                    if (corona && di) {
                        if (!m_declarativeItemContainer) {
                            m_declarativeItemContainer = new DeclarativeItemContainer();
                            scene->addItem(m_declarativeItemContainer);
                            corona->addOffscreenWidget(m_declarativeItemContainer);
                        }
                        m_declarativeItemContainer->setDeclarativeItem(di);
                    } else {
                        m_mainItem.data()->setY(-10000*s_numItems);
                        m_mainItem.data()->setY(10000*s_numItems);
                    }
                    break;
                }
            }
        }
    }

    if (!scene) {
        return;
    }

    m_view->setScene(scene);


    QRectF itemGeometry(QPointF(m_mainItem.data()->x(), m_mainItem.data()->y()),
                        QSizeF(m_mainItem.data()->boundingRect().size()));
    if (m_declarativeItemContainer) {
        m_view->resize(itemGeometry.size().toSize());
        m_view->setSceneRect(m_declarativeItemContainer->geometry());

    } else {
        QRectF itemGeometry(QPointF(m_mainItem.data()->x(), m_mainItem.data()->y()),
                        QSizeF(m_mainItem.data()->boundingRect().size()));
        m_view->resize(itemGeometry.size().toSize());
        m_view->setSceneRect(itemGeometry);
    }

    m_view->move(QApplication::desktop()->availableGeometry().center() - QPoint(m_view->width()/2, m_view->height()/2));
}

void FullScreenWindow::syncMainItemToView()
{
    if (!m_mainItem) {
        return;
    }

    m_mainItem.data()->setProperty("width", m_view->width());
    m_mainItem.data()->setProperty("height", m_view->height());

    if (m_declarativeItemContainer) {
        m_declarativeItemContainer->resize(m_view->size());
        m_view->setSceneRect(m_declarativeItemContainer->geometry());
    } else {
        QRectF itemGeometry(QPointF(m_mainItem.data()->x(), m_mainItem.data()->y()),
                        QSizeF(m_mainItem.data()->boundingRect().size()));
        m_view->setSceneRect(itemGeometry);
    }
}

bool FullScreenWindow::isVisible() const
{
    return m_view->isVisible();
}

void FullScreenWindow::setVisible(const bool visible)
{
    if (m_view->isVisible() != visible) {
        m_background->setVisible(visible);
        Plasma::WindowEffects::slideWindow(m_view->winId(), Plasma::BottomEdge, 0);
        m_view->setVisible(visible);
        unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager;
        KWindowSystem::setState(m_view->effectiveWinId(), state);
        KWindowSystem::setState(m_background->effectiveWinId(), state);
        if (visible) {
            m_view->raise();
            KWindowSystem::forceActiveWindow(m_view->effectiveWinId());
        }
    }
}

QGraphicsView *FullScreenWindow::view() const
{
    return m_view;
}


QDeclarativeListProperty<QGraphicsObject> FullScreenWindow::title()
{
    if (m_rootObject) {
        return m_rootObject.data()->property("title").value<QDeclarativeListProperty<QGraphicsObject> >();
    } else {
        return QDeclarativeListProperty<QGraphicsObject>(this, m_dummyTitleElements);
    }
}

QDeclarativeListProperty<QGraphicsObject> FullScreenWindow::content()
{
    if (m_rootObject) {
        return m_rootObject.data()->property("content").value<QDeclarativeListProperty<QGraphicsObject> >();
    } else {
        return QDeclarativeListProperty<QGraphicsObject>(this, m_dummyContentElements);
    }
}

QDeclarativeListProperty<QGraphicsObject> FullScreenWindow::buttons()
{
    if (m_rootObject) {
        return m_rootObject.data()->property("buttons").value<QDeclarativeListProperty<QGraphicsObject> >();
    } else {
        return QDeclarativeListProperty<QGraphicsObject>(this, m_dummyButtonsElements);
    }
}

DialogStatus::Status FullScreenWindow::status() const
{
    if (m_rootObject) {
        return (DialogStatus::Status)m_rootObject.data()->property("status").toInt();
    } else {
        return DialogStatus::Closed;
    }
}


void FullScreenWindow::statusHasChanged()
{
    if (status() == DialogStatus::Closed) {
        setVisible(false);
    } else {
        setVisible(true);
    }
    emit statusChanged();
}

void FullScreenWindow::open()
{
    if (m_rootObject) {
        QMetaObject::invokeMethod(m_rootObject.data(), "open");
    }
}

void FullScreenWindow::accept()
{
    if (m_rootObject) {
        QMetaObject::invokeMethod(m_rootObject.data(), "accept");
    }
}

void FullScreenWindow::reject()
{
    if (m_rootObject) {
        QMetaObject::invokeMethod(m_rootObject.data(), "reject");
    }
}

void FullScreenWindow::close()
{
    if (m_rootObject) {
        QMetaObject::invokeMethod(m_rootObject.data(), "close");
    }
}




bool FullScreenWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_mainItem.data() &&
        (event->type() == QEvent::GraphicsSceneResize)) {
        syncViewToMainItem();
    } else if (watched == m_view &&
        (event->type() == QEvent::Resize)) {
        syncMainItemToView();
    }
    return false;
}



#include "fullscreenwindow.moc"

