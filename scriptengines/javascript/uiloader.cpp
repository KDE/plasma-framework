/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
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

#include "uiloader.h"

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QStringList>

#include "widgets/busywidget.h"
#include "widgets/checkbox.h"
#include "widgets/combobox.h"
#include "widgets/flashinglabel.h"
#include "widgets/frame.h"
#include "widgets/groupbox.h"
#include "widgets/iconwidget.h"
#include "widgets/label.h"
#include "widgets/lineedit.h"
#include "widgets/meter.h"
#include "widgets/pushbutton.h"
#include "widgets/radiobutton.h"
#include "widgets/scrollbar.h"
#include "widgets/signalplotter.h"
#include "widgets/slider.h"
#include "widgets/svgwidget.h"
#include "widgets/tabbar.h"
#include "widgets/textedit.h"
#include "widgets/toolbutton.h"
#include "widgets/treeview.h"
#include "widgets/webview.h"

namespace Plasma
{

class UiLoaderPrivate
{
public:
    QStringList widgets;
    QStringList layouts;
};

UiLoader::UiLoader(QObject *parent)
    : QObject(parent),
      d(new UiLoaderPrivate())
{
    d->widgets
        << "BusyWidget"
        << "CheckBox"
        << "ComboBox"
        << "FlashingLabel"
        << "Frame"
        << "GroupBox"
        << "IconWidget"
        << "Label"
        << "LineEdit"
        << "Meter"
        << "PushButton"
        << "RadioButton"
        << "ScrollBar"
        << "SignalPlotter"
        << "Slider"
        << "SvgWidget"
        << "TabBar"
        << "TextEdit"
        << "ToolButton"
        << "TreeView"
        << "WebView";

    d->layouts
        << "GridLayout"
        << "LinearLayout";
}

UiLoader::~UiLoader()
{
    delete d;
}

QStringList UiLoader::availableWidgets() const
{
    return d->widgets;
}

QGraphicsWidget *UiLoader::createWidget(const QString &className, QGraphicsWidget *parent)
{
    if (className == QString("BusyWidget")) {
        return new BusyWidget(parent);
    } else if (className == QString("CheckBox")) {
        return new CheckBox(parent);
    } else if (className == QString("ComboBox")) {
        return new ComboBox(parent);
    } else if (className == QString("FlashingLabel")) {
        return new FlashingLabel(parent);
    } else if (className == QString("Frame")) {
        return new Frame(parent);
    } else if (className == QString("GroupBox")) {
        return new GroupBox(parent);
    } else if (className == QString("IconWidget")) {
        return new IconWidget(parent);
    } else if (className == QString("Label")) {
        return new Label(parent);
    } else if (className == QString("LineEdit")) {
        return new LineEdit(parent);
    } else if (className == QString("Meter")) {
        return new Meter(parent);
    } else if (className == QString("PushButton")) {
        return new PushButton(parent);
    } else if (className == QString("RadioButton")) {
        return new RadioButton(parent);
    } else if (className == QString("ScrollBar")) {
        return new ScrollBar(parent);
    } else if (className == QString("SignalPlotter")) {
        return new SignalPlotter(parent);
    } else if (className == QString("Slider")) {
        return new Slider(parent);
    } else if (className == QString("SvgWidget")) {
        return new SvgWidget(parent);
    } else if (className == QString("TabBar")) {
        return new TabBar(parent);
    } else if (className == QString("TextEdit")) {
        return new TextEdit(parent);
    } else if (className == QString("ToolButton")) {
        return new ToolButton(parent);
    } else if (className == QString("TreeView")) {
        return new TreeView(parent);
    } else if (className == QString("WebView")) {
        return new WebView(parent);
    }

    return 0;
}

QStringList UiLoader::availableLayouts() const
{
    return d->layouts;
}

QGraphicsLayout *UiLoader::createLayout(const QString &className, QGraphicsLayoutItem *parent)
{
    if (className == QString("GridLayout")) {
        return new QGraphicsGridLayout(parent);
    } else if (className == QString("LinearLayout")) {
        return new QGraphicsLinearLayout(parent);
    }

    return 0;
}

}

#include "uiloader.moc"

