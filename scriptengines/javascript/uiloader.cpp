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

#include <Plasma/BusyWidget>
#include <Plasma/CheckBox>
#include <Plasma/ComboBox>
#include <Plasma/FlashingLabel>
#include <Plasma/Frame>
#include <Plasma/GroupBox>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/Meter>
#include <Plasma/PushButton>
#include <Plasma/RadioButton>
#include <Plasma/ScrollBar>
#include <Plasma/SignalPlotter>
#include <Plasma/Slider>
#include <Plasma/SvgWidget>
#include <Plasma/TabBar>
#include <Plasma/TextEdit>
#include <Plasma/ToolButton>
#include <Plasma/TreeView>
#include <Plasma/WebView>

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
        return new Plasma::BusyWidget(parent);
    } else if (className == QString("CheckBox")) {
        return new Plasma::CheckBox(parent);
    } else if (className == QString("ComboBox")) {
        return new Plasma::ComboBox(parent);
    } else if (className == QString("FlashingLabel")) {
        return new Plasma::FlashingLabel(parent);
    } else if (className == QString("Frame")) {
        return new Plasma::Frame(parent);
    } else if (className == QString("GroupBox")) {
        return new Plasma::GroupBox(parent);
    } else if (className == QString("IconWidget")) {
        return new Plasma::IconWidget(parent);
    } else if (className == QString("Label")) {
        return new Plasma::Label(parent);
    } else if (className == QString("LineEdit")) {
        return new Plasma::LineEdit(parent);
    } else if (className == QString("Meter")) {
        return new Plasma::Meter(parent);
    } else if (className == QString("PushButton")) {
        return new Plasma::PushButton(parent);
    } else if (className == QString("RadioButton")) {
        return new Plasma::RadioButton(parent);
    } else if (className == QString("ScrollBar")) {
        return new Plasma::ScrollBar(parent);
    } else if (className == QString("SignalPlotter")) {
        return new Plasma::SignalPlotter(parent);
    } else if (className == QString("Slider")) {
        return new Plasma::Slider(parent);
    } else if (className == QString("SvgWidget")) {
        return new Plasma::SvgWidget(parent);
    } else if (className == QString("TabBar")) {
        return new Plasma::TabBar(parent);
    } else if (className == QString("TextEdit")) {
        return new Plasma::TextEdit(parent);
    } else if (className == QString("ToolButton")) {
        return new Plasma::ToolButton(parent);
    } else if (className == QString("TreeView")) {
        return new Plasma::TreeView(parent);
    } else if (className == QString("WebView")) {
        return new Plasma::WebView(parent);
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

#include "uiloader.moc"

