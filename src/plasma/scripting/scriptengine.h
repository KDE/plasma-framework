/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_SCRIPTENGINE_H
#define PLASMA_SCRIPTENGINE_H

#include <plasma/package.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

#include <QObject>
#include <QRect>
#include <QSizeF>

class QPainter;

namespace Plasma
{
class Applet;
class AppletScript;
class DataEngine;
class DataEngineScript;
class ScriptEnginePrivate;

/**
 * @class ScriptEngine plasma/scripting/scriptengine.h <Plasma/Scripting/ScriptEngine>
 *
 * @short The base class for scripting interfaces to be used in loading
 *        plasmoids of a given language.
 **/

class PLASMA_EXPORT ScriptEngine : public QObject
{
    Q_OBJECT

public:
    ~ScriptEngine();

    /**
     * Called when it is safe to initialize the internal state of the engine
     */
    virtual bool init();

protected:
    explicit ScriptEngine(QObject *parent = nullptr);

    /**
     * @return absolute path to the main script file for this plasmoid
     */
    virtual QString mainScript() const;

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     * @deprecated Since 5.83 Use kpackage API instead
     */
    PLASMA_DEPRECATED_VERSION(5, 83, "Use kpackage API instead")
    virtual Package package() const;
#endif

private:
    ScriptEnginePrivate *const d;
};

/**
 * @param types a set of ComponentTypes flags for which to look up the
 *            language support for
 * @return a list of all supported languages for the given type(s).
 **/
PLASMA_EXPORT QStringList knownLanguages(Types::ComponentTypes types);

/**
 * Loads an Applet script engine for the given language.
 *
 * @param language the language to load for
 * @param applet the Plasma::Applet for this script
 * @return pointer to the AppletScript or 0 on failure; the caller is responsible
 *         for the return object which will be parented to the Applet
 **/
PLASMA_EXPORT AppletScript *loadScriptEngine(const QString &language, Applet *applet, const QVariantList &args);

/**
 * Loads an DataEngine script engine for the given language.
 *
 * @param language the language to load for
 * @param dataEngine the Plasma::DataEngine for this script;
 * @return pointer to the DataEngineScript or 0 on failure; the caller is responsible
 *         for the return object which will be parented to the DataEngine
 **/
PLASMA_EXPORT DataEngineScript *loadScriptEngine(const QString &language, DataEngine *dataEngine, const QVariantList &args);

} // namespace Plasma

#endif
