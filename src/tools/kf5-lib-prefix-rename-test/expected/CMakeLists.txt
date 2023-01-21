find_package(KF6 REQUIRED IdleTime ItemModels WidgetsAddons WindowSystem Codecs Archive CoreAddons Solid ThreadWeaver GlobalAccel
                          Config Auth JS Wallet DBusAddons I18n GuiAddons ConfigWidgets
                          Service ItemViews Notifications IconThemes Completion JobWidgets Sonnet TextWidgets XmlGui Crash
                          Bookmarks Declarative UnitConversion Parts Kross KIO Init DocTools Su)

find_package(KF6CoreAddons REQUIRED)

find_package(KF6Solid REQUIRED)

find_package(KF6Su REQUIRED NO_MODULE)

find_package(KF6Declarative REQUIRED NO_MODULE)

find_package(KF6KDE4Support REQUIRED NO_MODULE)

include(KF6PlasmaMacros)
include(KF6PlasmaMacros.cmake)

target_link_libraries(Foo
                     Qt5::Test
                     KF6::Plasma
                     KF6::Archive KF6::CoreAddons
                     KF6::ConfigGui
                     KF6::I18n
                     KF6::KIOCore
                     KF6::Service
                     KF6::PlasmaQuick
                    )

target_link_libraries(plasma-example-kpart-shell KF6::Plasma KF6::Parts)
