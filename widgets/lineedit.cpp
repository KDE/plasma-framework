/*
 *   Copyright (C) 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "lineedit.h"

#include <QStyleOptionFrameV2>
#include <QTextDocument>
#include <QKeyEvent>
#include <KDebug>

namespace Plasma
{

class LineEdit::Private
{
};

LineEdit::LineEdit(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsTextItem(parent, scene),
      d(new Private())
{
    defaultText = QString("");
    defaultTextPlain = QString("");
    oldText = QString("");
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

LineEdit::~LineEdit()
{
    delete d;
}

void LineEdit::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionFrameV2 panel;
    panel.initFrom(widget);
    panel.state = option->state;
    panel.rect = boundingRect().toRect();

    widget->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, painter, widget);
    widget->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &panel, painter, widget);

    // QGraphicsTextItem paints a black frame when it has focus
    // and is selected. We want to use our own frame, so we
    // clear these flags.
    QStyleOptionGraphicsItem *style = const_cast<QStyleOptionGraphicsItem*>(option);
    style->state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

    QGraphicsTextItem::paint(painter, style, widget);
}

void LineEdit::updated(const QString&, const DataEngine::Data& data)
{
    DataEngine::DataIterator it(data);

    QString text;
    while (it.hasNext()) {
        it.next();
        if (it.value().canConvert(QVariant::String)) {
            text.append(QString("<p><b>%1</b>: %2</p>").arg(it.key(), it.value().toString()));
        }
    }
    setHtml(text);
}

Qt::Orientations LineEdit::expandingDirections() const
{
    return Qt::Vertical;
}

QSizeF LineEdit::minimumSize() const
{
    return boundingRect().size();
}

QSizeF LineEdit::maximumSize() const
{
    return minimumSize();
}

bool LineEdit::hasHeightForWidth() const
{
    return true;
}

qreal LineEdit::heightForWidth(qreal w) const
{
    QTextDocument* doc = document();
    doc->setTextWidth(w);
    qreal height = doc->size().height();
    kDebug() << "LineEdit::heightForWidth(" << w << ") is " << height << endl;
    return height;
}

bool LineEdit::hasWidthForHeight() const
{
    return false;
}

qreal LineEdit::widthForHeight(qreal h) const
{
    return 0;
}

QRectF LineEdit::geometry() const
{
    return boundingRect().toRect();
}

void LineEdit::setGeometry(const QRectF& geometry)
{
    prepareGeometryChange();
    setTextWidth(geometry.width());
    update();
}

QSizeF LineEdit::sizeHint() const
{
    kDebug() << "LineEdit::sizeeHint() " << document()->size() << endl;
    return document()->size();
}

void LineEdit::setDefaultText(QString text)
{
    defaultText = text;
    QGraphicsTextItem::setHtml(defaultText);
    defaultTextPlain = QGraphicsTextItem::toPlainText();
}

const QString LineEdit::toHtml()
{
    if (QGraphicsTextItem::toHtml().simplified() == defaultText.simplified())
        return QString("");
    else
        return QGraphicsTextItem::toHtml();
}

const QString LineEdit::toPlainText()
{
    if (QGraphicsTextItem::toHtml().simplified() == defaultText.simplified())
        return QString("");
    else
        return QGraphicsTextItem::toPlainText();
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit editingFinished();
    } else {
        QGraphicsTextItem::keyPressEvent(event); //let QT handle other keypresses
    }
    if (QGraphicsTextItem::toHtml() != oldText) {
        oldText = QGraphicsTextItem::toHtml();
        emit textChanged(oldText);
    }
//     if (QGraphicsTextItem::toPlainText().simplified() == "")
//         QGraphicsTextItem::setHtml(defaultText);
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
    if (QGraphicsTextItem::toPlainText().simplified() == defaultTextPlain.simplified())
        QGraphicsTextItem::setPlainText(QString(""));
    QGraphicsTextItem::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    if (QGraphicsTextItem::toPlainText().simplified() == "")
        QGraphicsTextItem::setHtml(defaultText);
    QGraphicsTextItem::focusOutEvent(event);
}

} // namespace Plasma

#include "lineedit.moc"

