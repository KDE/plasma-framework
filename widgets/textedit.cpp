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

#include "textedit.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>

#include <kmimetype.h>
#include <ktextedit.h>

#include "applet.h"
#include "private/style_p.h"
#include "private/themedwidgetinterface_p.h"
#include "svg.h"
#include "theme.h"

namespace Plasma
{

class TextEditPrivate : public ThemedWidgetInterface<TextEdit>
{
public:
    TextEditPrivate(TextEdit *textEdit)
        : ThemedWidgetInterface<TextEdit>(textEdit)
    {
    }

    ~TextEditPrivate()
    {
    }

    Plasma::Style::Ptr style;
};

TextEdit::TextEdit(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new TextEditPrivate(this))
{
    setNativeWidget(new KTextEdit);
    d->style = Plasma::Style::sharedStyle();
    d->initTheming();
}

TextEdit::~TextEdit()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void TextEdit::setText(const QString &text)
{
    static_cast<KTextEdit*>(widget())->setText(text);
}

QString TextEdit::text() const
{
    return static_cast<KTextEdit*>(widget())->toHtml();
}

void TextEdit::setReadOnly(bool readOnly)
{
    static_cast<KTextEdit*>(widget())->setReadOnly(readOnly);
}

bool TextEdit::isReadOnly() const
{
    return static_cast<KTextEdit*>(widget())->isReadOnly();
}

void TextEdit::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString TextEdit::styleSheet()
{
    return widget()->styleSheet();
}

void TextEdit::setNativeWidget(KTextEdit *nativeWidget)
{
    if (widget()) {
        widget()->deleteLater();
    }

    nativeWidget->setWindowFlags(nativeWidget->windowFlags()|Qt::BypassGraphicsProxyWidget);

    connect(nativeWidget, SIGNAL(textChanged()), this, SIGNAL(textChanged()));

    nativeWidget->setWindowIcon(QIcon());
    setWidget(nativeWidget);

    nativeWidget->setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget->setFrameShape(QFrame::NoFrame);
    nativeWidget->setTextBackgroundColor(Qt::transparent);
    nativeWidget->viewport()->setAutoFillBackground(false);
    nativeWidget->verticalScrollBar()->setStyle(d->style.data());
    nativeWidget->horizontalScrollBar()->setStyle(d->style.data());
}

KTextEdit *TextEdit::nativeWidget() const
{
    return static_cast<KTextEdit*>(widget());
}

void TextEdit::append(const QString &text)
{
    return nativeWidget()->append(text);
}

void TextEdit::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    KTextEdit *te = nativeWidget();
    te->clear();

    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            te->append(v.toString() + '\n');
        }
    }
}

void TextEdit::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *popup = nativeWidget()->mousePopupMenu();
    popup->exec(event->screenPos());
    delete popup;
}

void TextEdit::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

void TextEdit::changeEvent(QEvent *event)
{
    d->changeEvent(event);
    QGraphicsProxyWidget::changeEvent(event);
}

void TextEdit::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::AcceptingInputStatus);
    }
    QGraphicsProxyWidget::mousePressEvent(event);
}

void TextEdit::focusOutEvent(QFocusEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::UnknownStatus);
    }
    QGraphicsProxyWidget::focusOutEvent(event);
}

} // namespace Plasma

#include <textedit.moc>

