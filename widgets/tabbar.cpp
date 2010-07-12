/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
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

#include "tabbar.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneWheelEvent>
#include <QIcon>
#include <QStyleOption>
#include <QPainter>
#include <QParallelAnimationGroup>

#include <kdebug.h>

#include <plasma/animator.h>
#include <plasma/animations/animation.h>
#include <plasma/theme.h>

#include "private/nativetabbar_p.h"

namespace Plasma
{

class TabBarProxy : public QGraphicsProxyWidget
{
public:
    TabBarProxy(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent)
    {
        native = new NativeTabBar();
        native->setAttribute(Qt::WA_NoSystemBackground);
        setWidget(native);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        //Don't paint the child widgets
        static_cast<NativeTabBar *>(QGraphicsProxyWidget::widget())->render(
            painter, QPoint(0, 0), QRegion(), 0);
    }

    NativeTabBar *native;
};

class TabBarPrivate
{
public:
    TabBarPrivate(TabBar *parent)
        : q(parent),
          tabProxy(0),
          currentIndex(0),
          tabWidgetMode(true),
          oldPageAnimId(-1),
          newPageAnimId(-1),
          customFont(false)
    {
    }

    ~TabBarPrivate()
    {
    }

    void updateTabWidgetMode();
    void slidingCompleted(QGraphicsItem *item);
    void slidingNewPageCompleted();
    void slidingOldPageCompleted();
    void shapeChanged(const KTabBar::Shape shape);
    void setPalette();

    TabBar *q;
    TabBarProxy *tabProxy;
    QList<QGraphicsWidget *> pages;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsLinearLayout *tabWidgetLayout;
    QGraphicsLinearLayout *tabBarLayout;
    int currentIndex;
    bool tabWidgetMode;

    QWeakPointer<QGraphicsWidget>oldPage;
    QWeakPointer<QGraphicsWidget>newPage;
    int oldPageAnimId;
    int newPageAnimId;
    Animation *oldPageAnim;
    Animation *newPageAnim;
    QParallelAnimationGroup *animGroup;
    bool customFont;
    QWeakPointer<QGraphicsWidget> firstPositionWidget;
    QWeakPointer<QGraphicsWidget> lastPositionWidget;
};

void TabBarPrivate::updateTabWidgetMode()
{
    bool tabWidget = false;

    foreach (QGraphicsWidget *page, pages) {
        if (page->preferredSize() != QSize(0, 0)) {
            tabWidget = true;
            break;
        }
    }

    if (tabWidget != tabWidgetMode) {
        if (tabWidget) {
            mainLayout->removeAt(0);
            tabBarLayout->insertItem(1, tabProxy);
            mainLayout->addItem(tabWidgetLayout);
        } else {
            mainLayout->removeAt(0);
            tabBarLayout->removeAt(1);
            mainLayout->addItem(tabProxy);
        }
    }

    //always show the tabbar
    //FIXME: Qt BUG: calling show on a child of an hidden item it shows it anyways
    //so we avoid to call it if the parent is hidden
    if (!tabWidget && q->isVisible()) {
        q->setTabBarShown(true);
    }

    tabWidgetMode = tabWidget;
    if (!tabWidgetMode) {
        q->setMinimumSize(QSize(0, 0));
        q->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else {
        tabProxy->native->setMinimumSize(QSize(0,0));
        tabProxy->setMinimumSize(QSize(0,0));
    }
}

void TabBarPrivate::slidingNewPageCompleted()
{
    if (newPage) {
        tabWidgetLayout->addItem(newPage.data());
    }
    newPageAnimId = -1;
    mainLayout->invalidate();
    emit q->currentChanged(currentIndex);

   q->setFlags(0);
}

void TabBarPrivate::slidingOldPageCompleted()
{
    QGraphicsWidget *item = oldPageAnim->targetWidget();

    oldPageAnimId = -1;
    if (item) {
        item->hide();
    }
    q->setFlags(0);
}

void TabBarPrivate::shapeChanged(const QTabBar::Shape shape)
{
    //FIXME: QGraphicsLinearLayout doesn't have setDirection, so for now
    // North is equal to south and East is equal to West
    switch (shape) {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:

    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        q->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        tabBarLayout->setOrientation(Qt::Vertical);
        tabWidgetLayout->setOrientation(Qt::Horizontal);
        tabWidgetLayout->itemAt(0)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        if (tabWidgetLayout->count() > 1) {
            tabWidgetLayout->itemAt(1)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }
        tabProxy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        break;

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:

    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    default:
        q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        tabBarLayout->setOrientation(Qt::Horizontal);
        tabWidgetLayout->setOrientation(Qt::Vertical);
        tabWidgetLayout->itemAt(0)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        if (tabWidgetLayout->count() > 1) {
            tabWidgetLayout->itemAt(1)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }
        tabProxy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    tabProxy->setPreferredSize(tabProxy->native->sizeHint());
}

void TabBarPrivate::setPalette()
{
    QTabBar *native = q->nativeWidget();
    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette p = native->palette();

    p.setColor(QPalette::Normal, QPalette::Text, color);
    p.setColor(QPalette::Inactive, QPalette::Text, color);
    p.setColor(QPalette::Normal, QPalette::ButtonText, color);
    p.setColor(QPalette::Inactive, QPalette::ButtonText, color);
    p.setColor(QPalette::Normal, QPalette::Base, QColor(0,0,0,0));
    p.setColor(QPalette::Inactive, QPalette::Base, QColor(0,0,0,0));
    native->setPalette(p);

    if (!customFont) {
        q->nativeWidget()->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
    }
}


TabBar::TabBar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new TabBarPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setContentsMargins(0,0,0,0);
    d->tabProxy = new TabBarProxy(this);
    d->tabWidgetLayout = new QGraphicsLinearLayout(Qt::Vertical);
    d->tabBarLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->tabWidgetLayout->setContentsMargins(0,0,0,0);

    d->mainLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->mainLayout->addItem(d->tabWidgetLayout);

    setLayout(d->mainLayout);
    d->mainLayout->setContentsMargins(0,0,0,0);

    d->tabWidgetLayout->addItem(d->tabBarLayout);

    //tabBar is centered, so a stretch at begin one at the end
    d->tabBarLayout->addStretch();
    d->tabBarLayout->addItem(d->tabProxy);
    d->tabBarLayout->addStretch();
    d->tabBarLayout->setContentsMargins(0,0,0,0);
    //d->tabBarLayout->setStretchFactor(d->tabProxy, 2);


    d->newPageAnim = Animator::create(Animator::SlideAnimation);
    d->oldPageAnim = Animator::create(Animator::SlideAnimation);
    d->animGroup = new QParallelAnimationGroup(this);

    d->animGroup->addAnimation(d->newPageAnim);
    d->animGroup->addAnimation(d->oldPageAnim);

    connect(d->tabProxy->native, SIGNAL(currentChanged(int)),
            this, SLOT(setCurrentIndex(int)));
    connect(d->tabProxy->native, SIGNAL(shapeChanged(QTabBar::Shape)),
            this, SLOT(shapeChanged(QTabBar::Shape)));
    connect(d->newPageAnim, SIGNAL(finished()), this, SLOT(slidingNewPageCompleted()));
    connect(d->oldPageAnim, SIGNAL(finished()), this, SLOT(slidingOldPageCompleted()));
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(setPalette()));
}

TabBar::~TabBar()
{
    delete d;
}


int TabBar::insertTab(int index, const QIcon &icon, const QString &label,
                      QGraphicsLayoutItem *content)
{
    QGraphicsWidget *page = new QGraphicsWidget(this);
    page->setContentsMargins(0,0,0,0);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (content) {
        if (content->isLayout()) {
            page->setLayout(static_cast<QGraphicsLayout *>(content));
        } else {
            QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, page);
            layout->setContentsMargins(0,0,0,0);
            layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            layout->addItem(content);
            page->setLayout(layout);
        }
    } else {
        page->setPreferredSize(0, 0);
    }

    d->pages.insert(qBound(0, index, d->pages.count()), page);

    if (d->pages.count() == 1) {
        d->tabWidgetLayout->addItem(page);
        page->setVisible(true);
        page->setEnabled(true);
    } else {
        page->setVisible(false);
        page->setEnabled(false);
    }

    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
    d->updateTabWidgetMode();

    int actualIndex = d->tabProxy->native->insertTab(index, icon, label);
    d->currentIndex = d->tabProxy->native->currentIndex();
    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
    d->updateTabWidgetMode();
    return actualIndex;
}

int TabBar::insertTab(int index, const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(index, QIcon(), label, content);
}

int TabBar::addTab(const QIcon &icon, const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(d->pages.count(), icon, label, content);
}

int TabBar::addTab(const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(d->pages.count(), QIcon(), label, content);
}

int TabBar::currentIndex() const
{
    return d->tabProxy->native->currentIndex();
}

void TabBar::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    if (!d->tabWidgetMode) {
        d->tabProxy->setMinimumSize(event->newSize().toSize());
        setMinimumSize(QSize(0, 0));
        setMinimumHeight(d->tabProxy->widget()->minimumSizeHint().height());
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else {
        setMinimumSize(QSize(-1, -1));
        d->tabProxy->native->setMinimumSize(QSize(0,0));
    }
}

void TabBar::setCurrentIndex(int index)
{
    if (index >= d->tabProxy->native->count() ||
        d->tabProxy->native->count() <= 1 ||
        d->currentIndex == index) {
        return;
    }

    if (d->currentIndex >= 0) {
        d->oldPage = d->pages[d->currentIndex];
    }

    d->tabWidgetLayout->removeItem(d->oldPage.data());

    if (index >= 0) {
        d->newPage = d->pages[index];
    }

    setFlags(QGraphicsItem::ItemClipsChildrenToShape);

    //if an animation was in rogress hide everything to avoid an inconsistent state

    if (d->animGroup->state() != QAbstractAnimation::Stopped) {
        foreach (QGraphicsWidget *page, d->pages) {
            page->hide();
        }
        d->animGroup->stop();
    }

    if (d->newPage) {
        d->newPage.data()->show();
        d->newPage.data()->setEnabled(true);
    }

    if (d->oldPage) {
        d->oldPage.data()->show();
        d->oldPage.data()->setEnabled(false);
    }

    if (d->newPage && d->oldPage) {
        //FIXME: it seems necessary to resiz the thing 2 times to have effect
        d->newPage.data()->resize(1,1);
        d->newPage.data()->resize(d->oldPage.data()->size());

        QRect beforeCurrentGeom(d->oldPage.data()->geometry().toRect());
        beforeCurrentGeom.moveTopRight(beforeCurrentGeom.topLeft());

        if (index > d->currentIndex) {
            d->newPage.data()->setPos(d->oldPage.data()->geometry().topRight());
            d->newPageAnim->setProperty("movementDirection", Animation::MoveLeft);
            d->newPageAnim->setProperty("distancePointF", QPointF(d->oldPage.data()->size().width(), 0));
            d->newPageAnim->setTargetWidget(d->newPage.data());

            d->oldPageAnim->setProperty("movementDirection", Animation::MoveLeft);
            d->oldPageAnim->setProperty("distancePointF", QPointF(beforeCurrentGeom.width(), 0));
            d->oldPageAnim->setTargetWidget(d->oldPage.data());

            d->animGroup->start();
        } else {
            d->newPage.data()->setPos(beforeCurrentGeom.topLeft());
            d->newPageAnim->setProperty("movementDirection", Animation::MoveRight);
            d->newPageAnim->setProperty("distancePointF", QPointF(d->oldPage.data()->size().width(), 0));
            d->newPageAnim->setTargetWidget(d->newPage.data());

            d->oldPageAnim->setProperty("movementDirection", Animation::MoveRight);
            d->oldPageAnim->setProperty("distancePointF",
                                        QPointF(d->oldPage.data()->size().width(), 0));
            d->oldPageAnim->setTargetWidget(d->oldPage.data());

            d->animGroup->start();
        }
    } else if (d->newPage) {
        d->tabWidgetLayout->addItem(d->newPage.data());
    }

    d->currentIndex = index;

    d->tabProxy->native->setCurrentIndex(index);
}

int TabBar::count() const
{
    return d->pages.count();
}

void TabBar::removeTab(int index)
{
    if (index >= d->pages.count() || index < 0) {
        return;
    }

    d->newPageAnim->stop();
    d->oldPageAnim->stop();

    int oldCurrentIndex = d->tabProxy->native->currentIndex();
    d->tabProxy->native->removeTab(index);

    d->currentIndex = oldCurrentIndex;
    int currentIndex = d->tabProxy->native->currentIndex();

    if (oldCurrentIndex == index) {
        d->tabWidgetLayout->removeAt(1);
        if (d->tabProxy->native->count() > 0) {
            setCurrentIndex(currentIndex >= oldCurrentIndex ? currentIndex + 1 : currentIndex);
        }
    }

    QGraphicsWidget *page = d->pages.takeAt(index);
    scene()->removeItem(page);
    page->deleteLater();

    if (d->pages.count() > 0) {
        d->updateTabWidgetMode();
    }
    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
}

QGraphicsLayoutItem *TabBar::takeTab(int index)
{
    if (index >= d->pages.count()) {
        return 0;
    }

    int oldCurrentIndex = d->tabProxy->native->currentIndex();
    d->tabProxy->native->removeTab(index);
    QGraphicsWidget *page = d->pages.takeAt(index);

    int currentIndex = d->tabProxy->native->currentIndex();

    if (oldCurrentIndex == index) {
        d->tabWidgetLayout->removeAt(1);
    }

    QGraphicsLayoutItem *returnItem = 0;
    QGraphicsLayout *lay = page->layout();
    if (lay && lay->count() == 1) {
        returnItem = lay->itemAt(0);
        lay->removeAt(0);
    } else {
        returnItem = lay;
    }

    if (returnItem) {
        returnItem->setParentLayoutItem(0);
    }

    page->setLayout(0);
    scene()->removeItem(page);
    page->deleteLater();

    if (oldCurrentIndex != currentIndex) {
        setCurrentIndex(currentIndex);
    }

    d->updateTabWidgetMode();
    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());

    return returnItem;
}

QGraphicsLayoutItem *TabBar::tabAt(int index)
{
    if (index >= d->pages.count()) {
        return 0;
    }

    QGraphicsWidget *page = d->pages.value(index);

    QGraphicsLayoutItem *returnItem = 0;
    QGraphicsLayout *lay = page->layout();
    if (lay && lay->count() == 1) {
        returnItem = lay->itemAt(0);
    } else {
        returnItem = lay;
    }

    return returnItem;
}

void TabBar::setTabText(int index, const QString &label)
{
    if (index >= d->pages.count()) {
        return;
    }

    d->tabProxy->native->setTabText(index, label);
}

QString TabBar::tabText(int index) const
{
    return d->tabProxy->native->tabText(index);
}

void TabBar::setTabIcon(int index, const QIcon &icon)
{
    d->tabProxy->native->setTabIcon(index, icon);
}

QIcon TabBar::tabIcon(int index) const
{
    return d->tabProxy->native->tabIcon(index);
}

void TabBar::setTabBarShown(bool show)
{
    if (!show && !d->tabWidgetMode) {
        return;
    }

    if (!show && d->tabProxy->isVisible()) {
        d->tabProxy->hide();
        d->tabWidgetLayout->removeItem(d->tabBarLayout);
    } else if (show && !d->tabProxy->isVisible()) {
        d->tabProxy->show();
        d->tabWidgetLayout->insertItem(0, d->tabBarLayout);
    }
}

bool TabBar::isTabBarShown() const
{
    return d->tabProxy->isVisible();
}

void TabBar::setStyleSheet(const QString &stylesheet)
{
    d->tabProxy->native->setStyleSheet(stylesheet);
}

QString TabBar::styleSheet() const
{
    return d->tabProxy->native->styleSheet();
}

KTabBar *TabBar::nativeWidget() const
{
    return d->tabProxy->native;
}

void TabBar::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Q_UNUSED(event)
    //Still here for binary compatibility
}

void TabBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsWidget::changeEvent(event);
}

void TabBar::setFirstPositionWidget(QGraphicsWidget *widget)
{
    if (d->lastPositionWidget.data() == widget) {
        return;
    }

    if (d->firstPositionWidget) {
        QGraphicsWidget *widget = d->firstPositionWidget.data();
        d->tabBarLayout->removeItem(widget);
        scene()->removeItem(widget);
        widget->deleteLater();
    }

    d->firstPositionWidget = widget;
    if (widget) {
        widget->setParentItem(this);
        if (layoutDirection() == Qt::LeftToRight) {
            d->tabBarLayout->insertItem(0, widget);
        } else {
            d->tabBarLayout->addItem(widget);
        }
    }
}


QGraphicsWidget *TabBar::firstPositionWidget() const
{
    return d->firstPositionWidget.data();
}

void TabBar::setLastPositionWidget(QGraphicsWidget *widget)
{
    if (d->lastPositionWidget.data() == widget) {
        return;
    }

    if (d->lastPositionWidget) {
        QGraphicsWidget *widget = d->lastPositionWidget.data();
        d->tabBarLayout->removeItem(widget);
        scene()->removeItem(widget);
        widget->deleteLater();
    }

    d->lastPositionWidget = widget;
    if (widget) {
        widget->setParentItem(this);
        if (layoutDirection() == Qt::LeftToRight) {
            d->tabBarLayout->addItem(widget);
        } else {
            d->tabBarLayout->insertItem(0, widget);
        }
    }
}

QGraphicsWidget *TabBar::lastPositionWidget() const
{
    return d->lastPositionWidget.data();
}

} // namespace Plasma

#include <tabbar.moc>

