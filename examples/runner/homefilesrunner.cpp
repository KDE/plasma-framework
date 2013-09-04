/*
Copyright 2009 Aaron Seigo <aseigo@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "homefilesrunner.h"

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>

#include <KDebug>
#include <KMimeType>
#include <KRun>

HomeFilesRunner::HomeFilesRunner(QObject *parent, const QVariantList &args)
    : AbstractRunner(parent, args)
{
    setIgnoredTypes(Plasma::RunnerContext::NetworkLocation |
                    Plasma::RunnerContext::Executable |
                    Plasma::RunnerContext::ShellCommand);
    setSpeed(SlowSpeed);
    setPriority(LowPriority);
    setHasRunOptions(true);
}

void HomeFilesRunner::init()
{
    reloadConfiguration();
    connect(this, SIGNAL(prepare()), this, SLOT(prepareForMatchSession()));
    connect(this, SIGNAL(teardown()), this, SLOT(matchSessionFinished()));
}

void HomeFilesRunner::reloadConfiguration()
{
    KConfigGroup c = config();
    m_triggerWord = c.readEntry("trigger", QString());
    if (!m_triggerWord.isEmpty()) {
        m_triggerWord.append(' ');
    }

    m_path = c.readPathEntry("path", QDir::homePath());
    QFileInfo pathInfo(m_path);
    if (!pathInfo.isDir()) {
        m_path = QDir::homePath();
    }

    QList<Plasma::RunnerSyntax> syntaxes;
    Plasma::RunnerSyntax syntax(QString("%1:q:").arg(m_triggerWord),
                                i18n("Finds files matching :q: in the %1 folder", m_path));
    syntaxes.append(syntax);
    setSyntaxes(syntaxes);
}

void HomeFilesRunner::prepareForMatchSession()
{

}

void HomeFilesRunner::match(Plasma::RunnerContext &context)
{
    QString query = context.query();
    if (query == QChar('.') || query == "..") {
        return;
    }

    if (!m_triggerWord.isEmpty()) {
        if (!query.startsWith(m_triggerWord)) {
            return;
        }

        query.remove(0, m_triggerWord.length());
    }

    if (query.length() > 2) {
        query.prepend('*').append('*');
    }

    QDir dir(m_path);
    QList<Plasma::QueryMatch> matches;

    foreach (const QString &file, dir.entryList(QStringList(query))) {
        const QString path = dir.absoluteFilePath(file);
        if (!path.startsWith(m_path)) {
            // this file isn't in our directory; looks like we got a query with some
            // ..'s in it!
            continue;
        }

        if (!context.isValid()) {
            return;
        }

        Plasma::QueryMatch match(this);
        match.setText(i18n("Open %1", path));
        match.setData(path);
        match.setId(path);
        if (m_iconCache.contains(path)) {
            match.setIcon(m_iconCache.value(path));
        } else {
            KIcon icon(KMimeType::iconNameForUrl(path));
            m_iconCache.insert(path, icon);
            match.setIcon(icon);
        }

        if (file.compare(query, Qt::CaseInsensitive)) {
            match.setRelevance(1.0);
            match.setType(Plasma::QueryMatch::ExactMatch);
        } else {
            match.setRelevance(0.8);
        }

        matches.append(match);
    }

    context.addMatches(context.query(), matches);
}

void HomeFilesRunner::matchSessionFinished()
{
    m_iconCache.clear();
}

void HomeFilesRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    // KRun autodeletes itself, so we can just create it and forget it!
    KRun *opener = new KRun(match.data().toString(), 0);
    opener->setRunExecutables(false);
}

void HomeFilesRunner::createRunOptions(QWidget *widget)
{
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QCheckBox *cb = new QCheckBox(widget);
    cb->setText(i18n("This is just for show"));
    layout->addWidget(cb);
}

K_EXPORT_PLASMA_RUNNER(example-homefiles, HomeFilesRunner)

