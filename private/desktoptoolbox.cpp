/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "desktoptoolbox_p.h"

#include <QAction>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QGraphicsLinearLayout>
#include <QGraphicsView>
#include <QWeakPointer>
#include <QPropertyAnimation>

#include <kcolorscheme.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <plasma/animations/animation.h>
#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/framesvg.h>
#include <plasma/paintutils.h>
#include <plasma/theme.h>
#include <plasma/tooltipcontent.h>
#include <plasma/tooltipmanager.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/itembackground.h>

namespace Plasma
{

class EmptyGraphicsItem : public QGraphicsWidget
{
    public:
        EmptyGraphicsItem(QGraphicsItem *parent)
            : QGraphicsWidget(parent)
        {
            setAcceptsHoverEvents(true);
            m_layout = new QGraphicsLinearLayout(this);
            m_layout->setContentsMargins(0, 0, 0, 0);
            m_layout->setSpacing(0);
            m_background = new Plasma::FrameSvg(this);
            m_background->setImagePath("widgets/background");
            m_background->setEnabledBorders(FrameSvg::AllBorders);
            m_layout->setOrientation(Qt::Vertical);
            m_itemBackground = new Plasma::ItemBackground(this);
            updateMargins();
        }

        ~EmptyGraphicsItem()
        {
        }

        void updateMargins()
        {
            qreal left, top, right, bottom;
            m_background->getMargins(left, top, right, bottom);
            setContentsMargins(left, top, right, bottom);
        }

        void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *)
        {
            m_background->paintFrame(p, option->rect, option->rect);
        }

        void clearLayout()
        {
            while (m_layout->count()) {
                //safe? at the moment everything it's thre will always be QGraphicsWidget
                static_cast<QGraphicsWidget *>(m_layout->itemAt(0))->removeEventFilter(this);
                m_layout->removeAt(0);
            }
        }

        void addToLayout(QGraphicsWidget *widget)
        {
            qreal left, top, right, bottom;
            m_itemBackground->getContentsMargins(&left, &top, &right, &bottom);
            widget->setContentsMargins(left, top, right, bottom);
            m_layout->addItem(widget);
            widget->installEventFilter(this);

            if (m_layout->count() == 1) {
                m_itemBackground->hide();
                m_itemBackground->setTargetItem(widget);
            }
        }

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *)
        {
            m_background->resizeFrame(size());
        }

        bool eventFilter(QObject *watched, QEvent *event)
        {
            QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(watched);
            if (event->type() == QEvent::GraphicsSceneHoverEnter) {
                m_itemBackground->setTargetItem(widget);
            }
            return false;
        }

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
        {
            event->accept();
        }

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
        {
            m_itemBackground->hide();
        }

    private:
        QRectF m_rect;
        Plasma::FrameSvg *m_background;
        QGraphicsLinearLayout *m_layout;
        Plasma::ItemBackground *m_itemBackground;
};

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

class DesktopToolBoxPrivate
{
public:
    DesktopToolBoxPrivate(DesktopToolBox *toolbox)
      : q(toolbox),
        background(0),
        containment(0),
        icon("plasma"),
        toolBacker(0),
        animCircleFrame(0),
        animHighlightFrame(0),
        hovering(false)
    {}

    void adjustBackgroundBorders()
    {
        switch (q->corner()) {
          case InternalToolBox::TopRight:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::LeftBorder);
            break;
        case InternalToolBox::Top:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::LeftBorder|FrameSvg::RightBorder);
            break;
        case InternalToolBox::TopLeft:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::RightBorder);
            break;
        case InternalToolBox::Left:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::TopBorder|FrameSvg::RightBorder);
            break;
        case InternalToolBox::Right:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::TopBorder|FrameSvg::LeftBorder);
            break;
        case InternalToolBox::BottomLeft:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::RightBorder);
            break;
        case InternalToolBox::Bottom:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::LeftBorder|FrameSvg::RightBorder);
            break;
        case InternalToolBox::BottomRight:
        default:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::LeftBorder);
            break;
        }
    }

    DesktopToolBox *q;
    Plasma::FrameSvg *background;
    Containment *containment;
    KIcon icon;
    EmptyGraphicsItem *toolBacker;
    QWeakPointer<QPropertyAnimation> anim;
    qreal animCircleFrame;
    qreal animHighlightFrame;
    QRect shapeRect;
    QColor fgColor;
    QColor bgColor;
    bool hovering : 1;
    QMultiMap<AbstractToolBox::ToolType, IconWidget *> tools;
};

DesktopToolBox::DesktopToolBox(Containment *parent)
    : InternalToolBox(parent),
      d(new DesktopToolBoxPrivate(this))
{
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/toolbox");

    d->containment = parent;
    setZValue(10000000);

    setIsMovable(true);
    updateTheming();

    connect(this, SIGNAL(toggled()), this, SLOT(toggle()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(updateTheming()));
    ToolTipManager::self()->registerWidget(this);
}

DesktopToolBox::~DesktopToolBox()
{
    delete d;
}

QSize DesktopToolBox::cornerSize() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    return QSize(size() + left, size() + bottom);
}

QSize DesktopToolBox::fullWidth() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    int extraSpace = 0;
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+'x');
    }

    return QSize(size() + left + right + extraSpace, size() + bottom);
}

QSize DesktopToolBox::fullHeight() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    int extraSpace = 0;
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+'x');
    }

    return QSize(size() + left, size() + top + bottom + extraSpace);
}

void DesktopToolBox::toolTipAboutToShow()
{
    if (isShowing()) {
        return;
    }

    ToolTipContent c(i18n("Tool Box"),
                     i18n("Click to access configuration options and controls, or to add more widgets to the %1.",
                          containment()->name()),
                     KIcon("plasma"));
    c.setAutohide(false);
    ToolTipManager::self()->setContent(this, c);
}

void DesktopToolBox::toolTipHidden()
{
    ToolTipManager::self()->clearContent(this);
}

QRectF DesktopToolBox::boundingRect() const
{
    int extraSpace = size();

    d->adjustBackgroundBorders();

    //keep space for the label and a character more
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+'x');
    }

    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);

    QRectF rect;

    //disable text at corners
    if (corner() == TopLeft || corner() == TopRight || corner() == BottomLeft || corner() == BottomRight) {
        rect = QRectF(0, 0, size()+left+right, size()+top+bottom);
    } else if (corner() == Left || corner() == Right) {
        rect = QRectF(0, 0, size()+left+right, size()+extraSpace+top+bottom);
    //top or bottom
    } else {
        rect = QRectF(0, 0, size()+extraSpace+left+right, size()+top+bottom);
    }

    return rect;
}

void DesktopToolBox::updateTheming()
{
    d->bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    d->fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
}

void DesktopToolBox::toolTriggered(bool)
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (isShowing() && (!action || !action->autoRepeat())) {
        emit toggled();
    }
}

void DesktopToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QPainterPath p = shape();

    QPoint iconPos;
    QRect backgroundRect;
    const QRectF rect = boundingRect();
    const QSize icons = iconSize();

    QString cornerElement;

    switch (corner()) {
    case TopLeft:
        cornerElement = "desktop-northwest";
        break;
    case TopRight:
        cornerElement = "desktop-northeast";
        break;
    case BottomRight:
        cornerElement = "desktop-southeast";
        break;
    case BottomLeft:
        cornerElement = "desktop-southwest";
        break;
    default:
        break;
    }

    QString activityName;

    QSize textSize;
    if (cornerElement.isNull()) {
        activityName = d->containment->activity();
        textSize =  Plasma::Theme::defaultTheme()->fontMetrics().size(Qt::TextSingleLine, activityName+'x');
    }

    d->adjustBackgroundBorders();

    d->background->resizeFrame(rect.size());

    if (!cornerElement.isNull()) {
        d->background->paint(painter, rect, cornerElement);
    } else {
        d->background->paintFrame(painter, rect.topLeft());
    }


    QRect iconRect;
    QRect textRect;

    if (corner() == Left || corner() == Right) {
        Qt::Alignment alignment;

        if (activityName.isNull()) {
            alignment = Qt::Alignment(Qt::AlignCenter);
        } else {
            alignment = Qt::Alignment(Qt::AlignHCenter|Qt::AlignTop);
        }

        iconRect = QStyle::alignedRect(QApplication::layoutDirection(), alignment, iconSize(), d->background->contentsRect().toRect());

        QRect boundRect(QPoint(d->background->contentsRect().top(),
                               d->background->contentsRect().left()),
                        QSize(d->background->contentsRect().height(),
                              d->background->contentsRect().width()));

        textRect = QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight|Qt::AlignVCenter, textSize, boundRect);
        textRect.moveTopLeft(textRect.topLeft() + QPoint(rect.top(), rect.left()));
    } else {
        Qt::Alignment alignment;

        if (activityName.isNull()) {
            alignment = Qt::Alignment(Qt::AlignCenter);
        } else {
            alignment = Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter);
        }

        iconRect = QStyle::alignedRect(QApplication::layoutDirection(), alignment, iconSize(), d->background->contentsRect().toRect());

        textRect = QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight|Qt::AlignVCenter, textSize, d->background->contentsRect().toRect());
        textRect.moveTopLeft(textRect.topLeft() + rect.topLeft().toPoint());
    }

    iconRect.moveTopLeft(iconRect.topLeft() + rect.topLeft().toPoint());


    iconPos = iconRect.topLeft();

    const qreal progress = d->animHighlightFrame;

    if (qFuzzyCompare(qreal(1.0), progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()));
    } else if (qFuzzyCompare(qreal(1.0), 1 + progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()),
                      Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off);
        QPixmap enabled = d->icon.pixmap(iconSize());
        QPixmap result = PaintUtils::transition(
            d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off),
            d->icon.pixmap(iconSize()), progress);
        painter->drawPixmap(QRect(iconPos, iconSize()), result);
    }

    if (!cornerElement.isNull() || activityName.isNull()) {
        return;
    }

    QColor textColor = Plasma::Theme::defaultTheme()->color(Theme::TextColor);
    QColor shadowColor;
    QPoint shadowOffset;

    if (qGray(textColor.rgb()) > 192) {
        shadowColor = Qt::black;
        shadowOffset = QPoint(1,1);
    } else {
        shadowColor = Qt::white;
        shadowOffset = QPoint(0,0);
    }

    QPixmap shadowText = Plasma::PaintUtils::shadowText(activityName, textColor, shadowColor, shadowOffset);

    painter->save();
    if (corner() == Left || corner() == Right) {
        painter->rotate(90);
        painter->translate(textRect.left(), -textRect.top()-textRect.height());
        painter->drawPixmap(QPoint(0,0), shadowText);
    } else {
        painter->drawPixmap(textRect.topLeft(), shadowText);
    }

    painter->restore();
}

QPainterPath DesktopToolBox::shape() const
{
    const QRectF rect = boundingRect();
    const int w = rect.width();
    const int h = rect.height();

    QPainterPath path;
    switch (corner()) {
    case BottomLeft:
        path.moveTo(rect.bottomLeft());
        path.arcTo(QRectF(rect.left() - w, rect.top(), w * 2, h * 2), 0, 90);
        break;
    case BottomRight:
        path.moveTo(rect.bottomRight());
        path.arcTo(QRectF(rect.left(), rect.top(), w * 2, h * 2), 90, 90);
        break;
    case TopRight:
        path.moveTo(rect.topRight());
        path.arcTo(QRectF(rect.left(), rect.top() - h, w * 2, h * 2), 180, 90);
        break;
    case TopLeft:
        path.arcTo(QRectF(rect.left() - w, rect.top() - h, w * 2, h * 2), 270, 90);
        break;
    default:
        path.addRect(rect);
        break;
    }

    return path;
}

void DesktopToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (isShowing() || d->hovering) {
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }

    highlight(true);

    QGraphicsItem::hoverEnterEvent(event);
}

QGraphicsWidget *DesktopToolBox::toolParent()
{
    if (!d->toolBacker) {
        d->toolBacker = new EmptyGraphicsItem(this);
        d->toolBacker->hide();
    }

    return d->toolBacker;
}

void DesktopToolBox::showToolBox()
{
    if (isShowing()) {
        return;
    }

    if (!d->toolBacker) {
        d->toolBacker = new EmptyGraphicsItem(this);
    }

    d->toolBacker->setZValue(zValue() + 1);

    adjustToolBackerGeometry();

    d->toolBacker->setOpacity(0);
    d->toolBacker->show();
    Plasma::Animation *fadeAnim = Animator::create(Animator::FadeAnimation, d->toolBacker);
    fadeAnim->setTargetWidget(d->toolBacker);
    fadeAnim->setProperty("startOpacity", 0);
    fadeAnim->setProperty("targetOpacity", 1);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DesktopToolBox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    if (actions().contains(action)) {
        return;
    }

    InternalToolBox::addTool(action);
    Plasma::IconWidget *tool = new Plasma::IconWidget(toolParent());

    tool->setTextBackgroundColor(QColor());
    tool->setAction(action);
    tool->setDrawBackground(true);
    tool->setOrientation(Qt::Horizontal);
    tool->resize(tool->sizeFromIconSize(KIconLoader::SizeSmallMedium));
    tool->setPreferredIconSize(QSizeF(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
    tool->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(toolPosition(height));
    tool->setZValue(zValue() + 10);
    tool->setToolTip(action->text());

    //make enabled/disabled tools appear/disappear instantly
    connect(tool, SIGNAL(changed()), this, SLOT(updateToolBox()));

    ToolType type = AbstractToolBox::MiscTool;
    if (!action->data().isNull() && action->data().type() == QVariant::Int) {
        int t = action->data().toInt();
        if (t >= 0 && t < AbstractToolBox::UserToolType) {
            type = static_cast<AbstractToolBox::ToolType>(t);
        }
    }

    d->tools.insert(type, tool);
    //kDebug() << "added tool" << type << action->text();
}

void DesktopToolBox::updateToolBox()
{
    Plasma::IconWidget *tool = qobject_cast<Plasma::IconWidget *>(sender());
    if (tool && !tool->action()) {
        QMutableMapIterator<ToolType, IconWidget *> it(d->tools);
        while (it.hasNext()) {
            it.next();
            if (it.value() == tool) {
                it.remove();
                break;
            }
        }

        tool->deleteLater();
        tool = 0;
    }

    if (isShowing()) {
        showToolBox();
    } else if (tool && !tool->isEnabled()) {
        tool->hide();
    }

    adjustToolBackerGeometry();
}

void DesktopToolBox::removeTool(QAction *action)
{
    QMutableMapIterator<ToolType, IconWidget *> it(d->tools);
    while (it.hasNext()) {
        it.next();
        IconWidget *tool = it.value();
        //kDebug() << "checking tool" << tool
        if (tool && tool->action() == action) {
            //kDebug() << "tool found!";
            tool->deleteLater();
            it.remove();
            break;
        }
    }
}

void DesktopToolBox::adjustToolBackerGeometry()
{
    if (!d->toolBacker) {
        return;
    }

    d->toolBacker->clearLayout();
    QMapIterator<ToolType, IconWidget *> it(d->tools);
    while (it.hasNext()) {
        it.next();
        IconWidget *icon = it.value();
        //kDebug() << "showing off" << it.key() << icon->text();
        if (icon->isEnabled()) {
            icon->show();
            icon->setDrawBackground(false);
            d->toolBacker->addToLayout(icon);
        } else {
            icon->hide();
        }
    }

    qreal left, top, right, bottom;
    d->toolBacker->getContentsMargins(&left, &top, &right, &bottom);
    d->toolBacker->adjustSize();

    int x = 0;
    int y = 0;
    const int iconWidth = KIconLoader::SizeMedium;
    switch (corner()) {
    case TopRight:
        x = (int)boundingRect().left() - d->toolBacker->size().width();
        y = (int)boundingRect().top();
        break;
    case Top:
        x = (int)boundingRect().center().x() - (d->toolBacker->size().width() / 2);
        y = (int)boundingRect().bottom();
        break;
    case TopLeft:
        x = (int)boundingRect().right();
        y = (int)boundingRect().top();
        break;
    case Left:
        x = (int)boundingRect().left() + iconWidth;
        y = (int)boundingRect().y();
        break;
    case Right:
        x = (int)boundingRect().right() - iconWidth - d->toolBacker->size().width();
        y = (int)boundingRect().y();
        break;
    case BottomLeft:
        x = (int)boundingRect().left() + iconWidth;
        y = (int)boundingRect().bottom();
        break;
    case Bottom:
        x = (int)boundingRect().center().x() - (d->toolBacker->size().width() / 2);
        y = (int)boundingRect().top();
        break;
    case BottomRight:
    default:
        x = (int)boundingRect().right() - iconWidth - d->toolBacker->size().width();
        y = (int)boundingRect().top();
        break;
    }

    //kDebug() << "starting at" <<  x << startY;
    d->toolBacker->setPos(x, y);
    // now check that it actually fits within the parent's boundaries
    QRectF backerRect = mapToParent(d->toolBacker->geometry()).boundingRect();
    QSizeF parentSize = parentWidget()->size();
    if (backerRect.x() < 5) {
        d->toolBacker->setPos(mapFromParent(QPointF(5, 0)).x(), y);
    } else if (backerRect.right() > parentSize.width() - 5) {
        d->toolBacker->setPos(mapFromParent(QPointF(parentSize.width() - 5 - backerRect.width(), 0)).x(), y);
    }

    if (backerRect.y() < 5) {
        d->toolBacker->setPos(x, mapFromParent(QPointF(0, 5)).y());
    } else if (backerRect.bottom() > parentSize.height() - 5) {
        d->toolBacker->setPos(x, mapFromParent(QPointF(0, parentSize.height() - 5 - backerRect.height())).y());
    }
}

void DesktopToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos()
    //         << d->toolBacker->rect().contains(event->scenePos().toPoint());
    if (!d->hovering || isShowing()) {
        QGraphicsItem::hoverLeaveEvent(event);
        return;
    }

    highlight(false);

    QGraphicsItem::hoverLeaveEvent(event);
}

void DesktopToolBox::hideToolBox()
{
    if (d->toolBacker) {
        Plasma::Animation *fadeAnim = Animator::create(Animator::FadeAnimation, d->toolBacker);
        connect(fadeAnim, SIGNAL(finished()), this, SLOT(hideToolBacker()));
        fadeAnim->setTargetWidget(d->toolBacker);
        fadeAnim->setProperty("startOpacity", 1);
        fadeAnim->setProperty("targetOpacity", 0);
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    highlight(false);
}

void DesktopToolBox::hideToolBacker()
{
    d->toolBacker->hide();
}

void DesktopToolBox::highlight(bool highlighting)
{
    if (d->hovering == highlighting) {
        return;
    }

    d->hovering = highlighting;

    QPropertyAnimation *anim = d->anim.data();
    if (d->hovering) {
        if (anim) {
            anim->stop();
            d->anim.clear();
        }
        anim = new QPropertyAnimation(this, "highlight", this);
        d->anim = anim;
    }

    if (anim->state() != QAbstractAnimation::Stopped) {
        anim->stop();
    }

    anim->setDuration(250);
    anim->setStartValue(0);
    anim->setEndValue(1);

    if(d->hovering) {
        anim->start();
    } else {
        anim->setDirection(QAbstractAnimation::Backward);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

    }
}

void DesktopToolBox::setHighlight(qreal progress)
{
    d->animHighlightFrame = progress;
    update();
}

qreal DesktopToolBox::highlight()
{
    return d->animHighlightFrame;
}

void DesktopToolBox::toggle()
{
    setShowing(!isShowing());
}

} // plasma namespace

#include "desktoptoolbox_p.moc"
