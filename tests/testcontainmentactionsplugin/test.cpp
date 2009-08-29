/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#include "test.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>

#include <Plasma/Containment>

ContextTest::ContextTest(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
    setConfigurationRequired(true);
}

void ContextTest::contextEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            contextEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
            break;
        case QEvent::GraphicsSceneWheel:
            wheelEvent(static_cast<QGraphicsSceneWheelEvent*>(event));
            break;
        default:
            break;
    }
}

void ContextTest::contextEvent(QGraphicsSceneMouseEvent *event)
{
    kDebug() << "test!!!!!!!!!!!!!!!!!!!!!!!" << event->pos();
    kDebug() << event->buttons() << event->modifiers();

    Plasma::Containment *c = containment();
    if (c) {
        kDebug() << c->name();
    } else {
        kDebug() << "fail";
        return;
    }

    KMenu desktopMenu;
    desktopMenu.addTitle(m_text);
    desktopMenu.addAction(c->action("configure"));
    desktopMenu.exec(event->screenPos());

}

void ContextTest::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    kDebug() << "test!!!!!!!!!!!!!11111111!!";
    kDebug() << event->orientation() << event->delta();
    kDebug() << event->buttons() << event->modifiers();
}

void ContextTest::init(const KConfigGroup &config)
{
    m_text = config.readEntry("test-text", QString());
    setConfigurationRequired(m_text.isEmpty());
}

QWidget* ContextTest::createConfigurationInterface(QWidget* parent)
{
    //m_currentText = m_text;
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.text->setText(m_text);
    //FIXME this way or just get it on close?
    //connect(m_ui.text, SIGNAL(changed(const QColor&)), this, SLOT(setColor(const QColor&)));
    //connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return widget;
}

void ContextTest::configurationAccepted()
{
    m_text = m_ui.text->text();
}

void ContextTest::save(KConfigGroup &config)
{
    config.writeEntry("test-text", m_text);
}


#include "test.moc"
