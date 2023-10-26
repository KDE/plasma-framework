/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMATHEME_H
#define PLASMATHEME_H

#include <Kirigami/Platform/PlatformTheme>

#include <Plasma/Theme>
#include <QColor>
#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

class KIconLoader;

class PlasmaTheme : public Kirigami::Platform::PlatformTheme
{
    Q_OBJECT

public:
    explicit PlasmaTheme(QObject *parent = nullptr);
    ~PlasmaTheme() override;

    Q_INVOKABLE QIcon iconFromTheme(const QString &name, const QColor &customColor = Qt::transparent) override;

    void syncColors();

protected:
    bool event(QEvent *event) override;

private:
    Plasma::Theme m_theme;
};

#endif // PLASMATHEME_H
