/***************************************************************************
*   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "clocknumber.h"

class Number::Private
{
    public:
        char data;
};

Number::Number(QChar value)
    : d(new Private)
{
    d->data = value.toAscii();
}

Number::~Number()
{
    delete d;
}

void Number::operator--()
{
    if (d->data == '0') {
        d->data = '9';
    } else {
        d->data--;
    }
}

void Number::operator++()
{
    if (d->data == '9') {
        d->data = '0';
    } else {
        d->data++;
    }
}

void Number::operator=(QChar value)
{
    d->data = value.toAscii();
}

bool Number::operator==(char value)
{
    return d->data == value;
}

Number::operator char()
{
    return d->data;
}

