/*  This file is part of the KDE project
 *    Copyright (C) 2007 David Faure <faure@kde.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef PLASMACALENDARINTEGRATION_EXPORT_H
#define PLASMACALENDARINTEGRATION_EXPORT_H

#include <QtCore/QtGlobal>

#ifndef PLASMACALENDARINTEGRATION_EXPORT
/* We are building this library */
#  define PLASMACALENDARINTEGRATION_EXPORT __attribute__((visibility("default")))
# else
/* We are using this library */
#  define PLASMACALENDARINTEGRATION_EXPORT __attribute__((visibility("default")))
#endif

# ifndef PLASMACALENDARINTEGRATION_EXPORT_DEPRECATED
#  define PLASMACALENDARINTEGRATION_EXPORT_DEPRECATED Q_DECL_DEPRECATED PLASMACALENDARINTEGRATION_EXPORT
# endif

#endif
