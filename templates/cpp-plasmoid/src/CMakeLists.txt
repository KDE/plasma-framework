# TODO: adapt "org.kde.plasma" here & elsewhere if needed (see README)
add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.%{APPNAMELC}\")

add_library(plasma_applet_%{APPNAMELC} MODULE %{APPNAMELC}.cpp)

kcoreaddons_desktop_to_json(plasma_applet_%{APPNAMELC} package/metadata.desktop SERVICE_TYPES plasma-applet.desktop)

target_link_libraries(plasma_applet_%{APPNAMELC}
                      Qt5::Gui
                      KF5::Plasma
                      KF5::I18n)


install(TARGETS plasma_applet_%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.plasma.%{APPNAMELC})
