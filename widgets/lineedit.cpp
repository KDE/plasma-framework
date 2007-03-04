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

#include <QStyleOptionFrameV2>

#include "lineedit.h"

namespace Plasma
{

LineEdit::LineEdit(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsTextItem(parent, scene),
    DataVisualization()
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

LineEdit::~LineEdit()
{
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

void LineEdit::data(const DataSource::Data&)
{
}

} // namespace Plasma
