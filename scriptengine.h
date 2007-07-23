/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_SCRIPTENGINE_H
#define PLASMA_SCRIPTENGINE_H

#include <QtCore/QObject>

namespace Plasma
{

class Applet;
class Package;

/**
 * @brief The base class for scripting interfaces to be used in loading
 *        plasmoids of a given language.
 *
 * All ScriptEngines should export as consistent an interface as possible
 * so that the learning curve is limited. In particular, the following
 * API should be made available in the script environment:
 *
 * TODO: define the actual API ...
 * PlasmaApplet - the applet of this plasmoid
 * LoadUserInterface(String uiFile) - loads and returns a given UI file
 * LoadImage - loads an image resource out of the plasmoid's package
 * PlasmaSvg - creates and returns an Svg file
 **/
class ScriptEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * The default constructor for a ScriptEngine
     *
     * @param applet the Applet object that will house the plasmoid
     **/
    explicit ScriptEngine(QObject *parent, const QStringList &args);
    virtual ~ScriptEngine();

    void init(Applet* applet);

    /**
     * @return a list of all supported languages
     **/
    static QStringList knownLanguages();

    /**
     * Loads a script engine for the given language.
     *
     * @param language the language to load an engine for
     * @param applet the applet for this plasmoid
     * @return pointer to the ScriptEngine or 0 on failure
     **/
    static ScriptEngine* load(const QString &language, Applet *applet);

protected:
    /**
     * Called when the script should set up the script environment and
     * start the script itself
     **/
    virtual bool init();

    /**
     * @return absolute path to the main script file for this plasmoid
     **/
    QString mainScript() const;

    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    const Package* package() const;

    /**
     * @return the Applet that contains this plasmoid
     **/
    Applet* applet() const;

private:
    class Private;
    Private * const d;
};

#define K_EXPORT_PLASMA_SCRIPENGINE(libname, classname) \
        K_EXPORT_COMPONENT_FACTORY(                \
                        plasma_scriptengine_##libname,    \
                        KGenericFactory<classname>("plasma_scriptengine_" #libname))


} // namespace Plasma

#endif

