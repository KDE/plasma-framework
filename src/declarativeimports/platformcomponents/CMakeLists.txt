set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(platformcomponentsplugin SHARED)

target_sources(platformcomponentsplugin PRIVATE
    platformextensionplugin.cpp
    application.cpp
    icondialog.cpp
)

target_link_libraries(platformcomponentsplugin
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    KF5::WindowSystem
    KF5::IconThemes
)

install(TARGETS platformcomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/platformcomponents)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/platformcomponents)

ecm_generate_qmltypes(org.kde.plasma.platformcomponents 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/platformcomponents)
