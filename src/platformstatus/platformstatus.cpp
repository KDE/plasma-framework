#include <platformstatus.h>

#include <QDBusConnection>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDebug>
#include <KDirWatch>
#include <KPluginFactory>

#include "platformstatusadaptor.h"
#include "splash/splashwindow.h"

const char *defaultPackage = "org.kde.desktop";

K_PLUGIN_FACTORY(PlatformStatusFactory, registerPlugin<PlatformStatus>();)
K_EXPORT_PLUGIN(PlatformStatusFactory("platformstatus"))

PlatformStatus::PlatformStatus(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
{
    new PlatformStatusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/PlatformStatus", this);

    findShellPackage(false);

    const QString globalrcPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "kdeglobals");
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(fileDirtied(QString)));
    KDirWatch::self()->addFile(globalrcPath);
}

void PlatformStatus::findShellPackage(bool sendSignal)
{
    KConfigGroup group(KSharedConfig::openConfig("kdeglobals"), "DesktopShell");
    const QString package = group.readEntry("shellPackage", defaultPackage);

    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                "plasma/shells/" + package + '/',
                                                QStandardPaths::LocateDirectory);
    if (path.isEmpty()) {
        if (package != defaultPackage) {
            group.deleteEntry("ShellPackage");
            findShellPackage(sendSignal);
        }

        return;
    }

    m_shellPackage = package;

    QString runtimePlatform = group.readEntry("RuntimePlatform", QString());
    KConfig packageDefaults(path + "contents/defaults", KConfig::SimpleConfig);
    group = KConfigGroup(&packageDefaults, "Desktop");
    runtimePlatform = group.readEntry("RuntimePlatform", runtimePlatform);
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

void PlatformStatus::testShowSplash() const
{
    SplashWindow::splashAllScreens();
}

#include "platformstatus.moc"
