find_package(KF5 REQUIRED IdleTime ItemModels WidgetsAddons WindowSystem Codecs Archive CoreAddons Solid ThreadWeaver GlobalAccel
                          Config Auth JS Wallet DBusAddons I18n GuiAddons ConfigWidgets
                          Service ItemViews Notifications IconThemes Completion JobWidgets Sonnet TextWidgets XmlGui Crash
                          Bookmarks Declarative UnitConversion Parts Kross KIO Init DocTools Su)

find_package(KF5CoreAddons REQUIRED)

find_package(KF5Solid REQUIRED)

find_package(KF5Su REQUIRED NO_MODULE)

find_package(KF5Declarative REQUIRED NO_MODULE)

find_package(KF5KDE4Support REQUIRED NO_MODULE)

include(KF5PlasmaMacros)
include(KF5PlasmaMacros.cmake)

target_link_libraries(Foo
                     Qt5::Test
                     KF5::Plasma
                     KF5::Archive KF5::CoreAddons
                     KF5::ConfigGui
                     KF5::I18n
                     KF5::KIOCore
                     KF5::Service
                     KF5::PlasmaQuick
                    )

target_link_libraries(plasma-example-kpart-shell KF5::Plasma KF5::Parts)
