/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "label.h"

#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QDir>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class LabelPrivate
{
public:
    LabelPrivate(Label *label)
        : q(label),
          svg(0),
          customFont(false)
    {
    }

    ~LabelPrivate()
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
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml") || mime->is("image/svg+xml-compressed")) {
            if (!svg || svg->imagePath() != absImagePath) {
                delete svg;
                svg = new Svg();
                svg->setImagePath(imagePath);
                QObject::connect(svg, SIGNAL(repaintNeeded()), q, SLOT(setPixmap()));
            }

            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            delete svg;
            svg = 0;
            pm = QPixmap(absImagePath);
        }

        static_cast<QLabel*>(q->widget())->setPixmap(pm);
    }

    void setPalette()
    {
        QLabel *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
        QPalette p = native->palette();
        p.setColor(QPalette::Normal, QPalette::WindowText, color);
        p.setColor(QPalette::Inactive, QPalette::WindowText, color);
        color.setAlphaF(0.6);
        p.setColor(QPalette::Disabled, QPalette::WindowText, color);

        p.setColor(QPalette::Normal, QPalette::Link, Theme::defaultTheme()->color(Theme::LinkColor));
        p.setColor(QPalette::Normal, QPalette::LinkVisited, Theme::defaultTheme()->color(Theme::VisitedLinkColor));
        native->setPalette(p);

        if (!customFont) {
            q->nativeWidget()->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        }
    }

    Label *q;
    QString imagePath;
    QString absImagePath;
    Svg *svg;
    bool textSelectable;
    bool customFont;
};

Label::Label(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new LabelPrivate(this))
{
    QLabel *native = new QLabel;
    native->setWindowFlags(native->windowFlags()|Qt::BypassGraphicsProxyWidget);
    d->textSelectable = false;
    connect(native, SIGNAL(linkActivated(QString)), this, SIGNAL(linkActivated(QString)));
    connect(native, SIGNAL(linkHovered(QString)), this, SIGNAL(linkHovered(QString)));

    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setPalette()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(setPalette()));

    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setWordWrap(true);
    setWidget(native);
    native->setWindowIcon(QIcon());
    d->setPalette();
}

Label::~Label()
{
    delete d;
}

void Label::setText(const QString &text)
{
    static_cast<QLabel*>(widget())->setText(text);
    updateGeometry();
}

QString Label::text() const
{
    return static_cast<QLabel*>(widget())->text();
}

void Label::setImage(const QString &path)
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

QString Label::image() const
{
    return d->imagePath;
}

void Label::setScaledContents(bool scaled)
{
    static_cast<QLabel*>(widget())->setScaledContents(scaled);
}

bool Label::hasScaledContents() const
{
    return static_cast<QLabel*>(widget())->hasScaledContents();
}

void Label::setTextSelectable(bool enable)
{
    if (enable) {
        nativeWidget()->setTextInteractionFlags(Qt::TextBrowserInteraction);
    } else {
        nativeWidget()->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    }

    d->textSelectable = enable;
}

bool Label::textSelectable() const
{
  return d->textSelectable;
}

void Label::setAlignment(Qt::Alignment alignment)
{
    nativeWidget()->setAlignment(alignment);
}

Qt::Alignment Label::alignment() const
{
    return nativeWidget()->alignment();
}

void Label::setWordWrap(bool wrap)
{
    nativeWidget()->setWordWrap(wrap);
}

bool Label::wordWrap() const
{
    return nativeWidget()->wordWrap();
}

void Label::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString Label::styleSheet()
{
    return widget()->styleSheet();
}

QLabel *Label::nativeWidget() const
{
    return static_cast<QLabel*>(widget());
}

void Label::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName);

    QStringList texts;
    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            texts << v.toString();
        }
    }

    setText(texts.join(" "));
}

void Label::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QContextMenuEvent contextMenuEvent(QContextMenuEvent::Reason(event->reason()),
                                       event->pos().toPoint(), event->screenPos(), event->modifiers());
    QApplication::sendEvent(nativeWidget(), &contextMenuEvent);
}

void Label::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap();
    QGraphicsProxyWidget::resizeEvent(event);
}

void Label::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsProxyWidget::mousePressEvent(event);
    //FIXME: when QTextControl accept()s mouse press events (as of Qt 4.6.2, it processes them
    //but never marks them as accepted) the following event->accept() can be removed
    if (d->textSelectable) {
        event->accept();
    }
}

void Label::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->textSelectable) {
        QGraphicsProxyWidget::mouseMoveEvent(event);
    }
}

void Label::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    QLabel *native = nativeWidget();
    QFontMetrics fm = native->font();

    //indirect painting still used for fade out
    if (native->wordWrap() || native->text().isEmpty() || size().width() >= fm.width(native->text())) {
        QGraphicsProxyWidget::paint(painter, option, widget);
    } else {
        const int gradientLength = 25;
        QPixmap buffer(contentsRect().size().toSize());
        buffer.fill(Qt::transparent);

        QPainter buffPainter(&buffer);

        QGraphicsProxyWidget::paint(&buffPainter, option, widget);

        QLinearGradient gr;

        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        buffPainter.setPen(Qt::NoPen);

        if (option->direction == Qt::LeftToRight) {
            gr.setStart(size().width()-gradientLength, 0);
            gr.setFinalStop(size().width(), 0);
            gr.setColorAt(0, Qt::black);
            gr.setColorAt(1, Qt::transparent);
            buffPainter.setBrush(gr);

            buffPainter.drawRect(QRect(gr.start().toPoint(), QSize(gradientLength, size().height())));
        } else {
            gr.setStart(0, 0);
            gr.setFinalStop(gradientLength, 0);
            gr.setColorAt(0, Qt::transparent);
            gr.setColorAt(1, Qt::black);
            buffPainter.setBrush(gr);

            buffPainter.drawRect(QRect(0, 0, gradientLength, size().height()));
        }

        buffPainter.end();
        painter->drawPixmap(contentsRect(), buffer, buffer.rect());
    }
}

void Label::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange && font() != QApplication::font()) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsProxyWidget::changeEvent(event);
}

bool Label::event(QEvent *event)
{
    if (event->type() == QEvent::Show && font() != QApplication::font()) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }
    return QGraphicsProxyWidget::event(event);
}

QVariant Label::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == QGraphicsItem::ItemCursorHasChanged) {
        nativeWidget()->setCursor(cursor());
    }

    return QGraphicsWidget::itemChange(change, value);
}

QSizeF Label::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (sizePolicy().verticalPolicy() == QSizePolicy::Fixed) {
        return QGraphicsProxyWidget::sizeHint(Qt::PreferredSize, constraint);
    } else {
        return QGraphicsProxyWidget::sizeHint(which, constraint);
    }
}

} // namespace Plasma

#include <label.moc>

