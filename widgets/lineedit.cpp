/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
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

#include "lineedit.h"

#include <limits>

#include <QStyleOptionFrameV2>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPainter>

#include <KDebug>
#include <KColorScheme>
#include <KApplication>

#include <plasma/theme.h>
#include "plasma/layouts/layout.h"

namespace Plasma
{

class LineEdit::Private
{
    public:
        Private()
             : showingDefaultText(true), styled(true), multiline(false) {}

        QString defaultText;
        QString oldText;

        bool showingDefaultText;
        bool styled;
        bool multiline;

};

LineEdit::LineEdit(QGraphicsItem *parent)
    : QGraphicsTextItem(parent),
      d(new Private())
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setCursor(Qt::IBeamCursor);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

LineEdit::~LineEdit()
{
    delete d;
}

void LineEdit::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (d->styled && widget) {
        QStyleOptionFrameV2 panel;
        panel.initFrom(widget);
        panel.state = option->state;
        panel.rect = boundingRect().toRect();

        widget->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, painter, widget);
        widget->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &panel, painter, widget);
    }

    // QGraphicsTextItem paints a black frame when it has focus
    // and is selected. We want to use our own frame, so we
    // clear these flags.
    QStyleOptionGraphicsItem *style = const_cast<QStyleOptionGraphicsItem*>(option);
    style->state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

    QGraphicsTextItem::paint(painter, style, widget);
}

void LineEdit::dataUpdated(const QString&, const DataEngine::Data& data)
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

void LineEdit::setMultiLine(bool multi)
{
   d->multiline = multi;
}

bool LineEdit::multiLine() const
{
    return d->multiline;
}


void LineEdit::setStyled(bool style)
{
    d->styled = style;

    if (style) {
        setDefaultTextColor(kapp->palette().color(QPalette::Text));
    } else {
        setDefaultTextColor(Plasma::Theme::self()->textColor());
    }
}

bool LineEdit::styled() const
{
    return d->styled;
}

Qt::Orientations LineEdit::expandingDirections() const
{
    return Qt::Vertical;
}

QSizeF LineEdit::minimumSize() const
{
    QSizeF sh = document()->size();
    sh.setWidth(textWidth());
    return sh;
}

QSizeF LineEdit::maximumSize() const
{
    //TODO: well, this is useless, isn't it? ;) when ported to WoC, remove it
    return QSizeF(std::numeric_limits<qreal>::infinity(),
                  std::numeric_limits<qreal>::infinity());
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
    //kDebug() << "LineEdit::heightForWidth(" << w << ") is " << height;
    return height;
}

bool LineEdit::hasWidthForHeight() const
{
    return false;
}

qreal LineEdit::widthForHeight(qreal h) const
{
    Q_UNUSED(h);
    return 0;
}

QRectF LineEdit::geometry() const
{
    return QRectF(pos(), boundingRect().size());
}

void LineEdit::setGeometry(const QRectF& geometry)
{
    prepareGeometryChange();
    QRectF geom(geometry.topLeft(),
                geometry.size().boundedTo(maximumSize()).expandedTo(minimumSize()));
    setTextWidth(geom.width());
    setPos(geom.topLeft());
    update();
}

void LineEdit::updateGeometry()
{
    if (managingLayout()) {
        managingLayout()->invalidate();
    } else {
        setGeometry(QRectF(pos(), sizeHint()));
    }
}

QSizeF LineEdit::sizeHint() const
{
    QSizeF sh = document()->size();

    if (sh.width() < textWidth()) {
        sh.setWidth(textWidth());
    }

    return sh;
}

void LineEdit::setDefaultText(const QString &text)
{
    d->defaultText = text.simplified();
    //FIXME hardcoded colours aren't nice
    d->defaultText = QString("<font color=\"gray\">") + d->defaultText + QString("</font>");
    if (d->showingDefaultText) {
        QGraphicsTextItem::setHtml(d->defaultText);
    }
}

const QString LineEdit::toHtml()
{
    if (d->showingDefaultText) {
        return QString();
    } else {
        return QGraphicsTextItem::toHtml();
    }
}

const QString LineEdit::toPlainText()
{
    if (d->showingDefaultText) {
        return QString();
    } else {
        return QGraphicsTextItem::toPlainText();
    }
}

void LineEdit::setHtml(const QString &text)
{
    if (text.isEmpty()) {
        d->showingDefaultText = true;
        QGraphicsTextItem::setHtml(d->defaultText);
    } else {
        d->showingDefaultText = false;
        QGraphicsTextItem::setHtml(text);
    }
}

void LineEdit::setPlainText(const QString &text)
{
    if (text.isEmpty()) {
        d->showingDefaultText = true;
        QGraphicsTextItem::setHtml(d->defaultText);
    } else {
        d->showingDefaultText = false;
        QGraphicsTextItem::setPlainText(text);
    }
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
    if ( !d->multiline && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        emit editingFinished();
        event->accept();
    } else {
        QGraphicsTextItem::keyPressEvent(event); //let QT handle other keypresses
    }

    if (QGraphicsTextItem::toHtml() != d->oldText) {
        d->oldText = QGraphicsTextItem::toHtml();
        emit textChanged(QGraphicsTextItem::toHtml());
    }
//     if (QGraphicsTextItem::toPlainText().simplified().isEmtpy())
//         QGraphicsTextItem::setHtml(d->defaultText);
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
    if (d->showingDefaultText) {
        QGraphicsTextItem::setHtml(QString());
        d->showingDefaultText = false;
    }
    QGraphicsTextItem::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    if (QGraphicsTextItem::toPlainText().isEmpty()) {
        QGraphicsTextItem::setHtml(d->defaultText);
        d->showingDefaultText = true;
    }
    QGraphicsTextItem::focusOutEvent(event);
}

QPointF LineEdit::position() const
{
    return pos();
}

} // namespace Plasma

#include "lineedit.moc"

