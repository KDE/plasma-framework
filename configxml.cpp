/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "configxml.h"

#include <QColor>
#include <QFont>
#include <QXmlContentHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include <KDebug>
#include <KUrl>

namespace Plasma
{

class ConfigXml::Private
{
    public:
        ~Private()
        {
            qDeleteAll(bools);
            qDeleteAll(strings);
            qDeleteAll(stringlists);
            qDeleteAll(colors);
            qDeleteAll(fonts);
            qDeleteAll(ints);
            qDeleteAll(uints);
            qDeleteAll(urls);
            qDeleteAll(dateTimes);
        }

        bool* newBool()
        {
            bool* v = new bool;
            bools.append(v);
            return v;
        }

        QString* newString()
        {
            QString* v = new QString;
            strings.append(v);
            return v;
        }

        QStringList* newStringList()
        {
            QStringList* v = new QStringList;
            stringlists.append(v);
            return v;
        }

        QColor* newColor()
        {
            QColor* v = new QColor;
            colors.append(v);
            return v;
        }

        QFont* newFont()
        {
            QFont* v = new QFont;
            fonts.append(v);
            return v;
        }

        qint32* newInt()
        {
            qint32* v = new qint32;
            ints.append(v);
            return v;
        }

        quint32* newUint()
        {
            quint32* v = new quint32;
            uints.append(v);
            return v;
        }

        KUrl* newUrl()
        {
            KUrl* v = new KUrl;
            urls.append(v);
            return v;
        }

        QDateTime* newDateTime()
        {
            QDateTime* v = new QDateTime;
            dateTimes.append(v);
            return v;
        }

        QList<bool*> bools;
        QList<QString*> strings;
        QList<QStringList*> stringlists;
        QList<QColor*> colors;
        QList<QFont*> fonts;
        QList<qint32*> ints;
        QList<quint32*> uints;
        QList<KUrl*> urls;
        QList<QDateTime*> dateTimes;
};

class ConfigXmlHandler : public QXmlDefaultHandler
{
public:
    ConfigXmlHandler(ConfigXml* config, ConfigXml::Private* d);
    bool startElement(const QString &namespaceURI, const QString & localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);

private:
    void addItem();
    void resetState();

    ConfigXml* m_config;
    ConfigXml::Private* d;
    int m_min;
    int m_max;
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

ConfigXmlHandler::ConfigXmlHandler(ConfigXml* config, ConfigXml::Private* d)
    : QXmlDefaultHandler(),
      m_config(config),
      d(d)
{
    resetState();
}

bool ConfigXmlHandler::startElement(const QString &namespaceURI, const QString &localName,
                                    const QString &qName, const QXmlAttributes &attrs)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)

//     kDebug() << "ConfigXmlHandler::startElement(" << localName << qName;
    int numAttrs = attrs.count();
    QString tag = localName.toLower();
    if (tag == "group") {
        for (int i = 0; i < numAttrs; ++i) {
            QString name = attrs.localName(i).toLower();
            if (name == "name") {
                kDebug() << "set group to " << attrs.value(i);
                m_config->setCurrentGroup(attrs.value(i));
            }
        }
    } else if (tag == "entry") {
        for (int i = 0; i < numAttrs; ++i) {
            QString name = attrs.localName(i).toLower();
            if (name == "name") {
                m_name = attrs.value(i);
            } else if (name == "type") {
                m_type = attrs.value(i).toLower();
            } else if (name == "key") {
                m_key = attrs.value(i);
            }
        }
    } else if (tag == "choice") {
        m_choice.name.clear();
        m_choice.label.clear();
        m_choice.whatsThis.clear();
        for (int i = 0; i < numAttrs; ++i) {
            QString name = attrs.localName(i).toLower();
            if (name == "name") {
                m_choice.name = attrs.value(i);
            }
        }
        m_inChoice = true;
    }

    return true;
}

bool ConfigXmlHandler::characters(const QString &ch)
{
    m_cdata.append(ch);
    return true;
}

bool ConfigXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)

//     kDebug() << "ConfigXmlHandler::endElement(" << localName << qName;
    QString tag = localName.toLower();
    if (tag == "entry") {
        addItem();
        resetState();
    } else if (tag == "label") {
        if (m_inChoice) {
            m_choice.label = m_cdata;
        } else {
            m_label = m_cdata;
        }
    } else if (tag == "whatsthis") {
        if (m_inChoice) {
            m_choice.whatsThis = m_cdata;
        } else {
            m_whatsThis = m_cdata;
        }
    } else if (tag == "default") {
        m_default = m_cdata;
    } else if (tag == "min") {
        m_min = m_cdata.toInt(&m_haveMin);
    } else if (tag == "max") {
        m_max = m_cdata.toInt(&m_haveMax);
    } else if (tag == "choice") {
        m_enumChoices.append(m_choice);
        m_inChoice = false;
    }

    m_cdata.clear();
    return true;
}

void ConfigXmlHandler::addItem()
{
    if (m_name.isEmpty()) {
        return;
    }

    KConfigSkeletonItem* item = 0;

    if (m_type == "bool") {
        bool defaultValue = m_default.toLower() == "true";
        item = m_config->addItemBool(m_name, *d->newBool(), defaultValue, m_key);
    } else if (m_type == "color") {
        item = m_config->addItemColor(m_name, *d->newColor(), QColor(m_default), m_key);
    } else if (m_type == "datetime") {
        item = m_config->addItemDateTime(m_name, *d->newDateTime(),
                                         QDateTime::fromString(m_default), m_key);
    } else if (m_type == "enum") {
        KConfigSkeleton::ItemEnum* enumItem =
                new KConfigSkeleton::ItemEnum(m_config->currentGroup(),
                                              m_key, *d->newInt(),
                                              m_enumChoices,
                                              m_default.toUInt());
        enumItem->setName(m_name);
        m_config->addItem(enumItem, m_name);
        item = enumItem;
    } else if (m_type == "font") {
        item = m_config->addItemFont(m_name, *d->newFont(), QFont(m_default), m_key);
    } else if (m_type == "int") {
        KConfigSkeleton::ItemInt* intItem = m_config->addItemInt(m_name,
                *d->newInt(),
                           m_default.toInt(),
                                           m_key);
        if (m_haveMin) {
            intItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            intItem->setMaxValue(m_max);
        }
        item = intItem;
    } else if (m_type == "password") {
        item = m_config->addItemPassword(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == "path") {
        item = m_config->addItemPath(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == "string") {
        item = m_config->addItemString(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == "stringlist") {
        //FIXME: the split() is naive and will break on lists with ,'s in them
        item = m_config->addItemStringList(m_name, *d->newStringList(), m_default.split(","), m_key);
    } else if (m_type == "uint") {
        KConfigSkeleton::ItemUInt* uintItem = m_config->addItemUInt(m_name,
                *d->newUint(),
                            m_default.toUInt(),
                                             m_key);
        if (m_haveMin) {
            uintItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            uintItem->setMaxValue(m_max);
        }
        item = uintItem;
    } else if (m_type == "url") {
        KConfigSkeleton::ItemUrl* urlItem =
                new KConfigSkeleton::ItemUrl(m_config->currentGroup(),
                                             m_key, *d->newUrl(),
                                             m_default);
        urlItem->setName(m_name);
        m_config->addItem(urlItem, m_name);
        item = urlItem;
    }

    if (item) {
        item->setLabel(m_label);
        item->setWhatsThis(m_whatsThis);
    }
}

void ConfigXmlHandler::resetState()
{
    m_haveMin = false;
    m_min = 0;
    m_haveMax = false;
    m_max = 0;
    m_name.clear();
    m_type.clear();
    m_label.clear();
    m_default.clear();
    m_key.clear();
    m_whatsThis.clear();
    m_enumChoices.clear();
    m_inChoice = false;
}

ConfigXml::ConfigXml(const QString &configFile, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(configFile, parent),
      d(new Private)
{
    QXmlInputSource source(xml);
    QXmlSimpleReader reader;
    ConfigXmlHandler handler(this, d);
    reader.setContentHandler(&handler);
    reader.parse(&source, false);
}

ConfigXml::ConfigXml(KSharedConfig::Ptr config, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(config, parent),
      d(new Private)
{
    QXmlInputSource source(xml);
    QXmlSimpleReader reader;
    ConfigXmlHandler handler(this, d);
    reader.setContentHandler(&handler);
    reader.parse(&source, false);
}

ConfigXml::~ConfigXml()
{
    delete d;
}

} // Plasma namespace
