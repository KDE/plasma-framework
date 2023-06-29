/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmatheme.h"
#include <KIconLoader>
#include <QDebug>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScopeGuard>

#include <KConfigGroup>
#include <KIconColors>

PlasmaTheme::PlasmaTheme(QObject *parent)
    : PlatformTheme(parent)
{
    setSupportsIconColoring(true);

    auto parentItem = qobject_cast<QQuickItem *>(parent);
    if (parentItem) {
        connect(parentItem, &QQuickItem::enabledChanged, this, &PlasmaTheme::syncColors);
        connect(parentItem, &QQuickItem::visibleChanged, this, &PlasmaTheme::syncColors);
    }

    setDefaultFont(qGuiApp->font());

    KSharedConfigPtr ptr = KSharedConfig::openConfig();
    KConfigGroup general(ptr->group("general"));

    setSmallFont(general.readEntry("smallestReadableFont", []() {
        auto smallFont = qApp->font();
#ifndef Q_OS_WIN
        if (smallFont.pixelSize() != -1) {
            smallFont.setPixelSize(smallFont.pixelSize() - 2);
        } else {
            smallFont.setPointSize(smallFont.pointSize() - 2);
        }
#endif
        return smallFont;
    }()));

    syncColors();
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &PlasmaTheme::syncColors);
}

PlasmaTheme::~PlasmaTheme()
{
}

QIcon PlasmaTheme::iconFromTheme(const QString &name, const QColor &customColor)
{
    if (customColor != Qt::transparent) {
        KIconColors colors;
        colors.setText(customColor);
        return KDE::icon(name, colors);
    } else {
        return KDE::icon(name);
    }
}

void PlasmaTheme::syncColors()
{
    if (QCoreApplication::closingDown()) {
        return;
    }

    Plasma::Theme::ColorGroup group;
    switch (colorSet()) {
    case View:
        group = Plasma::Theme::ViewColorGroup;
        break;
    case Button:
        group = Plasma::Theme::ButtonColorGroup;
        break;
    case Tooltip:
        group = Plasma::Theme::ToolTipColorGroup;
        break;
    case Complementary:
        group = Plasma::Theme::ComplementaryColorGroup;
        break;
    case Header:
        group = Plasma::Theme::HeaderColorGroup;
        break;
    case Selection: // Plasma::Theme doesn't have selection group
    case Window:
    default:
        group = Plasma::Theme::NormalColorGroup;
    }

    // foreground
    setTextColor(m_theme.color(Plasma::Theme::TextColor, group));
    setDisabledTextColor(m_theme.color(Plasma::Theme::DisabledTextColor, group));
    setHighlightedTextColor(m_theme.color(Plasma::Theme::HighlightedTextColor, group));
    // Plasma::Theme doesn't have ActiveText, use PositiveTextColor
    setActiveTextColor(m_theme.color(Plasma::Theme::PositiveTextColor, group));
    setLinkColor(m_theme.color(Plasma::Theme::LinkColor, group));
    setVisitedLinkColor(m_theme.color(Plasma::Theme::VisitedLinkColor, group));
    setNegativeTextColor(m_theme.color(Plasma::Theme::NegativeTextColor, group));
    setNeutralTextColor(m_theme.color(Plasma::Theme::NeutralTextColor, group));
    setPositiveTextColor(m_theme.color(Plasma::Theme::PositiveTextColor, group));

    // background
    setBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setHighlightColor(m_theme.color(Plasma::Theme::HighlightColor, group));
    // Plasma::Theme doesn't have AlternateBackground
    setAlternateBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));

    // Plasma::Theme doesn't have any different background color type
    setActiveBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setLinkBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setVisitedLinkBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setNegativeBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setNeutralBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));
    setPositiveBackgroundColor(m_theme.color(Plasma::Theme::BackgroundColor, group));

    // decoration
    setHoverColor(m_theme.color(Plasma::Theme::HoverColor, group));
    setFocusColor(m_theme.color(Plasma::Theme::FocusColor, group));
}

bool PlasmaTheme::event(QEvent *event)
{
    if (event->type() == Kirigami::PlatformThemeEvents::ColorSetChangedEvent::type) {
        syncColors();
    }

    if (event->type() == Kirigami::PlatformThemeEvents::ColorGroupChangedEvent::type) {
        syncColors();
    }

    return PlatformTheme::event(event);
}

#include "moc_plasmatheme.cpp"
