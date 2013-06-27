/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#ifndef PLASMA_CONFIGLOADERHANDLER_P_H
#define PLASMA_CONFIGLOADERHANDLER_P_H

#include <QXmlDefaultHandler>

namespace Plasma
{

class ConfigLoaderHandler : public QXmlDefaultHandler
{
public:
    ConfigLoaderHandler(ConfigLoader *config, ConfigLoaderPrivate *d);
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &ch);

    QString name() const;
    void setName(const QString &name);
    QString key() const;
    void setKey(const QString &name);
    QString type() const;
    QString currentGroup() const;
    QString defaultValue() const;

private:
    virtual void addItem();
    void resetState();

    ConfigLoader *m_config;
    ConfigLoaderPrivate *d;
    int m_min;
    int m_max;
    QString m_currentGroup;
    QString m_name;
    QString m_key;
    QString m_type;
    QString m_label;
    QString m_default;
    QString m_cdata;
    QString m_whatsThis;
    KConfigSkeleton::ItemEnum::Choice m_choice;
    QList<KConfigSkeleton::ItemEnum::Choice> m_enumChoices;
    bool m_haveMin;
    bool m_haveMax;
    bool m_inChoice;
};

} // namespace Plasma

#endif

