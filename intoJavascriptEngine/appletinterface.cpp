#include "animations/animationscriptengine_p.h"
#include "animator.h"

    /**
     * Loads an animation from the applet package
     * @param animation the animation to load
     * @return an Animation object on success, a NULL pointer on failure
     * @since 4.5
     */
    Animation *loadAnimationFromPackage(const QString &name, QObject *parent);


Animation *AppletScript::loadAnimationFromPackage(const QString &name, QObject *parent)
{
    if (applet()) {
        const QString scopedName = applet()->pluginName() + ":" + name;
        if (!AnimationScriptEngine::isAnimationRegistered(scopedName)) {
            KConfig conf(applet()->package().path() + "/metadata.desktop", KConfig::SimpleConfig);
            KConfigGroup animConf(&conf, "Animations");
            QString file;
            foreach (const QString &possibleFile, animConf.keyList()) {
                const QStringList anims = animConf.readEntry(possibleFile, QStringList());
                if (anims.contains(name)) {
                        file = possibleFile;
                        break;
                }
            }

            if (file.isEmpty()) {
                return 0;
            }

            const QString path = applet()->package().filePath("animations", file);
            if (path.isEmpty()) {
#ifndef NDEBUG
                kDebug() << "file path was empty for" << file;
#endif
                return 0;
            }

            if (!AnimationScriptEngine::loadScript(path, applet()->pluginName() + ':') ||
                !AnimationScriptEngine::isAnimationRegistered(scopedName)) {
#ifndef NDEBUG
                kDebug() << "script engine loading failed for" << path;
#endif
                return 0;
            }
        }

        Animation *anim = Animator::create(scopedName, parent ? parent : this);
        return anim;
    }

    return 0;
}

    /**
     * Factory to build new animation objects from Javascript files. To control their behavior,
     * check \ref AbstractAnimation properties.
     * @since 4.5
     **/
    static Plasma::Animation *create(const QString &animationName, QObject *parent = 0);

Plasma::Animation *Animator::create(const QString &anim, QObject *parent)
{
    if (AnimationScriptEngine::animationFailedToLoad(anim)) {
        return 0;
    }

    if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
        const QString path = Theme::defaultTheme()->animationPath(anim);
        if (path.isEmpty()) {
            AnimationScriptEngine::addToLoadFailures(anim);
            //kError() << "************ failed to find script file for animation" << anim;
            return 0;
        }

        if (!AnimationScriptEngine::loadScript(path)) {
            AnimationScriptEngine::addToLoadFailures(anim);
            return 0;
        }

        if (!AnimationScriptEngine::isAnimationRegistered(anim)) {
            //kError() << "successfully loaded script file" << path << ", but did not get animation object for" << anim;
            AnimationScriptEngine::addToLoadFailures(anim);
            return 0;
        }
    }

    return new Plasma::JavascriptAnimation(anim, parent);
}


