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

#include "combobox.h"

#include <QComboBox>
#include <QPainter>

#include <KMimeType>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class ComboBox::Private
{
public:
    Private()
    {
    }

    ~Private()
    {
    }
};

ComboBox::ComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new Private)
{
    QComboBox* native = new QComboBox;
    connect(native, SIGNAL(activated(const QString &)), this, SIGNAL(activated(const QString &)));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
}

ComboBox::~ComboBox()
{
    delete d;
}

QString ComboBox::text() const
{
    return static_cast<QComboBox*>(widget())->currentText();
}

void ComboBox::setStylesheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ComboBox::stylesheet()
{
    return widget()->styleSheet();
}

QComboBox* ComboBox::nativeWidget() const
{
    return static_cast<QComboBox*>(widget());
}

void ComboBox::addItem(const QString &text)
{
    static_cast<QComboBox*>(widget())->addItem(text);
}

void ComboBox::clear()
{
    static_cast<QComboBox*>(widget())->clear();
}

void ComboBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

} // namespace Plasma

#include <combobox.moc>

