find_package(KF5 REQUIRED KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid ThreadWeaver KF5GlobalAccel
                          KConfig KAuth KJS KWallet KDBusAddons KI18n KGuiAddons KConfigWidgets
                          KService ItemViews KNotifications KIconThemes KCompletion KJobWidgets Sonnet KTextWidgets XmlGui KCrash
                          KBookmarks KDeclarative KUnitConversion KParts Kross KIO KInit KDocTools KDESu)

find_package(KCoreAddons REQUIRED)

find_package(Solid REQUIRED)

find_package(KDESu REQUIRED NO_MODULE)

find_package(KDeclarative REQUIRED NO_MODULE)

find_package(KDE4Support REQUIRED NO_MODULE)

include(PlasmaMacros)
include(PlasmaMacros.cmake)

target_link_libraries(Foo
                     Qt5::Test
                     KF5::Plasma
                     KF5::KArchive KF5::KCoreAddons
                     KF5::KConfigGui
                     KF5::KI18n
                     KF5::KIOCore
                     KF5::KService
                     KF5::PlasmaQuick
                    )

target_link_libraries(plasma-example-kpart-shell KF5::Plasma KF5::KParts)
