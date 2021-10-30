/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_DIALOGSHADOWS_H
#define PLASMA_DIALOGSHADOWS_H

#include <QSet>

#include "plasma/framesvg.h"
#include "plasma/svg.h"

class DialogShadows : public Plasma::Svg
{
    Q_OBJECT

public:
    explicit DialogShadows(QObject *parent = nullptr, const QString &prefix = QStringLiteral("dialogs/background"));
    ~DialogShadows() override;

    static DialogShadows *self();

    void addWindow(QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);
    void removeWindow(QWindow *window);

    void setEnabledBorders(QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders);

    bool enabled() const;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void updateShadows())
    Q_PRIVATE_SLOT(d, void windowDestroyed(QObject *deletedObject))
};

#endif
