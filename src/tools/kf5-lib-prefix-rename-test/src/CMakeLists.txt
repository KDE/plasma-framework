find_package(KF6 REQUIRED KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid ThreadWeaver KF6GlobalAccel
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
                     KF6::Plasma
                     KF6::KArchive KF6::KCoreAddons
                     KF6::KConfigGui
                     KF6::KI18n
                     KF6::KIOCore
                     KF6::KService
                     KF6::PlasmaQuick
                    )

target_link_libraries(plasma-example-kpart-shell KF6::Plasma KF6::KParts)
