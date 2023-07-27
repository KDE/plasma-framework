/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialogbackground_p.h"

#include <QQmlComponent>
#include <qquickitem.h>

#include "sharedqmlengine.h"

namespace PlasmaQuick
{

DialogBackground::DialogBackground(QQuickItem *parent)
    : QQuickItem(parent)
    , m_sharedEngine(new SharedQmlEngine(this))
{
    QQmlComponent component(m_sharedEngine->engine().get(), QStringLiteral(":/DialogBackground.qml"));

    QVariantHash props({{QStringLiteral("parent"), QVariant::fromValue(this)}});
    QObject *object = m_sharedEngine->createObjectFromComponent(&component, m_sharedEngine->rootContext(), props);

    m_frameSvgItem = qobject_cast<QQuickItem *>(object);
    Q_ASSERT(m_frameSvgItem);

    connect(m_frameSvgItem, SIGNAL(maskChanged()), this, SIGNAL(maskChanged()));
    connect(m_frameSvgItem->property("fixedMargins").value<QObject *>(), SIGNAL(marginsChanged()), this, SIGNAL(fixedMarginsChanged()));
}

DialogBackground::~DialogBackground()
{
}

QString DialogBackground::imagePath() const
{
    return m_frameSvgItem->property("imagePath").toString();
}

void DialogBackground::setImagePath(const QString &path)
{
    m_frameSvgItem->setProperty("imagePath", path);
}

void DialogBackground::setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders)
{
    m_frameSvgItem->setProperty("enabledBorders", QVariant::fromValue(borders));
}

Plasma::FrameSvg::EnabledBorders DialogBackground::enabledBorders() const
{
    return m_frameSvgItem->property("enabledBorders").value<Plasma::FrameSvg::EnabledBorders>();
}

QRegion DialogBackground::mask() const
{
    return m_frameSvgItem->property("mask").value<QRegion>();
}

qreal DialogBackground::leftMargin() const
{
    // assume margins is valid, as we asserted it's a valid FrameSvgItem
    QObject *margins = m_frameSvgItem->property("fixedMargins").value<QObject *>();
    return margins->property("left").value<qreal>();
}

qreal DialogBackground::topMargin() const
{
    QObject *margins = m_frameSvgItem->property("fixedMargins").value<QObject *>();
    return margins->property("top").value<qreal>();
}

qreal DialogBackground::rightMargin() const
{
    QObject *margins = m_frameSvgItem->property("fixedMargins").value<QObject *>();
    return margins->property("right").value<qreal>();
}

qreal DialogBackground::bottomMargin() const
{
    QObject *margins = m_frameSvgItem->property("fixedMargins").value<QObject *>();
    return margins->property("bottom").value<qreal>();
}

QObject *DialogBackground::fixedMargins() const
{
    return m_frameSvgItem->property("fixedMargins").value<QObject *>();
}

QObject *DialogBackground::inset() const
{
    return m_frameSvgItem->property("inset").value<QObject *>();
}
}

#include "private/moc_dialogbackground_p.cpp"
