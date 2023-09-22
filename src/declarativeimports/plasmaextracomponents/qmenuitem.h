/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMENUITEM_H
#define QMENUITEM_H

#include <QAction>
#include <QObject>

#include <qqmlregistration.h>

class QMenuItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(MenuItem)

    /**
     * The parent object
     */
    Q_PROPERTY(QObject *parent READ parent WRITE setParent)

    /**
     * If true, the menu item will behave like a separator
     */
    Q_PROPERTY(bool separator READ separator WRITE setSeparator NOTIFY separatorChanged)
    /**
     * If true, the menu item will behave like a section
     */
    Q_PROPERTY(bool section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QVariant icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QAction *action READ action WRITE setAction NOTIFY actionChanged)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY toggled)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit QMenuItem(QObject *parent = nullptr);

    QAction *action() const;
    void setAction(QAction *a);
    QVariant icon() const;
    void setIcon(const QVariant &i);
    bool separator() const;
    void setSeparator(bool s);
    bool section() const;
    void setSection(bool s);
    QString text() const;
    void setText(const QString &t);

    bool checkable() const;
    void setCheckable(bool checkable);

    bool checked() const;
    void setChecked(bool checked);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isVisible() const;
    void setVisible(bool visible);

Q_SIGNALS:
    void clicked();

    void actionChanged();
    void iconChanged();
    void separatorChanged();
    void sectionChanged();
    void textChanged();
    void toggled(bool checked);
    void checkableChanged();
    void enabledChanged();
    void visibleChanged();

private:
    QAction *m_action;
    QVariant m_icon;
    bool m_section;
};

#endif // QMENUITEM_H
