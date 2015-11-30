add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.%{APPNAMELC}\")

set(%{APPNAMELC}_SRCS
    %{APPNAMELC}.cpp
)

add_library(plasma_applet_%{APPNAMELC} MODULE ${%{APPNAMELC}_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_%{APPNAMELC} package/metadata.desktop)

target_link_libraries(plasma_applet_%{APPNAMELC}
                      Qt5::Gui
                      KF5::Plasma
                      KF5::I18n)


install(TARGETS plasma_applet_%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.%{APPNAMELC})
