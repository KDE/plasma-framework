#ifndef PLATFORMSTATUS_H
#define PLATFORMSTATUS_H

#include <kdedmodule.h>

#include <QStringList>

class PlatformStatus : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.PlatformStatus")
    Q_PROPERTY(QString shellPackage READ shellPackage NOTIFY shellPackageChanged)
    Q_PROPERTY(QString lookAndFeelPackage READ lookAndFeelPackage NOTIFY lookAndFeelPackageChanged)
    Q_PROPERTY(QStringList runtimePlatform READ runtimePlatform NOTIFY runtimePlatformChanged)

public:
    PlatformStatus(QObject *parent, const QVariantList &);

public Q_SLOTS:
    QString shellPackage() const;
    QString lookAndFeelPackage() const;
    QStringList runtimePlatform() const;

Q_SIGNALS:
    void shellPackageChanged(const QString &package);
    void lookAndFeelPackageChanged(const QString &package);
    void runtimePlatformChanged(const QStringList &runtimePlatform);

private:
    void findShellPackage(bool sendSignal);
    void findLookAndFeelPackage(bool sendSignal);

private Q_SLOTS:
    void fileDirtied(const QString &path);

private:
    QString m_shellPackage;
    QString m_lookAndFeelPackage;
    QStringList m_runtimePlatform;
};

#endif

