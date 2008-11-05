/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef SODEPHELPERS_P_H
#define SODEPHELPERS_P_H

#include <QtCore/QIODevice>
#include <QtCore/QString>

inline void sodepWrite(QIODevice &io, double value)
{
    const char *in = (const char*)&value;
    char out[8];

    for (int i=0; i<8; ++i) {
        out[i] = in[7-i];
    }

    io.write(out, 8);
}

inline void sodepWrite(QIODevice &io, qint32 value)
{
    const char *in = (const char*)&value;
    char out[4];

    for (int i=0; i<4; ++i) {
        out[i] = in[3-i];
    }

    io.write(out, 4);
}

inline void sodepWrite(QIODevice &io, qint64 value)
{
    const char *in = (const char*)&value;
    char out[8];

    for (int i=0; i<8; ++i) {
        out[i] = in[7-i];
    }

    io.write(out, 8);
}

inline void sodepWrite(QIODevice &io, const QString &value)
{
    QByteArray data = value.toUtf8();
    sodepWrite(io, data.size());
    io.write(data);
}

inline double sodepReadDouble(QIODevice &io)
{
    double d;
    char *out = (char*)&d;
    char in[8];

    io.read(in, 8);

    for (int i=0; i<8; ++i) {
        out[i] = in[7-i];
    }

    return d;
}

inline qint32 sodepReadInt32(QIODevice &io)
{
    qint32 i;
    char *out = (char*)&i;
    char in[4];

    io.read(in, 4);

    for (int j=0; j<4; ++j) {
        out[j] = in[3-j];
    }

    return i;
}

inline qint64 sodepReadInt64(QIODevice &io)
{
    qint64 i;
    char *out = (char*)&i;
    char in[8];

    io.read(in, 8);

    for (int j=0; j<8; ++j) {
        out[j] = in[7-j];
    }

    return i;
}

inline QString sodepReadString(QIODevice &io)
{
    qint32 length = sodepReadInt32(io);
    QByteArray data = io.read(length);
    return QString::fromUtf8(data);
}


#endif
