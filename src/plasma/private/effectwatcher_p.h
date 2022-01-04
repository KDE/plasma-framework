/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EFFECTWATCHER_P_H
#define EFFECTWATCHER_P_H

#include <QObject>

#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>

namespace Plasma
{
class EffectWatcher : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit EffectWatcher(const QString &property, QObject *parent = nullptr);

protected:
    bool isEffectActive() const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

Q_SIGNALS:
    void effectChanged(bool on);

private:
    void init(const QString &property);
    xcb_atom_t m_property;
    bool m_effectActive;
    bool m_isX11;
};

} // namespace Plasma

#endif
