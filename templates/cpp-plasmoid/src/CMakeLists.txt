# TODO: adapt "org.kde.plasma" here & elsewhere if needed (see README)
add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.%{APPNAMELC}\")

add_library(org.kde.plasma.%{APPNAMELC} MODULE %{APPNAMELC}.cpp)

target_link_libraries(org.kde.plasma.%{APPNAMELC}
                      Qt5::Gui
                      KF5::Plasma
                      KF5::I18n)


install(TARGETS org.kde.plasma.%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.plasma.%{APPNAMELC})
