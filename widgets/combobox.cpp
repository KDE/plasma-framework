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

#include <KComboBox>
#include <QPainter>

#include <KMimeType>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class ComboBoxPrivate
{
public:
    ComboBoxPrivate()
    {
    }

    ~ComboBoxPrivate()
    {
    }
};

ComboBox::ComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ComboBoxPrivate)
{
    KComboBox* native = new KComboBox;
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
    return static_cast<KComboBox*>(widget())->currentText();
}

void ComboBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ComboBox::styleSheet()
{
    return widget()->styleSheet();
}

KComboBox* ComboBox::nativeWidget() const
{
    return static_cast<KComboBox*>(widget());
}

void ComboBox::addItem(const QString &text)
{
    static_cast<KComboBox*>(widget())->addItem(text);
}

void ComboBox::clear()
{
    static_cast<KComboBox*>(widget())->clear();
}

void ComboBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

} // namespace Plasma

#include <combobox.moc>

