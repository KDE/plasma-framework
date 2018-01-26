/*
 * Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QColor>
#include <QTime>

#include <klocalizedstring.h>

#include "simpleEngine.h"

/*
 This DataEngine provides a static set of data that is created on
 engine creation. This is a common pattern for DataEngines that relay
 information such as hardware events and shows the most basic form of
 a DataEngine
*/

SimpleEngine::SimpleEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // we've passed the constructor's args to our parent class
    // we're done for now! Call init()
    init();
}

void SimpleEngine::init()
{

    // So now we will set up some sources.
    // Each DataEngine will, generally, be loaded once. Each DataEngine
    // can provide multiple sets of data keyed by a string, called "Sources".
    // In this simplest of cases, we just create some sources arbitrarily.

    // This is the simplest form, with  source name and one bit of data.
    // Note how the source name is not translated! They can be marked with
    // I18N_NOOP, however, if they should be translatable in a visualization.
    setData(QStringLiteral("Simple Source"), i18n("Very simple data"));

    // a source can have multiple entries, differentiated by key names,
    // which are also not translated:
    setData(QStringLiteral("Multiple Source"), QStringLiteral("First"), i18n("First"));
    setData(QStringLiteral("Multiple Source"), QStringLiteral("Second"), i18n("Second"));

    // We can also set the data up first and apply it all at once
    // Note how data types other than strings can be used as well; anything
    // that works with QVariant, in fact.
    Plasma::DataEngine::Data data;
    data.insert(QStringLiteral("Cow"), QStringLiteral("mooo"));
    data.insert(QStringLiteral("Black"), QColor(0, 0, 0));
    data.insert(QStringLiteral("Time"), QTime::currentTime());
    setData(QStringLiteral("Another Source"), data);
}

// export the plugin; use the plugin name and the class name
K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(org.kde.examples.simpleEngine, SimpleEngine, "plasma-dataengine-example-simpleEngine.json")

// include the moc file so the build system makes it for us
#include "simpleEngine.moc"

