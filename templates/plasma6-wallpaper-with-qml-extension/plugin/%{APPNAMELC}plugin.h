/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef %{APPNAMEUC}PLUGIN_H
#define %{APPNAMEUC}PLUGIN_H

#include <QQmlExtensionPlugin>

class %{APPNAME}Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // %{APPNAMEUC}PLUGIN_H
