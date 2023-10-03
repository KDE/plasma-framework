/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QRect>
#include <QSharedDataPointer>
#include <QtGlobal>

// This class is proposed for Qt6.something, but it's not there yet.
// keep as an implementation detail, and then drop eventually (famous last words)

class TransientPlacementHintPrivate;
class QWindow;

class TransientPlacementHint
{
public:
    TransientPlacementHint();
    ~TransientPlacementHint();
    TransientPlacementHint(const TransientPlacementHint &other);
    TransientPlacementHint &operator=(const TransientPlacementHint &other);
    bool isValid() const;
    void setParentAnchorArea(const QRect &parentAnchorRect);
    QRect parentAnchorArea() const;
    void setParentAnchor(Qt::Edges parentAnchor);
    Qt::Edges parentAnchor() const;
    void setPopupAnchor(Qt::Edges popupAnchor);
    Qt::Edges popupAnchor() const;
    Qt::Orientations slideConstraintAdjustments() const;
    void setSlideConstraintAdjustments(Qt::Orientations slideConstraintAdjustments);
    Qt::Orientations flipConstraintAdjustments() const;
    void setFlipConstraintAdjustments(Qt::Orientations flipConstraintAdjustments);
    int margin() const;
    void setMargin(int margin);

private:
    QSharedDataPointer<TransientPlacementHintPrivate> d;
};

namespace TransientPlacementHelper
{
QRect popupRect(QWindow *window, const TransientPlacementHint &placement);
}
