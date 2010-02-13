/*
 *   Copyright 2007-2008 Richard J. Moore <rich@kde.org>
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
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

#include <QGraphicsWidget>
#include <QScriptEngine>

#include <KConfigGroup>
#include <KIO/Job>

#include <Plasma/Applet>
#include <Plasma/Animation>
#include <Plasma/Extender>
#include <Plasma/VideoWidget>

#include "appletinterface.h"
#include "dataengine.h"
#include "variant.h"

//Q_DECLARE_METATYPE(SimpleJavaScriptApplet*)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsLayout*)

Q_DECLARE_METATYPE(KConfigGroup)

Q_DECLARE_METATYPE(Plasma::Animation*)
Q_DECLARE_METATYPE(Plasma::Applet*)
Q_DECLARE_METATYPE(Plasma::Extender*)
Q_DECLARE_METATYPE(Plasma::VideoWidget::Controls)

Q_DECLARE_METATYPE(AppletInterface*)

//Q_SCRIPT_DECLARE_QMETAOBJECT(AppletInterface, SimpleJavaScriptApplet*)


QScriptValue qScriptValueFromControls(QScriptEngine *engine, const Plasma::VideoWidget::Controls &controls)
{
    return QScriptValue(engine, controls);
}

void controlsFromScriptValue(const QScriptValue& obj, Plasma::VideoWidget::Controls &controls)
{
    int flagValue = obj.toInteger();
    //FIXME: it has to be a less ugly way to do that :)
    if (flagValue & Plasma::VideoWidget::Play) {
        controls |= Plasma::VideoWidget::Play;
    }
    if (flagValue & Plasma::VideoWidget::Pause) {
        controls |= Plasma::VideoWidget::Pause;
    }
    if (flagValue & Plasma::VideoWidget::Stop) {
        controls |= Plasma::VideoWidget::Stop;
    }
    if (flagValue & Plasma::VideoWidget::PlayPause) {
        controls |= Plasma::VideoWidget::PlayPause;
    }
    if (flagValue & Plasma::VideoWidget::Progress) {
        controls |= Plasma::VideoWidget::Progress;
    }
    if (flagValue & Plasma::VideoWidget::Volume) {
        controls |= Plasma::VideoWidget::Volume;
    }
    if (flagValue & Plasma::VideoWidget::OpenFile) {
        controls |= Plasma::VideoWidget::OpenFile;
    }
}

Q_DECLARE_METATYPE(KJob *)
typedef KJob* KJobPtr;
QScriptValue qScriptValueFromKJob(QScriptEngine *engine, const KJobPtr &job)
{
    return engine->newQObject(const_cast<KJob *>(job), QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void qKJobFromQScriptValue(const QScriptValue &scriptValue, KJobPtr &job)
{
    QObject *obj = scriptValue.toQObject();
    job = static_cast<KJob *>(obj);
}

Q_DECLARE_METATYPE(KIO::Job *)
typedef KIO::Job* KioJobPtr;
QScriptValue qScriptValueFromKIOJob(QScriptEngine *engine, const KioJobPtr &job)
{
    return engine->newQObject(const_cast<KIO::Job *>(job), QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void qKIOJobFromQScriptValue(const QScriptValue &scriptValue, KioJobPtr &job)
{
    QObject *obj = scriptValue.toQObject();
    job = static_cast<KIO::Job *>(obj);
}

QScriptValue qScriptValueFromKConfigGroup(QScriptEngine *engine, const KConfigGroup &config)
{
    QScriptValue obj = engine->newObject();

    if (!config.isValid()) {
        return obj;
    }

    QMap<QString, QString> entryMap = config.entryMap();
    QMap<QString, QString>::const_iterator it = entryMap.constBegin();
    QMap<QString, QString>::const_iterator begin = it;
    QMap<QString, QString>::const_iterator end = entryMap.constEnd();

    //setting the group name
    obj.setProperty("__name", QScriptValue(engine, config.name()));

    //setting the key/value pairs
    for (it = begin; it != end; ++it) {
        //kDebug() << "setting" << it.key() << "to" << it.value();
        QString prop = it.key();
        prop.replace(' ', '_');
        obj.setProperty(prop, variantToScriptValue(engine, it.value()));
    }

    return obj;
}

void kConfigGroupFromScriptValue(const QScriptValue& obj, KConfigGroup &config)
{
    KConfigSkeleton *skel = new KConfigSkeleton();
    config = KConfigGroup(skel->config(), obj.property("__name").toString());

    QScriptValueIterator it(obj);

    while (it.hasNext()) {
        it.next();
        //kDebug() << it.name() << "is" << it.value().toString();
        if (it.name() != "__name") {
            config.writeEntry(it.name(), it.value().toString());
        }
    }
}

typedef Plasma::Animation* AnimationPtr;
QScriptValue qScriptValueFromAnimation(QScriptEngine *engine, const AnimationPtr &anim)
{
    return engine->newQObject(const_cast<Animation *>(anim), QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void abstractAnimationFromQScriptValue(const QScriptValue &scriptValue, AnimationPtr &anim)
{
    QObject *obj = scriptValue.toQObject();
    anim = static_cast<Animation *>(obj);
}

typedef QGraphicsWidget * QGraphicsWidgetPtr;
QScriptValue qScriptValueFromQGraphicsWidget(QScriptEngine *engine, const QGraphicsWidgetPtr &anim)
{
    return engine->newQObject(const_cast<QGraphicsWidget *>(anim), QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void qGraphicsWidgetFromQScriptValue(const QScriptValue &scriptValue, QGraphicsWidgetPtr &anim)
{
    QObject *obj = scriptValue.toQObject();
    anim = static_cast<QGraphicsWidget *>(obj);
}

typedef Plasma::Extender *ExtenderPtr;
QScriptValue qScriptValueFromExtender(QScriptEngine *engine, const ExtenderPtr &extender)
{
    return engine->newQObject(const_cast<Plasma::Extender *>(extender), QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void extenderFromQScriptValue(const QScriptValue &scriptValue, ExtenderPtr &extender)
{
    QObject *obj = scriptValue.toQObject();
    extender = static_cast<Plasma::Extender *>(obj);
}

using namespace Plasma;

void registerSimpleAppletMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterMetaType<QGraphicsWidget*>(engine, qScriptValueFromQGraphicsWidget, qGraphicsWidgetFromQScriptValue);

    qScriptRegisterMetaType<KConfigGroup>(engine, qScriptValueFromKConfigGroup, kConfigGroupFromScriptValue, QScriptValue());
    qScriptRegisterMetaType<KJob *>(engine, qScriptValueFromKJob, qKJobFromQScriptValue);
    qScriptRegisterMetaType<KIO::Job *>(engine, qScriptValueFromKIOJob, qKIOJobFromQScriptValue);

    qScriptRegisterMetaType<Plasma::Animation *>(engine, qScriptValueFromAnimation, abstractAnimationFromQScriptValue);
    qScriptRegisterMetaType<Plasma::Extender *>(engine, qScriptValueFromExtender, extenderFromQScriptValue);
    qScriptRegisterMetaType<Plasma::VideoWidget::Controls>(engine, qScriptValueFromControls, controlsFromScriptValue, QScriptValue());

    registerDataEngineMetaTypes(engine);
}

