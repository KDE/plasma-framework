add_definitions(-DTRANSLATION_DOMAIN=\"plasma_wallpaper_org.kde.plasma.%{APPNAMELC}\")

add_library(%{APPNAMELC}plugin SHARED %{APPNAMELC}plugin.cpp)

target_link_libraries(%{APPNAMELC}plugin
    KF5::I18n
    Qt5::Gui
    Qt5::Qml
)
install(TARGETS %{APPNAMELC}plugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/%{APPNAMELC})
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/%{APPNAMELC})
