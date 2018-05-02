/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

#ifndef BLURWATCHER_H
#define BLURWATCHER_H

#include <QObject>

#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>

namespace Plasma
{

class EffectWatcher: public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    EffectWatcher(const QString &property, QObject *parent = nullptr);

protected:
    bool isEffectActive() const;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

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
