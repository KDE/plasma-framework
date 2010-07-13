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

#include "toolbutton.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QDir>
#include <QToolButton>
#include <QApplication>
#include <QPropertyAnimation>

#include <kicon.h>
#include <kiconeffect.h>
#include <kmimetype.h>
#include <kcolorutils.h>

#include "theme.h"
#include "svg.h"
#include "framesvg.h"
#include "animator.h"
#include "paintutils.h"
#include "private/actionwidgetinterface_p.h"

namespace Plasma
{

class ToolButtonPrivate : public ActionWidgetInterface<ToolButton>
{
public:
    ToolButtonPrivate(ToolButton *toolButton)
        : ActionWidgetInterface<ToolButton>(toolButton),
          q(toolButton),
          background(0),
          svg(0),
          customFont(false),
          underMouse(false)
    {
    }

    ~ToolButtonPrivate()
    {
        delete svg;
    }

    void setPixmap()
    {
        if (imagePath.isEmpty()) {
            delete svg;
            svg = 0;
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm;

        if (mime->is("image/svg+xml") || mime->is("image/svg+xml-compressed")) {
            if (!svg || svg->imagePath() != absImagePath) {
                delete svg;
                svg = new Svg();
                svg->setImagePath(imagePath);
                QObject::connect(svg, SIGNAL(repaintNeeded()), q, SLOT(setPixmap()));
                if (!svgElement.isNull()) {
                    svg->setContainsMultipleImages(true);
                }
            }

            //QPainter p(&pm);
            if (!svgElement.isNull() && svg->hasElement(svgElement)) {
                QSizeF elementSize = svg->elementSize(svgElement);
                float scale = pm.width() / qMax(elementSize.width(), elementSize.height());

                svg->resize(svg->size() * scale);
                pm = svg->pixmap(svgElement);
            } else {
                svg->resize(pm.size());
                pm = svg->pixmap();
            }
        } else {
            delete svg;
            svg = 0;
            pm = QPixmap(absImagePath);
        }

        static_cast<QToolButton*>(q->widget())->setIcon(KIcon(pm));
    }

    void syncActiveRect();
    void syncBorders();
    void animationUpdate(qreal progress);

    ToolButton *q;

    FrameSvg *background;
    QPropertyAnimation *animation;
    qreal opacity;
    QRectF activeRect;

    QString imagePath;
    QString absImagePath;
    Svg *svg;
    QString svgElement;
    bool customFont;
    bool underMouse;
};

void ToolButtonPrivate::syncActiveRect()
{
    background->setElementPrefix("normal");

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);

    background->setElementPrefix("active");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    background->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    activeRect = QRectF(QPointF(0, 0), q->size());
    activeRect.adjust(left - activeLeft, top - activeTop,
                      -(right - activeRight), -(bottom - activeBottom));

    background->setElementPrefix("normal");
}

void ToolButtonPrivate::syncBorders()
{
    //set margins from the normal element
    qreal left, top, right, bottom;

    background->setElementPrefix("normal");
    background->getMargins(left, top, right, bottom);
    q->setContentsMargins(left, top, right, bottom);

    //calc the rect for the over effect
    syncActiveRect();
}

void ToolButtonPrivate::animationUpdate(qreal progress)
{
    opacity = progress;

    // explicit update
    q->update();
}

ToolButton::ToolButton(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ToolButtonPrivate(this))
{
    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedFrames(true);
    d->background->setElementPrefix("normal");

    QToolButton *native = new QToolButton;
    connect(native, SIGNAL(clicked()), this, SIGNAL(clicked()));
    connect(native, SIGNAL(pressed()), this, SIGNAL(pressed()));
    connect(native, SIGNAL(released()), this, SIGNAL(released()));
    setWidget(native);
    native->setWindowIcon(QIcon());
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setAutoRaise(true);

    d->syncBorders();
    setAcceptHoverEvents(true);
    connect(d->background, SIGNAL(repaintNeeded()), SLOT(syncBorders()));

    d->animation = new QPropertyAnimation(this, "animationUpdate");
    d->animation->setStartValue(0);
    d->animation->setEndValue(1);
}

ToolButton::~ToolButton()
{
    delete d->animation;
    delete d;
}

void ToolButton::setAnimationUpdate(qreal progress)
{
    d->animationUpdate(progress);
}

qreal ToolButton::animationUpdate() const
{
    return d->opacity;
}

void ToolButton::setAction(QAction *action)
{
    d->setAction(action);
}

QAction *ToolButton::action() const
{
    return d->action;
}

void ToolButton::setAutoRaise(bool raise)
{
    nativeWidget()->setAutoRaise(raise);
}

bool ToolButton::autoRaise() const
{
    return nativeWidget()->autoRaise();
}

void ToolButton::setText(const QString &text)
{
    static_cast<QToolButton*>(widget())->setText(text);
    updateGeometry();
}

QString ToolButton::text() const
{
    return static_cast<QToolButton*>(widget())->text();
}

void ToolButton::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() &&
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            (path[0] == '/' || path.startsWith(QLatin1String(":/")))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap();
}

void ToolButton::setImage(const QString &path, const QString &elementid)
{
    d->svgElement = elementid;
    setImage(path);
}

void ToolButton::setIcon(const QIcon &icon)
{
    nativeWidget()->setIcon(icon);
}

QIcon ToolButton::icon() const
{
    return nativeWidget()->icon();
}

QString ToolButton::image() const
{
    return d->imagePath;
}

void ToolButton::setDown(bool down)
{
    nativeWidget()->setDown(down);
}

bool ToolButton::isDown() const
{
    return nativeWidget()->isDown();
}

void ToolButton::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ToolButton::styleSheet()
{
    return widget()->styleSheet();
}

QToolButton *ToolButton::nativeWidget() const
{
    return static_cast<QToolButton*>(widget());
}

void ToolButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap();

   if (d->background) {
        //resize all four panels
        d->background->setElementPrefix("pressed");
        d->background->resizeFrame(size());
        d->background->setElementPrefix("focus");
        d->background->resizeFrame(size());

        d->syncActiveRect();

        d->background->setElementPrefix("active");
        d->background->resizeFrame(d->activeRect.size());

        d->background->setElementPrefix("normal");
        d->background->resizeFrame(size());
   }

   QGraphicsProxyWidget::resizeEvent(event);
}

void ToolButton::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    if (!styleSheet().isNull()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QToolButton *button = nativeWidget();

    QStyleOptionToolButton buttonOpt;
    buttonOpt.initFrom(button);
    buttonOpt.icon = button->icon();
    buttonOpt.text = button->text();
    buttonOpt.iconSize = button->iconSize();
    buttonOpt.toolButtonStyle = button->toolButtonStyle();

    bool animationState = (d->animation->state() == QAbstractAnimation::Running)? \
                          1:0;
    if (button->isEnabled() && (animationState || !button->autoRaise() || d->underMouse || (buttonOpt.state & QStyle::State_On) || button->isChecked() || button->isDown())) {
        if (button->isDown() || (buttonOpt.state & QStyle::State_On) || button->isChecked()) {
            d->background->setElementPrefix("pressed");
        } else {
            d->background->setElementPrefix("normal");
        }
        d->background->resizeFrame(size());

        if (animationState) {
            QPixmap buffer = d->background->framePixmap();

            QPainter bufferPainter(&buffer);
            bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QColor alphaColor(Qt::black);
            alphaColor.setAlphaF(qMin(qreal(0.95), d->opacity));
            bufferPainter.fillRect(buffer.rect(), alphaColor);
            bufferPainter.end();

            painter->drawPixmap(QPoint(0,0), buffer);

            buttonOpt.palette.setColor(QPalette::ButtonText, KColorUtils::mix(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1-d->opacity));
        } else {
            d->background->paintFrame(painter);
            buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
        }

    } else {
        buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }

    QFont widgetFont;
    if (d->customFont) {
        widgetFont = font();
    } else {
        widgetFont = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    }
    buttonOpt.font = widgetFont;

    painter->setFont(widgetFont);
    button->style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, painter, button);
}

void ToolButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = true;
    if (nativeWidget()->isDown() || !nativeWidget()->autoRaise()) {
        return;
    }

    const int FadeInDuration = 75;

    if (d->animation->state() != QAbstractAnimation::Stopped) {
        d->animation->stop();
    }
    d->animation->setDuration(FadeInDuration);
    d->animation->setDirection(QAbstractAnimation::Forward);
    d->animation->start();

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ToolButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    d->underMouse = false;
    if (nativeWidget()->isDown() || !nativeWidget()->autoRaise()) {
        return;
    }

    const int FadeOutDuration = 150;

    if (d->animation->state() != QAbstractAnimation::Stopped) {
        d->animation->stop();
    }

    d->animation->setDuration(FadeOutDuration);
    d->animation->setDirection(QAbstractAnimation::Backward);
    d->animation->start();

    d->background->setElementPrefix("active");

    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

void ToolButton::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
    } else if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        d->underMouse = false;
    }

    QGraphicsProxyWidget::changeEvent(event);
}

QVariant ToolButton::itemChange(GraphicsItemChange change, const QVariant &value)
{
    //If the widget is hidden while it's hovered and then we show it again
    //we have to disable the hover otherwise it will remain hovered.
    if (change == ItemVisibleHasChanged){
         d->underMouse = false;
    }

    return QGraphicsProxyWidget::itemChange(change, value);
}

QSizeF ToolButton::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = QGraphicsProxyWidget::sizeHint(which, constraint);

    return hint;
}

} // namespace Plasma

#include <toolbutton.moc>

