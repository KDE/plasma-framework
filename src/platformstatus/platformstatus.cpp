#include <platformstatus.h>

#include <QDBusConnection>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDirWatch>
#include <KPluginFactory>

const char *defaultPackage = "org.kde.desktop";

K_PLUGIN_FACTORY(PlatformStatusFactory, registerPlugin<PlatformStatus>();)
K_EXPORT_PLUGIN(PlatformStatusFactory("platformstatus"))

PlatformStatus::PlatformStatus(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
{
    QDBusConnection::sessionBus().registerObject("/PlatformStatus", this,
                                                 QDBusConnection::ExportAllProperties |
                                                 QDBusConnection::ExportAllSignals);
    findShellPackage(false);
    const QString globalrcPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "kdeglobals");
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(fileDirtied(QString)));
    KDirWatch::self()->addFile(globalrcPath);
}

void PlatformStatus::findShellPackage(bool sendSignal)
{
    KConfigGroup group(KSharedConfig::openConfig("kdeglobals"), "DesktopShell");
    const QString package = group.readEntry("shellPackage", defaultPackage);

    const QString path = QStandardPaths::locate(QStandardPaths::DataLocation, "plasma/shells/" + package + "/");
    if (path.isEmpty()) {
        if (package != defaultPackage) {
            group.deleteEntry("ShellPackage");
            findShellPackage(sendSignal);
        }

        return;
    }

    m_shellPackage = package;

    QString runtimePlatform = group.readEntry("runtimePlatform", QString());
    KConfig packageDefaults(path + "contents/defaults", KConfig::SimpleConfig);
    group = KConfigGroup(&packageDefaults, "DesktopShell");
    runtimePlatform = group.readEntry("runtimePlatform", runtimePlatform);
    const bool runtimeChanged = runtimePlatform != m_runtimePlatform.join(',');
    if (runtimeChanged) {
        m_runtimePlatform = runtimePlatform.split(',');
    }

    if (sendSignal) {
        emit shellPackageChanged(m_shellPackage);
        emit runtimePlatformChanged(m_runtimePlatform);
    }
}

QString PlatformStatus::shellPackage() const
{
    return m_shellPackage;
}

QStringList PlatformStatus::runtimePlatform() const
{
    return m_runtimePlatform;
}

void PlatformStatus::fileDirtied(const QString &path)
{
    if (path.endsWith("kdeglobals")) {
        findShellPackage(true);
    }
}

#include "platformstatus.moc"
