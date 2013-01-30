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

#ifndef QTJOLIE_SODEPHELPERS_P_H
#define QTJOLIE_SODEPHELPERS_P_H

#include <QtCore/QIODevice>
#include <QtCore/QByteArray>

#include "value.h"
#include "fault.h"
#include "message.h"

namespace Jolie
{

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

inline void sodepWrite(QIODevice &io, const QByteArray &value)
{
    sodepWrite(io, value.size());
    io.write(value);
}

inline void sodepWrite(QIODevice &io, const Value &value)
{
    if (value.isDouble()) {
        io.putChar(3);
        sodepWrite(io, value.toDouble());
    } else if (value.isInt()) {
        io.putChar(2);
        sodepWrite(io, value.toInt());
    } else if (value.isByteArray()) {
        io.putChar(1);
        sodepWrite(io, value.toByteArray());
    } else {
        io.putChar(0);
    }

    sodepWrite(io, value.childrenNames().size());

    foreach (const QByteArray &name, value.childrenNames()) {
        sodepWrite(io, name);

        QList<Value> values = value.children(name);
        qint32 valueCount = values.size();
        sodepWrite(io, valueCount);
        for (int j=0; j<valueCount; ++j) {
            sodepWrite(io, values[j]);
        }
    }

}

inline void sodepWrite(QIODevice &io, const Fault &fault)
{
    if (!fault.isValid()) {
        io.putChar(0);
        return;
    }

    io.putChar(1);
    sodepWrite(io, fault.name());
    sodepWrite(io, fault.data());
}

inline void sodepWrite(QIODevice &io, const Message &message)
{
    sodepWrite(io, message.id());
    sodepWrite(io, message.resourcePath());
    sodepWrite(io, message.operationName());
    sodepWrite(io, message.fault());
    sodepWrite(io, message.data());
}

inline double sodepReadDouble(QIODevice &io)
{
    while (io.bytesAvailable()<8) {
        io.waitForReadyRead(-1);
    }

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
    while (io.bytesAvailable()<4) {
        io.waitForReadyRead(-1);
    }

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
    while (io.bytesAvailable()<8) {
        io.waitForReadyRead(-1);
    }

    qint64 i;
    char *out = (char*)&i;
    char in[8];

    io.read(in, 8);

    for (int j=0; j<8; ++j) {
        out[j] = in[7-j];
    }

    return i;
}

inline QByteArray sodepReadByteArray(QIODevice &io)
{
    qint32 length = sodepReadInt32(io);

    while (io.bytesAvailable()<length) {
        io.waitForReadyRead(-1);
    }

    char *data = new char[length+1];
    io.read(data, length);

    QByteArray result(data, length);
    delete[] data;

    return result;
}

inline Value sodepReadValue(QIODevice &io)
{
    Value result;

    while (io.bytesAvailable()<1) {
        io.waitForReadyRead(-1);
    }

    char code;
    io.getChar(&code);

    switch(code) {
    case 3: {
        result = Value(sodepReadDouble(io));
        break;
    }
    case 2: {
        result = Value(sodepReadInt32(io));
        break;
    }
    case 1: {
        result = Value(sodepReadByteArray(io));
        break;
    }
    default:
        break;
    }

    qint32 childrenCount = sodepReadInt32(io);

    for (int i=0; i<childrenCount; ++i) {
        QByteArray name = sodepReadByteArray(io);

        qint32 valueCount = sodepReadInt32(io);
        for (int j=0; j<valueCount; ++j) {
            result.children(name) << sodepReadValue(io);
        }
    }

    return result;
}

inline Fault sodepReadFault(QIODevice &io)
{
    while (io.bytesAvailable()<1) {
        io.waitForReadyRead(-1);
    }

    char code;
    io.getChar(&code);

    if (code!=1) {
        return Fault();
    }

    QByteArray name = sodepReadByteArray(io);
    Value data = sodepReadValue(io);

    return Fault(name, data);
}

inline Message sodepReadMessage(QIODevice &io)
{
    qint64 id = sodepReadInt64(io);
    QByteArray resourcePath = sodepReadByteArray(io);
    QByteArray operationName = sodepReadByteArray(io);

    Message result(resourcePath, operationName, id);

    result.setFault(sodepReadFault(io));
    result.setData(sodepReadValue(io));

    return result;
}

} // namespace Jolie

#endif

