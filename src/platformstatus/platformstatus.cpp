#include <platformstatus.h>

#include <QDBusConnection>
#include <QStandardPaths>

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <QDebug>
#include <kdirwatch.h>
#include <kpluginfactory.h>
#include <qstandardpaths.h>

#include "platformstatusadaptor.h"
#include "../plasma/config-plasma.h"

const char defaultPackage[] = "org.kde.plasma.desktop";
const char defaultLnFPackage[] = "org.kde.breeze.desktop";

K_PLUGIN_FACTORY_WITH_JSON(PlatformStatusFactory,
                           "platformstatus.json",
                           registerPlugin<PlatformStatus>();)

PlatformStatus::PlatformStatus(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
{
    new PlatformStatusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/PlatformStatus", this);

    findShellPackage(false);
    findLookAndFeelPackage(false);

    const QString globalrcPath = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, "kdeglobals");
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(fileDirtied(QString)));
    connect(KDirWatch::self(), SIGNAL(created(QString)), this, SLOT(fileDirtied(QString)));
    KDirWatch::self()->addFile(globalrcPath);
}

void PlatformStatus::findShellPackage(bool sendSignal)
{
    KConfigGroup group(KSharedConfig::openConfig("kdeglobals"), "DesktopShell");
    const QString package = group.readEntry("shellPackage", defaultPackage);

    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                         PLASMA_RELATIVE_DATA_INSTALL_DIR "/shells/" + package + '/',
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

void PlatformStatus::findLookAndFeelPackage(bool sendSignal)
{

    KConfigGroup group(KSharedConfig::openConfig("kdeglobals"), "KDE");
    QString package = group.readEntry("LookAndFeelPackage", QString());

    if (package.isEmpty()) {
        const QString shellPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                        PLASMA_RELATIVE_DATA_INSTALL_DIR "/shells/" + m_shellPackage + '/',
                        QStandardPaths::LocateDirectory);
        KConfig packageDefaults(shellPath + "contents/defaults", KConfig::SimpleConfig);
        group = KConfigGroup(&packageDefaults, "Desktop");
        package = group.readEntry("LookAndFeel", defaultLnFPackage);

    } else {
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                            PLASMA_RELATIVE_DATA_INSTALL_DIR "/look-and-feel/" + package + '/',
                            QStandardPaths::LocateDirectory);
        if (path.isEmpty()) {
            if (package != defaultPackage) {
                group.deleteEntry("LookAndFeelPackage");
                findLookAndFeelPackage(sendSignal);
            }

            return;
        }

    }

    const bool lnfChanged = (m_lookAndFeelPackage != package);
    m_lookAndFeelPackage = package;

    if (sendSignal && lnfChanged) {
        emit lookAndFeelPackageChanged(m_lookAndFeelPackage);
    }
}

QString PlatformStatus::shellPackage() const
{
    return m_shellPackage;
}

QString PlatformStatus::lookAndFeelPackage() const
{
    return m_lookAndFeelPackage;
}

QStringList PlatformStatus::runtimePlatform() const
{
    return m_runtimePlatform;
}

void PlatformStatus::fileDirtied(const QString &path)
{
    if (path.endsWith("kdeglobals")) {
        findShellPackage(true);
        findLookAndFeelPackage(true);
    }
}

#include "platformstatus.moc"
