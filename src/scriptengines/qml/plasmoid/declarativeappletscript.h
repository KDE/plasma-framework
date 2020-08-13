/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DECLARATIVE_APPLETSCRIPT_H
#define DECLARATIVE_APPLETSCRIPT_H

#include <QQmlEngine>
#include <QSet>

#include <plasma/scripting/appletscript.h>

class AppletInterface;

class DeclarativeAppletScript : public Plasma::AppletScript
{
    Q_OBJECT

public:
    DeclarativeAppletScript(QObject *parent, const QVariantList &args);
    ~DeclarativeAppletScript() override;

    QString filePath(const QString &type, const QString &file) const;

    QList<QAction *> contextualActions() override;

    void constraintsEvent(Plasma::Types::Constraints constraints) override;

public Q_SLOTS:
    void executeAction(const QString &name);

protected:
    bool init() override;

Q_SIGNALS:
    void formFactorChanged();
    void locationChanged();
    void contextChanged();

private:
    AppletInterface *m_interface;
    QVariantList m_args;
    friend class AppletLoader;
    friend class AppletInterface;
    friend class ContainmentInterface;
};

#endif
