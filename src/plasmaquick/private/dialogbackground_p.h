/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DIALOGBACKGROUND_P_H
#define DIALOGBACKGROUND_P_H

#include "sharedqmlengine.h"
#include <Plasma/FrameSvg>

#include <QQuickItem>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace PlasmaQuick
{
class SharedQmlEngine;

// This class wraps a FrameSvgITem created from QML, so is not necessary to statically link to it to be used in Dialog
class DialogBackground : public QQuickItem
{
    Q_OBJECT

public:
    DialogBackground(QQuickItem *parent = nullptr);
    ~DialogBackground() override;

    QString imagePath() const;
    void setImagePath(const QString &name);

    void setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders);
    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QRegion mask() const;

    qreal leftMargin() const;
    qreal topMargin() const;
    qreal rightMargin() const;
    qreal bottomMargin() const;

    // Needed for the QML api of Dialog
    QObject *fixedMargins() const;
    QObject *inset() const;

Q_SIGNALS:
    void fixedMarginsChanged();
    void maskChanged();

private:
    QQuickItem *m_frameSvgItem;
    SharedQmlEngine *m_sharedEngine;
};

}

#endif // multiple inclusion guard
