/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "textbrowser.h"

#include <QPainter>
#include <QScrollBar>
#include <QGraphicsSceneWheelEvent>
#include <QMenu>

#include <kmimetype.h>
#include <ktextbrowser.h>

#include "plasma/theme.h"
#include "plasma/svg.h"
#include "private/style_p.h"

namespace Plasma
{

class TextBrowserPrivate
{
public:
    TextBrowserPrivate(TextBrowser *browser)
        : q(browser),
          native(0),
          savedMinimumHeight(0),
          savedMaximumHeight(QWIDGETSIZE_MAX),
          wasNotFixed(true),
          customFont(false)
    {
    }

    void setFixedHeight()
    {
        if (native && native->document() &&
            q->sizePolicy().verticalPolicy() == QSizePolicy::Fixed &&
            native->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
            native->document()->setTextWidth(q->size().width());
            QSize s = native->document()->size().toSize();
            if (wasNotFixed) {
                savedMinimumHeight = q->minimumHeight();
                savedMaximumHeight = q->maximumHeight();
                wasNotFixed = false;
            }
            q->setMinimumHeight(s.height());
            q->setMaximumHeight(s.height());
        } else if (!wasNotFixed) {
            q->setMinimumHeight(savedMinimumHeight);
            q->setMaximumHeight(savedMaximumHeight);
            wasNotFixed = true;
        }
    }

    void setPalette()
    {
        KTextBrowser *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
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


    TextBrowser *q;
    KTextBrowser *native;
    Plasma::Style::Ptr style;
    int savedMinimumHeight;
    int savedMaximumHeight;
    bool wasNotFixed;
    bool customFont;
};

TextBrowser::TextBrowser(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new TextBrowserPrivate(this))
{
    KTextBrowser *native = new KTextBrowser;
    native->setWindowFlags(native->windowFlags()|Qt::BypassGraphicsProxyWidget);
    connect(native, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(native, SIGNAL(textChanged()), this, SLOT(setFixedHeight()));
    native->setWindowIcon(QIcon());
    setWidget(native);
    d->native = native;
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setFrameShape(QFrame::NoFrame);
    native->setTextBackgroundColor(Qt::transparent);
    native->viewport()->setAutoFillBackground(false);
    d->style = Plasma::Style::sharedStyle();
    native->verticalScrollBar()->setStyle(d->style.data());
    native->horizontalScrollBar()->setStyle(d->style.data());
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(setPalette()));
}

TextBrowser::~TextBrowser()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void TextBrowser::setText(const QString &text)
{
    static_cast<KTextBrowser*>(widget())->setText(text);
}

QString TextBrowser::text() const
{
    return static_cast<KTextBrowser*>(widget())->toHtml();
}

void TextBrowser::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    d->native->setHorizontalScrollBarPolicy(policy);
}

void TextBrowser::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    d->native->setVerticalScrollBarPolicy(policy);
}

void TextBrowser::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString TextBrowser::styleSheet()
{
    return widget()->styleSheet();
}

KTextBrowser *TextBrowser::nativeWidget() const
{
    return static_cast<KTextBrowser*>(widget());
}

void TextBrowser::append(const QString &text)
{
    return nativeWidget()->append(text);
}

void TextBrowser::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    KTextBrowser *te = nativeWidget();
    te->clear();

    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            te->append(v.toString() + '\n');
        }
    }
}

void TextBrowser::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *popup = nativeWidget()->createStandardContextMenu(event->screenPos());
    popup->exec(event->screenPos());
    delete popup;
}

void TextBrowser::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setFixedHeight();
    QGraphicsProxyWidget::resizeEvent(event);
}

void TextBrowser::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (d->native->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
        d->native->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        event->ignore();
    } else {
        QGraphicsProxyWidget::wheelEvent(event);
    }
}

void TextBrowser::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma

#include <textbrowser.moc>

