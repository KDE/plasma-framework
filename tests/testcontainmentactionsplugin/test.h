/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#ifndef CONTEXTTEST_HEADER
#define CONTEXTTEST_HEADER

#include "ui_config.h"
#include <plasma/containmentactions.h>
#include <QString>

class ContextTest : public Plasma::ContainmentActions
{
    Q_OBJECT
    public:
        ContextTest(QObject* parent, const QVariantList& args);

        void init(const KConfigGroup &config);
        void contextEvent(QEvent *event);
        void contextEvent(QGraphicsSceneMouseEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent *event);

        QWidget* createConfigurationInterface(QWidget* parent);
        void configurationAccepted();
        void save(KConfigGroup &config);

    private:
        Ui::Config m_ui;
        QString m_text;
};

K_EXPORT_PLASMA_CONTAINMENTACTIONS(test, ContextTest)

#endif
