/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "%{APPNAMELC}.h"
#include <klocalizedstring.h>

%{APPNAME}::%{APPNAME}(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_nativeText(i18n("Text coming from C++ plugin"))
{
}

%{APPNAME}::~%{APPNAME}()
{
}

QString %{APPNAME}::nativeText() const
{
    return m_nativeText;
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(%{APPNAMELC}, %{APPNAME}, "metadata.json")

#include "%{APPNAMELC}.moc"
